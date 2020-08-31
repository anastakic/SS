#include "Section.h"

Section::Section(string n, int nn) {
	name = n;
	num = nn;
	data = vector<DataSection>();
}

string Section::toString() {
	string print;
	for (DataSection ds : data) print += ds.encoding;
	return print;
}

Section::~Section() {
}

