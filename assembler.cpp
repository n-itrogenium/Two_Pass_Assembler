#include "assembler.h"
#include "section.h"
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
#include <iomanip>

/* ============================= FIRST PASS ============================== */

void Assembler::firstPass(std::ifstream &inputFile, std::ofstream &outputFile) {
	symbolTable = new SymbolTable();
	locationCounter = 0;
	currentSection = nullptr;
	absSymbols = new Section("abs_symbols");
	end = false;

	string line;

	while (std::getline(inputFile, line) && !end) {

		if (line.empty())
			continue;

		std::istringstream iss(line);
		string word;

		if (iss >> word) {

			if (word.front() == ':') {
				std::cerr << "ERROR! Invalid first character ':'" << std::endl;
				exit(3);
			}

			// LABEL
			if (word.back() == ':') {

				if (word.front() == '.') {
					std::cerr << "ERROR! Invalid first character '.'" << std::endl;
					exit(3);
				}

				if (!currentSection) {
					std::cerr << "ERROR! Label out of section" << std::endl;
					exit(3);
				}

				word.pop_back();

				Symbol* symbol = symbolTable->find(word);
				if (!symbol)
					symbolTable->insert(word, currentSection, locationCounter, 'L');
				/*else {
					Proverava da li je globalni simbol definisan ili ne,
					ali pošto je labela, verovatno nema potrebe za ovim
				}*/

				if (!(iss >> word)) // Provera da li je labela prazna
					word = "";
			}

			// DIRECTIVES AND INSTRUCTIONS
			if (!word.empty()) {
				bool isDirective = processDirective1(word, iss);
				if (!isDirective)
					if (!processInstruction1(word, iss)) {
						std::cerr << "ERROR! Directive or instruction does not exist" << std::endl;
						exit(3);
					}
			}
		}
	}

	if (!symbolTable->isDefined()) {
		std::cerr << "ERROR! Symbols not defined" << std::endl;
		exit(3);
	}

	symbolTable->printTable(outputFile);

}


bool Assembler::processDirective1(string word, std::istringstream& iss) {
	Directives directive = getDirectiveIndex(word);

	switch (directive) {

	case GLOBAL: 
	case EXTERN: {
		if (iss.str().back() == ',') {
			std::cerr << "ERROR! Expected an identifier" << std::endl;
			exit(3);
		}

		bool identifierExists = false;

		while (iss >> word) {
			if (word.back() == ',')
				word.pop_back(); // Oduzimamo ','

			if (word.empty() || word == "") {
				std::cerr << "ERROR! Expected an identifier" << std::endl;
				exit(3);
			}

			Symbol* symbol = symbolTable->find(word);
			int offset = (directive == GLOBAL) ? -1 : 0;
			char scope = (directive == GLOBAL) ? 'G' : 'E';

			if (!symbol) {
				symbolTable->insert(word, currentSection, offset, scope);
				symbolTable->find(word)->defined = true;
			}
			else {
				if (symbol->scope == 'E' && directive == GLOBAL || symbol->scope == 'G' && directive == EXTERN) {
					std::cerr << "ERROR! Symbol cannot be both global and extern" << std::endl;
					exit(3);
				}
				symbol->scope = scope;
				symbol->offset = offset;
				if (!currentSection)
					symbol->section = "?";
				symbol->defined = true;
			}
			identifierExists = true;
		}

		if (!identifierExists) { // Ako ne postoji nijedan identifikator nakon .global
			std::cerr << "ERROR! Expected at least one identifier" << std::endl;
			exit(3);
		}

		return true;
	}

	case SECTION: { // proveriti za .text, .data itd.
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a section name" << std::endl;
			exit(3);
		}
		if (word.front() == '.') {
			std::cerr << "ERROR! Invalid first character '.'" << std::endl;
			exit(3);
		}
		if (symbolTable->find(word)) {
			std::cerr << "ERROR! Name not available" << std::endl;
			exit(3);
		}
		string sectionName = word;
		if (iss >> word) {
			std::cerr << "ERROR! Unexpected token" << std::endl;
			exit(3);
		}

		Section* newSection = new Section(sectionName);
		sections[sectionName] = newSection;
		symbolTable->insert(sectionName, newSection, 0, 'L');
		symbolTable->find(sectionName)->defined = true;
		if (currentSection) {
			currentSection->size = locationCounter;
			locationCounter = 0;
		}
		currentSection = newSection;

		return true; 
	}

	case WORD: {
		if (!currentSection) {
			std::cerr << "ERROR! Directive '.word' out of section" << std::endl;
			exit(3);
		}
		if (iss.str().back() == ',') {
			std::cerr << "ERROR! Expected a token" << std::endl;
			exit(3);
		}

		bool identifierExists = false;

		while (iss >> word) {
			if (word.back() == ',')
				word.pop_back(); // Oduzimamo ','

			if (word.empty() || word == "") {
				std::cerr << "ERROR! Expected an identifier" << std::endl;
				exit(3);
			}

			if (std::regex_match(word, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
				Symbol* symbol = symbolTable->find(word);
				if (!symbol) {
					symbolTable->insert(word, currentSection, 0, 'L');
				}
			}
			else if (!std::regex_match(word, std::regex("^[0-9]+$"))) {
				std::cerr << "ERROR! Expected a symbol or literal" << std::endl;
				exit(3);
			} 
			// proveriti da nije slucajno neki registar naveden
			locationCounter += 2;
			identifierExists = true;
		}
		if (!identifierExists) {
			std::cerr << "ERROR! Expected at least one identifier" << std::endl;
			exit(3);
		}

		return true;
	}

	case SKIP: {
		if (!currentSection) {
			std::cerr << "ERROR! Directive '.skip' out of section" << std::endl;
			exit(3);
		}
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a token" << std::endl;
			exit(3);
		}
		if (std::regex_match(word, std::regex("^[0-9]+$"))) {
			locationCounter += stoi(word);
		}
		else {
			std::cerr << "ERROR! Expected a literal" << std::endl;
			exit(3);
		}

		if (iss >> word) {
			std::cerr << "ERROR! Unexpected token" << std::endl;
			exit(3);
		}
		return true;
	}

	case EQU: {
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a token" << std::endl;
			exit(3);
		}
		if (word.back() != ',') {
			std::cerr << "ERROR! Expected a ','" << std::endl;
			exit(3);
		}
		word.pop_back();
		if (std::regex_match(word, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) {
			Symbol* symbol = symbolTable->find(word);
			if (!symbol) {
				symbolTable->insert(word, absSymbols, 0, 'L'); // PROVERITI OFFSET
				symbol = symbolTable->find(word);
				symbol->defined = true;
			}
			else if (symbol->scope == 'E') {
				std::cerr << "ERROR! Symbol already defined externally" << std::endl;
				exit(3);
			}
			if (!(iss >> word)) {
				std::cerr << "ERROR! Expected a token" << std::endl;
				exit(3);
			}
			if (std::regex_match(word, std::regex("^[0-9]+$"))) {
				symbol->offset = stoi(word); // videti da li ce biti offset ili dodatno polje value
			}
			else {
				std::cerr << "ERROR! Expected a literal" << std::endl;
				exit(3);
			}
		}
		else {
			std::cerr << "ERROR! Expected a symbol" << std::endl;
			exit(3);
		}
		return true;
	}

	case END: {
		if (currentSection) {
			currentSection->size = locationCounter;
			locationCounter = 0;
		}
		end = true;
		return true;
	}

	case NOTFOUND: return false;
	}
}

bool Assembler::processInstruction1(string word, std::istringstream& iss) {
	if (!currentSection) {
		std::cerr << "ERROR! Instruction out of section" << std::endl;
		exit(3);
	}

	switch (getInstructionIndex(word)) {

	case HALT: case IRET: case RET: {
		if (iss >> word) {
			std::cerr << "ERROR! Unexpected token" << std::endl;
			exit(3);
		}
		locationCounter++;
		return true; 
	}

	case INT: case PUSH: case POP: case NOT: {
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		Operand* op = Operation::analyzeOperand(word, false, symbolTable);
		if (!op || op->addrMode != REGDIR) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		locationCounter += 2;
		return true; 
	}

	case CALL: case JMP: case JEQ: case JNE: case JGT: {
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected an operand" << std::endl;
			exit(3);
		}
		Operand* op = Operation::analyzeOperand(word, true, symbolTable);
		if (!op) {
			std::cerr << "ERROR! Expected an operand" << std::endl;
			exit(3);
		}
		locationCounter += 2 + op->bytes;
		return true; 
	}

	case XCHG:
	case ADD: case SUB: case MUL: case DIV: case CMP: 
	case AND: case OR: case XOR: case TEST: 
	case SHL: case SHR: {
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		if (word.back() != ',') {
			std::cerr << "ERROR! Expected a ','" << std::endl;
			exit(3);
		}
		word.pop_back();
		Operand* op = Operation::analyzeOperand(word, false, symbolTable);
		if (!op || op->addrMode != REGDIR) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		op = Operation::analyzeOperand(word, false, symbolTable);
		if (!op || op->addrMode != REGDIR) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		locationCounter += 2;
		return true; 
	}
	case LDR: case STR: {
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		if (word.back() != ',') {
			std::cerr << "ERROR! Expected a ','" << std::endl;
			exit(3);
		}
		word.pop_back();
		Operand* op = Operation::analyzeOperand(word, false, symbolTable);
		if (!op || op->addrMode != REGDIR) {
			std::cerr << "ERROR! Expected a register" << std::endl;
			exit(3);
		}
		if (!(iss >> word)) {
			std::cerr << "ERROR! Expected an operand" << std::endl;
			exit(3);
		}
		op = Operation::analyzeOperand(word, false, symbolTable);
		if (!op) {
			std::cerr << "ERROR! Expected an operand" << std::endl;
			exit(3);
		}
		locationCounter += 2 + op->bytes;
		return true;
	}
	case ILLEGAL: return false;
	}
}


/* ============================= SECOND PASS ============================== */


void Assembler::secondPass(std::ifstream& inputFile, std::ofstream& outputFile) {
	inputFile.seekg(0);
	end = false;
	string line;

	while (std::getline(inputFile, line) && !end) {
		if (line.empty())
			continue;

		std::istringstream iss(line);
		string word;

		if (iss >> word) {
			// LABEL
			if (word.back() == ':') {
				if (!(iss >> word))
					continue;
			}

			// DIRECTIVES AND INSTRUCTIONS
			bool isDirective = processDirective2(word, iss);
			if (!isDirective)
				if (!processInstruction2(word, iss)) {
					std::cerr << "ERROR! Directive or instruction failed" << std::endl;
					exit(3);
				}
		}
	}
	Section::printRelocationTable(outputFile, sections);
	Section::printSections(outputFile, sections);
}

bool Assembler::processDirective2(string word, std::istringstream& iss) {
	Directives directive = getDirectiveIndex(word);

	switch (directive) {

	case GLOBAL:
	case EXTERN: 
	case EQU: return true;

	case SECTION: { 
		iss >> word;
		/*if (currentSection) {
			currentSection->size = locationCounter;
			locationCounter = 0;
		}*/
		currentSection = sections[word];
		return true;
	}

	case WORD: {
		while (iss >> word) {
			if (word.back() == ',')
				word.pop_back(); // Oduzimamo ','
			if (std::regex_match(word, std::regex("^[a-zA-Z]+[a-zA-Z0-9]*$"))) { // symbol
				Symbol* symbol = symbolTable->find(word);
				currentSection->addByte(symbol->offset & 0xFF); // da li se za glob simb dodaje 0 ili vrednost?
				currentSection->addByte((symbol->offset >> 8) & 0xFF);

				Relocation *rel = new Relocation();
				rel->offset = currentSection->bytes.size();
				rel->type = ABS;
				rel->value = symbol->ordinal;
				currentSection->relocationTable.push_back(rel);
			}
			else { // literal
				int literal = stoi(word);
				currentSection->addByte(literal & 0xFF);
				currentSection->addByte((literal >> 8) & 0xFF);
			}
		}
		return true;
	}

	case SKIP: {
		iss >> word;
		int numOfBytes = stoi(word);
		for (int i = 0; i < numOfBytes; i++)
			currentSection->addByte(0);
		locationCounter += numOfBytes;
		return true;
	}

	case END: {
		end = true;
		return true;
	}
	case NOTFOUND: return false;
	}
}

bool Assembler::processInstruction2(string word, std::istringstream& iss) {
	byte InstrDescr, RegsDescr, AddrMode, DataHigh, DataLow;
	Instructions instruction = getInstructionIndex(word);
	InstrDescr = getCode(instruction);

	switch (instruction) {

	case HALT: {
		currentSection->addByte(InstrDescr);
		return true;
	}
	case INT: {
		iss >> word;
		RegsDescr = ((word[1] - '0') << 4) | 0x0F;
		currentSection->addByte(InstrDescr);
		currentSection->addByte(RegsDescr);
		return true;
	}
	case IRET: {
		currentSection->addByte(InstrDescr);
		return true;
	}
	case RET: {
		currentSection->addByte(InstrDescr);
		return true;
	}
	case CALL: case JMP: case JEQ: case JNE: case JGT: {
		iss >> word;
		bool isJump = (instruction == CALL) ? false : true;
		Operand* op = Operation::analyzeOperand(word, isJump, symbolTable);
		RegsDescr = 0xF0 | op->reg;
		AddrMode = op->addrMode & 0x0F; // 0 => UUUU
		DataHigh = op->dataHigh;
		DataLow = op->dataLow;
		currentSection->addByte(InstrDescr);
		currentSection->addByte(RegsDescr);
		currentSection->addByte(AddrMode);
		if (DataHigh != 0) {
			currentSection->addByte(DataHigh);
			currentSection->addByte(DataLow);
		}
		else if (DataLow != 0)
			currentSection->addByte(DataLow);

		if (op->rep == SYMBOL || op->rep == PCREL_SYMBOL) {
			Symbol* symbol = symbolTable->find(word); // SACUVATI NEGDE SIMBOL i to koristiti umesto word
			Relocation* rel = new Relocation();
			rel->offset = currentSection->bytes.size();
			rel->type = (op->rep == PCREL_SYMBOL) ? PC_REL : ABS; // OBRADITI %
			rel->value = symbol->ordinal;
			currentSection->relocationTable.push_back(rel);
		}

		return true;
	}

	case XCHG: case ADD: case SUB: case MUL: case DIV: case CMP: {
		iss >> word;
		word.pop_back();
		RegsDescr = (word[1] - '0') << 4;
		iss >> word;
		RegsDescr |= word[1] - '0';
		currentSection->addByte(InstrDescr);
		currentSection->addByte(RegsDescr);
		return true;
	}
	case NOT: case AND: case OR: case XOR: case TEST: case SHL: case SHR: {
		iss >> word;
		word.pop_back();
		RegsDescr = (word[1] - '0') << 4;
		iss >> word;
		RegsDescr |= word[1] - '0';
		currentSection->addByte(InstrDescr);
		currentSection->addByte(RegsDescr);
		return true;
	}
	case LDR: case STR: {
		iss >> word;
		word.pop_back();
		RegsDescr = (word[1] - '0') << 4;

		iss >> word;
		Operand* op = Operation::analyzeOperand(word, false, symbolTable);
		RegsDescr |= op->reg;
		AddrMode = op->addrMode & 0x0F;
		DataHigh = op->dataHigh;
		DataLow = op->dataLow;
		currentSection->addByte(InstrDescr);
		currentSection->addByte(RegsDescr);
		currentSection->addByte(AddrMode);
		if (DataHigh != 0) {
			currentSection->addByte(DataHigh);
			currentSection->addByte(DataLow);
		}
		else if (DataLow != 0)
			currentSection->addByte(DataLow);

		if (op->rep == SYMBOL || op->rep == PCREL_SYMBOL) {
			Symbol* symbol = symbolTable->find(word); // SACUVATI NEGDE SIMBOL i to koristiti umesto word
			Relocation* rel = new Relocation();
			rel->offset = currentSection->bytes.size();
			rel->type = (op->rep == PCREL_SYMBOL) ? PC_REL : ABS; // OBRADITI %
			rel->value = symbol->ordinal;
			currentSection->relocationTable.push_back(rel);
		}

		return true;
	}
	case PUSH:
	case POP: {
		return true;
	}
	case ILLEGAL: return false;
	}

}

/* ============================= CODES ============================== */


Directives Assembler::getDirectiveIndex(string directive) {
	if (directive == ".global") return GLOBAL;
	if (directive == ".extern") return EXTERN;
	if (directive == ".section") return SECTION;
	if (directive == ".word") return WORD;
	if (directive == ".skip") return SKIP;
	if (directive == ".equ") return EQU;
	if (directive == ".end") return END;
	return NOTFOUND;
}

Instructions Assembler::getInstructionIndex(string instruction) {
	if (instruction == "halt") return HALT;
	if (instruction == "int") return INT;
	if (instruction == "iret") return IRET;
	if (instruction == "call") return CALL;
	if (instruction == "ret") return RET;
	if (instruction == "jmp") return JMP;
	if (instruction == "jeq") return JEQ;
	if (instruction == "jne") return JNE;
	if (instruction == "jgt") return JGT;
	if (instruction == "push") return PUSH;
	if (instruction == "pop") return POP;
	if (instruction == "xchg") return XCHG;
	if (instruction == "add") return ADD;
	if (instruction == "sub") return SUB;
	if (instruction == "mul") return MUL;
	if (instruction == "div") return DIV;
	if (instruction == "cmp") return CMP;
	if (instruction == "not") return NOT;
	if (instruction == "and") return AND;
	if (instruction == "or") return OR;
	if (instruction == "xor") return XOR;
	if (instruction == "test") return TEST;
	if (instruction == "shl") return SHL;
	if (instruction == "shr") return SHR;
	if (instruction == "ldr") return LDR;
	if (instruction == "str") return STR;
	return ILLEGAL;
}

byte Assembler::getCode(Instructions instruction) {
	if (instruction == HALT) return 0x00;
	if (instruction == INT) return 0x10;
	if (instruction == IRET) return 0x20;
	if (instruction == CALL) return 0x30;
	if (instruction == RET) return 0x40;
	if (instruction == JMP) return 0x50;
	if (instruction == JEQ) return 0x51;
	if (instruction == JNE) return 0x52;
	if (instruction == JGT) return 0x53;
	if (instruction == XCHG) return 0x60;
	if (instruction == ADD) return 0x70;
	if (instruction == SUB) return 0x71;
	if (instruction == MUL) return 0x72;
	if (instruction == DIV) return 0x73;
	if (instruction == CMP) return 0x74;
	if (instruction == NOT) return 0x80;
	if (instruction == AND) return 0x81;
	if (instruction == OR) return 0x82;
	if (instruction == XOR) return 0x83;
	if (instruction == TEST) return 0x84;
	if (instruction == SHL) return 0x90;
	if (instruction == SHR) return 0x91;
	if (instruction == LDR) return 0xA0;
	if (instruction == STR) return 0xB0;
}