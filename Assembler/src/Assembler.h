#pragma once
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <regex>
#include <string>
#include <map>
#include <iostream>
#include <exception>
#include <climits>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "Symbol.h"
#include "Helper.h"
#include "Section.h"
#include "Instruction.h"
#include "EquSymbol.h"
#include "Expression.h"

using namespace std;

class Assembler {
public:
	static int activeSection;
	static map<string, pair<int, int>> instructions;
	static map<string, int> addressing;
	static map<string, int> registers;
	map<int, Section> sections;

	Assembler(string in, string out);
	virtual ~Assembler();

	void ReadFile(string filePath);
	void Pass();
	void PrintFile(string filePath);

	//symbols
	void addSymbol(bool sec, string symbolName, int val, bool gl = false, bool setSection = true, bool def = true, bool decl = true, bool ex = false, bool con = false);
	void updateGlobalSym(string symbolName);
	void updateSymbol(string symbolName, int val, bool ex = false, bool con = false);
	void addEquGlobalSymbol(string symbolName, int val, string equSym, bool constSym);
	void addEquExprSymbol(string symbolName, int val, Expression e);
	
	//check
	bool isDefined(string symbolName);
	bool isDeclared(string symbolName);
	bool isExtern(string symbolName);
	bool isGlobal(string symbolName);
	bool isConst(string symbolName);

	//getters
	Expression getSymExpr(string symbolName);
	Symbol getSymbol(string symbolName);
	int getSymbolNumber(string symbolName);
	int getSymbolValue(string symbolName);
	int getSymbolSection(string symbolName);
	

	//symbol numbers
	void addSymbolNumbers();

	//relocations
	void doRelocations();
	string singleRelocation(Relocation rel);

	//equ
	void doEqus();
	bool doEquSymbols();
	bool singleEquSymbol(EquSymbol es);
	bool doExpressions();
	bool singleExpression(Expression& e);
	void doIndexExpression(Expression& e);

	//directive
	void parseDirective(string line);
	void dirGlobal(string);
	void dirExtern(string);
	void dirByte(string);
	void dirWord(string);
	void dirSkip(string);
	void dirSection(string);
	void dirEqu(string);


	//instruction
	void parseInstruction(string line);
	void isInstrOk(string s);
	bool isByteOp(string s);
	int numOpInstr(string s);
	int instrOpCode(string s);
	unsigned char getFirstByteOfInstruction(int opCode, int isByteOp);

	Operand getOperand(string operand, Instruction i, int& numBytes, bool fstOp = false);
	Operand dataOperand(string operand, Instruction i, int& numBytes, bool fstOp = false);
	Operand jumpOperand(string operand, Instruction i, int& numBytes);
	Operand regAdrr(string error, string o, OpAddrType addrType, Instruction i);


	//print
	string printSymbolTable();
	string printSectionData();
	string printInstructions();
	string printRelocation();
	string printExpression();
	string format(string str);

private:
	string inFile;
	string outFile;

	int currentSection;
	Section currSection;
	int locCnt;

	vector<string> lines;
	vector<Symbol> symbolTable;
	vector<Section> sectionsAll;
	vector<Instruction> instructionList;
	vector<Relocation> relocationTable;
	vector<EquSymbol> equSymbols;
	vector<Expression> expressionList;
};

#endif