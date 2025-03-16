#define SZ_STACK 16
#define ROMBASE 0x200
#define FONTBASE 0x050

#define OPCODE(instr) ((instr & 0xF000) >> 12)
#define X(instr) ((instr & 0x0F00) >> 8)
#define Y(instr) ((instr & 0x00F0) >> 4)
#define N(instr) (instr & 0x000F)
#define NN(instr) (instr & 0x00FF)
#define NNN(instr) (instr & 0x0FFF)

#define SZ_PIXEL 10
