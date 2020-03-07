#ifndef __H__TABLE
#define __H__TABLE

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

template <class T>
class Table {
private:
	vector<T> table;
public:
	string toString() const;
	void pushBack(T elem);
	bool has(T elem);
	T* getElemByName(string name);
	int getSize() const;
	vector<T>& getTable();
};

template<class T>
vector<T>& Table<T>::getTable() {
	return table;
}

template<class T>
int Table<T>::getSize() const {
	return table.size();
}

template <class T>
string Table<T>::toString() const {
	ostringstream tmp;
	for (auto& e : table) {
		tmp << e.toString() << endl;
	}
	return tmp.str();	
}

template <class T>
void Table<T>::pushBack(T elem) {
	table.push_back(elem);
}

template <class T>
bool Table<T>::has(T elem) {
	return (find(table.begin(), table.end(), elem) != table.end());
}

template <class T>
T* Table<T>::getElemByName(string name) {
	for (auto& elem : table)
		if (name == elem.getName())
			return &elem;
	return nullptr;
}

#endif
