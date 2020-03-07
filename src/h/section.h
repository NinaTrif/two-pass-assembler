#ifndef __H__SECTION
#define __H__SECTION

#include <string>
#include <vector>
#include <cstdint>
using namespace std;

class Section {
private:
	string name;
	string flags = "";
	uint16_t startAddress;
	uint16_t size = 0;
	vector<uint8_t> data;
public:
	~Section();
	Section(string _name, uint16_t _startAddress);
	string toString() const;
	void setName(const string _name);
	void setFlags(const string _flags);
	void setStartAddress(uint16_t _startAddress);
	void setSize(uint16_t _size);
	void appendData(uint32_t _data, int bytes);
	string getName();
	string getFlags() const;
	uint16_t getStartAddress() const;
	uint16_t getSize();
	vector<uint8_t> getData() const;
};

#endif
