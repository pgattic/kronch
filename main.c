#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include "raylib.h"

/*

KRONCH: a CHIP-8 emulator
by Preston Corless

*/

#define SCREEN_SCALE 10
#define PRGMEM 4096 // 4KB
#define WORKMEM 4096 // 4KB
#define STACKMEM 1024 // 4 bytes per value, for a stack of 256

unsigned char prg[PRGMEM] = {0}; // 4KB of Program Memory
int head = 0x200; // Address Register

unsigned char V[16] = {0}; // V0-VF

unsigned char ram[WORKMEM] = {0};
short I; // Memory Pointer - Only uses 12 bits

unsigned short stack[256] = {0}; // 16-bit values; also only uses 12 of those bits
short stackptr; // Index of current stack frame

bool screen[32][64] = {false}; // 2-D array of bools

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
      fprintf(stderr, "\nERROR: Out of Program Memory. Program file too large.\nSee \"Notes\" in brainfetch.c.\n");
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
  memcpy(ram, chip8_fontset, 80 * sizeof(char));
}

int main(int argc, char** argv) {
  if (argc < 2) {				// end program if no file specified
    printHelp(argv[0]);
    return 1;
  }

  FILE* f = fopen("PONG.ch8", "r");	// open the file specified as an argument into the program

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

  //InitWindow(64 * SCREEN_SCALE, 32 * SCREEN_SCALE, "Kronch");
  //SetTargetFPS(60);
  while (head < PRGMEM /*&& !WindowShouldClose()*/) {
    unsigned char op1 = prg[head];
    unsigned char op2 = prg[head+1];
    unsigned short opcode = (op1 << 8) | op2;
    char X = op1 & 0x0f;
    char Y = op2 >> 4;
    head += 2;
    switch (op1 >> 4) {
      case 0x0: // Various instructions
        if (op2 == 0x0E) { // clear screen
          
        }
        else if (op2 == 0xEE) { // Return from subroutine (use stack)
          stackptr--;
          head = stack[stackptr];
        }
        break;
      case 0x1: // Goto (exclude stack)
        head = opcode & 0x0fff;
        break;
      case 0x2: // Subroutine (goto with stack)
        stack[stackptr] = head;
        stackptr++;
        head = opcode & 0x0fff;
        break;
      case 0x3: // Equality conditional
        if (V[X] == op2) {
          head += 2;
        }
        break;
      case 0x4: // Inequality conditional
        if (V[X] != op2) {
          head += 2;
        }
        break;
      case 0x5: // Equality conditional (compare registers)
        if (V[X] == V[Y]) {
          head += 2;
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
          head += 2;
        }
        break;
      case 0xA: // Set I (Memory access pointer)
        I = opcode & 0x0FFF;
        break;
      case 0xB: // Jump to predetermined address
        head = V[0] + (opcode & 0x0FFF);
        break;
      case 0xC:
        V[X] = rand() & op2;
        break;
      case 0xD:
        //draw(V[X], V[Y], op2 & 0x0F);
        break;
    }
    //BeginDrawing();

    //EndDrawing();
  }
  //CloseWindow();

  return 0;
}
