#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include <string>
#include "symboltable.h"
using std::string;

enum AddrMode {
	IMMED, REGDIR, REGIND, REGINDPOM, MEMDIR, REGDIRPOM, UNDEFINED
};

enum Representation {
	LITERAL, SYMBOL, PCREL_SYMBOL, NONE
};

class Operand {
public:
	AddrMode addrMode;
	Representation rep;
	string value;
	byte reg;
	byte dataHigh;
	byte dataLow;
	int bytes;
};

class Operation {
public:
	static Operand* analyzeOperand(string operand, bool isJump, SymbolTable* symbolTable);
};

#endif
