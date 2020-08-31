#include "InputFile.h"
#include <fstream>


InputFile::InputFile(string l){
	ifstream file(l);
	if (file.fail()) throw "Linker Error: Unable to open file " + l;
	
	lines = vector<string>();
	symbolTable = vector<Symbol>();
	sections = vector<Section>();
	relocations = vector<Relocation>();
	datasection = vector<DataSection>();
	expressions = vector<Expression>();

	string str;
	
	while (!file.eof()) {
		getline(file, str);
		Helper::trim(str);
		if (!str.empty()) lines.push_back(str);
	}

	parseSymbols();
	parseRelocations();
	parseSectionData();
	parseExpression();
}

void InputFile::parseSymbols(){
	bool end = false;
	for (string& line : lines) {
		if (line.at(0) == '=') { 
			if (end) return;
			end = true; continue; 
		}

		int isconst = line.find('~');
		if (isconst != string::npos) 
		{
			line = line.substr(2, line.size());
		}

		int space = line.find_first_of(' ');
		string name = line.substr(0, space);
		line = line.substr(space + 1, line.size());
		
		space = line.find_first_of(' ');
		int sec = stoi(line.substr(0, space), 0, 16);
		line = line.substr(space + 1, line.size());
		
		space = line.find_first_of(' ');
		bool global = (line.substr(0, space) == "global" ? true : false);
		line = line.substr(space + 1, line.size());
		
		space = line.find_first_of(' ');
		int value = stoi(line.substr(0, space), 0, 16);
		line = line.substr(space + 1, line.size());

		space = line.find_first_of(' ');
		int num = stoi(line.substr(0, space), 0, 10);
		line = line.substr(space + 1, line.size());
		
		space = line.find(' ');
		int size = (line.size() < 4 ? -1 : stoi(line.substr(0, space), 0, 16));

		Symbol sym = Symbol(name, value, global, sec);
		sym.number = num;
		sym.constant = (isconst != string::npos ? true : false);
		sym.sec = (size >= 0 ? true : false);
		if (sym.sec) sym.size = size;
		symbolTable.push_back(sym);
		if (sym.sec) {
			Section sec = Section(name, num);
			sec.size = size;
			sec.locCnt = 0;
			sections.push_back(sec);
		}
	}
}

void InputFile::parseRelocations() {
	bool rel = false;
	string sec = "";
	for (string& line : lines) {
		if (line.find(".rel") != string::npos) {
			rel = true; 
			int t = line.find_last_of('.');
			sec = line.substr(t+1, line.size());
		}
		else if (rel) {
			int space = line.find_first_of(' ');
			int offset = stoi(line.substr(0, space), 0, 16);
			line = line.substr(space + 1, line.size());

			space = line.find_first_of(' ');
			bool pcRel = (line.substr(0, space) == "R_386_PC16" ? true : false );
			line = line.substr(space + 1, line.size());

			space = line.find_first_of(' ');
			int ref = Helper::strToInt(line);
			line = line.substr(space + 1, line.size());


			space = line.find_first_of(' ');
			bool add = line.find('+') != string::npos;
			line = line.substr(space + 1, line.size());

			int size = Helper::strToInt(line);
			
			Relocation r = Relocation(0, offset, "", pcRel);
			r.ref = ref;
			r.add = add;
			r.size = size;
			for (Section& s : sections) {
				if (s.name == sec) {
					r.section = s.num;
					break;
				}
			}
			relocations.push_back(r);
		}
	}
}

void InputFile::parseSectionData() {
	bool rel = false;
	int sec = 0;
	string data = "";
	int i = 0;
	for (string& line : lines) {
		i++;
		if (line.find("=> .") != string::npos) {
			if (rel)
			{
				DataSection ds = DataSection(data);
				ds.section = sec;
				datasection.push_back(ds);
				data = "";
			}
			if (line.find(".rel .") != string::npos) break;

			rel = true;
			int t = line.find_last_of('.');
			for (Section& s : sections) {
				if (s.name == line.substr(t + 1, line.size())) {
					sec = s.num;
					break;
				}
			}
		}
		else if (rel) {
//			line.erase(remove_if(line.begin(), line.end(), isspace), line.end());	//obrisi sve razmake 
			line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

			
			data += line;
			if (i == lines.size()) {			
				DataSection ds = DataSection(data);
				ds.section = sec;
				datasection.push_back(ds);
			}
		}
	}
}

void InputFile::parseExpression() {
	bool rel = false;
	int sec = 0;
	int value = 0;
	string sym = "";
	string data = "";

	for (string& line : lines) {
		if (line.find("classification") != string::npos) {
			rel = true;
		}
		else if (rel){
			if (line.find("=>") != string::npos) break; 

			int t = line.find(':');
			if (t != string::npos) 
			{
				sym = line.substr(0, t);
				line = line.substr(t + 1, line.size());
				int space = line.find_first_of(' ');
				value = Helper::strToInt(line.substr(0, space));
				line = line.substr(space + 1, line.size());
			}

			int space = line.find_first_of(' ');
			int sec = Helper::strToInt(line.substr(0, space));
			line = line.substr(space + 1, line.size());
			
			space = line.find_first_of(' ');
			int index = Helper::strToInt(line.substr(0, space));
			line = line.substr(space + 1, line.size());
			
			string expr = line;
			if (index == 1) 
			{
				Expression e = Expression(sym, value);
				e.elemList.push_back(ExpressionElem(expr.substr(1, expr.size()), expr.at(0) == '+'));
				expressions.push_back(e);

				for (Symbol& s : symbolTable) {
					if (s.name == sym) {
						s.expression = e;
					}
				}

			}
		}
	}
}

bool InputFile::resolveSections() {
	bool res = true;
	for (Section& s : sections) res &= s.linker;
	return res;
}

string InputFile::toString() {
	string print = "";
	for (Symbol& s : symbolTable) {
		print += to_string(s.linkerNum) + ".\t" + s.toString() + Helper::wordToHexStringEqu(s.size) + "\r\r\n";
	}

	for (Section& s : sections) {
		print += (s.name +" "+ to_string(s.num) + " " + Helper::wordToHexStringEqu(s.locCnt) + " " + Helper::wordToHexStringEqu(s.size) + "\r\r\n");
	}

	for (Relocation& r : relocations) {
		print += r.toString() + "\tsekcija: " + to_string(r.section) + "\r\r\n";
	}

	for (DataSection& ds : datasection) {
		print += to_string(ds.section) + ":\r\r\n" + ds.encoding + "\r\r\n";
	}

	for (Expression& e : expressions) {
		print += e.symbolName + " = " + to_string(e.numResult);
		for (ExpressionElem ee : e.elemList) {
			print += ee.toStringRel();
		}
		print += "\r\r\n";
	}

	return print;
}

InputFile::~InputFile() {}