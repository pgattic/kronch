
# Kronch 🍫

A simple CHIP-8 Emulator written in C!

Requires [Raylib](https://www.raylib.com/). 

## Usage (Linux)

1. `git clone https://github.com/pgattic/kronch && cd kronch`
2. `make`
3. `./kronch [ROM].ch8`

The output binary is called "kronch". After compiling, you can install the emulator system-wide with:

4. `sudo make install`
5. Start the emulator with `kronch [ROM].ch8`

## Usage (others)

1. Try running `gcc src/main.c -o kronch -lraylib -lGL -lm -lpthread -ldl -lrt` from the root of this repository and see what it spits out! No guarantees that it will all work.

