#pragma once
#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <vector>
#include <algorithm>
#include <locale>
#include <sstream>
#include <iomanip>
#include <cstddef>
#include <ctype.h>


using namespace std;

class Helper {

public:
	static inline void trim(string& s) {
		ltrim(s);
		rtrim(s);
		removeExtraBlanks(s);
		removeComment(s);
		ltrim(s);
		rtrim(s);
	}

	static inline void trimWithComm(string& s) {
		ltrim(s);
		rtrim(s);
		removeExtraBlanks(s);
		ltrim(s);
		rtrim(s);
	}
	static void ltrim(string& s);
	static void rtrim(string& s);
	static void removeExtraBlanks(string& s);
	static void removeComment(string& s);
	static bool endsWith(string const& s, string const& ending);
	static string charToHexString(unsigned char c);
	static string intToHexString(int i);
	static string wordToHexString(int i);
	static string wordToHexStringEqu(int i);
	static string byteToHexString(int i);

	static int strToInt(string s);
	static bool isNumber(string s);

	/*
	static vector<string> directiveParametersInBytes(string s);

	static Instruction getInstruction(string s, int& numBytes);
	static string getFirstByteOfInstruction(int opCode, int isByteOp);
	static Operand getOperand(string s, bool isByteOp, int& numBytes);
	static Operand getAddressingType(string s, bool isByteOp, int& numBytes);
	static bool isInstrOk(string s);
	static bool isByteOp(string s);
	static int numOpInstr(string s);
	static int instrEncode(string s);
	*/
};

#endif