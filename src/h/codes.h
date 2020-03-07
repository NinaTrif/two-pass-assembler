#ifndef __H__CODES
#define __H__CODES

#include <map>
using namespace std;

static map<string, int> opCodes = { { "ADD", 0 }, {"SUB", 1 }, { "MUL", 2 }, { "DIV", 3 }, { "CMP", 4 }, { "AND", 5 }, { "OR", 6 }, { "NOT", 7 }, { "TEST", 8 }, { "PUSH", 9 }, { "POP", 10 }, { "CALL", 11 }, { "IRET", 12 }, { "MOV", 13 }, { "SHL", 14 }, { "SHR", 15 } };

static map<string, int> addrModes = { { "immed", 0 }, { "imm", 0 }, { "psw", 0 }, { "regdir", 1 }, { "mem", 2 }, { "reginddisp", 3 } };

static map<string, int> registers = { { "R0", 0 }, { "R1", 1 }, { "R2", 2 }, { "R3", 3 }, { "R4", 4 }, { "R5", 5 }, { "R6", 6 }, { "R7", 7 }, { "SP", 6 }, { "PC", 7 }, { "r0", 0 }, { "r1", 1 }, { "r2", 2 }, { "r3", 3 }, { "r4", 4 }, { "r5", 5 }, { "r6", 6 }, { "r7", 7 }, { "sp", 6 }, { "pc", 7 } };
//R7 == PC, R6 == SP
//PSW: Mx...xNCOZ (flags: M - mask all interrupts, N - negative, C - carry, O - overflow, Z - zero)

static map<string, int> condCodes = { {"eq", 0}, {"ne", 1}, {"gt", 2}, {"al", 3} };

#endif
