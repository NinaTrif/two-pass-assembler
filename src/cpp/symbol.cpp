#include "symbol.h"
#include <sstream>
#include <iomanip>
using namespace std;

uint16_t Symbol::ID = 0;

Symbol::~Symbol() {}

Symbol::Symbol(bool _isGlobal, bool _isSection, uint16_t _offset, string _name, string _section, bool _isDefined) : isGlobal(_isGlobal), isSection(_isSection), offset(_offset), name(_name), section(_section), isDefined(_isDefined) {}

string Symbol::toString() const {
	char global = isGlobal ? 'G' : 'L';
	ostringstream tmp;
	ostringstream addr;
	addr << "0x" << hex << setfill('0') << setw(4) << offset;
	tmp << left << setw(15) << std::setfill(' ') << id << left << setw(15) << std::setfill(' ') << name << left << setw(15) << std::setfill(' ') << section.substr(1) << left << setw(15) << std::setfill(' ') << addr.str() << left << setw(15) << std::setfill(' ') << global;
	return tmp.str();
}

bool Symbol::getIsGlobal() const {
	return isGlobal;
}

bool Symbol::getIsSection() const {
	return isSection;
}

uint16_t Symbol::getOffset() const {
	return offset;
}

string Symbol::getName() {
	return name;
}

/*Section* Symbol::getSection() const {
	return section;
}*/

string Symbol::getSection() const {
	return section;
}

uint16_t Symbol::getId() const {
	return id;
}

bool Symbol::getIsDefined() const {
	return isDefined;
}

void Symbol::setIsGlobal(bool _isGlobal) {
	isGlobal = _isGlobal;
}

void Symbol::setIsSection(bool _isSection) {
	isSection = _isSection;
}

void Symbol::setOffset(uint16_t _offset) {
	offset = _offset;
}

void Symbol::setName(const string _name) {
	name = _name;
}

/*void Symbol::setSection(Section *_section) {
	section = _section;
}*/

void Symbol::setSection(string _section) {
	section = _section;
}

void Symbol::setIsDefined(bool _isDefined) {
	isDefined = _isDefined;
}
