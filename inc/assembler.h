#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <iostream>
#include <string>
#include "symboltable.h"
#include "operations.h"

using std::string;

enum Directives {
	NOTFOUND, GLOBAL, EXTERN, SECTION, WORD, SKIP, EQU, END
};

enum Instructions {
	HALT, INT, IRET, CALL, RET, JMP, JEQ, JNE, JGT, PUSH, POP, XCHG, ADD, 
	SUB, MUL, DIV, CMP, NOT, AND, OR, XOR, TEST, SHL, SHR, LDR, STR, ILLEGAL
};

class Assembler {
public:
	static SymbolTable* symbolTable;
	static int locationCounter;
	static Section *currentSection;
	static std::map<string, Section*> sections;
	static bool end;
	static void firstPass(std::ifstream &inputFile, std::ofstream &outputFile);
	static void secondPass(std::ifstream &inputFile, std::ofstream &outputFile);
	static bool processDirective1(string word, std::istringstream &iss);
	static bool processInstruction1(string word, std::istringstream &iss);
	static bool processDirective2(string word, std::istringstream& iss);
	static bool processInstruction2(string word, std::istringstream& iss);
	static Directives getDirectiveIndex(string directive);
	static Instructions getInstructionIndex(string instruction);
	static int8_t getCode(Instructions instruction);
};

#endif

