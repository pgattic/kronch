#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "raylib.h"

/*

KRONCH: a CHIP-8 emulator
by Preston Corless

*/

#define SCREEN_SCALE 10
#define PRGMEM 4096 // 4KB
#define WORKMEM 4096 // 4KB
#define STACKMEM 1024 // 4 bytes per value, for a stack of 256

unsigned char prg[PRGMEM] = {0}; // 4KB of Program Memory
int pc = 0x200; // Program Counter

unsigned char V[16] = {0}; // V0-VF

unsigned char ram[WORKMEM] = {0};
unsigned short I; // Memory Pointer - Only uses 12 bits

unsigned short stack[256] = {0}; // 16-bit values; also only uses 12 of those bits
unsigned short stackptr; // Index of current stack frame

bool screen[32][64] = {false}; // 2-D array of bools

unsigned char delayTimer = 0;
unsigned char soundTimer = 0;

unsigned char keyboard[16];

bool drawFlag = true;

void printHelp(char* arg) {
  printf("Usage:\n  %s [FILE].ch8 [OPTIONS]\nOptions:\n  -d	debug mode\n", arg);
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
    if (rhead > PRGMEM) {
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
  memset(ram, 0, 4096);
  memcpy(ram, chip8_fontset, 80 * sizeof(char));
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
}

void clearScreen() {
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 64; j++) {
      screen[i][j] = false;
    }
  }
  drawFlag = true;
}

void drawSprite(unsigned char x, unsigned char y, unsigned char sprite) {
//  if (drawFlag) {
    printf("Yeet\n");
    for (int row = 0; row < sprite; row++) {
      for (int pixel = 0; pixel < 8; pixel++) {
        bool memPX = (ram[I+row] >> pixel) % 2;
        //bool memPX = (ram[I] & (unsigned char)(1 << pixel)) ? true : false;
        if (memPX) {
          printf("Yeet2\n");
        screen[y+row][x+(7-pixel)] = true;
        }
      }
    }
  //}
  drawFlag = true;
}

void drawScreen() {
  BeginDrawing();

  //if (drawFlag) {

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 64; j++) {
      Color targetColor = screen[i][j] ? BLACK : WHITE;
      //Color targetColor = BLACK;
      DrawRectangle(j * SCREEN_SCALE, i * SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE, targetColor);
    }
  }
  //printf("yeet draw\n");
  //DrawRectangle(5, 5, 6, 7, GREEN);

  //}
  drawFlag = true;

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

  int fileSize = loadCode(f, prg);

  if (fileSize < 0) {
    return 1;
  }

  initChip8();

  InitWindow(64 * SCREEN_SCALE, 32 * SCREEN_SCALE, "Kronch");
  SetTargetFPS(60);
  while (pc < PRGMEM && !WindowShouldClose()) {
    unsigned char op1 = prg[pc];
    unsigned char op2 = prg[pc+1];
    unsigned short opcode = (op1 << 8) | op2;
    char X = op1 & 0x0f;
    char Y = op2 >> 4;
    pc += 2;
    readKeys();
    switch (op1 >> 4) {
      case 0x0: // Various instructions
        if (op2 == 0xE0) { // clear screen
          clearScreen();
        }
        else if (op2 == 0xEE) { // Return from subroutine (use stack)
          stackptr--;
          pc = stack[stackptr];
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
          }
            break;
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
            V[X] >>= 1;
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
        V[X] = rand() & op2;
        break;
      case 0xD:
        V[0xF] = 0;
        drawSprite(V[X] % 64, V[Y] % 32, op2 & 0x0F);
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
              if (keyboard[1]) {
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
            ram[I] = V[X] * 5;
            break;
          case 0x33:{
            ram[I] = (int)(V[X]/100);
            ram[I+1] = (int)((V[X] % 100)/10);
            ram[I+2] = (int)(V[X] % 10);
          }
          case 0x55:
            for (char i = 0; i < 16; i++) {
              ram[I+i] = V[i];
            }
            break;
          case 0x65:
            for (char i = 0; i < 16; i++) {
              V[i] = ram[I+i];
            }
            break;
        }
        break;
      default:
        printf("Opcode Error: %x\n", opcode);
        break;
    }
    drawScreen();
  }
  CloseWindow();

  return 0;
}
