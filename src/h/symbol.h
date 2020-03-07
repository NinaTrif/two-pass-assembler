#ifndef __H__SYMBOL
#define __H__SYMBOL

#include <cstdint>
#include <string>
#include "section.h"
using namespace std;

class Symbol {
private:
	bool isGlobal;
	bool isSection; //section is local
	uint16_t offset;
	string name;
	//Section *section; //0 - external, -1 - absolute
	string section;
	static uint16_t ID;
	uint16_t id = ID++;
	bool isDefined;
public:
	~Symbol();
	//Symbol(bool _isGlobal, bool _isSection, uint16_t _offset, string _name, Section* _section, bool _isDefined);
	Symbol(bool _isGlobal, bool _isSection, uint16_t _offset, string _name, string _section, bool _isDefined);
	string toString() const;
	bool getIsGlobal() const;
	bool getIsSection() const;
	uint16_t getOffset() const;
	string getName();
	//Section* getSection() const;
	string getSection() const;
	uint16_t getId() const;
	bool getIsDefined() const;
	void setIsGlobal(bool _isGlobal);
	void setIsSection(bool _isSection);
	void setOffset(uint16_t _offset);
	void setName(const string _name);
	//void setSection(Section *_section);
	void setSection(string _section);
	void setIsDefined(bool _isDefined);
};

#endif
