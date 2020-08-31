#pragma once
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "Operand.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class Instruction {
public:

	Instruction(int lc, int num = -1, int numOper = -1, bool isB = false);
	string toString();
	virtual ~Instruction();

	int numBytes;
	int numOp;
	int locCnt;
	bool isByte;
	InstrDescrType instrType;

	string line;

	unsigned char firstByte = 0x00;

	Operand firstOp;
	Operand secOp;

};

#endif