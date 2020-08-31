#include "Linker.h"

map<int, string> Linker::places;
vector<string> Linker::files = vector<string>();
int Linker::secId = 1;

void Linker::parseInput(string input) {
	regex checkPlace("^-place=(.+)@(.+)$");
	regex checkFile("^(.+).o$");
	string pathToTest = "../tests/";
	if (regex_match(input, checkPlace)) {
		int begin = input.find('=') + 1;
		int end = input.find('@');
		if (!Helper::isNumber(input.substr(end + 1, input.size()))) 
			throw "Invalid command " + input +". Expected -place=<section_name>@<number>";
		int addr = Helper::strToInt(input.substr(end + 1, input.size()));
		if (addr >= 0x0000 && addr < 0x0010) throw "Invalid command " + input + ". Addresses 0x0000 - 0x0010 are reserved for IVT.";
		if (addr >= 0xFF00 && addr <= 0xFFFF) throw "Invalid command " + input + ". Addresses 0xFF00 - 0xFFFF are reserved for registers.";
		if (addr > 0xFFFF)throw "Invalid command " + input + ". Address 0x" + Helper::intToHexString(addr) +" out of range.";
		
		for (auto& a : places) {
			if (a.second == (input.substr(begin, end)).substr(0, end - begin)) throw "Invalid command " + input + ". Start address for section " + (input.substr(begin, end)).substr(0, end - begin) + " is already given.";
		}
		
		places.insert(pair<int, string>(addr, (input.substr(begin, end)).substr(0, end - begin)));
		
	}
	else if (regex_match(input, checkFile)){
		if (input != "IVTinit.o") files.push_back(pathToTest + input);
		else files.push_back(input);
	}
	else throw "Invalid command " + input + ". Expected -place=<section_name>@<number> or <input_file.o>";
}

Linker::Linker() {
	inputFiles = vector<InputFile>();
	symbols = vector<Symbol>();

	memory = new unsigned char[0x10000]();
	
	if (Linker::files.empty()) throw string("Linker Error: Invalid command file list is empty.");
	string str;
	
	for (string filePath : Linker::files) {
		InputFile i = InputFile(filePath);
		this->inputFiles.push_back(i);
	}

	processSections();
	processSymbols();
	processDataSection();
	processRelocations();

	addGlobalSymbols();
	processExpressions();

	doRelocations();
	mergeDataSections();

	addNumbers();
	generateMemory();

	doStartIvt();
	
}

void Linker::doStartIvt() {
	for (Symbol& s : symbols) {
		if (s.name == "_start") startAddr = s.value;
		if (s.name == "ivt_0")
		{
			memory[0] = s.value & 0x00FF;
			memory[1] = s.value >> 8;
		}
		if (s.name == "ivt_1")
		{
			memory[2] = s.value & 0x00FF;
			memory[3] = s.value >> 8;
		}
		if (s.name == "ivt_2")
		{
			memory[4] = s.value & 0x00FF;
			memory[5] = s.value >> 8;
		}
		if (s.name == "ivt_3")
		{
			memory[6] = s.value & 0x00FF;
			memory[7] = s.value >> 8;
		}
		if (s.name == "ivt_4")
		{
			memory[8] = s.value & 0x00FF;
			memory[9] = s.value >> 8;
		}
		if (s.name == "ivt_5")
		{
			memory[10] = s.value & 0x00FF;
			memory[11] = s.value >> 8;
		}
		if (s.name == "ivt_6")
		{
			memory[12] = s.value & 0x00FF;
			memory[13] = s.value >> 8;
		}
		if (s.name == "ivt_7")
		{
			memory[14] = s.value & 0x00FF;
			memory[15] = s.value >> 8;
		}
	}
}

void Linker::processSections() {
	int place = 0; // 0x0010; // za slucaj da nema ni jedan place
	int secoff = 0;
	bool find = false;
	string prevsec = "";
	for (auto& s : places) {
		if ((place + secoff) > s.first) {
			throw "Linker Error: There is no space for section " + prevsec + ".";
		}
		else { place = s.first; }
		secoff = 0;
		prevsec = s.second;
		Section sekcija = Section(s.second, secId);
		sekcija.locCnt = place;
		for (InputFile& inf : inputFiles) {
			for (Section& sec : inf.sections) {
				if (sec.name == s.second) {
					sec.locCnt = place + secoff;
					secoff += sec.size;
					sec.linker = true;
					find = true;
					break;
				}
			}
		}
		sekcija.size = secoff;
		sections.insert(pair<int, Section>(secId++, sekcija));
		if (!find) throw "Linker Error: Undefined section " + s.second;
		find = false;
	}
	place += secoff;

	string secname = "";
	bool newsec = true;
	while (true) {
		for (InputFile& inf : inputFiles) {
			if (inf.resolveSections()) continue;
			for (Section& sec : inf.sections) {
				if (!sec.linker) {
					if (!newsec) {
						secname = sec.name;
						newsec = true;
						Section sekcija = Section(sec.name, secId);
						sekcija.locCnt = place;
						for (InputFile& ii : inputFiles) {
							for (Section& ss : ii.sections) {
								if (ss.name == sec.name) {
									sekcija.size += ss.size;
								}
							}
						}
						sections.insert(pair<int, Section>(secId++, sekcija));
					}
					if (secname == sec.name) {
						sec.locCnt = place;
						place += sec.size;
						if (place >= 0xff00) throw "Linker Error: Section " + secname + " is out of range. Addresses 0xFF00 - 0xFFFF are reserved for registers.";
						sec.linker = true;
					}
				}
			}
		}
		if (!newsec) break;
		newsec = false;
	}
}

void Linker::processSymbols() {
	for (InputFile& inf : inputFiles) {
		for (Symbol& sym : inf.symbolTable) {
			if (sym.sec) {
				int offs = 0;
				int secnum = 0;
				for (Section& sec : inf.sections) {
					if (sym.name == sec.name) {
						offs = sec.locCnt;
						secnum = sec.num;
						break;
					}
				}
				for (Symbol& sym : inf.symbolTable) {
					if (sym.section == secnum) sym.value += offs;
				}
			}
		}
	}

	for (InputFile& inf : inputFiles) {
		for (auto& s : sections) {
			for (Section& sec : inf.sections) {
				if (sec.name == s.second.name) {
					sec.likerNum = s.first;
				}
			}
			for (Symbol& sym : inf.symbolTable) {
				if ((sym.name == s.second.name)) {
					sym.linkerNum = s.first;
				}
			}
		}
		for (Symbol& sym : inf.symbolTable) {
			if (!sym.sec) {
				sym.linkerNum = secId++;
			}
		}
	}
}

void Linker::processDataSection() {
	for (InputFile& inf : inputFiles) {
		for (DataSection& ds : inf.datasection) {
			for (Section& sec : inf.sections) {
				if (ds.section == sec.num) {
					ds.section = sec.likerNum;
					break;
				}
			}
		}
	}
}

void Linker::processRelocations() {
	for (InputFile& inf : inputFiles) {
		for (Relocation& r : inf.relocations) {
			for (Symbol& sym : inf.symbolTable) {
				if (r.ref == sym.number) {
					r.ref = sym.linkerNum;
					break;
				}
			}
			for (Section& sec : inf.sections) {
				if (r.section == sec.num) {
					r.section = sec.likerNum;
					break;
				}
			}
		}
	}
}

void Linker::processExpressions() {
	for (InputFile& inf : inputFiles) {
		for (Symbol& sym : inf.symbolTable) {
			if (!sym.expression.elemList.empty()) {
				sym.value = sym.expression.numResult;
				for (Symbol& s : inf.symbolTable) {
					if (s.name == sym.expression.elemList.at(0).symElemName) {
						sym.value += (sym.expression.elemList.at(0).add ? s.value : -s.value);
						sym.section = s.section;
					}
				}
			}
		}
	}
}

void Linker::addGlobalSymbols() {
	
	for (InputFile& inf : inputFiles) {
		for (Symbol& sym : inf.symbolTable) {
			if ((sym.global && (sym.section != 0)) || (sym.global && (sym.section == 0) && sym.constant))
			{
				bool find = false;
				for (Symbol& symLinker : this->symbols) {
					if ((sym.name == symLinker.name) && symLinker.global && symLinker.section != 0)
						throw "Linker Error: Multiple definition of global symbol " + sym.name;
					else if ((sym.name == symLinker.name) && symLinker.global && symLinker.section == 0)
					{
						symLinker.section = sym.section;
						symLinker.value = sym.value;
						find = true;
						break;
					}
				}
				if (!find) {
					sym.number = sym.linkerNum;
					this->symbols.push_back(sym);
				}
			}
		}
	}

	for (InputFile& inf : inputFiles) {
		for (Symbol& sym : inf.symbolTable) {

			if (sym.global && (sym.section == 0))
			{
				bool find = false;
				for (Symbol& symLinker : this->symbols) {
					if ((sym.name == symLinker.name) && symLinker.global && symLinker.section == 0 && symLinker.constant == false) { find = true;/* ok */ }
					else if ((sym.name == symLinker.name) && ((symLinker.global && symLinker.section != 0) || (symLinker.global && symLinker.section == 0 && symLinker.constant)))
					{
						sym.section = symLinker.section;
						sym.value = symLinker.value;
						find = true;
						break;
					}
				}
				if (!find) {
					sym.number = sym.linkerNum;
					this->symbols.push_back(sym);
				}
			}
		}
	}

	vector<string> undef = vector<string>();
	for (Symbol& sym : this->symbols) {
		if (sym.global && (sym.section == 0) && !sym.constant && sym.expression.elemList.empty()) {
			undef.push_back(sym.name);
		}
	}
	string error = "";
	for (string e : undef) {
		error += ("Linker Error: Undefined global symbol " + e + ".\r\n");
	}
	if (!undef.empty()) throw error;

//	for (Symbol& s : symbols) {
//		cout << s.toString() << (s.constant ? " const" : " ") 
//			<< (s.expression.elemList.empty() ? "" : (s.expression.toString() + s.expression.elemList.at(0).toStringRel())) << "\r\n";
//	}

}

void Linker::doRelocations() {
	for (InputFile& inf : inputFiles) {
		for (Relocation& r : inf.relocations) {
			bool pcRel = r.pcRel;
			bool add = r.add;
			int sec = r.section;
			int relOfs = r.offset;
			int ref = r.ref;
			int size = r.size;

			int value = 0;
			int secLC = 0;

			r.global = (sections.find(sec) != sections.end()) ? false : true;

			for (Symbol& sym : inf.symbolTable) {
				if (sym.linkerNum == ref) {
					value = sym.value;
					break;
				}
			}
			
			if (r.global) {
				for (Symbol& sym : symbols) {
					if (sym.linkerNum == ref) {
						value = sym.value;
						break;
					}
				}
			}

			for (Symbol& sym : inf.symbolTable) {
				if (sym.linkerNum == sec) {
					secLC = sym.value;
					break;
				}
			}

			int fromMem = 0;
			for (DataSection& s : inf.datasection) {
				if (s.section == r.section) {
					
					string littleEndian = s.encoding.substr(2 * relOfs, 2 * size);
					
					if (size == 2) {
						string pom = littleEndian.substr(0, 2);
						littleEndian = littleEndian.substr(2, 2);
						littleEndian += pom;
					}
					
					fromMem = stoi(littleEndian, 0, 16);
					
					//cout << r.toString() <<"\t" << (size == 2 ? Helper::wordToHexStringEqu(fromMem) : Helper::byteToHexString(fromMem)) << " -> ";
					if (pcRel) {
						if (r.global) {
							fromMem += (add ? value : -value) - (relOfs + secLC);
						}
						else {
							fromMem += (add ? value : -value) - (relOfs + secLC);
						}
					}
					else {
						if (r.global) {
							fromMem += (add ? value : -value);	// vr globalnog simbola
						}
						else {
							fromMem += (add ? value : -value);	// pomeraj sekcije
						}
					}

					s.encoding.replace(2 * relOfs, 2*size, (size == 2 ? Helper::wordToHexString(fromMem): Helper::byteToHexString(fromMem)));

					//cout << (size == 2 ? Helper::wordToHexStringEqu(fromMem) : Helper::byteToHexString(fromMem)) << "\r\n";

					break;
				}
			}
		}
	}
}

void Linker::mergeDataSections() {

	for (auto& a : sections) {
		datasection.insert(pair<int, DataSection>(a.first, DataSection("")));
	}

	for (InputFile& inf : inputFiles) {
		for (DataSection& ds : inf.datasection) {
			datasection.find(ds.section)->second.encoding += ds.encoding;
		}
	}
}

void Linker::addNumbers() {
	int num = sections.size();
	for (Symbol& s : symbols)
	{
		s.number = num++;
	}
	std::sort(symbols.begin(), symbols.end());
}

void Linker::generateMemory() {
	for (auto& sec : sections) {
		int offs = sec.second.locCnt;
		
		for (auto& ds : datasection) {
			if (ds.first == sec.second.num) {
				for (int i = 0; i < ds.second.encoding.size(); i += 2) {
					memory[offs++] = stoi(ds.second.encoding.substr(i, 2), 0, 16);
				}
			}
		}
	}
}

string Linker::toString() {
	string print = "\r\n#  symbol table\r\n#  name" + string(18, ' ') + "sec  scope   value     num \r\n";

	for (auto s : symbols) {
		print += " " + s.toString() + "\r\n";
	}
	print += "\r\n";

	print += "#  header table\r\n#  num     segment_name        value     size\r\n  ";
	
	for (auto s : sections) {
		print += Helper::wordToHexStringEqu(s.first) + "     " + s.second.name + string(20-s.second.name.length(), ' ')
			+ Helper::wordToHexStringEqu(s.second.locCnt) +"      "+ Helper::wordToHexStringEqu(s.second.size) + "\r\n  ";
	}
	print += "\r\n";
	
	for (auto s : datasection) {
		print += "#  segment " + Helper::wordToHexStringEqu(s.first) + "\r\n";
		print += format(s.second.encoding) + "\r\n\r\n";// +" = " + Helper::wordToHexStringEqu(s.second.encoding.size() / 2) + "\r\n";
	}

//print += "\r\n";

//	for (InputFile input : inputFiles) {
//		print += input.toString() ;
//	}

	/*
	for (int i = 0; i < 0x10000 ; i++) {
		if ((i % 16) == 0) 
		{
			cout << "\r\n" << Helper::wordToHexStringEqu(i) << ": ";
		}
		cout << Helper::byteToHexString(memory[i])<< " ";
	}
	*/
	return print;
}

string Linker::format(string str) {
	string result = "";
	int num = 0;
	for (unsigned i = 0; i < str.length(); i++)
	{
		if (num == 40)
		{
			num = 0;
			result += "\r\n";
		}
		num++;
		result += str[i];
		if (i % 2) result += " ";
	}
	return result;
}

Linker::~Linker() {}
