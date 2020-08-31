#pragma once
#ifndef	EMULATOR_H
#define EMULATOR_H
#include <map>
#include <string>
#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <iostream>
//#include <exception>
#include <vector>
//#include <thread>
#include <ctime>
#include <stdint.h>
#include <regex>
#include <string>
#include <map>
#include <iostream>
#include <thread>
//#include <ncurses.h>
#include "Linker.h"
#include "Enums.h"

using namespace std;


class Emulator {
public:
	Emulator(unsigned char* memory, unsigned short startAddr);
	~Emulator();
	
	unsigned char* memory;
	unsigned short* registers;

	vector<string> runInstr;

	const static int REG_CYCLE = 1;
	const static int MEM_CYCLE = 5;

	const static int sp = 6;
	const static int pc = 7;
	const static int psw = 15;
	const static unsigned short data_out = 0xFF00;
	const static unsigned short data_in = 0xFF02;
	const static unsigned short timer_cfg = 0xFF10;

	unsigned short lastInstrPC = 0;

	static map<unsigned char, double> timerInt;

	short extendNegative(short b);
	clock_t timer;
	unsigned char timerPeriod;
	
	thread timerThread;
	void consumeKeyboard();
	
	char data_input = 0;
	bool terminalIntr = false;
	bool runningProgram = true;

	void handleInterrupt();
	void checkTimer();
	void checkTerminal();

	void emulate();

	unsigned char getNextByte();

	unsigned short getWordAt(unsigned short adr);
	void writeMem(bool isByte, unsigned short value, unsigned short address);

	bool getPSW_I();
	bool getPSW_N();
	bool getPSW_C();
	bool getPSW_O();
	bool getPSW_Z();
	bool getPSW_Terminal();
	bool getPSW_Timer();

	void setPSW_I(bool);
	void setPSW_Tl(bool);
	void setPSW_Tr(bool);
	void setPSW_N(bool);
	void setPSW_C(bool);
	void setPSW_O(bool);
	void setPSW_Z(bool);

	void setPSW_OpWord(int intrType, short opDst, short opSrc, short result);
	void setPSW_OpByte(int intrsType, char opDst, char opSrc, char result);

	void doIret(bool isByte);
	void doRet(bool isByte);
	void doInt(bool isByte);
	void doCall(bool isByte);
	void doJmp(bool isByte);
	void doJeq(bool isByte);
	void doJne(bool isByte);
	void doJgt(bool isByte);
	void doPush(bool isByte);
	void doPop(bool isByte);
	void doXchg(bool isByte);
	void doMov(bool isByte);
	void doAdd(bool isByte);
	void doSub(bool isByte);
	void doMul(bool isByte);
	void doDiv(bool isByte);
	void doCmp(bool isByte);
	void doNot(bool isByte);
	void doAnd(bool isByte);
	void doOr(bool isByte);
	void doXor(bool isByte);
	void doTest(bool isByte);
	void doShl(bool isByte);
	void doShr(bool isByte);

	void getOpRegDirSrc(short& src, unsigned char reg1, bool isHighSrc, bool isByte);
	void getOpRegDirDst(short& dst, unsigned char reg2, bool isHighDst, bool isByte);
	
	void getOpRegIndSrc(short& src, unsigned short& addrsrc, unsigned char reg1, bool isHighSrc, bool isByte);
	void getOpRegIndDst(short& dst, unsigned short& addrdst, unsigned char reg2, bool isHighDst, bool isByte);

	void getOpRegIndPomFirstOp(short& src, unsigned short& addrsrc, unsigned char reg1, bool isHighSrc, bool isByte);
	void getOpRegIndPomSecOp(short& dst, unsigned short& addrdst, unsigned char reg2, bool isHighDst, bool isByte);

	void getOpMemSrc(short& src, unsigned short& addrsrc, bool isByte);
	void getOpMemDst(short& dst, unsigned short& addrdst, bool isByte);

	void getOpImmSrc(short& src, bool isByte);

	void insrtToString();
	void memoryToString(int from, int to);
	void registersToString();

};

#endif
