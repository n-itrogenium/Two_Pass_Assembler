#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <string>
#include <map>
#include "section.h"
using std::string;

class Symbol {
public:
	static int staticID;
	int ordinal = ++staticID;
	string name;
	string section;
	int offset;
	char scope;
	bool defined;
	bool absolute;
	Symbol(string name, Section* section, int offset, char scope);
};

class SymbolTable {
public:
	std::map<string, Symbol*> sectionTable;
	std::map<string, Symbol*> table;
	Symbol* find(string name);
	void insert(string name, Section* section, int locationCounter, char scope, bool isSection);
	bool isDefined();
	void printTable(std::ofstream &outfile, std::map<string, Section*> sections);
};

#endif

