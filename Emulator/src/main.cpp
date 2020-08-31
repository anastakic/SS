//#include "Assembler.h"
//#include "Helper.h"
//#include "Enums.h"
#include "Linker.h"
#include "Emulator.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
	
	//initscr();
	
	/////////////////////////////////////////
	////// main za Emulator ///////
	////////////////////////////////////////
	try {
		Linker::parseInput("-place=ivt@0x0010");
		Linker::parseInput("IVTinit.o");

		
	for (int i = 1; i < argc; i++) {
		Linker::parseInput(argv[i]);
	}
	
		Linker linker = Linker();
		cout << "\r\n" << linker.toString() << "\r\n";

		Emulator emulator(linker.memory, linker.startAddr);

	//	emulator.insrtToString();
		emulator.registersToString();
		emulator.memoryToString(0x0000, 0x10000);
		cout << "\r\n";
	}
	catch (string s) {
		cout << s << "\r\n";
		return -1;
	}
	
	//endwin();
	return 0;
}