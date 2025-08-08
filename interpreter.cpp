#include "interpreter.hpp"
#include "peripheral.hpp"
#include <fstream>
#include <algorithm>
#include <random>
#include <cstring>

// Memory and CPU state
uint8_t ram[4096] = {};
uint8_t display[64*32] = {};
uint16_t pc = ROMBASE;
uint16_t i_reg;
uint16_t stack[SZ_STACK];
uint8_t sp = SZ_STACK;
uint8_t v_regs[16] = {};

// Timers
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

// Legacy behavior flag
bool legacy = false;

// Random number generation
std::random_device rd;
std::mt19937 gen(rd());

// Font data
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

// Stack functions
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
    uint16_t out = stack[sp];
    sp += 1;
    return out;
}

// CPU functions
uint16_t fetch() {
    uint16_t instr = (ram[pc] << 8) | ram[pc+1];
    pc += 2;
    return instr;
}

int LoadROM(const char* path) {
    std::ifstream rom(path, std::ios::binary | std::ios::ate);
    if (!rom) {
        std::cerr << "Error: ROM can't be opened at path " << path << std::endl;
        return -1;
    }
    
    std::streampos size = rom.tellg();
    rom.seekg(0, std::ios::beg);

    uint8_t* buffer = new uint8_t[size];

    if (!rom.read((char*)buffer, size)) {
        std::cerr << "Error: can't read ROM binary into buffer" << std::endl;
        return -1;
    }

    std::copy(buffer, buffer + size, ram + ROMBASE);
    delete[] buffer;
    return 0;
}

int LoadFont() {
    for (uint i = 0; i < 80; i++) {
        ram[FONTBASE + i] = font[i];
    }
    return 0;
}

void decodeExecute(uint16_t instr) {
    switch (OPCODE(instr)) {
        case 0x0:
            if (instr == 0x00E0)
                memset(display, 0, 64*32);
            else if (instr == 0x00EE) {
                uint16_t ret_addr = pop();
                pc = ret_addr;
            }
            break;
        case 0x1:
            pc = NNN(instr);
            break;
        case 0x2:
            push(pc);
            pc = NNN(instr);
            break;
        case 0x3:
            if(v_regs[X(instr)] == NN(instr))
                pc += 2;
            break;
        case 0x4:
            if(v_regs[X(instr)] != NN(instr))
                pc += 2;
            break;
        case 0x5:
            if(v_regs[X(instr)] == v_regs[Y(instr)])
                pc += 2;
            break;
        case 0x6:
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
                case 0x4: {
                    uint16_t sum = v_regs[X(instr)] + v_regs[Y(instr)];
                    v_regs[X(instr)] = sum & 0xFF;
                    v_regs[0xF] = (sum > 0xFF) ? 1 : 0;
                    break;
                }
                case 0x5: {
                    uint8_t old_vx = v_regs[X(instr)];
                    uint8_t old_vy = v_regs[Y(instr)];
                    v_regs[X(instr)] = old_vx - old_vy;
                    v_regs[0xF] = (old_vx >= old_vy) ? 1 : 0;
                    break;
                }
                case 0x6: {
                    if (legacy) {
                        v_regs[X(instr)] = v_regs[Y(instr)];
                        uint8_t oldVX = v_regs[X(instr)];
                        v_regs[X(instr)] = v_regs[X(instr)] >> 1;
                        v_regs[0xF] = oldVX & 0x1;
                    } else {
                        uint8_t oldVX = v_regs[X(instr)];
                        v_regs[X(instr)] = v_regs[X(instr)] >> 1;
                        v_regs[0xF] = oldVX & 0x1;
                    }
                    break;
                }
                case 0x7: {
                    uint8_t old_vx = v_regs[X(instr)];
                    uint8_t old_vy = v_regs[Y(instr)];
                    v_regs[X(instr)] = old_vy - old_vx;
                    v_regs[0xF] = (old_vy >= old_vx) ? 1 : 0;
                    break;
                }
                case 0xE: {
                    if (legacy) {
                        v_regs[X(instr)] = v_regs[Y(instr)];
                        uint8_t oldVX = v_regs[X(instr)];
                        v_regs[X(instr)] = v_regs[X(instr)] << 1;
                        v_regs[0xF] = (oldVX & 0x80) >> 7;
                    } else {
                        uint8_t oldVX = v_regs[X(instr)];
                        v_regs[X(instr)] = v_regs[X(instr)] << 1;
                        v_regs[0xF] = (oldVX & 0x80) >> 7;
                    }
                    break;
                }
            }
            break;
        }
        case 0x9:
            if(v_regs[X(instr)] != v_regs[Y(instr)])
                pc += 2;
            break;
        case 0xA:
            i_reg = NNN(instr);
            break;
        case 0xB:
            if (legacy) {
                pc = NNN(instr) + v_regs[0];
            } else {
                pc = NNN(instr) + v_regs[X(instr)];
            }
            break;
        case 0xC: {
            std::uniform_int_distribution<> distrib(0, 0xFF);
            v_regs[X(instr)] = distrib(gen) & NN(instr);
            break;
        }
        case 0xD: {
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
        case 0xE: {
            switch (NN(instr)) {
                case 0x9E:
                    if (keys[v_regs[X(instr)]]) {
                        pc += 2;
                    }
                    break;
                case 0xA1:
                    if (!keys[v_regs[X(instr)]]) {
                        pc += 2;
                    }
                    break;
            }
            break;
        }
        case 0xF: {
            switch (NN(instr)) {
                case 0x07:
                    v_regs[X(instr)] = delay_timer;
                    break;
                case 0x0A: {
                    bool keyReleased = false;
                    for (int i = 0; i < 16; ++i) {
                        if (keysReleased[i]) {
                            v_regs[X(instr)] = i;
                            keyReleased = true;
                            break;
                        }
                    }
                    if (!keyReleased) {
                        pc -= 2;
                    }
                    break;
                }
                case 0x15:
                    delay_timer = v_regs[X(instr)];
                    break;
                case 0x18:
                    sound_timer = v_regs[X(instr)];
                    break;
                case 0x1E: {
                    uint16_t old_i = i_reg;
                    i_reg += v_regs[X(instr)];
                    if (old_i + v_regs[X(instr)] > 0xFFF) {
                        v_regs[0xF] = 1;
                    } else {
                        v_regs[0xF] = 0;
                    }
                    break;
                }
                case 0x29: {
                    i_reg = FONTBASE + ((v_regs[X(instr)] & 0x0F) * 5);
                    break;
                }
                case 0x33: {
                    uint8_t val = v_regs[X(instr)];
                    ram[i_reg] = val / 100;
                    ram[i_reg + 1] = (val / 10) % 10;
                    ram[i_reg + 2] = val % 10;
                    break;
                }
                case 0x55: {
                    for (int i = 0; i <= X(instr); i++) {
                        ram[i_reg + i] = v_regs[i];
                    }
                    if (legacy) {
                        i_reg += X(instr) + 1;
                    }
                    break;
                }
                case 0x65: {
                    for (int i = 0; i <= X(instr); i++) {
                        v_regs[i] = ram[i_reg + i];
                    }
                    if (legacy) {
                        i_reg += X(instr) + 1;
                    }
                    break;
                }
            }
            break;
        }
    }
}
