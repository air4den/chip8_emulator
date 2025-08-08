# CHIP-8 Emulator

My C++ implementation of a CHIP-8 emulator using SFML for graphics, audio, and input handling. 

## What is CHIP-8?

[CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) is an interpreted programming language developed by Joseph Weisbecker in the mid-1970s. It was originally designed for the COSMAC VIP and Telmac 1800 microcomputers.The CHIP-8 virtual machine has:

- **4KB of RAM** (0x000-0xFFF)
- **16 general-purpose registers** (V0-VF)
- **Stack** - stores just return addresses for subroutine 
- **64x32 monochrome display**
- **16-key hexadecimal keypad**
- **Simple sound support**
- **35 opcodes** for basic operations


## Dependencies

- **SFML**
- **C++11 compatible compiler** (GCC, Clang, MSVC)

### Installing SFML

**macOS (with Homebrew):**
```bash
brew install sfml
```

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```

**Windows:**
Download from [SFML website](https://www.sfml-dev.org/download.php) or use vcpkg:
```bash
vcpkg install sfml
```

## Building

```bash
# Build the executable
make

# Clean build files
make clean
```

## Usage

### Basic Commands

```bash
# Run with default ROM (IBM Logo)
make run

# Run with specific ROM
make run ROM=<rom_path>

# Run with legacy COSMAC VIP behavior
make run ROM=<rom_path> LEGACY=--legacy
```

```bash
# Run Executable
./chip8 <rom_path>
```

### Command Line Options

- `--legacy, -l`: Enable original COSMAC VIP behavior
- `--help, -h`: Show usage information
- `[rom_path]`: Path to CHIP-8 ROM file

### Example

```bash
# Play Tetris
make run ROM=./roms/tetris.ch8

./chip8 ./roms/tetris.ch8
```

## Controls

The emulator maps keyboard keys to the CHIP-8's 16-key hexadecimal keypad:

```
COSMAC VIP →    KEYBOARD
1 2 3 C    →    1 2 3 4
4 5 6 D    →    Q W E R
7 8 9 E    →    A S D F
A 0 B F    →    Z X C V
```

## Included ROMs

### Test ROMs (`rooms/tests/*`)
- IBM Logo
- IBM Logo 2
- Corax+
- Flags
- Keypad
- Quirks

### Game ROMs (`roms/*`)
ROMs are sourced from the [Super-Chip Emulator repository](https://github.com/dario-santos/Super-Chip-Emulator):

- Airplane
- Breakout
- LunarLander
- Pong
- Socce3r
- SpaceInvaders
- Tank
- Tetris
- Wipeoff
- Worm
