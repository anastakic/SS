#pragma once
#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include "Enums.h"
#include "Helper.h"

using namespace std;

class Operand {
public:

	Operand();
	string toString();
	virtual ~Operand();


	string symbol;						// ako je 
	bool _useSym = false;		// operand simbol


	bool isByte = false;
	bool pcRel = false;
	int offset;
	int size;
	OpAddrType addrType;

	unsigned char opDesc;		//prvi bajt
	short ImDiAd = 0x0000;		//drugi i/ili treci bajt
};

#endif