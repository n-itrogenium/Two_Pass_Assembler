#include "operations.h"
#include <regex>
#include <iostream>

Operand* Operation::analyzeOperand(string operand, bool isJump, SymbolTable* symbolTable) { 
	Operand* op = new Operand();
	Operand* result = nullptr;
	op->addrMode = UNDEFINED;
	op->rep = NONE;
	op->dataHigh = 0;
	op->dataLow = 0;
	switch (operand[0]) {
	case '$': {
		operand = operand.substr(1);
		if (isJump) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		op->value = operand;
		op->addrMode = IMMED;
		op->bytes = 2;

		// $<literal>
		if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->rep = LITERAL;
			int literal;
			// HEX
			if (operand[1] == 'x') {
				std::stringstream ss;
				ss << std::hex << operand;
				ss >> literal;
			} 
			// DEC
			else {
				literal = stoi(operand);
			}
			op->dataHigh = (literal >> 8) & 0xFF;
			op->dataLow = literal & 0xFF;
		}
		else
			// $<symbol>
			if (std::regex_match(operand, std::regex("^[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
				op->rep = SYMBOL;
				Symbol* symbol = symbolTable->find(operand);
				if (symbol) {
					if (symbol->defined) { 
						op->dataHigh = (symbol->offset >> 8) & 0xFF;
						op->dataLow = symbol->offset & 0xFF;
					}
				}
				else {
					symbolTable->insert(operand, nullptr, 0, 'L', false);
				}
			}
			else {
				std::cerr << "ERROR! Wrong operand" << std::endl;
				exit(3);
			}
		result = op;
		break;
	}
	case '[': {
		if (operand.back() != ']' || isJump) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		operand = operand.substr(1);
		operand.pop_back();
		std::remove_if(operand.begin(), operand.end(), isspace);
		if (!std::regex_match(operand, std::regex("^[rR][0-7]+(\\++([0-9]+$|0x[0-9A-Fa-f]+$|[a-zA-Z_]+[a-zA-Z0-9_]*$))?"))) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}

		op->value = operand;

		// [<reg> + <literal>]
		if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++([0-9]+$|0x[0-9A-Fa-f]+$)"))) {
			op->rep = LITERAL;
			op->addrMode = REGINDPOM;
			op->value = operand.substr(3);
			int literal;
			// HEX
			if (op->value[1] == 'x') {
				std::stringstream ss;
				ss << std::hex << op->value;
				ss >> literal;
			} 
			// DEC
			else {
				literal = stoi(op->value);
			}
			op->dataHigh = (literal >> 8) & 0xFF;
			op->dataLow = literal & 0xFF;
			op->bytes = 2;
		}
		else
			// [<reg> + <symbol>]
			if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
				op->rep = SYMBOL;
				op->addrMode = REGINDPOM;
				op->bytes = 2;
				op->value = operand.substr(3);
				Symbol* symbol = symbolTable->find(op->value);
				if (symbol) {
					if (symbol->defined) {
						op->dataHigh = (symbol->offset >> 8) & 0xFF;
						op->dataLow = symbol->offset & 0xFF;
					}
				}
				else {
					symbolTable->insert(op->value, nullptr, 0, 'L', false);
				}
			}
		// [<reg>]
			else
				op->addrMode = REGIND;
		op->reg = operand[1] - '0';
		result = op;
		break;
	}
	case '*': {
		if (!isJump) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		operand = operand.substr(1);
		op->value = operand;
		// *<reg>
		if (std::regex_match(operand, std::regex("^[rR][0-7]$"))) {
			op->addrMode = REGDIR;
			op->reg = operand[1] - '0';
		}
		// *<literal>
		else if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->addrMode = MEMDIR;
			op->rep = LITERAL;
			int literal;
			// HEX
			if (operand[1] == 'x') {
				std::stringstream ss;
				ss << std::hex << operand;
				ss >> literal;
			} 
			// DEC
			else {
				literal = stoi(operand);
			}
			op->dataHigh = (literal >> 8) & 0xFF;
			op->dataLow = literal & 0xFF;
			op->bytes = 2;
		}
		// *<symbol>
		else if (std::regex_match(operand, std::regex("^[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
			op->addrMode = MEMDIR;
			op->rep = SYMBOL;
			op->bytes = 2;
			Symbol* symbol = symbolTable->find(operand);
			if (symbol) {
				if (symbol->defined) {
					op->dataHigh = (symbol->offset >> 8) & 0xFF;
					op->dataLow = symbol->offset & 0xFF;
				}
			}
			else {
				symbolTable->insert(operand, nullptr, 0, 'L', false);
			}
		}
		else if (operand[0] == '[') {
			if (operand.back() != ']') {
				std::cerr << "ERROR! Wrong operand" << std::endl;
				exit(3);
			}
			operand = operand.substr(1);
			operand.pop_back();
			std::remove_if(operand.begin(), operand.end(), isspace);
			if (!std::regex_match(operand, std::regex("^[rR][0-7]+(\\++([0-9]+$|0x[0-9A-Fa-f]+$|[a-zA-Z_]+[a-zA-Z0-9_]*$))?"))) {
				std::cerr << "ERROR! Wrong operand" << std::endl;
				exit(3);
			}
			op->value = operand;

			// *[<reg> + <literal>]
			if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++([0-9]+$|0x[0-9A-Fa-f]+$)"))) {
				op->rep = LITERAL;
				op->addrMode = REGINDPOM;
				op->value = operand.substr(3);
				int literal;
				// HEX
				if (op->value[1] == 'x') {
					std::stringstream ss;
					ss << std::hex << op->value;
					ss >> literal;
				} 
				// DEC
				else {
					literal = stoi(op->value);
				}
				op->dataHigh = (literal >> 8) & 0xFF;
				op->dataLow = literal & 0xFF;
				op->bytes = 2;
			}
			else
				// *[<reg> + <symbol>]
				if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
					op->rep = SYMBOL;
					op->addrMode = REGINDPOM;
					op->bytes = 2;
					op->value = operand.substr(3);
					Symbol* symbol = symbolTable->find(op->value);
					if (symbol) {
						if (symbol->defined) {
							op->dataHigh = (symbol->offset >> 8) & 0xFF;
							op->dataLow = symbol->offset & 0xFF;
						}
					}
					else {
						symbolTable->insert(op->value, nullptr, 0, 'L', false);
					}
				}
			// *[<reg>]
				else
					op->addrMode = REGIND;
			op->reg = operand[1] - '0';
		}
		else {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}

		result = op;
		break;
	}
	case '%': {
		operand = operand.substr(1);
		if (!std::regex_match(operand, std::regex("^[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		op->rep = PCREL_SYMBOL;
		op->addrMode = isJump ? IMMED : MEMDIR;
		op->dataHigh = (-2 >> 8) & 0xFF;
		op->dataLow = -2 & 0xFF;
		op->bytes = 2;
		op->value = operand;
		Symbol* symbol = symbolTable->find(operand);
		if (symbol) {
			if (symbol->defined) {
				op->dataHigh = ((symbol->offset - 2) >> 8) & 0xFF;
				op->dataLow = (symbol->offset - 2) & 0xFF;
			}
		}
		else {
			symbolTable->insert(op->value, nullptr, 0, 'L', false);
		}
		result = op;
		break;
	}
	default: {
		// <reg>
		if (std::regex_match(operand, std::regex("^[rR][0-7]$")) && !isJump) {
			op->addrMode = REGDIR;
			op->reg = operand[1] - '0';
		}
		// <literal>
		else if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->rep = LITERAL;
			op->addrMode = isJump ? IMMED : MEMDIR;
			int literal;
			// HEX
			if (operand[1] == 'x') {
				std::stringstream ss;
				ss << std::hex << operand;
				ss >> literal;
			} 
			// DEC
			else {
				literal = stoi(operand);
			}
			op->dataHigh = (literal >> 8) & 0xFF;
			op->dataLow = literal & 0xFF;
			op->bytes = 2;
		}
		// <symbol>
		else if (std::regex_match(operand, std::regex("^[a-zA-Z_]+[a-zA-Z0-9_]*$"))) {
			op->rep = SYMBOL;
			op->addrMode = isJump ? IMMED : MEMDIR;
			op->bytes = 2;
			Symbol* symbol = symbolTable->find(operand);
			if (symbol) {
				if (symbol->defined) {
					op->dataHigh= (symbol->offset >> 8) & 0xFF;
					op->dataLow = symbol->offset & 0xFF;
				}
			}
			else {
				symbolTable->insert(operand, nullptr, 0, 'L', false);
			}
		}
		else {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		op->value = operand;

		result = op;
		break;
	}
	}

	return result;
}


