#include "Assembler.h"
#include "Helper.h"
#include "Enums.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {

	//////////////////////////////////////////
	////// main za Asembler ///////
	//////////////////////////////////////////
	string pathToFile = "../tests/";
	
	if (argc != 4) {
		cout << "Invalid command. Expected format: assembler -o outputfile inputfile" << endl;
		return -1;
	}
	
	string inFile, outFile;

	int pos = -1;

	for (int i = 1; i < 4; i++)
	{
		if (!strcmp(argv[i], "-o")) pos = i;
	}

	if (pos == -1) {
		cout << "Invalid command -o was not found" << "\r\n";
		return -1;
	}

	if (pos != 1) {
		cout << "Invalid command. Expected format: assembler -o outputfile inputfile" << "\r\n";
		return -1;
	}
	else {
		outFile = (argv[2]);
		inFile = (pathToFile + argv[3]);
	}
	
	try {
		Assembler asembler = Assembler(inFile, outFile);
	}
	catch (string s) {
		cout << s << "\r\n";
		return -1;
	}

	return 0;
}