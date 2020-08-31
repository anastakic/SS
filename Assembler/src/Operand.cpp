#include "Operand.h"

Operand::Operand() {}

string Operand::toString() {
	string design = Helper::byteToHexString(opDesc);

	if (addrType != REGDIR && addrType != REGIND)
		design += (isByte ? Helper::byteToHexString(ImDiAd) : Helper::wordToHexString(ImDiAd));

	return design;
}

Operand::~Operand() {}