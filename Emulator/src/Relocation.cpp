#include "Relocation.h"

using namespace std;

Relocation::Relocation(int sec, int off, string s, bool pc) {
	section = sec;
	offset = off;
	symbolName = s;
	pcRel = pc;
}

string Relocation::toString() {
	return (Helper::intToHexString(offset) + "  " + (pcRel ? "R_386_PC16" : "R_386_16  ") 
		+ "     " +to_string(ref) +  (ref >= 10 ? string(5, ' ') : string(6, ' '))
									
		//	treba znak je extern moze i sa - !!!
		/* + symbolName + string(25-symbolName.length(), ' ')*/ 
		+ (add ? "+" : "-")	+ string(6, ' ') + to_string(size));
}

Relocation::~Relocation() {}