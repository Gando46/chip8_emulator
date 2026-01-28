# CHIP-8 Emulator

A CHIP-8 emulator written in modern C++ (C++17) using Raylib for graphics and input handling.

## About CHIP-8

CHIP-8 is an interpreted programming language developed in the 1970s for simple video games. This emulator accurately recreates the CHIP-8 virtual machine, allowing you to run classic CHIP-8 programs and games.

## Features

- [x] Full CHIP-8 instruction set implementation
- [x] 64x32 pixel monochrome display
- [x] Keyboard input mapping
- [x] Sound support (beep timer)
- [x] Configurable clock speed
- [ ] Debug mode (coming soon)
- [ ] Pause/Resume functionality (coming soon)

## Technical Specifications

- **Memory**: 4KB RAM (4096 bytes)
- **Registers**: 16 x 8-bit (V0-VF)
- **Display**: 64x32 monochrome pixels
- **Stack**: 16 levels
- **Timers**: Delay and Sound (60Hz countdown)
- **Input**: 16-key hexadecimal keypad

## Prerequisites

- C++17 compatible compiler (GCC, Clang, MSVC)
- CMake 3.15 or higher
- Raylib 4.5 or higher

## Building

### Linux/macOS

```bash
# Install Raylib (Ubuntu/Debian)
sudo apt-get install libraylib-dev

# Or build from source
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install

# Build the emulator
mkdir build
cd build
cmake ..
make
```

### Windows (Visual Studio)

```bash
# Using vcpkg to install Raylib
vcpkg install raylib:x64-windows

# Build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Usage

```bash
./chip8-emulator path/to/rom.ch8
```

### Keyboard Mapping

CHIP-8 uses a 16-key hexadecimal keypad (0-F). The mapping is:

```
CHIP-8 Keypad    ->    Keyboard
1 2 3 C                1 2 3 4
4 5 6 D                Q W E R
7 8 9 E                A S D F
A 0 B F                Z X C V
```

## Project Structure

```
chip8-emulator/
├── src/
│   ├── chip8.h         # CHIP-8 class definition
│   ├── chip8.cpp       # CHIP-8 implementation
│   └── main.cpp        # Entry point and Raylib integration
├── roms/               # ROM files (.ch8)
└── CMakeLists.txt      # Build configuration
```

## Resources & References

- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [Awesome CHIP-8](https://chip-8.github.io/links/)
- [Test ROMs](https://github.com/Timendus/chip8-test-suite)

## Development Progress

### Phase 1: Foundation ✅
- [x] Project structure
- [x] CHIP-8 class skeleton
- [x] Memory and registers
- [x] Basic initialization

### Phase 2: Core Implementation 🚧
- [ ] Opcode fetching and decoding
- [ ] All 35 instructions
- [ ] Display rendering
- [ ] Input handling

### Phase 3: Polish 📋
- [ ] Sound implementation
- [ ] Debug features
- [ ] Performance optimization
- [ ] Error handling

## License

MIT License - Feel free to use this for learning purposes.

## Author

Built as a learning project to understand emulation, low-level programming, and C++.

## Acknowledgments

- Raylib for making graphics programming accessible
- The CHIP-8 community for excellent documentation
- Classic game developers who created timeless CHIP-8 programs
