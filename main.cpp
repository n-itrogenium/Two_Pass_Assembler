#include <iostream>
#include <fstream>
#include <cstring>
#include "assembler.h"

int main(int argc, char* argv[]) {

	if (argc != 4 || strcmp(argv[1], "-o") != 0) {
		std::cerr << "ERROR! Invalid command" << std::endl;
		return 1;
	}

	std::string outputFile = argv[2];
	std::string inputFile = argv[3];

	std::ifstream infile(inputFile);
	std::ofstream outfile(outputFile);
	if (!infile.is_open() || !outfile.is_open()) {
		std::cerr << "ERROR! Cannot open file" << std::endl;
		exit(2);
	}

	Assembler::firstPass(infile, outfile);
	Assembler::secondPass(infile, outfile);

	infile.close();
	outfile.close();

	std::cout << "Happy ending!" << std::endl;

	return 0;
}