#include "symboltable.h"
#include <iomanip>
#include <fstream>
#include <iostream>

using std::setw;
using std::setfill;

int Symbol::staticID = 0;

Symbol* SymbolTable::find(string name) {
   std::map<string, Symbol*>::iterator i = table.find(name);
   if (i == table.end()) {
	   i = sectionTable.find(name);
	   if (i == sectionTable.end())
	   		return nullptr;
   }
   return i->second;
}

void SymbolTable::insert(string name, Section* section, int locationCounter, char scope, bool isSection) {
	if (isSection)
		sectionTable[name] = new Symbol(name, section, locationCounter, scope);
	else 
		table[name] = new Symbol(name, section, locationCounter, scope);
}

bool SymbolTable::isDefined() {
	std::map<string, Symbol*>::iterator i;
	for (i = table.begin(); i != table.end(); i++)
		if (!i->second->defined)
			return false;
	return true;
}

void SymbolTable::printTable(std::ofstream &outfile) {
	outfile << "=======================SYMBOL TABLE=========================" << std::endl;
	outfile << setw(15) << setfill(' ') << "Label";
	outfile << setw(15) << setfill(' ') << "Section";
	outfile << setw(10) << setfill(' ') << "Offset";
	outfile << setw(10) << setfill(' ') << "Scope";
	outfile << setw(10) << setfill(' ') << "Ordinal";
	outfile << std::endl << "------------------------------------------------------------" << std::endl;

	std::map<string, Symbol*>::iterator i;
	int ordinal = 1;
	for (i = sectionTable.begin(); i != sectionTable.end(); i++) {
		char scope = (i->second->scope == 'E') ? 'G' : i->second->scope;
		outfile << setw(15) << setfill(' ') << i->second->name;
		outfile << setw(15) << setfill(' ') << i->second->section;
		outfile << setw(10) << setfill(' ') << i->second->offset;
		outfile << setw(10) << setfill(' ') << scope;
		i->second->ordinal = ordinal++;
		outfile << setw(10) << setfill(' ') << i->second->ordinal;
		outfile << std::endl;
	}
	for (i = table.begin(); i != table.end(); i++) {
		char scope = (i->second->scope == 'E') ? 'G' : i->second->scope;
		outfile << setw(15) << setfill(' ') << i->second->name;
		outfile << setw(15) << setfill(' ') << i->second->section;
		outfile << setw(10) << setfill(' ') << i->second->offset;
		outfile << setw(10) << setfill(' ') << scope;
		i->second->ordinal = ordinal++;
		outfile << setw(10) << setfill(' ') << i->second->ordinal;
		outfile << std::endl;
	}
	outfile << std::endl << std::endl << std::endl;
}

Symbol::Symbol(string name, Section* section, int offset, char scope) {
	this->name = name;
	this->offset = offset;
	this->scope = scope;
	if (section)
		this->section = section->name;
	else
		this->section = "?";
	this->defined = false;
	this->absolute = false;
}
