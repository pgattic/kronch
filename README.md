
# Kronch 🍫

A simple CHIP-8 Emulator written in C!

Requires [Raylib](https://www.raylib.com/). 

## Overview

CHIP-8 is regarded by some as an interpreted programming language. It is meant to serve as a "fantasy" hardware instruction set. This is my attempt at making an emulator capable of running software written to be used on that architecture.

I created this software mainly as a challenge to myself, since I have long been fascinated by emulators and their innter workings. I feel that I still have much to learn, but I am really glad to have learned what I have learned so far. I hope to eventually apply these principles later on, especially as I learn more about software virtualization and containerization.

A demo of me running this emulator can be found [here](https://youtu.be/Wt4zm8NBFr0).

## Development Environment

To develop this emulator, I used:

- [Neovim](https://neovim.io/) (as my IDE)
- [Raylib](https://www.raylib.com/) (Graphics library)
- The C compiler provided by the [GNU Compiler Collection](https://gcc.gnu.org/)

## Useful Websites

Here are some resources that proved most helpful to me:

- Source code for JohnEarnest's [OCTO](https://github.com/JohnEarnest/Octo), a CHIP-8 IDE, which includes a very well-made JavaScript implementation
- Wikipedia's page on the [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) (although its lack of documentation on hardware quirks may have slowed me down overall)
- Tobias V. Langhoff's [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- Timendus's [CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite), a powerful collection of test ROMs that really helped me identify what little things I got wrong

## Usage (Linux)

1. `git clone https://github.com/pgattic/kronch && cd kronch`
2. `make`
3. `./kronch [ROM].ch8`

The output binary is called "kronch". After compiling, you can install the emulator system-wide with:

4. `sudo make install`
5. Start the emulator with `kronch [ROM].ch8`

## Usage (others)

1. Try running `gcc src/main.c -o kronch -lraylib -lGL -lm -lpthread -ldl -lrt` from the root of this repository! No guarantees that it will all work on other operating systems.

