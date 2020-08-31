#include "Symbol.h"

Symbol::Symbol(string symbolName, int val, bool gl, int s, bool def, bool decl, bool e):expression(symbolName,0){
	name = symbolName;
	section = s;
	value = val;
	global = gl;
	declared = decl;
	defined = def;
	number = 0;
	ex = e;
	expression.elemList = vector<ExpressionElem>();
}

void Symbol::setNumber(int n) { number = n; }

string Symbol::getNumber() { return to_string(number); }

string Symbol::toString() {
	return  ( name + string(20 - name.length(), ' ') + "\t" + to_string(section) + "    " +
		(global ? "global" : "local ") + "  " + Helper::intToHexString(value)
		+ "  " + to_string(number) + "     ");
		/*+ "\t" +(constant?"const":"     ")+"\t" +(expression.resolve()?"ok  ": "--  ")*/ 
}

Symbol::~Symbol() {}