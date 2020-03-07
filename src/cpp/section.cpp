#include "section.h"
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

Section::~Section() {}

Section::Section(string _name, uint16_t _startAddress) : name(_name), startAddress(_startAddress) {}

string Section::toString() const {
	ostringstream tmp;
	tmp << "#" << name << endl;
	for (size_t i = 0; i < data.size(); i++) {
		if ((i != 0) && (i % 16 == 0) && i < data.size() - 1)
			tmp << endl;
		tmp << hex << setfill('0') << setw(2) << static_cast<int>(data.at(i)) << " ";
	}
	return tmp.str();
}

void Section::setName(string _name) {
	name = _name;
}

void Section::setFlags(string _flags) {
	flags = _flags;
}

void Section::setStartAddress(uint16_t _startAddress) {
	startAddress = _startAddress;
}

void Section::setSize(uint16_t _size) {
	size = _size;
}

void Section::appendData(uint32_t _data, int bytes) {
	int shiftBy = 0;
	for (int i = 0; i < bytes; i++) {
		uint8_t byte = _data >> shiftBy;
		data.push_back(byte);
		shiftBy += 8;
	}
}

string Section::getName() {
	return name;
}

string Section::getFlags() const {
	return flags;
}

uint16_t Section::getStartAddress() const {
	return startAddress;
}

uint16_t Section::getSize() {
	return size;
}

vector<uint8_t> Section::getData() const {
	return data;
}
