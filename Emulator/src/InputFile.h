#pragma once
#ifndef INPUTFILE_H
#define INPUTFILE_H
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include "Helper.h"
#include "Symbol.h"
#include "Section.h"
#include "Relocation.h"

using namespace std;

class InputFile {
public:
	vector<string> lines;
	vector<Symbol> symbolTable;
	vector<Section> sections;
	vector<Relocation> relocations;
	vector<DataSection> datasection;
	vector<Expression> expressions;

	InputFile(string l);
	string toString();
	virtual ~InputFile();

	void parseSymbols();
	void parseRelocations();
	void parseSectionData();
	void parseExpression();

	bool resolveSections();
};

#endif