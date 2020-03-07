#include "parser.h"
#include "section.h"
#include "symbol.h"
#include "sectionTable.h"
#include "symbolTable.h"
#include "util.h"
#include "codes.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <cinttypes>
#include <stack>
#include <regex>
#include <iomanip>
using namespace std;

Parser::Parser(uint16_t _startAddress) {
	startAddress = _startAddress;
}

int Parser::getWeight(char ch) const {
	switch (ch) {
		case '/':
		case '*': return 2;
		case '+':
		case '-': return 1;
		default: return 0;
	}
}

string Parser::infixToPostfix(string infix) {
	map<char, int> weights = { { '*', 2 }, { '/', 2 }, { '+', 1 }, { '-', 1 } };
	stack<char> stack;
	int weight = 0;
	string postfix;

	for (char& i : infix) {
		if (i == '(') {
			stack.push(i);
			continue;
		}
		if (i == ')') {
			while (!stack.empty() && stack.top() != '(') {
				if (postfix.at(postfix.size() - 1) != ' ')
					postfix += ' ';
				postfix += stack.top();
				postfix += ' ';
				stack.pop();
			}
			if (!stack.empty()) {
				stack.pop();
			}
			continue;
		}
		switch (i) {
		case '*':
		case '/':
		case '+':
		case '-': {
			weight = getWeight(i);
			if (postfix.at(postfix.size() - 1) != ' ')
				postfix += ' ';
			if (stack.empty())
				stack.push(i);
			else {
				while (!stack.empty() && stack.top() != '(' && weight <= getWeight(stack.top())) {
					postfix += stack.top();
					stack.pop();
				}
				stack.push(i);
				if (postfix.at(postfix.size() - 1) != ' ')
					postfix += ' ';
			}
			continue;
		}
		default: postfix += i;
		}
	}

	while (!stack.empty()) {
		postfix += ' ';
		postfix += stack.top();
		stack.pop();
	}

	return postfix;
}

uint32_t Parser::calcPostfix(string postfix, bool& relNeeded, string& lab) {
	stack<pair<int, Symbol*>> stack;
	vector<char> ops = { '+', '-', '*', '/' };
	string delimiters = " ";
	map<string, int> sectionCnt = { { ".text", 0 }, { ".data", 0 }, { ".rodata", 0 }, { ".bss", 0 }, { ".UND", 0 } };
	Symbol *textSymbol = nullptr, *dataSymbol = nullptr, *rodataSymbol = nullptr, *undSymbol = nullptr, *bssSymbol = nullptr;
	int notDelim = postfix.find_first_not_of(delimiters, 0);
	int delim = postfix.find_first_of(delimiters, notDelim);
	while (notDelim != string::npos || delim != string::npos) {
		/*if (isdigit(postfix[notDelim])) {	//number
			int val = 0;
			for (int i = notDelim; i < delim; i++)
				val = val * 10 + postfix[i] - '0';
			stack.push(make_pair(val, nullptr));
		}*/
		if (isNumber(postfix.substr(notDelim, delim - notDelim))) {	//number
			int val = 0;
			for (int i = notDelim; i < delim; i++)
			val = val * 10 + postfix[i] - '0';
			stack.push(make_pair(val, nullptr));
		}
		else if (find(ops.begin(), ops.end(), postfix[notDelim]) != ops.end()) {	//operation
			int op2 = stack.top().first;
			Symbol *sym1 = stack.top().second;
			stack.pop();
			int op1 = stack.top().first;
			Symbol *sym2 = stack.top().second;
			stack.pop();
			int res;
			switch (postfix[notDelim]) {
			case '+': res = op1 + op2; break;
			case '-': res = op1 - op2; break;
			case '*': res = op1 * op2; break;
			case '/': res = op1 / op2; break;
			}
			if (sym1 != nullptr) {
				if (sym1->getSection() == ".UND") {
					undSymbol = sym1;
					map<string, int>::iterator it = sectionCnt.find(".UND");
					it->second++;
					if (it->second == 0)
						undSymbol = nullptr;
				}
				else if (sym1->getSection() == ".text") {
					textSymbol = sym1;
					map<string, int>::iterator it = sectionCnt.find(".text");
					it->second++;
					if (it->second == 0)
						textSymbol = nullptr;
				}
				else if (sym1->getSection() == ".data") {
					dataSymbol = sym1;
					map<string, int>::iterator it = sectionCnt.find(".data");
					it->second++;
					if (it->second == 0)
						dataSymbol = nullptr;
				}
				else if (sym1->getSection() == ".rodata") {
					rodataSymbol = sym1;
					map<string, int>::iterator it = sectionCnt.find(".rodata");
					it->second++;
					if (it->second == 0)
						rodataSymbol = nullptr;
				}
				else if (sym1->getSection() == ".bss") {
					bssSymbol = sym1;
					map<string, int>::iterator it = sectionCnt.find(".bss");
					it->second++;
					if (it->second == 0)
						bssSymbol = nullptr;
				}
			}
			if (sym2 != nullptr) {
				if (sym2->getSection() == ".UND") {
					undSymbol = sym2;
					map<string, int>::iterator it = sectionCnt.find(".UND");
					it->second++;
					if (it->second == 0)
						undSymbol = nullptr;
				}
				else if (sym2->getSection() == ".text") {
					textSymbol = sym2;
					map<string, int>::iterator it = sectionCnt.find(".text");
					if (postfix[notDelim] == '+')
						it->second++;
					else if (postfix[notDelim] == '-')
						it->second--;
					if (it->second == 0)
						textSymbol = nullptr;
				}
				else if (sym2->getSection() == ".data") {
					dataSymbol = sym2;
					map<string, int>::iterator it = sectionCnt.find(".data");
					if (postfix[notDelim] == '+')
						it->second++;
					else if (postfix[notDelim] == '-')
						it->second--;
					if (it->second == 0)
						dataSymbol = nullptr;
				}
				else if (sym2->getSection() == ".rodata") {
					rodataSymbol = sym2;
					map<string, int>::iterator it = sectionCnt.find(".rodata");
					if (postfix[notDelim] == '+')
						it->second++;
					else if (postfix[notDelim] == '-')
						it->second--;
					if (it->second == 0)
						rodataSymbol = nullptr;
				}
				else if (sym2->getSection() == ".bss") {
					bssSymbol = sym2;
					map<string, int>::iterator it = sectionCnt.find(".bss");
					if (postfix[notDelim] == '+')
						it->second++;
					else if (postfix[notDelim] == '-')
						it->second--;
					if (it->second == 0)
						bssSymbol = nullptr;
				}
			}
			stack.push(make_pair(res, nullptr));
		}
		else {	//label
			string label = postfix.substr(notDelim, delim - notDelim);
			Symbol* sym = symTab.getElemByName(label);
			if (sym != nullptr) {
				//uint16_t val = sym->getOffset();
				uint16_t val = sym->getIsGlobal() ? 0 : sym->getOffset();
				stack.push(make_pair(val, sym));
			}
			else {
				lab = "Symbol not defined!";
				return 0;
			}
		}

		notDelim = postfix.find_first_not_of(delimiters, delim);
		delim = postfix.find_first_of(delimiters, notDelim);
	}
	int relocCnt = 0;
	Symbol *relSym = nullptr;
	if (textSymbol != nullptr) {
		relSym = textSymbol;
		relocCnt++;
	}
	if (dataSymbol != nullptr) {
		relSym = dataSymbol;
		relocCnt++;
	}
	if (rodataSymbol != nullptr) {
		relSym = rodataSymbol;
		relocCnt++;
	}
	if (bssSymbol != nullptr) {
		relSym = bssSymbol;
		relocCnt++;
	}
	if (undSymbol != nullptr) {
		relSym = undSymbol;
		relocCnt++;
	}
	if (relocCnt == 0)
		relNeeded = false;
	else if (relocCnt == 1) {
		relNeeded = true;
		lab = textSymbol != nullptr ? textSymbol->getName() : (dataSymbol != nullptr ? dataSymbol->getName() : (rodataSymbol != nullptr ? rodataSymbol->getName() : (bssSymbol != nullptr ? bssSymbol->getName() : undSymbol->getName())));
	}
	else {
		relNeeded = true;
		lab = "Relocation based on multiple sections needed!";
	}

	uint32_t retVal = stack.top().first;
	if (relSym != nullptr && relSym->getIsGlobal())
		retVal -= relSym->getOffset();
	return retVal;
}

bool Parser::writeToFile(string path) {
	ofstream outFile(path);

	outFile << "#symbol table" << endl;
	outFile << left << setw(15) << std::setfill(' ') << "#Index" << left << setw(15) << std::setfill(' ') << "Name" << left << setw(15) << std::setfill(' ') << "Section" << left << setw(15) << std::setfill(' ') << "Value" << left << setw(15) << std::setfill(' ') << "Global?" << left << setw(15) << std::setfill(' ') << "Size" << left << setw(15) << std::setfill(' ') << "Attr" << endl;

	for (auto& sym : symTab.getTable()) {
		outFile << sym.toString();
		if (sym.getIsSection()) {
			Section *sec = secTab.getElemByName(sym.getName());
			ostringstream secSize;
			secSize << "0x" << hex << setfill('0') << setw(4) << sec->getSize();
			string attr;
			if (sym.getName() == ".text")
				attr = "R-E";
			else if (sym.getName() == ".data")
				attr = "RW-";
			else if (sym.getName() == ".rodata")
				attr = "R--";
			else
				attr = "RW-";
			outFile << left << setw(15) << std::setfill(' ') << secSize.str() << left << setw(15) << std::setfill(' ') << attr;
		}
		outFile << endl;
	}

	writeRelocationTablesToFile(outFile);

	outFile.close();
	return true;
}

bool Parser::writeSymbolTableToFile(ofstream& out) const {
	out << symTab.toString() << endl;
	return true;
}

bool Parser::writeRelocationTablesToFile(ofstream& out) {
	if (textRelTab.getSize() > 0) {
		out << "#.rel.text" << endl;
		out << left << setw(15) << std::setfill(' ') << "#Offset" << left << setw(15) << std::setfill(' ') << "Type" << left << setw(15) << std::setfill(' ') << "Value" << endl;
		out << textRelTab.toString();
	}
	if (dataRelTab.getSize() > 0) {
		out << "#.rel.data" << endl;
		out << left << setw(15) << std::setfill(' ') << "#Offset" << left << setw(15) << std::setfill(' ') << "Type" << left << setw(15) << std::setfill(' ') << "Value" << endl;
		out << dataRelTab.toString();
	}
	if (rodataRelTab.getSize() > 0) {
		out << "#.rel.rodata" << endl;
		out << left << setw(15) << std::setfill(' ') << "#Offset" << left << setw(15) << std::setfill(' ') << "Type" << left << setw(15) << std::setfill(' ') << "Value" << endl;
		out << rodataRelTab.toString();
	}
	Section *tmpSec = secTab.getElemByName(".text");
	if (tmpSec != nullptr && tmpSec->getSize() > 0) {
		out << tmpSec->toString() << endl;;
	}
	tmpSec = secTab.getElemByName(".data");
	if (tmpSec != nullptr && tmpSec->getSize() > 0) {
		out << tmpSec->toString() << endl;
	}
	tmpSec = secTab.getElemByName(".rodata");
	if (tmpSec != nullptr && tmpSec->getSize() > 0) {
		out << tmpSec->toString() << endl;
	}
	/*tmpSec = secTab.getElemByName(".bss");
	if (tmpSec != nullptr && tmpSec->getSize() > 0) {
		out << tmpSec->toString() << endl;
	}*/
	return true;
}

void Parser::toUppercase(string& str) {
	for (auto& i : str)
		i = toupper(i);
}

vector<string> Parser::parseLineIntoTokens(string str, string delimiters) {
	vector<string> tokens;
	if (!str.empty()) {
		string::size_type notDelim = str.find_first_not_of(delimiters, 0);
		string::size_type delim = str.find_first_of(delimiters, notDelim);

		while (notDelim != string::npos || delim != string::npos) {
			string token = str.substr(notDelim, delim - notDelim);
			if (token.find(";") != string::npos)
				return tokens;

			tokens.push_back(token);
			notDelim = str.find_first_not_of(delimiters, delim);
			delim = str.find_first_of(delimiters, notDelim);
		}
	}
	return tokens;
}

bool Parser::readFile(string path) {
	ifstream inFile;
	inFile.open(path);

	if (!inFile.is_open()) {
		cout << "FAILED TO OPEN INPUT FILE!" << endl;
		return false;
	}

	string line, delimiters = " ,\t";
	while (getline(inFile, line)) {
		vector<string> tokens = parseLineIntoTokens(line, delimiters);
		if (!tokens.empty())
			instructions.push_back(tokens);
	}

	inFile.close();
	if (instructions.size() == 0) {
		cout << "FILE EMPTY! NOTHING TO PARSE!" << endl;
		return false;
	}

	return true;
}

uint8_t Parser::mapInstructionToSize(string inst, vector<string> line, bool labelFlag) const {
	if (find(twoOperandInstructions.begin(), twoOperandInstructions.end(), inst) != twoOperandInstructions.end()) {
		bool first = false, isNotReg;
		if (isNotReg = !regex_match(line.at(1 + labelFlag), register_regex))
			first = true;

		bool second = false;
		if (!regex_match(line.at(2 + labelFlag), register_regex))
			second = true;

		/*if (regex_match(line.at(1 + labelFlag), immed_regex) && isNotReg) {
			cout << "First operand's address mode cannot be immediate!" << endl;
			return 0;
		}*/

		string op1 = line.at(1 + labelFlag);

		if ((isNumber(op1) || isHex(op1) || isBinary(op1) || op1[0] == '*') && isNotReg) {
			cout << "First operand's address mode cannot be immediate!" << endl;
			return 0;
		}

		if (first && second) {
			cout << "Only one of operands can require 2 extra bytes!" << endl;
			return 0;
		}

		if (first || second)
			return 4;
		else
			return 2;
	}
	else if (find(oneOperandInstructions.begin(), oneOperandInstructions.end(), inst) != oneOperandInstructions.end() || find(jmpInstructions.begin(), jmpInstructions.end(), inst) != jmpInstructions.end()) {
		bool isPushOrCall = inst == "PUSH" || inst == "CALL" || inst == "EQPUSH" || inst == "NEPUSH" || inst == "GRPUSH" || inst == "ALPUSH" || inst == "EQCALL" || inst == "NECALL" || inst == "GRCALL" || inst == "ALCALL" || inst == "JMP" || inst == "JZ" || inst == "JNZ" || inst == "JGT";
		bool first = false, isNotReg;
		if (isNotReg = !regex_match(line.at(1 + labelFlag), register_regex)) {
			first = true;
		}

		if (regex_match(line.at(1 + labelFlag), immed_regex) && !isPushOrCall && isNotReg) {
			cout << "First operand's address mode cannot be immediate!" << endl;
			return 0;
		}

		if (first)
			return 4;
		else
			return 2;
	}
	else {
		return 2;
	}
}

bool Parser::firstPass() {

	Section *currentSection = nullptr;
	uint16_t locCnt = startAddress;
	bool labelFlag = false;
	bool relNeeded = false;
	string lab;

	symTab.pushBack(Symbol(false, false, 0, "", ".UND", true));

	for (auto& line : instructions) {
		bool mneumonicFound = false;
		for (auto& token : line) {
			if (token == ".end")
				return true;	//nothing left to parse

			if (token == ".global" || token == ".globl")
				break;	//nothing to do in first run

			if (token.find(":") != string::npos) {	//label
				if (currentSection == nullptr) {
					cout << "Label must be defined within a section!" << endl;
					return false;
				}

				if (mneumonicFound) {
					cout << "Label must be at the beginning of the line!" << endl;
					return false;
				}

				string label = token.substr(0, token.find_first_of(":"));
				if (symTab.getElemByName(label) != nullptr) {
					cout << "Symbol " << label << " already defined!" << endl;
					return false;
				}

				symTab.pushBack(Symbol(false, false, locCnt, label, currentSection->getName(), true));
				if (line.size() != 1)
					labelFlag = true;
				continue;
			}

			if (find(directives.begin(), directives.end(), token) != directives.end()) {	//directive
				if (currentSection == nullptr) {
					cout << "Directives .char, .word, .long, .align, .skip cannot be outside of a section!";
					return false;
				}
				if (defDirectives.find(token) != defDirectives.end()) {
					int numOfNonInit = 0;
					for (size_t i = find(line.begin(), line.end(), token) - line.begin() + 1; i < line.size(); i++)
						if (line.at(i) == "?")
							numOfNonInit++;
					if ((numOfNonInit < line.end() - find(line.begin(), line.end(), token)) && (currentSection->getName() == ".bss")) {
						cout << "Data cannot be initialized in .bss section!" << endl;
						return false;
					}

					uint16_t numOfBytes = (line.size() - 1 - labelFlag) * defDirectives.at(token);
					currentSection->setSize(currentSection->getSize() + numOfBytes);
					locCnt += numOfBytes;
				}
				else {
					uint32_t val = 0;
					string absExp = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);
					if (isHex(absExp)) {
						val = strtoul(absExp.substr(2).c_str(), nullptr, 16);
					}
					else if (isBinary(absExp)) {
						val = strtoul(absExp.substr(2).c_str(), nullptr, 2);
					}
					else if (isExpression(absExp)) {
						absExp = infixToPostfix(absExp);
						val = calcPostfix(absExp, relNeeded, lab);
					}
					else {
						val = strtoul(absExp.c_str(), nullptr, 10);
					}

					if (token == ".align")
						if (locCnt % val != 0)
							val = (locCnt / val + 1) * val - locCnt;
						else
							val = 0;

					currentSection->setSize(currentSection->getSize() + val);
					locCnt += val;
				}
				labelFlag = false;
				break;
			}

			if (token[0] == '.') {	//section
				if (symTab.getElemByName(token) != nullptr) {
					cout << "Section " << token << " found twice!" << endl;
					return false;
				}

				if (find(secOrder.begin(), secOrder.end(), token) == secOrder.end())
					secOrder.push_back(token);


				secTab.pushBack(Section(token, locCnt));
				currentSection = secTab.getElemByName(token);

				symTab.pushBack(Symbol(false, true, locCnt, token, currentSection->getName(), true));
				break;
			}

			toUppercase(token);
			if (find(operations.begin(), operations.end(), token) != operations.end()) {	//operation
				if (currentSection == nullptr || (currentSection != nullptr && currentSection->getName() != ".text")) {
					cout << "Instructions must be in .text section!" << endl;
					return false;
				}
				if (mneumonicFound) {
					cout << "Cannot have multiple instructions per line!" << endl;
					return false;
				}

				mneumonicFound = true;
				uint8_t instSize = mapInstructionToSize(token, line, labelFlag);

				if (instSize == 0) {
					return false;
				}

				currentSection->setSize(currentSection->getSize() + instSize);
				locCnt += instSize;

				labelFlag = false;
				break;
			}

			cout << "Unknown directive or operation!" << endl;
			return false;
		}
	}
	currentSection = nullptr;
	return true;
}

bool Parser::secondPass() {
	Section *currentSection = nullptr;
	uint16_t locCnt = startAddress;
	string lab;
	bool labelFlag = false, relNeeded = false;
	for (auto& line : instructions) {
		for (auto& token : line) {
			if (token == ".end")	//end
				return true;

			if (token.find(":") != string::npos) {	//label
				labelFlag = true;
				continue;	//nothing to do in second run
			}

			if (token == ".global" || token == ".globl") {	//global, can be done in the first run or in second, the directive should always be at the beginning of the file
				for (size_t i = 1; i < line.size(); i++) {
					Symbol* sym = symTab.getElemByName(line.at(i));
					if (sym != nullptr)
						sym->setIsGlobal(true);
					else
						symTab.pushBack(Symbol(true, false, 0, line.at(i), ".UND", false));
				}
				break;
			}

			if (find(directives.begin(), directives.end(), token) != directives.end()) {	//directive
				if (defDirectives.find(token) != defDirectives.end()) {
					for (size_t i = find(line.begin(), line.end(), token) - line.begin() + 1; i < line.size(); i++) {
						string exp = line.at(i);
						uint32_t val = 0;
						if (exp == "?") {
							val = 0;
						}
						else if (isHex(exp)) {
							val = strtoul(exp.substr(2).c_str(), nullptr, 16);
						}
						else if (isBinary(exp)) {
							val = strtoul(exp.substr(2).c_str(), nullptr, 2);
						}
						else if (isExpression(exp)) {
							exp = infixToPostfix(exp);
							val = calcPostfix(exp, relNeeded, lab);
							if (relNeeded) {
								relNeeded = false;
								if (lab == "Relocation based on multiple sections needed!" || lab == "Symbol not defined!") {
									cout << lab << endl;
									return false;
								}
								Symbol *sym = symTab.getElemByName(lab);
								uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

								if (currentSection->getName() == ".text")
									textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
								else if (currentSection->getName() == ".data")
									dataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
								else if (currentSection->getName() == ".rodata")
									rodataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							}
						}
						else if (isNumber(exp)) {
							val = strtoul(exp.c_str(), nullptr, 10);
						}
						else if (exp[0] == '?') {
							val = 0;
						}
						else if (exp[0] == '&') {
							//SHOULDN'T HAPPEN!
							Symbol *sym = symTab.getElemByName(exp.substr(1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << exp.substr(1) << endl;
								return false;
							}
							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

							if (currentSection->getName() == ".text")
								textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							else if (currentSection->getName() == ".data")
								dataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							else if (currentSection->getName() == ".rodata")
								rodataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							
							//val = sym->getOffset();
							val = sym->getIsGlobal() ? 0 : sym->getOffset();
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(exp);
							if (sym == nullptr) {
								cout << "Unknown symbol " << exp << endl;
								return false;
							}
							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

							if (currentSection->getName() == ".text")
								textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							else if (currentSection->getName() == ".data")
								dataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							else if (currentSection->getName() == ".rodata")
								rodataRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

							//val = sym->getOffset();
							val = sym->getIsGlobal() ? 0 : sym->getOffset();
						}
						int num = (token == ".long") ? 4 : ((token == ".word") ? 2 : 1);
						currentSection->appendData(val, num);
						locCnt += num;
					}
				}
				else {
					uint16_t val = 0;
					string absExp = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);
					if (isHex(absExp)) {
						val = strtoul(absExp.substr(2).c_str(), nullptr, 16);
					}
					else if (isBinary(absExp)) {
						val = strtoul(absExp.substr(2).c_str(), nullptr, 2);
					}
					else if (isExpression(absExp)) {
						absExp = infixToPostfix(absExp);
						val = calcPostfix(absExp, relNeeded, lab);
					}
					else {
						val = strtoul(absExp.c_str(), nullptr, 10);
					}

					if (token == ".align")
						if (locCnt % val != 0)
							val = (locCnt / val + 1) * val - locCnt;
						else
							val = 0;

					currentSection->appendData(0, val);
					locCnt += val;
				}

				labelFlag = false;
				break;
			}

			if (token[0] == '.') {	//section
				currentSection = secTab.getElemByName(token);
				if (token == ".text")
					currentRelTab = &textRelTab;
				else if (token == ".data")
					currentRelTab = &dataRelTab;
				else if (token == ".bss")
					currentRelTab = nullptr;
				else
					currentRelTab = &rodataRelTab;
				break;
			}

			toUppercase(token);
			if (find(operations.begin(), operations.end(), token) != operations.end()) {	//operation
				locCnt += 2;





				//PUSH
				if (isPush(token)) {	//unconditional or conditional, 2 or 4B
					uint16_t code = 0;
					if (token.substr(0, 2) == "EQ")
						code |= condCodes.at("eq") << 14;
					else if (token.substr(0, 2) == "NE")
						code |= condCodes.at("ne") << 14;
					else if (token.substr(0, 2) == "GT")
						code |= condCodes.at("gt") << 14;
					else
						code |= condCodes.at("al") << 14;
					code |= opCodes.at("PUSH") << 10;

					string op1 = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);

					if (op1[0] == '$' || op1[0] == '&') {	//$ or &
						Symbol *sym = symTab.getElemByName(op1.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1.substr(1) << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						if (op1[0] == '$') {
							//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
							code |= addrModes.at("reginddisp") << 3 | registers.at("PC");
							if (!(sym->getSection() == currentSection->getName()))
								textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));
						}
						else {
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
						}

						//uint16_t data = sym->getOffset();
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						if (op1[0] == '$') {
							data -= 2;
							if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
								data -= locCnt;
							/*if (sym->getIsGlobal())
								data -= sym->getOffset();*/
						}
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '*') {	//*
						//code |= addrModes.at("mem") << 8;
						code |= addrModes.at("mem") << 3;
						uint16_t data = strtoul(op1.substr(1).c_str(), nullptr, 10);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1.substr(0, 2) == "0x") {	//imm hex
						uint16_t data = strtoul(op1.substr(2).c_str(), nullptr, 16);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1.substr(0, 2) == "0b") {	//imm binary
						uint16_t data = strtoul(op1.substr(2).c_str(), nullptr, 2);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (isNumber(op1)) {	//imm decimal
						uint16_t data = strtoul(op1.c_str(), nullptr, 10);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (regex_match(op1, reginddisp_regex)) {	//reginddisp
						//code |= addrModes.at("reginddisp") << 8 | registers.at(op1.substr(0, 2)) << 5;
						code |= addrModes.at("reginddisp") << 3 | registers.at(op1.substr(0, 2));
						uint16_t data;
						if (isNumber(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1))) {	//imm
							data = strtoul(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (isRegister(op1)) {	//regdir
						//code |= addrModes.at("regdir") << 8 | registers.at(op1) << 5;
						code |= addrModes.at("regdir") << 3 | registers.at(op1);
						currentSection->appendData(code, 2);
					}
					else {	//mem
						//code |= addrModes.at("mem") << 8;
						code |= addrModes.at("mem") << 3;
						Symbol *sym = symTab.getElemByName(op1);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//uint16_t data = sym->getOffset();
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
				}





				//POP
				else if (isPop(token)) {	//unconditional or conditional, 2 or 4B
					uint16_t code = 0;
					if (token.substr(0, 2) == "EQ")
						code |= condCodes.at("eq") << 14;
					else if (token.substr(0, 2) == "NE")
						code |= condCodes.at("ne") << 14;
					else if (token.substr(0, 2) == "GT")
						code |= condCodes.at("gt") << 14;
					else
						code |= condCodes.at("al") << 14;
					code |= opCodes.at("POP") << 10;

					string op1 = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);

					if (isHex(op1) || isBinary(op1) || (op1[0] == '&') || isNumber(op1)) {	//error - immediate value
						cout << "POP instruction: destination cannot be immediate!" << endl;
						return false;
					}

					if (isRegister(op1)) {	//regdir
						code |= addrModes.at("regdir") << 8 | registers.at(op1) << 5;
						currentSection->appendData(code, 2);
					}
					else if (op1[0] == '*') { //*
						code |= addrModes.at("mem") << 8;
						uint16_t data = strtoul(op1.substr(1).c_str(), nullptr, 10);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '$') {	//$ (pcrel)
						Symbol *sym = symTab.getElemByName(op1.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1.substr(1) << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

						code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						if (!(sym->getSection() == currentSection->getName()))
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));

						//uint16_t data = sym->getOffset() - 2;
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						data -= 2;
						if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
							data -= locCnt;
						/*if (sym->getIsGlobal())
							data -= sym->getOffset();*/
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (regex_match(op1, reginddisp_regex)) {	//reginddisp
						code |= addrModes.at("reginddisp") << 8 | registers.at(op1.substr(0, 2)) << 5;
						uint16_t data;
						if (isNumber(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1))) {	//imm
							data = strtoul(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else {	//mem
						code |= addrModes.at("mem") << 8;
						Symbol *sym = symTab.getElemByName(op1);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//uint16_t data = sym->getOffset();
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
				}





				//RET, IRET
				else if (token == "RET" || token == "IRET") {	//unconditional, 2B
					uint16_t code = condCodes.at("al") << 14;
					if (token == "RET")
						code |= opCodes.at("POP") << 10 | addrModes.at("regdir") << 8 | registers.at("PC") << 5;
					else
						code |= opCodes.at(token) << 10;
					currentSection->appendData(code, 2);
				}





				//ADD, SUB, MUL, DIV, CMP, AND, OR, NOT, TEST, MOV, SHL, SHR
				else if (isAluOrMov(token)) {	//unconditional or conditional, 2 or 4B
					string op1 = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);
					string op2 = line.at(find(line.begin(), line.end(), token) - line.begin() + 2);

					bool hasCond = true, hasData1 = false, hasData2 = false;

					if (op1[0] == '&' || isNumber(op1) || isHex(op1) || isBinary(op1)) {	//imm
						cout << "ALU instruction cannot have immediate value as destination!" << endl;
						return false;
					}

					uint16_t code = 0;
					uint16_t data = 0;

					if (token.substr(0, 2) == "EQ")
						code |= condCodes.at("eq") << 14;
					else if (token.substr(0, 2) == "NE")
						code |= condCodes.at("ne") << 14;
					else if (token.substr(0, 2) == "GT")
						code |= condCodes.at("gt") << 14;
					else if (token.substr(0, 2) == "AL")
						code |= condCodes.at("al") << 14;
					else {
						code |= condCodes.at("al") << 14;
						hasCond = false;
					}

					if (hasCond)
						code |= opCodes.at(token.substr(2)) << 10;
					else
						code |= opCodes.at(token) << 10;

					//DESTINATION
					if (isRegister(op1)) {	//regdir
						code |= addrModes.at("regdir") << 8 | registers.at(op1) << 5;
					}
					else if (op1[0] == '*') {	//*
						hasData1 = true;
						code |= addrModes.at("mem") << 8;
						data = strtoul(op1.substr(1).c_str(), nullptr, 10);
					}
					else if (op1[0] == '$') {	//$ (pcrel)
						hasData1 = true;
						Symbol *sym = symTab.getElemByName(op1.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1.substr(1) << "!" << endl;
							return false;
						}

						//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						code |= addrModes.at("immed") << 8;

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

						if (!(sym->getSection() == currentSection->getName()))
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));

						//data = sym->getOffset() - 2;
						data = sym->getIsGlobal() ? 0 : sym->getOffset();
						data -= 2;
 						if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
							data -= locCnt;
						/*if (sym->getIsGlobal())
							data -= sym->getOffset();*/

					}
					else if (regex_match(op1, reginddisp_regex)) {	//reginddisp
						hasData1 = true;
						code |= addrModes.at("reginddisp") << 8 | registers.at(op1.substr(0, 2)) << 5;
						if (isNumber(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]")))) {	//imm
							data = strtoul(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]")).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]")));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]")) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}
					}
					else {	//mem
						hasData1 = true;
						code |= addrModes.at("mem") << 8;
						Symbol *sym = symTab.getElemByName(op1);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//data = sym->getOffset();
						data = sym->getIsGlobal() ? 0 : sym->getOffset();
					}

					//SOURCE
					if (op2[0] == '&') {
						hasData2 = true;
						Symbol *sym = symTab.getElemByName(op2.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op2.substr(1) << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//data = sym->getOffset();
						data = sym->getIsGlobal() ? 0 : sym->getOffset();
					}
					else if (op2.substr(0, 2) == "0x") {	//imm hex
						hasData2 = true;
						data = strtoul(op2.substr(2).c_str(), nullptr, 16);
					}
					else if (op2.substr(0, 2) == "0b") {	//imm binary
						hasData2 = true;
						data = strtoul(op1.substr(2).c_str(), nullptr, 2);
					}
					else if (isNumber(op2)) {	//imm decimal
						hasData2 = true;
						data = strtoul(op2.c_str(), nullptr, 10);
					}
					else if (isRegister(op2)) {	//regdir
						code |= addrModes.at("regdir") << 3 | registers.at(op2);
					}
					else if (op2[0] == '*') {	//*
						hasData2 = true;
						code |= addrModes.at("mem") << 3;
						data = strtoul(op2.substr(1).c_str(), nullptr, 10);
					}
					else if (op2[0] == '$') {	//$ (pcrel)
						hasData2 = true;
						Symbol *sym = symTab.getElemByName(op2.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op2.substr(1) << "!" << endl;
							return false;
						}

						code |= addrModes.at("reginddisp") << 3 | registers.at("PC");

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

						if (!(sym->getSection() == currentSection->getName()))
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));

						//data = sym->getOffset() - 2;
						data = sym->getIsGlobal() ? 0 : sym->getOffset();
						data -= 2;
						if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
							data -= locCnt;
						/*if (sym->getIsGlobal())
							data -= sym->getOffset();*/

					}
					else if (regex_match(op2, reginddisp_regex)) {	//reginddisp
						hasData2 = true;
						code |= addrModes.at("reginddisp") << 3 | registers.at(op2.substr(0, 2));
						if (isNumber(op2.substr(op2.find_first_of("[") + 1, op2.find_first_of("]") - op2.find_first_of("[") - 1))) {	//imm
							data = strtoul(op2.substr(op2.find_first_of("[") + 1, op2.find_first_of("]") - op2.find_first_of("[") - 1).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op2.substr(op2.find_first_of("[") + 1, op2.find_first_of("]") - op2.find_first_of("[") - 1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op2.substr(op2.find_first_of("[") + 1, op2.find_first_of("]") - op2.find_first_of("[") - 1) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}
					}
					else {	//mem
						hasData2 = true;
						code |= addrModes.at("mem") << 3;
						Symbol *sym = symTab.getElemByName(op2);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op2 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//data = sym->getOffset();
						data = sym->getIsGlobal() ? 0 : sym->getOffset();
					}

					if (hasData1 && hasData2) {
						cout << "Only one operand can require extra data!" << endl;
						return false;
					}

					currentSection->appendData(code, 2);
					if (hasData1 || hasData2) {
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
				}





				//CALL
				else if (token == "CALL") {
					uint16_t code = 0;

					if (token.substr(0, 2) == "EQ")
						code |= condCodes.at("eq") << 14;
					else if (token.substr(0, 2) == "NE")
						code |= condCodes.at("ne") << 14;
					else if (token.substr(0, 2) == "GT")
						code |= condCodes.at("gt") << 14;
					else
						code |= condCodes.at("al") << 14;

					code |= opCodes.at(token) << 10;

					string op1 = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);

					if (op1[0] == '$') {	//$ (pcrel)
						Symbol *sym = symTab.getElemByName(op1.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1.substr(1) << "!" << endl;
							return false;
						}

						//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						code |= addrModes.at("immed") << 3 | 1;
						//code |= addrModes.at("immed") << 8;

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

						if (!(sym->getSection() == currentSection->getName()))
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));

						//uint16_t data = sym->getOffset() - 2;
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						data -= 2;
						if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
							data -= locCnt;
						/*if (sym->getIsGlobal())
							data -= sym->getOffset();*/

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '*') {	//*
						//code |= addrModes.at("mem") << 8;
						code |= addrModes.at("mem") << 3;
						uint16_t data = strtoul(op1.substr(1).c_str(), nullptr, 10);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '&' || isNumber(op1) || isHex(op1) || isBinary(op1)) {	//imm
						//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						//code |= addrModes.at("reginddisp") << 3 | registers.at("PC");
						code |= addrModes.at("immed") << 3;

						uint16_t data = 0;
						if (op1[0] == '&') {
							/*Symbol *sym = symTab.getElemByName(op2.substr(1));
							if (sym == nullptr) {
							cout << "Unknown symbol " << op2.substr(1) << "!" << endl;
							return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

							data = sym->getOffset();*/
							cout << "CALL cannot be used with &!" << endl;
							return false;
						}
						else if (isNumber(op1)) {
							data = strtoul(op1.c_str(), nullptr, 10);
						}
						else if (isHex(op1))
							data = strtoul(op1.substr(1).c_str(), nullptr, 16);
						else
							data = strtoul(op1.substr(1).c_str(), nullptr, 2);

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (isRegister(op1)) {	//regdir
						//code |= addrModes.at("regdir") << 8 | registers.at(op1) << 5;
						code |= addrModes.at("regdir") << 3 | registers.at(op1);
						currentSection->appendData(code, 2);
					}
					else if (regex_match(op1, reginddisp_regex)) {	//reginddisp
						//code |= addrModes.at("reginddisp") << 8 | registers.at(op1.substr(0, 2)) << 5;
						code |= addrModes.at("reginddisp") << 3 | registers.at(op1.substr(0, 2));
						uint16_t data;
						if (isNumber(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1))) {	//imm
							data = strtoul(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else {	//mem
						//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						//code |= addrModes.at("immed") << 8;
						code |= addrModes.at("immed") << 3;
						Symbol *sym = symTab.getElemByName(op1);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//uint16_t data = sym->getOffset();
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
				}





				//JMP, JZ, JNZ, JGT
				else if (token == "JMP" || token == "JZ" || token == "JNZ" || token == "JGT") {	//unconditional or conditional, 2 or 4B
					uint16_t code = 0;
					if (token == "JMP")
						code |= condCodes.at("al") << 14;
					else if (token == "JNZ")
						code |= condCodes.at("ne") << 14;
					else if (token == "JGT")
						code |= condCodes.at("gt") << 14;

					string op1 = line.at(find(line.begin(), line.end(), token) - line.begin() + 1);

					if (op1[0] != '$')	//not pcrel -> mov
						code |= opCodes.at("MOV") << 10;

					code |= addrModes.at("regdir") << 8 | registers.at("PC") << 5;

					if (op1[0] == '$') {	//$ (pcrel)
						Symbol *sym = symTab.getElemByName(op1.substr(1));
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1.substr(1) << "!" << endl;
							return false;
						}

						//code |= addrModes.at("reginddisp") << 8 | registers.at("PC") << 5;
						code |= addrModes.at("immed") << 3;

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();

						if (!(sym->getSection() == currentSection->getName()))
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_PC16, index));

						//uint16_t data = sym->getOffset() - 2;
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						data -= 2;
						if (sym->getSection() == currentSection->getName() && !sym->getIsGlobal())
							data -= locCnt;
						/*if (sym->getIsGlobal())
							data -= sym->getOffset();*/

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '*') {	//*
						code |= addrModes.at("mem") << 3;
						uint16_t data = strtoul(op1.substr(1).c_str(), nullptr, 10);
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (op1[0] == '&' || isNumber(op1) || isHex(op1) || isBinary(op1)) {	//imm
						code |= addrModes.at("immed") << 3;
						/*code &= 0b1100001111111111;
						code |= opCodes.at("ADD") << 10;*/

						uint16_t data = 0;
						if (op1[0] == '&') {
							/*Symbol *sym = symTab.getElemByName(op2.substr(1));
							if (sym == nullptr) {
							cout << "Unknown symbol " << op2.substr(1) << "!" << endl;
							return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

							data = sym->getOffset();*/
							cout << "Jump instructions cannot be used with &!" << endl;
							return false;
						}
						else if (isNumber(op1))
							data = strtoul(op1.c_str(), nullptr, 10);
						else if (isHex(op1))
							data = strtoul(op1.substr(1).c_str(), nullptr, 16);
						else
							data = strtoul(op1.substr(1).c_str(), nullptr, 2);

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else if (isRegister(op1)) {	//regdir
						code |= addrModes.at("regdir") << 3 | registers.at(op1);
						currentSection->appendData(code, 2);
					}
					else if (regex_match(op1, reginddisp_regex)) {	//reginddisp
						code |= addrModes.at("reginddisp") << 3 | registers.at(op1.substr(0, 2));
						uint16_t data;
						if (isNumber(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1))) {	//imm
							data = strtoul(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1).c_str(), nullptr, 10);
						}
						else {	//symbol
							Symbol *sym = symTab.getElemByName(op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1));
							if (sym == nullptr) {
								cout << "Unknown symbol " << op1.substr(op1.find_first_of("[") + 1, op1.find_first_of("]") - op1.find_first_of("[") - 1) << "!" << endl;
								return false;
							}

							uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
							textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));
							//data = sym->getOffset();
							data = sym->getIsGlobal() ? 0 : sym->getOffset();
						}

						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
					else {	//mem
						code |= addrModes.at("immed") << 3;
						/*code &= 0b1100001111111111;
						code |= opCodes.at("ADD") << 10;*/
						Symbol *sym = symTab.getElemByName(op1);
						if (sym == nullptr) {
							cout << "Unknown symbol " << op1 << "!" << endl;
							return false;
						}

						uint16_t index = sym->getIsGlobal() ? sym->getId() : symTab.getElemByName(sym->getSection())->getId();
						textRelTab.pushBack(RelocEntry(locCnt - currentSection->getStartAddress(), relocType::R_386_16, index));

						//uint16_t data = sym->getOffset();
						uint16_t data = sym->getIsGlobal() ? 0 : sym->getOffset();
						currentSection->appendData(code, 2);
						currentSection->appendData(data, 2);
						locCnt += 2;
					}
				}

				labelFlag = false;
				break;
			}

			cout << "Unknown directive or operation!" << endl;
			return false;
		}
	}
	return false;
}
