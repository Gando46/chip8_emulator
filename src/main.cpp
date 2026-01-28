#include "chip8.h"
#include "raylib.h"
#include <iostream>
#include <string>

/*
 * CHIP-8 Emulator - Main Application
 * 
 * This file handles:
 * 1. Window creation and graphics rendering (Raylib)
 * 2. Input handling (keyboard mapping)
 * 3. Main emulation loop timing
 * 4. Audio output (beep sound)
 */

// Display configuration
constexpr int SCALE_FACTOR = 15;  // Each CHIP-8 pixel = 15x15 screen pixels
constexpr int WINDOW_WIDTH = Chip8::DISPLAY_WIDTH * SCALE_FACTOR;   // 960
constexpr int WINDOW_HEIGHT = Chip8::DISPLAY_HEIGHT * SCALE_FACTOR; // 480

// Emulation speed
constexpr int CPU_FREQ_HZ = 700;  // CHIP-8 CPU cycles per second
constexpr int TIMER_FREQ_HZ = 60; // Timer updates per second

/*
 * Keyboard Mapping: CHIP-8 to Modern Keyboard
 * 
 * Original CHIP-8 keypad:     Modern keyboard mapping:
 * 1 2 3 C                     1 2 3 4
 * 4 5 6 D                     Q W E R
 * 7 8 9 E                     A S D F
 * A 0 B F                     Z X C V
 * 
 * This array maps Raylib KEY_* codes to CHIP-8 key values (0x0-0xF)
 */
struct KeyMapping {
    int raylibKey;
    uint8_t chip8Key;
};

constexpr KeyMapping keyMap[] = {
    {KEY_ONE,   0x1}, {KEY_TWO,   0x2}, {KEY_THREE, 0x3}, {KEY_FOUR, 0xC},
    {KEY_Q,     0x4}, {KEY_W,     0x5}, {KEY_E,     0x6}, {KEY_R,    0xD},
    {KEY_A,     0x7}, {KEY_S,     0x8}, {KEY_D,     0x9}, {KEY_F,    0xE},
    {KEY_Z,     0xA}, {KEY_X,     0x0}, {KEY_C,     0xB}, {KEY_V,    0xF}
};

/*
 * Handle Input
 * 
 * Checks all mapped keys and updates CHIP-8 input state
 */
void handleInput(Chip8& chip8) {
    for (const auto& mapping : keyMap) {
        // Check if key is currently pressed
        if (IsKeyDown(mapping.raylibKey)) {
            chip8.setKey(mapping.chip8Key, true);
        } 
        // Check if key was just released
        else if (IsKeyUp(mapping.raylibKey)) {
            chip8.setKey(mapping.chip8Key, false);
        }
    }
}

/*
 * Render Display
 * 
 * Draws the CHIP-8 64x32 display scaled up to window size
 * Each CHIP-8 pixel becomes a SCALE_FACTOR x SCALE_FACTOR rectangle
 */
void renderDisplay(const Chip8& chip8) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Draw each pixel
    for (int y = 0; y < Chip8::DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < Chip8::DISPLAY_WIDTH; ++x) {
            if (chip8.getPixel(x, y)) {
                // Pixel is ON - draw white rectangle
                DrawRectangle(
                    x * SCALE_FACTOR,
                    y * SCALE_FACTOR,
                    SCALE_FACTOR,
                    SCALE_FACTOR,
                    WHITE
                );
            }
        }
    }
    
    // Draw FPS counter
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    
    EndDrawing();
}

/*
 * Main Function
 */
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>\n";
        std::cerr << "Example: " << argv[0] << " roms/pong.ch8\n";
        return 1;
    }
    
    std::string romPath = argv[1];
    
    // Initialize CHIP-8
    Chip8 chip8;
    if (!chip8.loadROM(romPath)) {
        std::cerr << "[ERROR] Failed to load ROM\n";
        return 1;
    }
    
    // Initialize Raylib window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8 Emulator");
    SetTargetFPS(60);  // Cap at 60 FPS for smooth rendering
    
    // Initialize audio for beep sound
    InitAudioDevice();
    
    // Generate a simple beep sound (440 Hz sine wave)
    Wave wave = LoadWaveFromMemory(".wav", nullptr, 0);  // We'll create a proper beep
    // For now, we'll use a simple approach - in Phase 3 we'll improve this
    Sound beepSound = LoadSound("resources/beep.wav");  // You'll need to provide this
    // If you don't have a beep sound, we can generate one programmatically later
    
    std::cout << "\n==============================================\n";
    std::cout << "CHIP-8 EMULATOR STARTED\n";
    std::cout << "==============================================\n";
    std::cout << "ROM: " << romPath << "\n";
    std::cout << "Controls: See README.md for key mapping\n";
    std::cout << "Press ESC to quit\n";
    std::cout << "==============================================\n\n";
    
    // Timing variables
    double lastCycleTime = GetTime();
    double lastTimerUpdate = GetTime();
    const double cycleInterval = 1.0 / CPU_FREQ_HZ;      // Time per CPU cycle
    const double timerInterval = 1.0 / TIMER_FREQ_HZ;    // Time per timer update
    
    // Main emulation loop
    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        
        // Handle input
        handleInput(chip8);
        
        // Execute CPU cycles
        // Run multiple cycles per frame to achieve target CPU speed
        if (currentTime - lastCycleTime >= cycleInterval) {
            chip8.emulateCycle();
            lastCycleTime = currentTime;
        }
        
        // Update timers at 60Hz
        if (currentTime - lastTimerUpdate >= timerInterval) {
            chip8.updateTimers();
            lastTimerUpdate = currentTime;
        }
        
        // Play beep sound if sound timer is active
        // Note: We'll implement proper audio in Phase 3
        // if (chip8.shouldBeep()) {
        //     if (!IsSoundPlaying(beepSound)) {
        //         PlaySound(beepSound);
        //     }
        // }
        
        // Render display if draw flag is set
        if (chip8.shouldDraw()) {
            renderDisplay(chip8);
            chip8.clearDrawFlag();
        } else {
            // Still render to show FPS and handle window events
            renderDisplay(chip8);
        }
    }
    
    // Cleanup
    // UnloadSound(beepSound);
    CloseAudioDevice();
    CloseWindow();
    
    std::cout << "\n[CHIP-8] Emulator stopped\n";
    
    return 0;
}
