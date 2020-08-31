#pragma once
#ifndef EQUSYMBOL_H
#define EQUSYMBOL_H

#include <string>
#include <iostream>
#include "Helper.h"

using namespace std;

class EquSymbol {
public:
	string symbol1;
	string symbol2;

	bool resolve = false;

	// .equ sym1, sym2
	EquSymbol(string sym1, string sym2);

	string toString();
	virtual ~EquSymbol();
};

#endif