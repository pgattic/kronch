/*

KRONCH: a CHIP-8 emulator
by Preston Corless

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "raylib.h"

#define SCREEN_SCALE 20
#define MEMORY 4096 // 4KB
#define STACKMEM 1024 // 4 bytes per value, for a stack of 256
#define CLOCK_SPEED 800 // Hertz
#define REFRESH_RATE 60 // Hertz
#define CYCLES_PER_FRAME (CLOCK_SPEED/REFRESH_RATE)
#define FF_SPEED 10

Color white = {0xbb, 0xcc, 0xaa, 0xff};
Color black = {0x33, 0x44, 0x22, 0xff};

unsigned char mem[MEMORY] = {0}; // 4KB of Program Memory
int pc = 0x200; // Program Counter

unsigned char V[16] = {0}; // V0-VF

unsigned short I; // Memory Pointer - Only uses 12 bits

unsigned short stack[256] = {0}; // 16-bit values; also only uses 12 of those bits
unsigned short stackptr; // Index of current stack frame

bool screen[32][64] = {false}; // 2-D array of bools

unsigned char delayTimer = 0;
unsigned char soundTimer = 0;

unsigned char keyboard[16];
bool fastForward = false;

bool drawFlag = true;

void printHelp(char* arg) {
  printf("Usage:\n  %s [FILE].ch8 [OPTIONS]\n", arg);
}

int loadCode(FILE* f, char* dest) {

  fseek(f, 0, SEEK_END);
  int size = ftell(f); 
  fseek(f, 0, SEEK_SET);

  char ch;
  int rhead = 0;
  do {				// transfer file data to the prg array (copy all characters in debug mode, else copy only command characters)
    ch = fgetc(f);
    dest[rhead + 0x200] = ch;
    rhead++;
    if (rhead > MEMORY) {
      fprintf(stderr, "\nERROR: Out of Program Memory.");
      return -1;
    }
  } while (rhead <= size);
  return size;
}

void initChip8() {

  unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  memset(mem, 0, 4096);
  memcpy(&mem[0x50], chip8_fontset, 80 * sizeof(char));
  memcpy(mem, chip8_fontset, 80 * sizeof(char));
  memset(stack, 0, 256);
  memset(V, 0, 16);
  memset(screen, 0, 2048);
  memset(keyboard, 0, 16);
}

void readKeys() {
  keyboard[0] = IsKeyDown(KEY_X);
  keyboard[1] = IsKeyDown(KEY_ONE);
  keyboard[2] = IsKeyDown(KEY_TWO);
  keyboard[3] = IsKeyDown(KEY_THREE);
  keyboard[4] = IsKeyDown(KEY_Q);
  keyboard[5] = IsKeyDown(KEY_W);
  keyboard[6] = IsKeyDown(KEY_E);
  keyboard[7] = IsKeyDown(KEY_A);
  keyboard[8] = IsKeyDown(KEY_S);
  keyboard[9] = IsKeyDown(KEY_D);
  keyboard[0xA] = IsKeyDown(KEY_Z);
  keyboard[0xB] = IsKeyDown(KEY_C);
  keyboard[0xC] = IsKeyDown(KEY_FOUR);
  keyboard[0xD] = IsKeyDown(KEY_R);
  keyboard[0xE] = IsKeyDown(KEY_F);
  keyboard[0xF] = IsKeyDown(KEY_V);
  fastForward = IsKeyDown(KEY_TAB);
}

void drawScreen() {
  BeginDrawing();

  if (drawFlag) {
    for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
        Color targetColor = screen[i][j] ? black : white;
        DrawRectangle(j * SCREEN_SCALE, i * SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE, targetColor);
      }
    }
  }
  drawFlag = false;

  if (delayTimer) {
    delayTimer--;
  }

  EndDrawing();
}

int main(int argc, char** argv) {
  if (argc < 2) {				// end program if no file specified
    printHelp(argv[0]);
    return 1;
  }

  FILE* f = fopen(argv[1], "r");	// open the file specified as an argument into the program

  if (!f) {					// end program if file not found
    fprintf(stderr,"%s: no such file\n\n", argv[1]);
    printHelp(argv[0]);
    return 1;
  }

  srandom(69);
  initChip8();

  int fileSize = loadCode(f, mem);

  if (fileSize < 0) {
    return 1;
  }

  InitWindow(64 * SCREEN_SCALE, 32 * SCREEN_SCALE, "Kronch");
  SetTargetFPS(REFRESH_RATE);
  while (pc < MEMORY && !WindowShouldClose()) {
    readKeys();
    for (int cycles = 0; cycles < (CYCLES_PER_FRAME * (fastForward ? FF_SPEED : 1)) && pc < MEMORY; cycles++) {
      unsigned char op1 = mem[pc];
      unsigned char op2 = mem[pc+1];
      unsigned short opcode = (op1 << 8) | op2;
      char X = op1 & 0x0F;
      char Y = op2 >> 4;
      pc += 2;
      switch (op1 >> 4) {
        case 0x0: // Various instructions
          if (op2 == 0xE0) { // clear screen
            for (int i = 0; i < 32; i++) {
              for (int j = 0; j < 64; j++) {
                screen[i][j] = false;
              }
            }
            drawFlag = true;
          } else if (op2 == 0xEE) { // Return from subroutine (use stack)
            stackptr--;
            pc = stack[stackptr];
          } else {
            printf("Opcode Error 0XXX, address %x\n", pc-2);
          }
          break;
        case 0x1: // Goto (exclude stack)
          pc = opcode & 0x0fff;
          break;
        case 0x2: // Subroutine (goto with stack)
          stack[stackptr] = pc;
          stackptr++;
          pc = opcode & 0x0fff;
          break;
        case 0x3: // Equality conditional
          if (V[X] == op2) {
            pc += 2;
          }
          break;
        case 0x4: // Inequality conditional
          if (V[X] != op2) {
            pc += 2;
          }
          break;
        case 0x5: // Equality conditional (compare registers)
          if (V[X] == V[Y]) {
            pc += 2;
          }
          break;
        case 0x6: // Set register
          V[X] = op2;
          break;
        case 0x7: // Add to register (no carry flag)
          V[X] += op2;
          break;
        case 0x8: // Register Math
          switch (op2 & 0x0f) {
            case 0x0: // Transfer value
              V[X] = V[Y];
              break;
            case 0x1: // OR Operation
              V[X] |= V[Y];
              break;
            case 0x2: // AND Operation
              V[X] &= V[Y];
              break;
            case 0x3: // XOR Operation
              V[X] ^= V[Y];
              break;
            case 0x4:{ // Add Operation
              int i = (int)V[X] + (int)V[Y];
              //V[0xF] = (i > 0xFF);
              if (i > 0xff) {
                V[0xF] = 1;
              } else {
                V[0xF] = 0;
              }
              V[X] = i & 0xFF;
              break;
            }
            case 0x5: // Subtract Operation
              V[0xF] = V[X] >= V[Y];
              V[X] -= V[Y];
              break;
            case 0x6: // Right Bit Shift Operation
              V[0xF] = V[X] & 0x1;
              V[X] >>= 1;
              break;
            case 0x7: // Subtract Operation
              V[0xF] = V[Y] >= V[X];
              V[X] = V[Y] - V[X];
              break;
            case 0xE: // Left Bit Shift Operation
              V[0xF] = (V[X] & 0x80) >> 7;
              V[X] <<= 1;
              break;
            default:
              printf("Opcode Error 8XXX, address %x\n", pc-2);
              break;
          }
          break;
        case 0x9: // Register inequality
          if (V[X] != V[Y]) {
            pc += 2;
          }
          break;
        case 0xA: // Set I (Memory access pointer)
          I = opcode & 0x0FFF;
          break;
        case 0xB: // Jump to predetermined address
          pc = V[0] + (opcode & 0x0FFF);
          break;
        case 0xC:
          V[X] = random() & op2;
          break;
        case 0xD:
          V[0xF] = 0;
          int coordY = V[Y] % 32;
          int height = op2 & 0xF;

          for (int row = 0; row < height; row++) {
            unsigned char sprRow = mem[I + row];
            int coordX = V[X] % 64;
            for (int pixel = 0; pixel < 8; pixel++) {
              bool filled = (sprRow >> (7-pixel)) & 1;
              if (filled && screen[coordY][coordX]) {
                screen[coordY][coordX] = false;
                V[0xF] = true;
              } else if (filled) {
                screen[coordY][coordX] = true;
              }
              //coordX = (coordX + 1) % 64;
              coordX++;
              if (coordX > 63) { break; }
            }
            coordY++;
            if (coordY > 31) { break; }
          }
          drawFlag = true;
          break;
        case 0xE:
          if (op2 == 0x9E) {
            if (keyboard[V[X]]) {
              pc += 2;
            }
          } else if (op2 == 0xA1) {
            if (!keyboard[V[X]]) {
              pc += 2;
            }
          }
          break;
        case 0xF:
          switch (op2) {
            case 0x07:
              V[X] = delayTimer;
              break;
            case 0x0A:{
              bool isKeyPressed = 0;
              for (int i = 0; i < 16; i++) {
                if (keyboard[i]) {
                  isKeyPressed |= keyboard[i];
                  V[X] = i;
                }
              }
              if (!isKeyPressed) {
                pc -= 2;
              }
              break;
            }
            case 0x15:
              delayTimer = V[X];
              break;
            case 0x18:
              soundTimer = V[X];
              break;
            case 0x1E:
              I += V[X];
              break;
            case 0x29:
              I = V[X] * 5 + 0x50;
              break;
            case 0x33:{
              mem[I] = (int)(V[X]/100);
              mem[I+1] = (int)((V[X] % 100)/10);
              mem[I+2] = (int)(V[X] % 10);
              break;
            }
            case 0x55:
              for (char i = 0; i <= (op1 & 0xF); i++) {
                mem[I+i] = V[i];
              }
              break;
            case 0x65:
              for (char i = 0; i <= (op1 & 0xF); i++) {
                V[i] = mem[I+i];
              }
              break;
          }
          break;
        default:
          printf("Opcode Error: %x\n", opcode);
          break;
      }
    }
    drawScreen();
  }
  CloseWindow();

  return 0;
}
