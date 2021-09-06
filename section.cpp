#include "section.h"

Section::Section(std::string name) {
	this->name = name;
	bytes = std::vector<byte>();
}

void Section::addByte(byte newByte) {
	bytes.push_back(newByte);
}

void Section::printRelocationTable(std::ofstream& outfile, std::map<string, Section*> sections)
{
}

void Section::printSections(std::ofstream& outfile, std::map<string, Section*> sections)
{
}


