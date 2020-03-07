#ifndef __H__PARSER
#define __H__PARSER

#include <vector>
#include <string>
#include "symbolTable.h"
#include "sectionTable.h"
#include "relocTable.h"
using namespace std;

class Parser {
private:
	vector<vector<string>> instructions;
	vector<string> parseLineIntoTokens(string std, string delimiters);
	SymbolTable symTab;
	SectionTable secTab;
	uint8_t mapInstructionToSize(string inst, vector<string> line, bool labelFlag) const;
	void toUppercase(string& str);
	string infixToPostfix(string infix);
	uint32_t calcPostfix(string postfix, bool& relNeeded, string& lab);
	int getWeight(char ch) const;
	RelocTable textRelTab;
	RelocTable dataRelTab;
	RelocTable rodataRelTab;
	RelocTable *currentRelTab;
	bool writeSymbolTableToFile(ofstream& out) const;
	bool writeRelocationTablesToFile(ofstream& out);
	vector<string> secOrder;
	uint16_t startAddress;
public:
	Parser(uint16_t _startAddress);
	bool firstPass();
	bool secondPass();
	bool readFile(string path);
	bool writeToFile(string path);
	friend class Symbol;
};

#endif
