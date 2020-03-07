#ifndef __H__UTIL
#define __H__UTIL

#include <regex>
#include <vector>
#include <string>
#include <map>
using namespace std;

static regex register_regex("^((R[0-7]||r[0-7])||SP||PC||PSW)$");
static regex reginddisp_regex("^(((R||r)([0-7]))||PC||SP)\\[*(0x||0b)?[a-zA-z0-9]+\\]$");
//static regex reginddisp_regex("^((R||r)([0-7])||PC||SP) *(\\+||\\-) *(0x||0b)?[a-zA-z0-9]+$");
static regex immed_regex("^\\*?([0-9A-F]+||[a-zA-Z][a-zA-Z0-9]*)$");
static regex mem_regex("^(\\$\\&)?[a-zA-Z][a-zA-Z0-9]*$");

enum class addrModes { regdir, reginddisp, immed, mem };

static vector<string> regs = { "r0", "R0", "r1", "R1", "r2", "R2", "r3", "R3", "r4", "R4", "r5", "R5", "r6", "R6", "r7", "R7", "PC", "SP", "PSW" };

static vector<string> sections = { "text", "data", "rodata", "bss" };

static vector<string> directives = { ".char", ".word", ".long", ".align", ".skip" };

static map<string, int> defDirectives = { { ".char", 1 }, { ".word", 2 }, { ".long", 4 } };

static vector<string> operations = { "ADD", "EQADD", "NEADD", "GRADD", "ALADD", "SUB", "EQSUB", "NESUB", "GRSUB", "ALSUB", "MUL", "EQMUL", "NEMUL", "GRMUL", "ALMUL", "DIV", "EQDIV", "NEDIV", "GRDIV", "ALDIV", "CMP", "EQCMP", "NECMP", "GRCMP", "ALCMP", "AND", "EQAND", "NEAND", "GRAND", "ALAND", "OR", "EQOR", "NEOR", "GROR", "ALOR", "NOT", "EQNOT", "NENOT", "GRNOT", "ALNOT", "TEST", "EQTEST", "NETEST", "GRTEST", "ALTEST", "PUSH", "EQPUSH", "NETEST", "GRTEST", "ALTEST", "POP", "EQPOP", "NEPOP", "GRPOP", "ALPOP", "CALL", "EQCALL", "NECALL", "GRCALL", "ALCALL", "IRET", "EQIRET", "NEIRET", "GRIRET", "ALIRET", "MOV", "EQMOV", "NEMOV", "GRMOV", "ALMOV", "SHL", "EQSHL", "NESHL", "GRSHL", "ALSHL", "SHR", "EQSHR", "NESHR", "GRSHR", "ALSHR", "JMP", "JZ", "JNZ", "JGT", "RET" };

static vector<string> twoOperandInstructions = { "ADD", "EQADD", "NEADD", "GRADD", "SUB", "EQSUB", "NESUB", "GRSUB", "ALSUB", "MUL", "EQMUL", "NEMUL", "GRMUL", "ALMUL", "DIV", "EQDIV", "NEDIV", "GRDIV", "ALDIV", "CMP", "EQCMP", "NECMP", "GRCMP", "ALCMP", "AND", "EQAND", "NEAND", "GRAND", "ALAND", "OR", "EQOR", "NEOR", "GROR", "ALOR", "NOT", "EQNOT", "NENOT", "GRNOT", "ALNOT", "TEST", "EQTEST", "NETEST", "GRTEST", "ALTEST", "MOV", "EQMOV", "NEMOV", "GRMOV", "ALMOV", "SHL", "EQSHL", "NESHL", "GRSHL", "ALSHL", "SHR", "EQSHR", "NESHR", "GRSHR", "ALSHR" };
static vector<string> oneOperandInstructions = { "PUSH", "EQPUSH", "NEPUSH", "GRPUSH", "ALPUSH", "POP", "EQPOP", "NEPOP", "GRPOP", "ALPOP", "CALL", "EQCALL", "NECALL", "GRCALL", "ALCALL" };
static vector<string> noOperandInstructions = { "IRET", "RET" };
static vector<string> jmpInstructions = { "JMP", "JZ", "JNZ", "JGT" };	//all jump instructions are one operand instructions

static bool isNumber(string exp) {
	bool ret = true;
	for (auto& e : exp) {
		if (!isdigit(e)) {
			ret = false;
			break;
		}
	}
	return ret;
}

static bool isRegister(string exp) {
	return  (find(regs.begin(), regs.end(), exp) != regs.end());
}

static bool isExpression(string exp) {
	return (exp.find('+') != string::npos) || (exp.find('-') != string::npos) || (exp.find('*') != string::npos) || (exp.find('/') != string::npos);
}

static bool isBinary(string val) {
	return val.substr(0, 2).compare("0b") == 0;
}

static bool isHex(string val) {
	return val.substr(0, 2).compare("0x") == 0;
}

static bool isAdd(string inst) {
	return inst == "ADD" || inst == "EQADD" || inst == "NEADD" || inst == "GTADD" || inst == "ALADD";
}

static bool isSub(string inst) {
	return inst == "SUB" || inst == "EQSUB" || inst == "NESUB" || inst == "GTSUB" || inst == "ALSUB";
}

static bool isMul(string inst) {
	return inst == "MUL" || inst == "EQMUL" || inst == "NEMUL" || inst == "GTMUL" || inst == "ALMUL";
}

static bool isDiv(string inst) {
	return inst == "DIV" || inst == "EQDIV" || inst == "NEDIV" || inst == "GTDIV" || inst == "ALDIV";
}

static bool isCmp(string inst) {
	return inst == "CMP" || inst == "EQCMP" || inst == "NECMP" || inst == "GTCMP" || inst == "ALCMP";
}

static bool isAnd(string inst) {
	return inst == "AND" || inst == "EQAND" || inst == "NEAND" || inst == "GTAND" || inst == "ALAND";
}

static bool isOr(string inst) {
	return inst == "OR" || inst == "EQOR" || inst == "NEOR" || inst == "GTOR" || inst == "ALOR";
}

static bool isNot(string inst) {
	return inst == "NOT" || inst == "EQNOT" || inst == "NENOT" || inst == "GTNOT" || inst == "ALNOT";
}

static bool isTest(string inst) {
	return inst == "TEST" || inst == "EQTEST" || inst == "NETEST" || inst == "GTTEST" || inst == "ALTEST";
}

static bool isPush(string inst) {
	return inst == "PUSH" || inst == "EQPUSH" || inst == "NEPUSH" || inst == "GTPUSH" || inst == "ALPUSH";
}

static bool isPop(string inst) {
	return inst == "POP" || inst == "EQPOP" || inst == "NEPOP" || inst == "GTPOP" || inst == "ALPOP";
}

static bool isMov(string inst) {
	return inst == "MOV" || inst == "EQMOV" || inst == "NEMOV" || inst == "GTMOV" || inst == "ALMOV";
}

static bool isShl(string inst) {
	return inst == "SHL" || inst == "EQSHL" || inst == "NESHL" || inst == "GTSHL" || inst == "ALSHL";
}

static bool isShr(string inst) {
	return inst == "SHR" || inst == "EQSHR" || inst == "NESHR" || inst == "GTSHR" || inst == "ALSHR";
}

static bool isAluOrMov(string inst) {	//ALU or MOV
	return isAdd(inst) || isSub(inst) || isMul(inst) || isDiv(inst) || isCmp(inst) || isAnd(inst) || isOr(inst) || isNot(inst) || isTest(inst) || isMov(inst) || isShl(inst) || isShr(inst);
}

#endif
