#include "Instruction.h"

Instruction::Instruction(int lc, int num, int numOper, bool isB) :firstOp(), secOp() {
	numBytes = num;
	numOp = numOper;
	locCnt = lc;
	isByte = isB;
}

string Instruction::toString() {
	string design = Helper::byteToHexString(firstByte);
	if (numOp >= 1)
	{
		design += firstOp.toString();
		if (numOp == 2) design += secOp.toString();
	}
	return design;
}

Instruction::~Instruction() {}
