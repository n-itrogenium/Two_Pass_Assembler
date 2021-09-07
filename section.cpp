#include "section.h"
#include <fstream>
#include <iomanip>
#include <iostream>

using std::setw;
using std::setfill;

Section::Section(std::string name) {
	this->name = name;
	bytes = std::vector<int8_t>();
}

void Section::addByte(int8_t newByte) {
	bytes.push_back(newByte);
}

void Section::printRelocationTable(std::ofstream& outfile, std::map<string, Section*> sections) {
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;

		if (!section->relocationTable.empty()) {
			outfile << "#.rel " << section->name << std::endl << "#";
			outfile << setw(9) << setfill(' ') << "Offset";
			outfile << setw(15) << setfill(' ') << "Rel. tip";
			outfile << setw(10) << setfill(' ') << "R. Br." << std::endl; // vrednost?
			
			for (int j = 0; j < section->relocationTable.size(); j++) {
				string relType = (section->relocationTable[j]->type == ABS) ? "R_X86_64_32" : "R_X86_64_PC32";
				outfile << setw(10) << setfill(' ') << section->relocationTable[j]->offset;
				outfile << setw(15) << setfill(' ') << relType;
				outfile << setw(10) << setfill(' ') << section->relocationTable[j]->value << std::endl;
			}

			outfile << std::endl;
		}
	}
}

void Section::printSections(std::ofstream& outfile, std::map<string, Section*> sections) {
	std::map<string, Section*>::iterator i;
	for (i = sections.begin(); i != sections.end(); i++) {
		Section* section = i->second;

		if (!section->bytes.empty()) {
			outfile << "#" << section->name << "	" << section->size << std::endl; //<< std::hex;
			std::stringstream stream;
			for (int j = 0; j < section->size; j++) {
				stream << setw(2) << setfill('0') << std::hex << section->bytes[j];
				if (j%16 == 1)
					stream << ' ';
				else
					stream << std::endl;
			}
			std::string stringBytes(stream.str());
			outfile << stringBytes << std::dec << std::endl << std::endl;
		}
	}
}


