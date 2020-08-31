#include "Expression.h"

Expression::Expression(string s, int nr) {
	symbolName = s;
	numResult = nr;
	elemList = vector<ExpressionElem>();
}

bool Expression::resolve() {
	bool res = true;
	for (ExpressionElem el : elemList) res &= el.resolve;
	return res;
}

string Expression::toString() { 
	string design = /*symbolName + " = " +*/ (numResult == 0 ? "" : to_string(numResult));
	for (ExpressionElem el : elemList) design += el.toString();
	return design;
}

string Expression::indexToString() {
	int i = 0;
	if (index.empty()) return "";
	string design = "  " + symbolName + ":" + to_string(numResult) + string(25-symbolName.length(), ' ') + "\t";
	for (auto& elem : index)
	{
		if (i++ > 0) design += string(29, ' ') + "\t";
		design += to_string(elem.first) + string(8, ' ') 
			+ to_string(elem.second.value) 
			+ (elem.second.value >= 10 ? string(6, ' ') : string(5, ' ')) 
			+ elem.second.toString() + "\n";
	}
	return design;
}

Expression::~Expression() {}