#pragma once
#include <cstdint>
#include <iostream>

// Memory and CPU constants
constexpr uint16_t ROMBASE = 0x200;
constexpr uint16_t FONTBASE = 0x50;
constexpr uint8_t SZ_STACK = 16;
constexpr uint8_t SZ_PIXEL = 10;

// Memory and CPU state
extern uint8_t ram[4096];
extern uint8_t display[64*32];
extern uint16_t pc;
extern uint16_t i_reg;
extern uint16_t stack[SZ_STACK];
extern uint8_t sp;
extern uint8_t v_regs[16];

// Timers
extern uint8_t delay_timer;
extern uint8_t sound_timer;

// Legacy behavior flag
extern bool legacy;

// CPU functions
uint16_t fetch();
void decodeExecute(uint16_t instr);
int LoadROM(const char* path);
int LoadFont();

// Opcode macros
#define OPCODE(instr) ((instr >> 12) & 0xF)
#define X(instr) ((instr >> 8) & 0xF)
#define Y(instr) ((instr >> 4) & 0xF)
#define N(instr) (instr & 0xF)
#define NN(instr) (instr & 0xFF)
#define NNN(instr) (instr & 0xFFF)

// Stack functions
int push(uint16_t arg);
int pop();