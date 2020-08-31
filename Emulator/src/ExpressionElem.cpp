#include "ExpressionElem.h"

ExpressionElem::ExpressionElem(string s, bool a) {
	symElemName = s;
	add = a;
}

string ExpressionElem::toString() {
	return (rel ? ((add ? "+" : "-") + symElemName) : "");// + "(" + (rel ? "R_" : "") + to_string(val)+")" );
}

string ExpressionElem::toStringRel() {
	return ((add ? "+" : "-") + symElemName);// + "(" + (rel ? "R_" : "") + to_string(val)+")" );
}

ExpressionElem::~ExpressionElem() {}