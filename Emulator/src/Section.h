#pragma once
#ifndef SECTION_H
#define SECTION_H

#include <string>
#include <map>
#include <iostream>
#include <exception>
#include <vector>
#include <fstream>
#include "Relocation.h"

using namespace std;

struct DataSection {
	bool undef;
	string encoding;
	Relocation rel;
	int section = 0;
	DataSection(string e, bool u = false) :undef(u), encoding(e), rel(0, 0, "") {}
	
};

class Section {

public:

	Section(string n, int nn);
	string toString();
	virtual ~Section();

	string name;
	int locCnt;
	int num;
	int size;
	vector<DataSection> data;

	bool linker = false;
	int likerNum = 0;
};

#endif 