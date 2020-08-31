#pragma once
#ifndef LINKER_H
#define LINKER_H
#include <map>
#include <string>
#include <vector>
#include <exception>
#include <regex>
#include <iostream>
#include <fstream>
#include "Helper.h"
#include "InputFile.h"
#include "Section.h"

using namespace std;

class Linker {
public:
	unsigned char* memory;
	unsigned short startAddr;

	static int secId;
	static map<int, string> places;
	static vector<string> files;
	void static parseInput(string input);
	
	Linker();
	string toString();
	virtual ~Linker();

	map<int, Section> sections;
	map<int, DataSection> datasection;
	vector<Symbol> symbols;
	vector<InputFile> inputFiles;
	
	void processSections();
	void processSymbols();
	void processRelocations();
	void processDataSection();
	void processExpressions();

	void addGlobalSymbols();
	void doRelocations();

	void mergeDataSections();
	void addNumbers();

	void generateMemory();
	void doStartIvt();

	string format(string s);
};

#endif