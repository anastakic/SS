#pragma once
#ifndef RELSYMBOL_H
#define RELSYMBOL_H

#include <string>
#include <iostream>
#include "Helper.h"

using namespace std;

class Relocation {
public:
	int offset;
	string symbolName;
	bool pcRel;
	
	int ref = 0;
	bool global = false;
	int size = 2;

	string op = "";
	bool wordOrByte = false;
	bool jmpInstr = false;
	int section = 0;

	bool add = true;

	//na osnovu imena dohvatiti sekciju i da li je globalan
	Relocation(int sec, int off, string s, bool pc = false);

	string toString();
	virtual ~Relocation();
};

#endif