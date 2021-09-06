#ifndef _SECTION_H_
#define _SECTION_H_

#include <string>
#include <vector>
#include <map>
#include "relocation.h"

using std::vector;
using std::string;

typedef char byte; 
class Section {
public:
	string name;
	int size;
	vector<byte> bytes;
	vector<Relocation*> relocationTable;

	Section(string name);
	void addByte(byte newByte);
	static void printRelocationTable(std::ofstream& outfile, std::map<string, Section*> sections);
	static void printSections(std::ofstream& outfile, std::map<string, Section*> sections);
};

#endif

