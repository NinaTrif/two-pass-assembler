#ifndef __H__RELOCENTRY
#define __H__RELOCENTRY

#include <cstdint>
#include <string>
using namespace std;

enum class relocType { R_386_16, R_386_PC16 };

class RelocEntry {
private:
	uint16_t offset;
	relocType type;
	uint16_t index;
public:
	RelocEntry(uint16_t _offset, relocType _type, uint16_t _index);
	string toString() const;
	void setOffset(uint16_t _offset);
	void setType(relocType _type);
	void setIndex(uint16_t _index);
	uint16_t getOffset() const;
	relocType getType() const;
	uint16_t getIndex() const;
};

#endif
