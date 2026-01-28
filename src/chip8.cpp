#include "chip8.h"
#include <fstream>      // For file I/O
#include <iostream>     // For error messages
#include <cstring>      // For memset
#include <random>       // For RNG instruction

/*
 * CHIP-8 Constructor
 * 
 * Default constructor - actual initialization happens in initialize()
 * This is good practice: constructors should be lightweight
 */
Chip8::Chip8() {
    initialize();
}

/*
 * Initialize/Reset the CHIP-8 System
 * 
 * This function sets up the emulator to its power-on state:
 * 1. Clear all memory
 * 2. Reset registers to 0
 * 3. Load font data into memory
 * 4. Set PC to ROM start address
 * 5. Clear display, stack, and input states
 */
void Chip8::initialize() {
    // Set program counter to start of ROM area
    // WHY 0x200? The first 512 bytes (0x000-0x1FF) were reserved
    // for the CHIP-8 interpreter on original systems
    pc = ROM_START_ADDRESS;
    
    // Reset current opcode
    opcode = 0;
    
    // Reset index register
    I = 0;
    
    // Reset stack pointer
    sp = 0;
    
    // Clear display
    // std::array's fill() is safer than memset for C++ types
    display.fill(0);
    drawFlag = true;  // Draw the cleared screen
    
    // Clear stack
    stack.fill(0);
    
    // Clear registers V0-VF
    V.fill(0);
    
    // Clear memory
    memory.fill(0);
    
    // Load fontset into memory (addresses 0x000 to 0x04F)
    // WHY std::copy? It's type-safe and works with iterators
    // Could also use: std::memcpy(memory.data(), fontset.data(), FONTSET_SIZE);
    std::copy(fontset.begin(), fontset.end(), memory.begin());
    
    // Reset timers
    delayTimer = 0;
    soundTimer = 0;
    
    // Clear key states
    keys.fill(false);
    
    std::cout << "[CHIP-8] System initialized\n";
}

/*
 * Load a ROM file into memory
 * 
 * CHIP-8 ROMs are loaded starting at address 0x200
 * 
 * @param filename: Path to the .ch8 ROM file
 * @return: true if successful, false on error
 * 
 * ROM SIZE LIMITS:
 * - Memory: 0x200 to 0xFFF = 3584 bytes available
 * - Most ROMs are much smaller (typically 1-2KB)
 */
bool Chip8::loadROM(const std::string& filename) {
    // Open file in binary mode at the end to get size
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open ROM: " << filename << "\n";
        return false;
    }
    
    // Get file size
    // tellg() returns current position, which is at end due to ios::ate
    std::streamsize size = file.tellg();
    
    // Check if ROM fits in available memory
    // Memory from 0x200 to 0xFFF = 4096 - 512 = 3584 bytes
    if (size > (MEMORY_SIZE - ROM_START_ADDRESS)) {
        std::cerr << "[ERROR] ROM too large: " << size << " bytes\n";
        std::cerr << "[ERROR] Maximum size: " << (MEMORY_SIZE - ROM_START_ADDRESS) << " bytes\n";
        file.close();
        return false;
    }
    
    // Move back to beginning of file
    file.seekg(0, std::ios::beg);
    
    // Read file directly into memory starting at 0x200
    // WHY reinterpret_cast<char*>? read() expects char*, but we have uint8_t*
    // This is safe because uint8_t and unsigned char are guaranteed to have same size
    file.read(reinterpret_cast<char*>(&memory[ROM_START_ADDRESS]), size);
    
    file.close();
    
    std::cout << "[CHIP-8] Loaded ROM: " << filename << "\n";
    std::cout << "[CHIP-8] ROM size: " << size << " bytes\n";
    
    return true;
}

/*
 * Emulate One CPU Cycle
 * 
 * The fetch-decode-execute cycle:
 * 1. FETCH: Read opcode from memory at PC
 * 2. DECODE & EXECUTE: Determine what instruction it is and run it
 * 3. UPDATE: Move to next instruction (PC += 2)
 * 
 * TIMING: Original CHIP-8 ran at ~500Hz (500 instructions/second)
 * Modern emulators often run much faster or use configurable speed
 */
void Chip8::emulateCycle() {
    // FETCH: Get the opcode
    // CHIP-8 opcodes are 2 bytes, stored big-endian
    // 
    // BITWISE EXPLANATION:
    // memory[pc] is the high byte, memory[pc+1] is the low byte
    // Example: memory[pc] = 0x61, memory[pc+1] = 0x23
    // 
    // Step 1: memory[pc] << 8
    //   0x61 << 8 = 0x6100 (shift left by 8 bits)
    // 
    // Step 2: memory[pc+1]
    //   0x23 (stays as is)
    // 
    // Step 3: OR them together
    //   0x6100 | 0x0023 = 0x6123
    // 
    // WHY << 8? Shifts bits left by 8 positions, moving byte to high position
    // WHY |? Combines the two bytes without affecting existing bits
    opcode = (memory[pc] << 8) | memory[pc + 1];
    
    // DECODE & EXECUTE: Process the opcode
    executeOpcode();
    
    // Note: PC increment is handled by executeOpcode() because
    // some instructions (jumps, calls) modify PC directly
}

/*
 * Execute the Current Opcode
 * 
 * CHIP-8 has 35 different opcodes, identified by their first nibble (4 bits)
 * and sometimes additional nibbles.
 * 
 * OPCODE FORMAT: Each opcode is 16 bits, written as: ANNN
 * - A: First nibble (4 bits) - identifies instruction family
 * - NNN: 12-bit value used differently by each instruction
 * 
 * BITWISE MASKING EXPLANATION:
 * To extract parts of the opcode, we use bitwise AND with masks:
 * 
 * Example opcode: 0x6A15 (binary: 0110 1010 0001 0101)
 * 
 * Extract first nibble (A):
 *   opcode & 0xF000 = 0x6A15 & 0xF000 = 0x6000
 *   Then >> 12 to get just 0x6
 * 
 * Extract X (second nibble):
 *   opcode & 0x0F00 = 0x6A15 & 0x0F00 = 0x0A00
 *   Then >> 8 to get just 0xA
 * 
 * Extract Y (third nibble):
 *   opcode & 0x00F0 = 0x6A15 & 0x00F0 = 0x0010
 *   Then >> 4 to get just 0x1
 * 
 * Extract N (fourth nibble):
 *   opcode & 0x000F = 0x6A15 & 0x000F = 0x0005
 * 
 * Extract NN (last byte):
 *   opcode & 0x00FF = 0x6A15 & 0x00FF = 0x0015
 * 
 * Extract NNN (last 12 bits):
 *   opcode & 0x0FFF = 0x6A15 & 0x0FFF = 0x0A15
 */
void Chip8::executeOpcode() {
    // Extract common operands used by many instructions
    // These are calculated once here to avoid repetition
    
    // X: Second nibble, often indicates a register (VX)
    uint8_t X = (opcode & 0x0F00) >> 8;
    
    // Y: Third nibble, often indicates another register (VY)
    uint8_t Y = (opcode & 0x00F0) >> 4;
    
    // N: Fourth nibble, 4-bit value
    uint8_t N = opcode & 0x000F;
    
    // NN: Last byte, 8-bit value
    uint8_t NN = opcode & 0x00FF;
    
    // NNN: Last 12 bits, memory address
    uint16_t NNN = opcode & 0x0FFF;
    
    // Decode based on first nibble
    // We'll use a switch statement for clarity and efficiency
    switch (opcode & 0xF000) {
        case 0x0000:
            // Multiple opcodes start with 0x0
            switch (NN) {
                case 0x00E0:  // 00E0: Clear screen
                    display.fill(0);
                    drawFlag = true;
                    pc += 2;
                    break;
                    
                case 0x00EE:  // 00EE: Return from subroutine
                    --sp;  // Decrement stack pointer
                    pc = stack[sp];  // Get return address
                    pc += 2;  // Move past the CALL instruction
                    break;
                    
                default:
                    std::cerr << "[ERROR] Unknown opcode: 0x" << std::hex << opcode << "\n";
                    pc += 2;
            }
            break;
            
        case 0x1000:  // 1NNN: Jump to address NNN
            pc = NNN;
            break;
            
        case 0x2000:  // 2NNN: Call subroutine at NNN
            stack[sp] = pc;  // Store current PC
            ++sp;  // Increment stack pointer
            pc = NNN;  // Jump to subroutine
            break;
            
        case 0x3000:  // 3XNN: Skip next instruction if VX == NN
            pc += (V[X] == NN) ? 4 : 2;
            break;
            
        case 0x4000:  // 4XNN: Skip next instruction if VX != NN
            pc += (V[X] != NN) ? 4 : 2;
            break;
            
        case 0x5000:  // 5XY0: Skip next instruction if VX == VY
            pc += (V[X] == V[Y]) ? 4 : 2;
            break;
            
        case 0x6000:  // 6XNN: Set VX to NN
            V[X] = NN;
            pc += 2;
            break;
            
        case 0x7000:  // 7XNN: Add NN to VX (no carry flag)
            V[X] += NN;
            pc += 2;
            break;
            
        case 0x8000:
            // Arithmetic and logic operations (8XYN family)
            // We'll implement these in the next phase
            // Placeholder for now:
            std::cerr << "[TODO] Implement 0x8XYN opcodes\n";
            pc += 2;
            break;
            
        case 0x9000:  // 9XY0: Skip next instruction if VX != VY
            pc += (V[X] != V[Y]) ? 4 : 2;
            break;
            
        case 0xA000:  // ANNN: Set index register I to NNN
            I = NNN;
            pc += 2;
            break;
            
        // More opcodes will be implemented in the next phase
        // Placeholders for now:
        
        case 0xB000:  // BNNN: Jump to address NNN + V0
        case 0xC000:  // CXNN: Set VX to random number AND NN
        case 0xD000:  // DXYN: Draw sprite
        case 0xE000:  // Input handling
        case 0xF000:  // Various operations
        default:
            std::cerr << "[TODO] Opcode not yet implemented: 0x" 
                      << std::hex << opcode << "\n";
            pc += 2;
    }
}

/*
 * Update Timers
 * 
 * TIMING: Should be called at 60Hz (60 times per second)
 * Both timers count down to 0 when non-zero
 * 
 * IMPLEMENTATION NOTE: In our main loop, we'll use a timer
 * to call this function approximately 60 times per second,
 * independent of the CPU cycle speed
 */
void Chip8::updateTimers() {
    if (delayTimer > 0) {
        --delayTimer;
    }
    
    if (soundTimer > 0) {
        --soundTimer;
        // Main loop checks shouldBeep() to play sound
    }
}

/*
 * Set Key State
 * 
 * @param key: Which key (0x0 to 0xF)
 * @param pressed: true if pressed, false if released
 */
void Chip8::setKey(uint8_t key, bool pressed) {
    if (key < KEY_COUNT) {
        keys[key] = pressed;
    }
}

/*
 * Get Pixel State
 * 
 * @param x: X coordinate (0-63)
 * @param y: Y coordinate (0-31)
 * @return: true if pixel is on, false if off
 * 
 * 1D INDEX CALCULATION:
 * For a 2D array stored as 1D, the formula is: y * WIDTH + x
 * 
 * Example: Get pixel at (5, 3)
 * Index = 3 * 64 + 5 = 197
 */
bool Chip8::getPixel(uint8_t x, uint8_t y) const {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return false;  // Out of bounds
    }
    return display[y * DISPLAY_WIDTH + x] != 0;
}
