#pragma once
#ifndef ENUMS_H
#define ENUMS_H

enum InstrDescrType {
	HALT, IRET, RET, INT,
	CALL, JMP, JEQ, JNE, JGT,
	PUSH, POP, XCHG, MOV,
	ADD, SUB, MUL, DIV,
	CMP, NOT, AND, OR, XOR,
	TEST, SHL, SHR
};

enum OpAddrType {
	IMM, REGDIR,
	REGIND, REGIND_POM,
	MEM
};


#endif

