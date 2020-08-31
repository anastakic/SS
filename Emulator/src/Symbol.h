#pragma once
#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>
#include "Helper.h"
#include "Expression.h"

using namespace std;

class Symbol {
public:
	string name;
	int section;
	int value;
	int size;

	bool constant = false;

	int number;
	int linkerNum;

	bool global;
	bool declared;
	bool defined;

	bool ex;
	bool sec = false;

	Symbol(string symbolName, int val, bool gl, int s = -1, bool def = false, bool decl = false, bool ex = false);

	void setNumber(int n);
	string getNumber();

	string toString();
	virtual ~Symbol();

	Expression expression;

	bool operator < (const Symbol& sym) const
	{
		return (number < sym.number);
	}
};

#endif