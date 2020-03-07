#include "relocEntry.h"
#include <sstream>
#include <iomanip>
using namespace std;

RelocEntry::RelocEntry(uint16_t _offset, relocType _type, uint16_t _index) : offset(_offset), type(_type), index(_index) {}

string RelocEntry::toString() const {
	ostringstream tmp;
	ostringstream addr;
	addr << "0x" << hex << setfill('0') << setw(4) << offset;
	string typeStr = type == relocType::R_386_16 ? "R_386_16" : "R_386_PC16";
	tmp << left << setw(15) << std::setfill(' ') << addr.str() << left << setw(15) << std::setfill(' ') << typeStr << left << setw(15) << std::setfill(' ') << index;
	return tmp.str();
}

void RelocEntry::setOffset(uint16_t _offset) {
	offset = _offset;
}

void RelocEntry::setType(relocType _type) {
	type = _type;
}

void RelocEntry::setIndex(uint16_t _index) {
	index = _index;
}

uint16_t RelocEntry::getOffset() const {
	return offset;
}

relocType RelocEntry::getType() const {
	return type;
}

uint16_t RelocEntry::getIndex() const {
	return index;
}
