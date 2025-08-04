#include "main.hpp"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstring>

uint8_t ram[4096] = {};         // 4KB memory
uint8_t display[64*32] = {};   // 64x32 Display 
uint16_t pc = ROMBASE;          // PC
uint16_t i_reg;                 // Index Register
uint16_t stack[SZ_STACK];       // 16 stack entries
uint8_t sp = SZ_STACK;
uint8_t v_regs[0xF] = {};        // General Purpose Registers V0-VF

bool incrementIFX55FX65 = false;

uint8_t delay_timer = 0;        // 60 Hz Delay Timer
uint8_t sound_timer = 0;        // Sound Timer

// Sound system
sf::SoundBuffer beepBuffer;
sf::Sound beepSound;
bool soundInitialized = false;

const uint8_t font[80] = {
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

int push(uint16_t arg) {
    if (!sp) {
        std::cerr << "Error: STACK OVERFLOW" << std::endl;
        return -1;
    }
    sp -= 1;
    stack[sp] = arg;
    return 0;
}

int pop() {
    if (sp == SZ_STACK) {
        std::cerr << "Error: STACK EMPTY" << std::endl;
        return -1;
    }
    std::uint16_t out = stack[sp];
    sp += 1;
    return out;
}

int LoadROM(char const *path) {
    std::ifstream rom(path, std::ios::binary | std::ios::ate);
    if (!rom) {
        std::cerr << "Error: ROM can't be opened at path " << path << std::endl;
        return -1;
    } 
    
    std::streampos size = rom.tellg();
    rom.seekg(0, std::ios::beg);

    uint8_t *buffer = new uint8_t[size];

    if (!rom.read((char *)buffer, size)) {
        std:: cerr << "Error: can't read ROM binary into buffer" << std::endl;
        return -1;
    }

    std::copy(buffer, buffer + size, ram+ROMBASE);
    delete[] buffer;
    return 0;
}

int LoadFont() {
    for (uint i = 0; i < 80; i++) {
        ram[FONTBASE + i] = font[i];
    }
    return 0;
}

int InitSound() {
    // Generate a simple beep sound (440 Hz sine wave)
    const unsigned int sampleRate = 44100;
    const float frequency = 440.0f;  // A4 note
    const float duration = 0.1f;     // 100ms beep
    const unsigned int numSamples = static_cast<unsigned int>(sampleRate * duration);
    
    std::vector<sf::Int16> samples(numSamples);
    
    for (unsigned int i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<sf::Int16>(32767.0f * std::sin(2.0f * M_PI * frequency * t));
    }
    
    if (!beepBuffer.loadFromSamples(samples.data(), numSamples, 1, sampleRate)) {
        std::cerr << "Error: Failed to create beep sound" << std::endl;
        return -1;
    }
    
    beepSound.setBuffer(beepBuffer);
    beepSound.setVolume(50.0f);  // 50% volume
    soundInitialized = true;
    return 0;
}

void draw_display(sf::RenderWindow &window) {
    for (int y=0; y<32; ++y) {
        for (int x=0; x<64; ++x) {
            int idx = y*64 + x;
            if (display[idx]) {
                sf::RectangleShape px(sf::Vector2f(SZ_PIXEL, SZ_PIXEL));
                px.setPosition(x*SZ_PIXEL, y*SZ_PIXEL);
                px.setFillColor(sf::Color::White);
                window.draw(px);
            }
        }
    }
}

uint16_t fetch() {
    // Fetch 
    uint16_t instr = (ram[pc] << 8) | ram[pc+1];
    pc += 2;   
    return instr;
}

void decode_exe(uint16_t instr) {
    // Decode & Execute
    switch (OPCODE(instr)) {
        case 0x0:
            if (instr == 0x00E0)            // CLEAR Screen
                memset(display, 0, 64*32);
            else if (instr == 0x00EE) {     // return from subroutine
                uint16_t ret_addr = pop();
                pc = ret_addr;
            }
            break;
        case 0x1:   // JUMP
            pc = NNN(instr);
            break;
        case 0x2:   // Subroutine at NNN
            push(pc);
            pc = NNN(instr);
            break;
        case 0x3:
            // 3XNN skips 1 instr if VX value == NN
            if(v_regs[X(instr)] == NN(instr))   
                pc += 2;
            break;
        case 0x4:
            // 4XNN skips 1 instr if VX value != NN
            if(v_regs[X(instr)] != NN(instr))   
                pc += 2;
            break;
        case 0x5:
            // 5XY0 skips 1 instr if VX value == VY value
            if(v_regs[X(instr)] == v_regs[Y(instr)])   
                pc += 2;
            break;
        case 0x6:   // SET REGISTER VX
            v_regs[X(instr)] = NN(instr);
            break;
        case 0x7:
            v_regs[X(instr)] += NN(instr);
            break;
        case 0x8: {
            switch (N(instr)) {
                case 0x0:
                    v_regs[X(instr)] = v_regs[Y(instr)];
                    break;
                case 0x1:
                    v_regs[X(instr)] = v_regs[X(instr)] | v_regs[Y(instr)];
                    break;
                case 0x2:
                    v_regs[X(instr)] = v_regs[X(instr)] & v_regs[Y(instr)];
                    break;
                case 0x3:
                    v_regs[X(instr)] = v_regs[X(instr)] ^ v_regs[Y(instr)];
                    break;
                case 0x4:
                    uint16_t sum = v_regs[X(instr)] + v_regs[Y(instr)];
                    v_regs[X(instr)] = sum & 0xFF;
                    v_regs[0xF] = (sum > 0xFF) ? 1 : 0;  // Set carry flag on overflow
                    break;
            }
            break;
        }
        case 0x9: // 9XY0 skips 1 instr if VX value != VY value
            if(v_regs[X(instr)] != v_regs[Y(instr)])   
                pc += 2;
            break;
        case 0xA:
            i_reg = NNN(instr);
            break;
        case 0xD:
            uint16_t x_coord = v_regs[X(instr)] % 64;
            uint16_t y_coord = v_regs[Y(instr)] % 32;
            v_regs[0xF] = 0;
            for (int row = 0; row < N(instr); row++) {
                if (y_coord>=32) break;
                uint8_t sprite_byte = ram[i_reg + row];
                for (int bit=0; bit<8; bit++){
                    uint16_t px_X = x_coord + bit;
                    if (px_X >= 64) break;
                    uint8_t sprite_px = (sprite_byte >> (7-bit)) & 0x1;
                    if (sprite_px) {
                        uint16_t idx_displ = y_coord * 64 + px_X;
                        if (display[idx_displ]) {
                            v_regs[0xF] = 1;
                        } 
                        display[idx_displ] ^= 1;
                    }
                }
                y_coord++;
            }
            break;
        case 0xF: {
            switch (NN(instr)) {
                case 0x07:  // FX07: Set VX to the value of the delay timer
                    v_regs[X(instr)] = delay_timer;
                    break;
                case 0x15:  // FX15: Set the delay timer to the value of VX
                    delay_timer = v_regs[X(instr)];
                    break;
                case 0x18:  // FX18: Set the sound timer to the value of VX
                    sound_timer = v_regs[X(instr)];
                    break;
                case 0x1E:  // FX1E: Add VX to I
                    uint16_t old_i = i_reg;
                    i_reg += v_regs[X(instr)];
                    // Check for overflow beyond 0xFFF
                    if (old_i + v_regs[X(instr)] > 0xFFF) {
                        v_regs[0xF] = 1;  // Set flag
                    } else {
                        v_regs[0xF] = 0;  // Clear flag
                    }
                    break;
                case 0x29:  // FX29: Set I to the location of the sprite for digit VX
                    i_reg = FONTBASE + ((v_regs[X(instr)] & 0x0F) * 5);
                    break;
                case 0x33:  // FX33: Store BCD representation of VX
                    uint8_t val = v_regs[X(instr)];
                    ram[i_reg] = val / 100;
                    ram[i_reg + 1] = (val / 10) % 10;
                    ram[i_reg + 2] = val % 10;
                    break;
                case 0x55:  // FX55: Store V0 to VX in memory starting at address I
                    for (int i = 0; i <= X(instr); i++) {
                        ram[i_reg + i] = v_regs[i];
                    }
                    if (incrementIFX55FX65) {
                        i_reg += X(instr) + 1;  // Original COSMAC VIP behavior
                    }
                    break;
                case 0x65:  // FX65: Fill V0 to VX with values from memory starting at address I
                    for (int i = 0; i <= X(instr); i++) {
                        v_regs[i] = ram[i_reg + i];
                    }
                    if (incrementIFX55FX65) {
                        i_reg += X(instr) + 1;  // Original COSMAC VIP behavior
                    }
                    break;
            }
            break;
        }
    }   
}

float instrPerSecond = 10.f;
float cycleInterval = 1.f / instrPerSecond;
float cpuAccumulator = 0.f;
sf::Clock cpu_clk;

sf::Clock timer_clk;
float timerAccumulator = 0.f;
constexpr float TIMER_INTERVAL = 1.f / 60.f;

int main(int argc, char* argv[]) {
    const char* romPath = "./roms/tests/ibm_logo.ch8";
    
    // Parse command line args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--legacy") == 0 || strcmp(argv[i], "-l") == 0) {
            incrementIFX55FX65 = true;
            std::cout << "Using legacy COSMAC VIP behavior for FX55/FX65" << std::endl;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: " << argv[0] << " [options] [rom_path]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --legacy, -l    Use original COSMAC VIP behavior for instructionFX55/FX65. This enables you to play older games from the 1970s - 1980s." << std::endl;
            std::cout << "  --help, -h      Show this help message" << std::endl;
            std::cout << "Default ROM path: " << romPath << std::endl;
            return 0;
        } else if (argv[i][0] != '-') {
            romPath = argv[i];
        }
    }
    
    if (LoadROM(romPath) == -1 || LoadFont() < 0 || InitSound() < 0) {
        std::cout << "Failed init. Terminating program.";
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(64*SZ_PIXEL,32*SZ_PIXEL), "Chip8 Emulator");
    window.setFramerateLimit(60);

    cpu_clk.restart();
    timer_clk.restart();

    while(window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type==sf::Event::Closed) 
                window.close();
        }

        float cpu_dt = cpu_clk.restart().asSeconds();
        cpuAccumulator += cpu_dt;
        while (cpuAccumulator >= cycleInterval) {
            // one cycle
            uint16_t instr = fetch();
            decode_exe(instr);

            cpuAccumulator -= cycleInterval;
        }
        
        // timer handling
        float dt = timer_clk.restart().asSeconds();
        timerAccumulator += dt;
        while (timerAccumulator >= TIMER_INTERVAL) {
            if (delay_timer > 0) 
                delay_timer -= 1;

            if (sound_timer > 0) {
                sound_timer -= 1;
                // Play beep sound if not already playing
                if (soundInitialized && beepSound.getStatus() != sf::Sound::Playing) {
                    beepSound.play();
                }
            } else {
                // Stop beep sound when timer reaches 0
                if (soundInitialized && beepSound.getStatus() == sf::Sound::Playing) {
                    beepSound.stop();
                }
            }
            timerAccumulator -= TIMER_INTERVAL;
        }

        // only redraw if instruction changed display 
        window.clear(sf::Color::Black);
        draw_display(window);
        window.display();
    }

    return 0;
}