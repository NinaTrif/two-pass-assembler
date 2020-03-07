#include <iostream>
#include "parser.h"
using namespace std;

int main(int argc, char **argv) {
	if (argc < 4) {
		cout << "Unsufficient number of arguments!" << endl;
		return -1;
	}
	Parser p (strtoul(argv[3], nullptr, 10));
	p.readFile(argv[1]);
	bool correct = p.firstPass();
	if (correct)
		correct = p.secondPass();
	if (correct)
		p.writeToFile(argv[2]);
	return 0;
}
