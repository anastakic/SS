#pragma once
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <vector>
#include <map>
#include "ExpressionElem.h"

using namespace std;

struct Index{
	
	int value;
	vector<ExpressionElem> elems;
	int offset = 0;

	Index(int v) {
		value = v;
		elems = vector<ExpressionElem>();
	}

	string toString() {
		string design = "";
		for (ExpressionElem elem : elems) {
			design += elem.toString();
		}
		//design += (" = off("+ to_string(offset) + ")") ;
		return design;
	}
};

class Expression {
public:
	
	string symbolName;
	int numResult;
	vector<ExpressionElem> elemList;
	
	map<int, Index> index;

	Expression(string s, int nr);

	bool constant = false;

	bool resolve();
	string toString();
	string indexToString();
	virtual ~Expression();

};

#endif