#include "Emulator.h"

map<unsigned char, double> Emulator::timerInt = {
//	{maska, period}
	{0,		0.5	},
	{1,		1	},
	{2,		1.5	},
	{3,		2	},
	{4,		5	},
	{5,		10	},
	{6,		30	},
	{7,		60	}
};

Emulator::Emulator(unsigned char* memory, unsigned short startAddr) {
	this->memory = memory;
	registers = new unsigned short[16]();
	runInstr = vector<string>();

	registers[pc] = startAddr;
	registers[sp] = 0x2000;

	timer = clock();
	timerPeriod = 0x00;

	timerThread = thread(&Emulator::consumeKeyboard, this);
	timerThread.detach();

	emulate();

}

void Emulator::insrtToString() {
	cout << "\r\n";

	for (string s : runInstr) {
		cout << s;
	}
}

void Emulator::memoryToString(int from, int to) {
	for (int i = from; i < to; i++) {
		if ((i % 16) == 0) cout << "\r\n" << Helper::wordToHexStringEqu(i) << ": ";
		cout << Helper::byteToHexString(memory[i]) << " ";
	}
}

void Emulator::registersToString() {

	cout << "\r\n";
	for (int i = 0; i < 16; i++) {
		if (i == 6) {
			cout << "sp" << ":  " << Helper::wordToHexStringEqu(registers[i]) << "\r\n";
		}
		else if (i == 7) {
			cout << "pc" << ":  " << Helper::wordToHexStringEqu(registers[i]) << "\r\n";
		}
		else if (i == 15) {
			cout << "psw" << ": " << Helper::wordToHexStringEqu(registers[i]) << " [" << bitset<16>(registers[i]) << "]" << "\r\n";
			cout << string(10, ' ') << "[ILR         NCOZ]\r\n";

		}
		else {
			cout << "r" << to_string(i) << (i <= 9 ? ":  " : ": ") << Helper::wordToHexStringEqu(registers[i]) << "\r\n";
		}
	}
	cout << "\r\n\r\n";
}


void Emulator::emulate() {

	while (runningProgram) {
		lastInstrPC = registers[pc];
		unsigned char byte = getNextByte();
	
		bool isByte = ((byte >> 2) & 0x1) == 0x00;
		int instrType = (byte >> 3);
		runInstr.push_back(Helper::wordToHexStringEqu(lastInstrPC) + ": ");
		try {
			switch (instrType) {
			case HALT:	runInstr.push_back("halt\r\n"); /*cout << "\r\nHappy end!!!\r\n";*/ runningProgram = false; break;
			case IRET:	doIret(isByte);	break;
			case RET:	doRet(isByte); break;
			case INT:	doInt(isByte); break;
			case CALL:	doCall(isByte); break;
			case JMP:	doJmp(isByte); break;
			case JEQ:	doJeq(isByte); break;
			case JNE:	doJne(isByte); break;
			case JGT:	doJgt(isByte); break;
			case PUSH:	doPush(isByte); break;
			case POP:	doPop(isByte); break;
			case XCHG:	doXchg(isByte); break;
			case MOV:	doMov(isByte); break;
			case ADD:	doAdd(isByte); break;
			case SUB:	doSub(isByte); break;
			case MUL:	doMul(isByte); break;
			case DIV:	doDiv(isByte); break;
			case CMP:	doCmp(isByte); break;
			case NOT:	doNot(isByte); break;
			case AND:	doAnd(isByte); break;
			case OR:	doOr(isByte); break;
			case XOR:	doXor(isByte); break;
			case TEST:	doTest(isByte); break;
			case SHL:	doShl(isByte); break;
			case SHR:	doShr(isByte); break;
			default:	throw string("Emulator Error: Invalid instruction code at address " + Helper::wordToHexStringEqu(lastInstrPC));
			}
			handleInterrupt();
		}
		catch (string s) {
			cout << s << "\r\n";
			memory[--registers[sp]] = registers[pc];
			memory[--registers[sp]] = (registers[pc] >> 8);

			memory[--registers[sp]] = registers[psw];
			memory[--registers[sp]] = (registers[psw] >> 8);

			registers[pc] = (memory[0x0002] & 0x00FF) | (memory[0x0003] << 8);
		}
	}
	
}


void Emulator::handleInterrupt() {
	if (getPSW_I() == true || runningProgram == false) return;
	
	checkTerminal(); checkTimer();
}

void Emulator::checkTimer()
{
	clock_t currentTime = clock();
	if (((currentTime - timer) / (1.0 * CLOCKS_PER_SEC)) > (timerInt.find(timerPeriod)->second * 1.0)) {
		timer = currentTime;
		
		if (getPSW_Timer() == false) {
			//cout << "\r\n\r\n * \r\n\r\n";

			timerPeriod = (memory[timer_cfg] & 0x07);
			
			memory[--registers[sp]] = registers[pc];
			memory[--registers[sp]] = (registers[pc] >> 8);

			memory[--registers[sp]] = registers[psw];
			memory[--registers[sp]] = (registers[psw] >> 8);

			setPSW_Tr(true);

			registers[pc] = (memory[0x0004] & 0x00FF) | (memory[0x0005] << 8);
		}
	}
}

void Emulator::checkTerminal() {
	if (getPSW_Terminal() == false && terminalIntr) {
		memory[data_in] = data_input;
		
		terminalIntr = false;
		
		memory[--registers[sp]] = registers[pc];
		memory[--registers[sp]] = (registers[pc] >> 8);

		memory[--registers[sp]] = registers[psw];
		memory[--registers[sp]] = (registers[psw] >> 8);

		registers[pc] = (memory[0x0006] & 0x00FF) | (memory[0x0007] << 8);
	}
}

void Emulator::consumeKeyboard() {
	//initscr();
	//cbreak();
	
	while (runningProgram) {
		//data_input = wgetch(stdscr);
		//data_input = getch();
		char tmp;
        cin >> noskipws >> tmp;
        data_input = tmp;
		terminalIntr = true;
		while (terminalIntr);
	}
	//endwin();
}



short Emulator::extendNegative(short b) {
	if (b & 0b10000000) b |= 0xFF00;
	return b;
}

void Emulator::doRet(bool isByte) {
	runInstr.push_back("ret\r\n");

	unsigned short data = memory[registers[sp]++];
	data <<= 8; data |= memory[registers[sp]++]; 

	registers[pc] = data;

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");
}

void Emulator::doIret(bool isByte) {
	runInstr.push_back("iret\r\n");

	unsigned short  data = memory[registers[sp]++];
	data <<= 8; 
	data |= memory[registers[sp]++]; 
	
	registers[psw] = data;

	data = memory[registers[sp]++];
	data <<= 8; data |= memory[registers[sp]++];
	registers[pc] = data;

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");
}

void Emulator::doInt(bool isByte) {
	runInstr.push_back("int\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); if (isByte) dst = extendNegative(dst); break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute

	dst %= 8;
	dst *= 2;
	dst = getWordAt(dst);

	memory[--registers[sp]] = registers[pc];
	memory[--registers[sp]] = (registers[pc] >> 8);
	
	memory[--registers[sp]] = registers[psw];
	memory[--registers[sp]] = (registers[psw] >> 8);


	registers[pc] = dst;

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");

}

void Emulator::doCall(bool isByte) {
	runInstr.push_back("call\r\n");

	if (isByte) throw string("Assembler Error: Instruction call cannot be combined with 'b' extension.");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); addrdst = dst; break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); addrdst = dst; break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	memory[--registers[sp]] = registers[pc];
	memory[--registers[sp]] = (registers[pc] >> 8);
	registers[pc] = addrdst;

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");

}

void Emulator::doJmp(bool isByte) {

	runInstr.push_back("jmp\r\n");

	if (isByte) throw string("Assembler Error: Instruction jmp cannot be combined with 'b' extension.");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); addrdst = dst; break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); addrdst = dst; break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	registers[pc] = addrdst;
}

void Emulator::doJeq(bool isByte) {
	runInstr.push_back("jeq\r\n");

	if (isByte) throw string("Assembler Error: Instruction jeq cannot be combined with 'b' extension.");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); addrdst = dst; break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); addrdst = dst; break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	if (!getPSW_Z()) return;
	registers[pc] = addrdst;
}

void Emulator::doJne(bool isByte) {
	runInstr.push_back("jne\r\n");

	if (isByte) throw string("Assembler Error: Instruction jne cannot be combined with 'b' extension.");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); addrdst = dst; break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); addrdst = dst; break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	if (getPSW_Z()) return;
	registers[pc] = addrdst;
}

void Emulator::doJgt(bool isByte) {
	runInstr.push_back("jgt\r\n");

	if (isByte) throw string("Assembler Error: Instruction jgt cannot be combined with 'b' extension.");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short dst = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); addrdst = dst; break; //throw string("Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); addrdst = dst; break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	// (N ^ V) | Z == 0
	if (((getPSW_N() ^ getPSW_O()) | getPSW_Z())) return;
	registers[pc] = addrdst;
}

void Emulator::doPush(bool isByte) {

	runInstr.push_back("push\r\n");
	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0;
	short src = 0;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	//execute
	memory[--registers[sp]] = src;
	if (!isByte) { memory[--registers[sp]] = (src >> 8); }

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");
}

void Emulator::doPop(bool isByte) {

	runInstr.push_back("pop\r\n");
	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrdst = 0;
	short data = 0, dst = 0;

	data = memory[registers[sp]++];
	if (!isByte) { data <<= 8; data |= memory[registers[sp]++]; }

	if (registers[sp] < 0x0010) throw string("Emulator Error: Stack underflow. Addresses 0x0000 - 0x0010 are reserved for IVT.");
	if (registers[sp] > 0xFF00) throw string("Emulator Error: Stack overflow. Addresses 0xFF00 - 0xFFFF are reserved for registers.");


	switch (AddrModeDst) {
	case IMM: throw string(Helper::wordToHexStringEqu(registers[pc]) + "Emulator Error: Invalid instruction pop. Immediate addressing mode not allowed for destination operand.");
	
	case REGDIR:		getOpRegDirDst(dst, reg1, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg1, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg1, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute

	switch (AddrModeDst)
	{
	case REGDIR: {
		short pom = isHighDst ? ((registers[reg1] & 0x00ff) | (data << 8)) : ((registers[reg1] & 0xff00) | (data & 0x00ff));
		registers[reg1] = isByte ? pom : data;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, data, addrdst);
		break;
	}
	}
}

void Emulator::doXchg(bool isByte) {

	runInstr.push_back("xchg\r\n");
	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM: throw string("Emulator Error: Invalid instruction xchg. Immediate addressing mode not allowed for source operand.");
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction xchg. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	short temp = dst;
	dst = src;
	src = temp;
	//execute
	switch (AddrModeSrc)
	{
		case REGDIR: 
		{
			pom = isHighDst ? ((registers[reg1] & 0x00ff) | (src << 8)) : ((registers[reg1] & 0xff00) | (src & 0x00ff));
			registers[reg1] = isByte ? pom : src;
			break;
		}
		case REGIND:
		case REGIND_POM:
		case MEM:
		{
			writeMem(isByte, src, addrsrc);
			break;
		}
	}
	switch (AddrModeDst)
	{
		case REGDIR: {
			pom = isHighDst ? ((registers[reg2] & 0x00ff) | (dst << 8)) : ((registers[reg2] & 0xff00) | (dst & 0x00ff));
			registers[reg2] = isByte ? pom : dst;
			break;
		}
		case REGIND: 
		case REGIND_POM: 
		case MEM:
		{
			writeMem(isByte, dst, addrdst);
			break;
		}
	}
}

void Emulator::doMov(bool isByte) {

	runInstr.push_back("mov\r\n");
	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
		case IMM:			getOpImmSrc(src, isByte); break;
		case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
		case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
		case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
		case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
		case IMM: throw string("Emulator Error: Invalid instruction mov. Immediate addressing mode not allowed for destination operand.");
		case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
		case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
		case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
		case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute

	switch (AddrModeDst)
	{
		case REGDIR: {
			pom = isHighDst ? ((registers[reg2] & 0x00ff) | (src << 8)) : ((registers[reg2] & 0xff00) | (src & 0x00ff));
			registers[reg2] = isByte ? pom : src;
			break;
		}
		case REGIND:
		case REGIND_POM:
		case MEM:
		{
			writeMem(isByte, src, addrdst);
			break;
		}
	}
}

void Emulator::doAdd(bool isByte) {
	runInstr.push_back("add\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction add. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	short result = dst + src;
	

	// set psw
	if (!isByte) setPSW_OpWord(ADD, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) + (src & 0x00ff);
		setPSW_OpByte(ADD, dst & 0x00ff, src & 0x00ff, res);
	}


	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doSub(bool isByte) {
	runInstr.push_back("sub\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction sub. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	//execute
	short result = dst - src;

	// set psw
	
	if (!isByte) setPSW_OpWord(SUB, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) - (src & 0x00ff);
		setPSW_OpByte(SUB, dst & 0x00ff, src & 0x00ff, res);
    } 


	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doMul(bool isByte) {
	runInstr.push_back("mul\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction mul. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	//execute
	short result = dst * src;

	// set psw
	if(!isByte) setPSW_OpWord(MUL, dst, src, result);
    else
	{
		char res = (dst & 0x00ff) * (src & 0x00ff);
		setPSW_OpByte(MUL, dst & 0x00ff, src & 0x00ff, res);    
    }  

	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doDiv(bool isByte) {
	runInstr.push_back("div\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction div. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (src == 0) throw "Emulator Error: Division by zero encountered";
	if (isByte) src = extendNegative(src);
	//execute
	short result = dst / src;

	// set psw
	
	if(!isByte) setPSW_OpWord(DIV, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) / (src & 0x00ff);
		setPSW_OpByte(DIV, dst & 0x00ff, src & 0x00ff, res);
	}
	
	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	
	case REGIND:
	case REGIND_POM:
	case MEM: 
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doCmp(bool isByte) {
	runInstr.push_back("cmp\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); if (isByte) dst = extendNegative(dst); break;
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	//execute
	short result = dst - src;

	// set psw
	// rez samo psw ===>
	
	if (!isByte) setPSW_OpWord(CMP, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) - (src & 0x00ff);
		setPSW_OpByte(CMP, dst & 0x00ff, src & 0x00ff, res);
	}

}

void Emulator::doNot(bool isByte) {
	runInstr.push_back("not\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction not. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	
	//execute
	short result = ~src;

	// set psw

	if (!isByte) setPSW_OpWord(NOT, dst, src, result);
	else
	{
		char res = ~(src & 0x00ff);
		setPSW_OpByte(NOT, dst & 0x00ff, src & 0x00ff, res);
	}

	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doAnd(bool isByte) {
	runInstr.push_back("and\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction and. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	
	//execute
	short result = dst & src;

	// set psw

	if (!isByte) setPSW_OpWord(AND, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) & (src & 0x00ff);
		setPSW_OpByte(AND, dst & 0x00ff, src & 0x00ff, res);
	}

	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doOr(bool isByte) {
	runInstr.push_back("or\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction or. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	//execute
	short result = dst | src;

	// set psw

	if (!isByte) setPSW_OpWord(OR, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) | (src & 0x00ff);
		setPSW_OpByte(OR, dst & 0x00ff, src & 0x00ff, res);
	}


	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doXor(bool isByte) {
	runInstr.push_back("xor\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction xor. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	//execute
	short result = dst ^ src;

	// set psw

	if (!isByte) setPSW_OpWord(XOR, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) ^ (src & 0x00ff);
		setPSW_OpByte(XOR, dst & 0x00ff, src & 0x00ff, res);
	}
	
	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doTest(bool isByte) {
	runInstr.push_back("test\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM:			getOpImmSrc(dst, isByte); dst = extendNegative(dst); break;
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	if (isByte) src = extendNegative(src);
	//execute
	short result = dst & src;

	// set psw
	if (!isByte) setPSW_OpWord(TEST, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) & (src & 0x00ff);
		setPSW_OpByte(TEST, dst & 0x00ff, src & 0x00ff, res);
	}

}

void Emulator::doShl(bool isByte) {
	runInstr.push_back("shl\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeSrc) {
	case IMM:			getOpImmSrc(src, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); if (isByte) src = extendNegative(src); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); if (isByte) src = extendNegative(src); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); if (isByte) src = extendNegative(src); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction shl. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	
	//execute
	short result = dst << src;

	// set psw
	if (!isByte) setPSW_OpWord(SHL, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) << (src & 0x00ff);
		setPSW_OpByte(SHL, dst & 0x00ff, src & 0x00ff, res);
	}
	
	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}

void Emulator::doShr(bool isByte) {
	runInstr.push_back("shr\r\n");

	unsigned char OpDescr = getNextByte();
	unsigned char AddrModeDst = OpDescr >> 5;
	unsigned char reg2 = (OpDescr >> 1) & 0b00001111;
	bool isHighDst = OpDescr & 0b00000001;

	unsigned short addrsrc = 0, addrdst = 0;
	short src = 0, dst = 0, pom = 0;

	switch (AddrModeDst) {
	case IMM: throw string("Emulator Error: Invalid instruction shr. Immediate addressing mode not allowed for destination operand.");
	case REGDIR:		getOpRegDirDst(dst, reg2, isHighDst, isByte); break;
	case REGIND:		getOpRegIndDst(dst, addrdst, reg2, isHighDst, isByte); break;
	case REGIND_POM:	getOpRegIndPomSecOp(dst, addrdst, reg2, isHighDst, isByte); break;
	case MEM:			getOpMemDst(dst, addrdst, isByte); break;
	}

	OpDescr = getNextByte();
	unsigned char AddrModeSrc = OpDescr >> 5;
	unsigned char reg1 = (OpDescr >> 1) & 0b00001111;
	bool isHighSrc = OpDescr & 0b00000001;

	switch (AddrModeSrc) {
	case IMM: 			getOpImmSrc(dst, isByte); break;
	case REGDIR:		getOpRegDirSrc(src, reg1, isHighSrc, isByte); break;
	case REGIND:		getOpRegIndSrc(src, addrsrc, reg1, isHighSrc, isByte); if (isByte) src = extendNegative(src); break;
	case REGIND_POM:	getOpRegIndPomFirstOp(src, addrsrc, reg1, isHighSrc, isByte); if (isByte) src = extendNegative(src); break;
	case MEM:			getOpMemSrc(src, addrsrc, isByte); if (isByte) src = extendNegative(src); break;
	}


	//execute
	short result = dst >> src;

	// set psw
	if (!isByte) setPSW_OpWord(SHR, dst, src, result);
	else
	{
		char res = (dst & 0x00ff) >> (src & 0x00ff);
		setPSW_OpByte(SHR, dst & 0x00ff, src & 0x00ff, res);
	}

	switch (AddrModeDst)
	{
	case REGDIR: {
		pom = isHighDst ? ((registers[reg2] & 0x00ff) | (result << 8)) : ((registers[reg2] & 0xff00) | (result & 0x00ff));
		registers[reg2] = isByte ? pom : result;
		break;
	}
	case REGIND:
	case REGIND_POM:
	case MEM:
	{
		writeMem(isByte, result, addrdst);
		break;
	}
	}
}


void Emulator::setPSW_OpWord(int intrType, short dst, short src, short result) {
	
		//	Z N				mov, mul, div, not, and, or, xor, test
	setPSW_N(result < 0);
	setPSW_Z(result == 0);
	
		//	Z O C N			add, sub, cmp 
	if (intrType == ADD || intrType == SUB || intrType == CMP) 
	{	
		if (intrType == ADD) 
		{
			setPSW_O((result < 0 && dst > 0 && src > 0) || (result > 0 && dst < 0 && src < 0));
			setPSW_C((result < 0 && dst > 0 && src > 0) || (result > 0 && dst < 0 && src < 0));
		}
		else 
		{
			setPSW_O((result < 0 && dst > 0 && src < 0) || (result > 0 && dst < 0 && src > 0));
			setPSW_C((result < 0 && dst > 0 && src < 0) || (result > 0 && dst < 0 && src > 0));
		}
	}

		//	Z C N			shl, shr
	else if (intrType == SHL) setPSW_C(src != 0 ? dst << (src - 1) & 1 : 0);
	else if (intrType == SHR) setPSW_C(src != 0 ? dst >> (src - 1) & 1 : 0);
}

void Emulator::setPSW_OpByte(int intrType, char dst, char src, char result) {

		//	Z N				mov, mul, div, not, and, or, xor, test
	setPSW_N(result < 0);
	setPSW_Z(result == 0);

		//	Z O C N			add, sub, cmp 
	if (intrType == ADD || intrType == SUB || intrType == CMP)
	{
		if (intrType == ADD) 
		{
			setPSW_O((result < 0 && dst > 0 && src > 0) || (result > 0 && dst < 0 && src < 0));
			setPSW_C((result < 0 && dst > 0 && src > 0) || (result > 0 && dst < 0 && src < 0));
		}
		else 
		{
			setPSW_O((result < 0 && dst > 0 && src < 0) || (result > 0 && dst < 0 && src > 0));
			setPSW_C((result < 0 && dst > 0 && src < 0) || (result > 0 && dst < 0 && src > 0));
		}
	}

		//	Z C N			shl, shr
	else if (intrType == SHL) setPSW_C(src != 0 ? dst << (src - 1) & 1 : 0);
	else if (intrType == SHR) setPSW_C(src != 0 ? dst >> (src - 1) & 1 : 0);
}

void Emulator::getOpRegDirSrc(short& src, unsigned char reg1, bool isHighSrc, bool isByte) {
	src = registers[reg1];
	if (isByte)
	{
		src = (isHighSrc ? src >> 8 : src & 0x00FF);
	}
}

void Emulator::getOpRegDirDst(short& dst, unsigned char reg2, bool isHighDst, bool isByte) {
	dst = registers[reg2];
	if (isByte) {
		dst = (isHighDst ? dst >> 8 : dst & 0x00FF);
		dst = extendNegative(dst);
	}
}

void Emulator::getOpRegIndSrc(short& src, unsigned short& addrsrc, unsigned char reg1, bool isHighSrc, bool isByte) {

	short offs = 0x0000;
	addrsrc = registers[reg1] + offs;
	src = isByte ? memory[addrsrc] : getWordAt(addrsrc);

}

void Emulator::getOpRegIndDst(short& dst, unsigned short& addrdst, unsigned char reg2, bool isHighDst, bool isByte) {
	
	short offs = 0x0000;
	addrdst = registers[reg2] + offs;
	dst = isByte ? extendNegative(memory[addrdst]) : getWordAt(addrdst);

}

void Emulator::getOpRegIndPomSecOp(short& src, unsigned short& addrsrc, unsigned char reg1, bool isHighSrc, bool isByte) {
	short offs = getNextByte();
	if (!isByte) offs |= getNextByte() << 8;
	addrsrc = registers[reg1] + offs;
	src = isByte ? memory[addrsrc] : getWordAt(addrsrc);
}

void Emulator::getOpRegIndPomFirstOp(short& dst, unsigned short& addrdst, unsigned char reg2, bool isHighDst, bool isByte) {
	short offs = getNextByte();
	if (!isByte) offs |= getNextByte() << 8;
	addrdst = registers[reg2];
	if (reg2 == pc) {
		unsigned char nextByte = memory[registers[pc]];
		int type = (nextByte >> 5);
		if (type == IMM || type == REGIND_POM || type == MEM) offs += isByte ? 2 : 3;
		else if (type == REGDIR || type == REGIND) offs += 1;
	}
	addrdst += offs;
	dst = isByte ? extendNegative(memory[addrdst]) : getWordAt(addrdst);
}

void Emulator::getOpMemSrc(short& src, unsigned short& addrsrc, bool isByte) {
	addrsrc = getNextByte();
	if (!isByte) addrsrc |= getNextByte() << 8;
	src = isByte ? memory[addrsrc] : getWordAt(addrsrc);
}

void Emulator::getOpMemDst(short& dst, unsigned short& addrdst, bool isByte) {
	addrdst = getNextByte();
	if (!isByte) addrdst |= getNextByte() << 8;
	dst = isByte ? extendNegative(memory[addrdst]) : getWordAt(addrdst);
}

void Emulator::getOpImmSrc(short& src, bool isByte) {
	src = getNextByte();
	if (!isByte) src |= (getNextByte() << 8);
}

bool Emulator::getPSW_I() {
	return registers[psw] >> 15 & 1;
}

bool Emulator::getPSW_Terminal() {
	return registers[psw] >> 14 & 1;
}

bool Emulator::getPSW_Timer() {
	return registers[psw] >> 13 & 1;
}

bool Emulator::getPSW_N() {
	return registers[psw] >> 3 & 1;
}

bool Emulator::getPSW_C() {
	return registers[psw] >> 2 & 1;
}

bool Emulator::getPSW_O() {
	return registers[psw] >> 1 & 1;
}

bool Emulator::getPSW_Z() {
	return registers[psw] >> 0 & 1;
}

void Emulator::setPSW_I(bool b) {
	if (b) registers[psw] |= 0x8000;
	else registers[psw] &= 0x7FFF;
}

void Emulator::setPSW_Tl(bool b) {
	if (b) registers[psw] |= 0x4000;
	else registers[psw] &= 0xBFFF;
}

void Emulator::setPSW_Tr(bool b) {
	if (b) registers[psw] |= 0x2000;
	else registers[psw] &= 0xDFFF;
}

void Emulator::setPSW_N(bool b) {
	if (b) registers[psw] |= 0x0008;
	else registers[psw] &= 0xFFF7;
}

void Emulator::setPSW_C(bool b) {
	if (b) registers[psw] |= 0x0004;
	else registers[psw] &= 0xFFFB;
}

void Emulator::setPSW_O(bool b) {
	if (b) registers[psw] |= 0x0002;
	else registers[psw] &= 0xFFFD;
}

void Emulator::setPSW_Z(bool b) {
	if (b) registers[psw] |= 0x0001;
	else registers[psw] &= 0xFFFE;
}

unsigned char Emulator::getNextByte() {
	if (registers[pc] == 0xFFFF) throw string("Fatal error: Segmentation fault.");
	return memory[registers[pc]++];
}

unsigned short Emulator::getWordAt(unsigned short adr) {
	unsigned short ret = memory[adr] | (memory[adr + 1] << 8);
	return ret;
}


void Emulator::writeMem(bool isByte, unsigned short value, unsigned short address) {

	memory[address] = (value & 0x00FF);
	
	if (address == data_out) cout << (char)(memory[address]);
	if (address == timer_cfg) timerPeriod = (value % 8);

	if (!isByte) {
		memory[address + 1] = value >> 8;
		if (address + 1 == data_out) cout << (char)(memory[address + 1]);// << Helper::byteToHexString(memory[address + 1]);
		if (address + 1 == timer_cfg) timerPeriod = (value % 8);
	}
}


Emulator::~Emulator() {}
