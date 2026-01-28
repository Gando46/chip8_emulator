#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>  // For fixed-width integer types
#include <array>    // For std::array (safer than C arrays)
#include <string>   // For ROM loading error messages

/*
 * CHIP-8 Emulator Class
 * 
 * This class emulates the CHIP-8 virtual machine, which was originally
 * designed in the 1970s for programming simple video games on 8-bit computers.
 * 
 * ARCHITECTURE OVERVIEW:
 * - 4KB (4096 bytes) of RAM
 * - 16 general-purpose 8-bit registers (V0-VF)
 * - One 16-bit index register (I)
 * - One 16-bit program counter (PC)
 * - 64x32 monochrome display
 * - Two 8-bit timers (delay and sound)
 * - 16-level stack for subroutine calls
 */

class Chip8 {
public:
    // Constructor and Destructor
    Chip8();
    ~Chip8() = default;

    // Core emulation functions
    void initialize();                    // Reset the emulator to initial state
    bool loadROM(const std::string& filename);  // Load a CHIP-8 program into memory
    void emulateCycle();                  // Execute one fetch-decode-execute cycle
    
    // Timer management (should be called at 60Hz)
    void updateTimers();
    
    // Input handling
    void setKey(uint8_t key, bool pressed);  // Set key state (0-F)
    
    // Graphics access
    bool getPixel(uint8_t x, uint8_t y) const;  // Get pixel state at (x,y)
    bool shouldDraw() const { return drawFlag; }
    void clearDrawFlag() { drawFlag = false; }
    
    // Audio access
    bool shouldBeep() const { return soundTimer > 0; }

    // Constants for CHIP-8 specifications
    static constexpr int MEMORY_SIZE = 4096;    // 4KB of RAM
    static constexpr int REGISTER_COUNT = 16;   // V0-VF registers
    static constexpr int STACK_SIZE = 16;       // 16 levels of nesting
    static constexpr int KEY_COUNT = 16;        // 0-F hexadecimal keypad
    static constexpr int DISPLAY_WIDTH = 64;    // Display width in pixels
    static constexpr int DISPLAY_HEIGHT = 32;   // Display height in pixels
    static constexpr int FONTSET_SIZE = 80;     // 16 chars * 5 bytes each
    static constexpr uint16_t ROM_START_ADDRESS = 0x200;  // Programs start at 0x200

private:
    /*
     * WHY uint8_t, uint16_t instead of unsigned char, unsigned short?
     * 
     * Fixed-width integer types (uint8_t, uint16_t) are preferred because:
     * 1. PORTABILITY: They guarantee exact sizes across all platforms
     *    - uint8_t is ALWAYS 8 bits (unsigned char might be 8+ bits on exotic systems)
     *    - uint16_t is ALWAYS 16 bits (unsigned short is at least 16, could be more)
     * 
     * 2. CLARITY: The name explicitly states the size
     *    - uint8_t: "unsigned 8-bit integer" - crystal clear
     *    - unsigned char: Could be 8, 9, or even 16 bits depending on platform
     * 
     * 3. EMULATION ACCURACY: CHIP-8 has exact specifications
     *    - Memory is EXACTLY 4096 bytes (not "at least 4096")
     *    - Opcodes are EXACTLY 16 bits (not "at least 16")
     *    - If our types don't match, bitwise operations may produce wrong results
     * 
     * 4. BITWISE OPERATIONS: Fixed sizes prevent unexpected behavior
     *    - (uint8_t)0xFF << 4 always produces predictable results
     *    - With unsigned char, integer promotion rules can cause surprises
     */

    // ==================== MEMORY ====================
    /*
     * CHIP-8 Memory Map:
     * 0x000-0x1FF: Reserved for interpreter (we store font data here)
     * 0x200-0xFFF: Program ROM and work RAM
     * 
     * We use std::array instead of C-style arrays for:
     * - Bounds checking in debug mode (with .at())
     * - No implicit pointer decay
     * - Works with modern C++ algorithms
     * - Size is part of the type
     */
    std::array<uint8_t, MEMORY_SIZE> memory;

    // ==================== REGISTERS ====================
    /*
     * V0-VF: 16 general-purpose 8-bit registers
     * 
     * IMPORTANT: VF (V[15]) is special!
     * - Used as a flag register by some instructions
     * - Collision detection (sprites)
     * - Carry/borrow flag (arithmetic)
     * - Should NOT be used by programs as general-purpose storage
     */
    std::array<uint8_t, REGISTER_COUNT> V;

    /*
     * Index Register (I): 16-bit register used for memory addressing
     * - Stores memory addresses (12-bit in original CHIP-8, but we use 16-bit)
     * - Used with sprite drawing, BCD operations, and memory operations
     * - Example: "I = 0x300" means "point to memory address 0x300"
     */
    uint16_t I;

    /*
     * Program Counter (PC): 16-bit register pointing to current instruction
     * - Stores the address of the next instruction to execute
     * - Starts at 0x200 (ROM_START_ADDRESS)
     * - Increments by 2 after each instruction (opcodes are 2 bytes)
     * - Jump instructions modify PC directly
     */
    uint16_t pc;

    // ==================== GRAPHICS ====================
    /*
     * Display Buffer: 64x32 monochrome pixels
     * 
     * We use a 1D array instead of 2D because:
     * 1. Memory layout is contiguous (better cache performance)
     * 2. Easier to clear (single memset)
     * 3. Index calculation is simple: y * WIDTH + x
     * 
     * Each uint8_t represents one pixel (0 = off, 1 = on)
     * We could use a bitfield (8 pixels per byte) for memory efficiency,
     * but clarity and simplicity are more important for learning.
     */
    std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> display;
    
    /*
     * Draw Flag: Signals when the display needs to be redrawn
     * - Set to true when any DRAW instruction (DXYN) is executed
     * - The main loop checks this and renders when true
     * - Reset to false after rendering
     */
    bool drawFlag;

    // ==================== TIMERS ====================
    /*
     * Delay Timer: Counts down at 60Hz when non-zero
     * - Programs use this for timing events
     * - Example: Wait for 5 seconds = set delay timer to 300 (5 * 60)
     */
    uint8_t delayTimer;

    /*
     * Sound Timer: Counts down at 60Hz, beeps when non-zero
     * - When > 0, the system should play a beep sound
     * - Counts down automatically at 60Hz
     * - Used for simple sound effects in games
     */
    uint8_t soundTimer;

    // ==================== STACK ====================
    /*
     * Stack: Used for subroutine calls (CALL instruction)
     * - Stores return addresses when entering subroutines
     * - 16 levels deep (can nest 16 subroutine calls)
     * - Each entry is 16-bit (stores a memory address)
     * 
     * Example flow:
     * 1. CALL 0x300: Push current PC to stack, jump to 0x300
     * 2. RETURN: Pop address from stack, jump back
     */
    std::array<uint16_t, STACK_SIZE> stack;
    
    /*
     * Stack Pointer: Points to the top of the stack
     * - Starts at 0 (empty stack)
     * - Increments on CALL (push)
     * - Decrements on RETURN (pop)
     * - Should never exceed STACK_SIZE-1 or go below 0
     */
    uint8_t sp;

    // ==================== INPUT ====================
    /*
     * Keyboard State: 16 keys (0x0-0xF)
     * - True if key is currently pressed, false otherwise
     * - Original CHIP-8 used a hexadecimal keypad:
     *   1 2 3 C
     *   4 5 6 D
     *   7 8 9 E
     *   A 0 B F
     */
    std::array<bool, KEY_COUNT> keys;

    // ==================== FONT DATA ====================
    /*
     * Built-in Font: Hexadecimal digits 0-F (5 bytes each)
     * - Stored in memory from 0x000 to 0x04F
     * - Each character is 4 pixels wide, 5 pixels tall
     * - Used by the "draw digit" instruction (FX29)
     * 
     * Font is stored as bitmaps:
     * Example: '0' =
     * 11110000  (0xF0)
     * 10010000  (0x90)
     * 10010000  (0x90)
     * 10010000  (0x90)
     * 11110000  (0xF0)
     */
    static constexpr std::array<uint8_t, FONTSET_SIZE> fontset = {
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

    // ==================== CURRENT OPCODE ====================
    /*
     * Opcode: Current 16-bit instruction being executed
     * - CHIP-8 instructions are 2 bytes (16 bits)
     * - Fetched from memory at address PC and PC+1
     * - Format: ANNN where A is the operation, NNN are operands
     * 
     * Example: 0x6A15
     * - Broken down as: 6 A 15
     * - Meaning: "Set register VA to value 0x15"
     */
    uint16_t opcode;

    // Private helper functions for opcode execution
    // (We'll implement these in chip8.cpp)
    void executeOpcode();  // Decode and execute current opcode
};

#endif // CHIP8_H
