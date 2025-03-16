#include "main.hpp"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <SFML/Graphics.hpp>

uint8_t ram[4096] = {};         // 4KB memory
uint8_t display[64*32] = {};   // 64x32 Display 
uint16_t pc = ROMBASE;          // PC
uint16_t i_reg;                 // Index Register
uint16_t stack[SZ_STACK];       // 16 stack entries
uint8_t sp = SZ_STACK;
uint8_t delay_timer = 0;        // 60 Hz Delay Timer
uint8_t sound_timer = 0;        // Sount Timer
uint8_t v_regs[16] = {};        // General Purpose Registers V0-VF

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

int main() {
    LoadROM("./roms/ibm-logo.ch8");
    LoadFont();

    sf::RenderWindow window(sf::VideoMode(64*SZ_PIXEL,32*SZ_PIXEL), "Chip8 Emulator");
    window.setFramerateLimit(60);

    while(window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type==sf::Event::Closed) 
                window.close();
        }
        

        // Fetch 
        uint16_t instr = (ram[pc] << 8) | ram[pc+1];
        pc += 2;    

        // Decode & Execute
        switch (OPCODE(instr)) {
            case 0x0:
                if (instr == 0x00E0) // CLEAR Screen
                    memset(display, 0, 64*32);
                break;
            case 0x1:   // JUMP
                pc = NNN(instr);
                break;
            case 0x6:   // SET REGISTER VX
                v_regs[X(instr)] = NN(instr);
                break;
            case 0x7:
                v_regs[X(instr)] += NN(instr);
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
        }   
        // only redraw if instruction changed display 
        window.clear(sf::Color::Black);
        draw_display(window);
        window.display();
    }

    return 0;
}