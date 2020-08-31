#pragma once
#ifndef EXPRELEM_H
#define EXPRELEM_H

#include <string>
#include <vector>

using namespace std;

class ExpressionElem {
public:
	bool constant = false;
	bool resolve = false;

	bool rel = false;

	string symElemName;
	bool add;
	int val = 0;
	
	ExpressionElem(string s, bool a);
	string toString();
	string toStringRel();
	virtual ~ExpressionElem();

};

#endif