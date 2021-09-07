#include "operations.h"
#include <regex>
#include <iostream>

Operand* Operation::analyzeOperand(string operand, bool isJump, SymbolTable* symbolTable) { // NE ZABORAVITI % PCREL
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

		// $<literal>
		if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->rep = LITERAL;
			op->dataHigh = (stoi(operand) & 0xFF00) >> 8;
			op->dataLow = stoi(operand) & 0xFF;
			op->bytes = (op->dataHigh != 0) ? 2 : 1;
		}
		else
			// $<symbol>
			if (std::regex_match(operand, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
				op->rep = SYMBOL;
				op->bytes = 1;
				Symbol* symbol = symbolTable->find(operand);
				if (symbol) {
					if (symbol->defined) { // sta ako je eksterni?
						op->dataHigh = (symbol->offset & 0xFF00) >> 8;
						op->dataLow = symbol->offset & 0xFF;
						op->bytes = (op->dataHigh != 0) ? 2 : 1;
					}
				}
				else {
					symbolTable->insert(operand, nullptr, 0, 'L');
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
		if (!std::regex_match(operand, std::regex("^[rR][0-7]+(\\++([0-9]+$|0x[0-9A-Fa-f]+$|[a-zA-Z]+[a-zA-Z0-9]*$))?"))) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}

		op->value = operand;
		op->bytes = 1;

		// [<reg> + <literal>]
		if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++([0-9]+$|0x[0-9A-Fa-f]+$)"))) {
			op->rep = LITERAL;
			op->addrMode = REGINDPOM;
			op->value = operand.substr(3);
			op->dataHigh = (stoi(op->value) & 0xFF00) >> 8;
			op->dataLow = stoi(op->value) & 0xFF;
			op->bytes += (op->dataHigh != 0) ? 2 : 1;
		}
		else
			// [<reg> + <symbol>]
			if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++[a-zA-Z]+[a-zA-Z0-9]*$"))) {
				op->rep = SYMBOL;
				op->addrMode = REGINDPOM;
				op->bytes += 1;
				op->value = operand.substr(3);
				Symbol* symbol = symbolTable->find(op->value);
				if (symbol) {
					if (symbol->defined) {
						op->dataHigh = (symbol->offset & 0xFF00) >> 8;
						op->dataLow = symbol->offset & 0xFF;
						op->bytes += (op->dataHigh != 0) ? 1 : 0;
					}
				}
				else {
					symbolTable->insert(op->value, nullptr, 0, 'L');
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
			op->bytes = 1;
			op->reg = operand[1] - '0';
		}
		// *<literal>
		else if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->addrMode = MEMDIR;
			op->rep = LITERAL;
			op->dataHigh = (stoi(operand) & 0xFF00) >> 8;
			op->dataLow = stoi(operand) & 0xFF;
			op->bytes = (op->dataHigh != 0) ? 2 : 1;
		}
		// *<symbol>
		else if (std::regex_match(operand, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
			op->addrMode = MEMDIR;
			op->rep = SYMBOL;
			op->bytes = 1;
			Symbol* symbol = symbolTable->find(operand);
			if (symbol) {
				if (symbol->defined) {
					op->dataHigh = (symbol->offset & 0xFF00) >> 8;
					op->dataLow = symbol->offset & 0xFF;
					op->bytes = (op->dataHigh != 0) ? 2 : 1;
				}
			}
			else {
				symbolTable->insert(operand, nullptr, 0, 'L');
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
			if (!std::regex_match(operand, std::regex("^[rR][0-7]+(\\++([0-9]+$|0x[0-9A-Fa-f]+$|[a-zA-Z]+[a-zA-Z0-9]*$))?"))) {
				std::cerr << "ERROR! Wrong operand" << std::endl;
				exit(3);
			}
			op->value = operand;
			op->bytes = 1;

			// *[<reg> + <literal>]
			if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++([0-9]+$|0x[0-9A-Fa-f]+$)"))) {
				op->rep = LITERAL;
				op->addrMode = REGINDPOM;
				op->value = operand.substr(3);
				op->dataHigh = (stoi(op->value) & 0xFF00) >> 8;
				op->dataLow = stoi(op->value) & 0xFF;
				op->bytes += (op->dataHigh != 0) ? 2 : 1;
			}
			else
				// *[<reg> + <symbol>]
				if (std::regex_match(operand, std::regex("^[rR][0-7]+\\++[a-zA-Z]+[a-zA-Z0-9]*$"))) {
					op->rep = SYMBOL;
					op->addrMode = REGINDPOM;
					op->bytes += 1;
					op->value = operand.substr(3);
					Symbol* symbol = symbolTable->find(op->value);
					if (symbol) {
						if (symbol->defined) {
							op->dataHigh = (symbol->offset & 0xFF00) >> 8;
							op->dataLow = symbol->offset & 0xFF;
							op->bytes += (op->dataHigh != 0) ? 1 : 0;
						}
					}
					else {
						symbolTable->insert(op->value, nullptr, 0, 'L');
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
		if (!std::regex_match(operand, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
			std::cerr << "ERROR! Wrong operand" << std::endl;
			exit(3);
		}
		op->rep = PCREL_SYMBOL;
		op->addrMode = isJump ? IMMED : MEMDIR;
		op->dataHigh = (-2 & 0xFF00) >> 8;
		op->dataLow = -2 & 0xFF;
		op->bytes = 3;
		op->value = operand;
		result = op;
		// PROVERITI DA LI NEKAD TREBA -1 ILI NESTO STO NIJE -2
		break;
	}
	default: {
		op->bytes = 1;

		// <reg>
		if (std::regex_match(operand, std::regex("^[rR][0-7]$")) && !isJump) {
			op->addrMode = REGDIR;
			op->reg = operand[1] - '0';
		}
		// <literal>
		else if (std::regex_match(operand, std::regex("^([0-9]+|0x[0-9A-Fa-f]+)$"))) {
			op->rep = LITERAL;
			op->addrMode = isJump ? IMMED : MEMDIR;
			op->dataHigh = (stoi(operand) & 0xFF00) >> 8;
			op->dataLow = stoi(operand) & 0xFF;
			op->bytes += (op->dataHigh != 0) ? 2 : 1;
		}
		// <symbol>
		else if (std::regex_match(operand, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
			op->rep = SYMBOL;
			op->addrMode = isJump ? IMMED : MEMDIR;
			op->bytes += 1;
			Symbol* symbol = symbolTable->find(operand);
			if (symbol) {
				if (symbol->defined)
					op->bytes += ((symbol->offset & 0xFF00) != 0) ? 1 : 0;
			}
			else {
				symbolTable->insert(operand, nullptr, 0, 'L');
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


