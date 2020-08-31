#include "Assembler.h"
#include <fstream>

int Assembler::activeSection = 0;

map<string, pair<int, int>> Assembler::instructions = {
	{ "halt", {0x00, 0}},
	{ "iret", {0x01, 0 }},
	{ "ret", {0x02, 0 }},
	{ "int", {0x03, 1 }},
	{ "call",{0x04, 1 }},
	{ "jmp",{0x05, 1 }},
	{ "jeq",{0x06, 1 }},
	{ "jne",{0x07, 1 }},
	{ "jgt",{0x08, 1 }},
	{ "push",{0x09, 1 }},
	{ "pop", {0x0a, 1 }},
	{ "xchg", {0x0b, 2 }},
	{ "mov", {0x0c, 2 }},
	{ "add", {0x0d, 2 }},
	{ "sub", {0x0e, 2 }},
	{ "mul", {0x0f, 2 }},
	{ "div", {0x10, 2 }},
	{ "cmp", {0x11, 2 }},
	{ "not", {0x12, 2 }},
	{ "and", {0x13, 2 }},
	{ "or", {0x14, 2 }},
	{ "xor", {0x15, 2 }},
	{ "test", {0x16, 2 }},
	{ "shl", {0x17, 2 }},
	{ "shr", {0x18, 2 }}
};
map<string, int> Assembler::addressing = {
	{"imm", 0x0},
	{"reg", 0x1},
	{"regind", 0x2},
	{"regind16", 0x3},
	{"mem", 0x4}
};
map<string, int> Assembler::registers = {
	{"r0", 0x0},
	{"r1", 0x1},
	{"r2", 0x2},
	{"r3", 0x3},
	{"r4", 0x4},
	{"r5", 0x5},
	{"r6", 0x6},
	{"r7", 0x7},
	{"pc", 0x7},
	{"sp", 0x6},
	{"psw", 0xF}
};


Assembler::Assembler(string in, string out) :currSection("undefined", 0)
{
	string pathToFileAsm = "../tests/";
	string pathToFileEm = "../../Emulator/tests/";
	inFile = in;
	outFile = out;

	currentSection = 0;
	locCnt = 0;
	sections.insert(pair<int, Section>(0, currSection));

	lines = vector<string>();
	symbolTable = vector<Symbol>();
	sectionsAll = vector<Section>();
	instructionList = vector<Instruction>();
	relocationTable = vector<Relocation>();
	equSymbols = vector<EquSymbol>();
	expressionList = vector<Expression>();

	ReadFile(inFile);
	Pass();
	PrintFile(pathToFileAsm + outFile);
	PrintFile(pathToFileEm + outFile);

}

Assembler::~Assembler() {}

void Assembler::ReadFile(string filePath) {
	ifstream file(filePath);

	if (file.fail()) {
		throw "Assembler Error: Unable to open file " + filePath;
	}

	string str;
	bool concatenate = false;
	string baseStr;
	while (!file.eof()) {
		getline(file, str);
		Helper::trim(str);
		if (str.empty()) continue;
		if (str == ".end" || Helper::endsWith(str, ".end")) {
			if (concatenate) { str = baseStr + str; }
			lines.push_back(str);
			return;
		}
		if (Helper::endsWith(str, ":")) {	//kraj labele
			if (concatenate) { baseStr += str; }
			else {
				concatenate = true;
				baseStr = str;
			}
		}
		else {
			if (concatenate) {
				str = baseStr + " " + str;
				concatenate = false;
			}
			lines.push_back(str);
		}
	}
	lines.push_back(".end");
};

void Assembler::PrintFile(string filePath) {
	ofstream outFile(filePath);
	outFile << printSymbolTable()
			<< printExpression()
			<< printSectionData()
		//	<< printInstructions()
			<< printRelocation();
	outFile.close();
}

//check

bool Assembler::isDefined(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			if (s.defined && s.declared) return true;
		}
	}
	return false;
}

bool Assembler::isDeclared(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			if (!s.defined && s.declared) return true;
		}
	}
	return false;
}

bool Assembler::isExtern(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.ex;
		}
	}
	return false;
}

bool Assembler::isGlobal(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.global;
		}
	}
	return false;
}

bool Assembler::isConst(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.constant;
		}
	}
	return false;
}



//getters

Expression Assembler::getSymExpr(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.expression;
		}
	}
}

Symbol Assembler::getSymbol(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s;
		}
	}
	throw "Assembler error: Undefined symbol " + symbolName;
}

int Assembler::getSymbolNumber(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.number;
		}
	}
	return -1;
}

int Assembler::getSymbolValue(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.value;
		}
	}
	return 0;
}

int Assembler::getSymbolSection(string symbolName) {
	for (Symbol s : symbolTable) {
		if (s.name == symbolName) {
			return s.section;
		}
	}
	return 0;
}


//symbols

void Assembler::updateGlobalSym(string symbolName) {
	for (Symbol& s : symbolTable) {
		if (s.name == symbolName) {
			if (!s.ex)
			{
				s.global = true;
				s.defined = true;
				s.declared = true;
			}
			else throw "Assembler Error: Multiple definition of symbol " + symbolName;
		}
	}
}

void Assembler::updateSymbol(string symbolName, int val, bool ex, bool con) {
	for (Symbol& s : symbolTable) {
		if (s.name == symbolName) {
			if (s.defined) throw "Assembler Error: Multiple definition of symbol " + symbolName;
			s.section = Assembler::activeSection;
			s.value = val;
			s.defined = true;
			s.declared = true;
			s.constant = con;
			if (ex) { s.ex = true; s.global = true; s.section = 0; }
		}
	}
}

void Assembler::addSymbol(bool sec, string symbolName, int val, bool gl, bool setSection, bool def, bool decl, bool ex, bool con) {

	if (isDeclared(symbolName)) {
		throw "Assembler Error: Multiple definition of symbol " + symbolName;
	}

	if (isDefined(symbolName)) {
		throw "Assembler Error: Multiple definition of symbol " + symbolName;
	}

	//obrisan if
	Symbol s = Symbol(symbolName, val, gl, setSection ? Assembler::activeSection : 0, def, decl, ex);
	s.sec = sec;
	s.constant = con;
	symbolTable.push_back(s);

}

void Assembler::addEquGlobalSymbol(string symbolName, int val, string equSym, bool constSym) {
	
	ExpressionElem ee(equSym, true);
	Expression e(symbolName, 0);
	e.elemList.push_back(ee);

	if (!getSymExpr(equSym).elemList.empty()) {
		e = getSymExpr(equSym);
	}

	for (Symbol& s : symbolTable) {
		if (s.name == symbolName) {
			if (s.defined) throw "Assembler Error: Multiple definition of symbol " + symbolName;
			if (s.declared) 
			{	// update
				s.section = Assembler::activeSection;
				s.value = val;
				s.defined = true;
				s.declared = true;
				s.expression = e;
				expressionList.push_back(e);
				return;
			}
		}
	}
	Symbol s = Symbol(symbolName, val, false, Assembler::activeSection, true, true);
	s.expression = e;
	s.constant = constSym;
	expressionList.push_back(e);
	symbolTable.push_back(s);
}

void Assembler::addEquExprSymbol(string symbolName, int val, Expression e) {
	for (Symbol& s : symbolTable) {
		if (s.name == symbolName) {
			if (s.defined) throw "Assembler Error: Multiple definition of symbol " + symbolName;
			if (s.declared)
			{	// update
				s.section = Assembler::activeSection;
				s.value = val;
				s.defined = true;
				s.declared = true;
				s.expression = e;
				expressionList.push_back(e);
				return;
			}
		}
	}
	Symbol s = Symbol(symbolName, val, false, Assembler::activeSection, true, true);
	s.expression = e;
	expressionList.push_back(e);
	symbolTable.push_back(s);
}


//symbol numbers

void Assembler::addSymbolNumbers() {
	int num = activeSection + 1;
	for (Symbol& s : symbolTable)
	{	
		if (!s.defined) throw "Assembler error: Undefined symbol " + s.name;
		if (!s.expression.resolve()) {
			for(ExpressionElem ee: s.expression.elemList) 
				if(!ee.resolve) throw "Assembler error: Undefined symbol " + ee.symElemName;
		}
		if (!s.sec) s.number = num++;
		else  s.number = s.section;
	}
	std::sort(symbolTable.begin(), symbolTable.end());
}


//relocations

void Assembler::doRelocations() {
	for (Section& s : sectionsAll) {
		for (DataSection& ds : s.data) {
			if (ds.undef) ds.encoding = singleRelocation(ds.rel);
		}
	}
}

string Assembler::singleRelocation(Relocation rel) {

	Symbol sym = getSymbol(rel.symbolName);

	int value = sym.value;
	int section = sym.section;
	int number = sym.number;
	bool constSym = sym.constant;
	rel.global = sym.global;


	if (rel.section == 0) return "";

	//	za konstante nema relokacija
	//	upisuje se value 
	if (constSym)
	{
		return (rel.size == 1) ? Helper::byteToHexString(value) : Helper::wordToHexString(value);
	}

	//	ako je simbol lokalan 
	//	i definisan u istoj sekciji
	//	i instrukcija je skok na adresu simbola, tj jmp <symbol>
	//	onda ne treba relokacija!!!
	//	VEC POMERAJ DO TOG SIMBOLA!
	if (rel.section == section && !rel.global && rel.jmpInstr && rel.pcRel && sym.expression.index.empty())
	{
		//	jmp d		14: 74 ** **	pomeraj je 00 00 => 17 - (14 + 3)  
		//	d:			17:
		int pomeraj = value - rel.offset - rel.size;
		return (rel.size == 1) ? Helper::byteToHexString(pomeraj) : Helper::wordToHexString(pomeraj);
	}

	//	pravi relokacije za one simbole koji nisu dobijeni .equ
	//	klasicno
	if (sym.expression.index.empty())
	{
		if (rel.pcRel)				//	pc rel
		{
			if (!rel.global)		//lokalan simbol + pc rel 
									//vrednost simbola - size_of_symbol (do slededce instrukcije !!!!!)
			{

				rel.op = (rel.size == 1) ? Helper::byteToHexString(value - 1 - rel.valueForPcRel) : Helper::wordToHexString(value - 2 - rel.valueForPcRel);
				rel.ref = section;
				relocationTable.push_back(rel);
			}
			else					//globalan simbol + pc rel 
									// - size_of_symbol
			{
				rel.op = (rel.size == 1) ? Helper::byteToHexString(-1 - rel.valueForPcRel) : Helper::wordToHexString(-2 - rel.valueForPcRel);
				rel.ref = number;				//globalni simbol relokacija referise na sam simbol
				relocationTable.push_back(rel);
			}
		}
		else {						// abs rel

			if (!rel.global)		//lokalan simbol + abs rel 
			{
				rel.op = (rel.size == 1) ? Helper::byteToHexString(value) : Helper::wordToHexString(value);
				rel.ref = section;
				relocationTable.push_back(rel);
			}
			else					//globalan simbol + abs rel 
									//asembler popuni nulama, linker ce da razresi
			{
				rel.op = (rel.size == 1) ? "00" : "0000";
				rel.ref = number;				//globalni simbol relokacija referise na sam simbol
				relocationTable.push_back(rel);
			}
		}
		return rel.op;
	}

	//	ako su dobijeni .equ i imaju izraz
	else
	{
		string relname = rel.symbolName;
		for (auto& elem : sym.expression.index) {
			for (auto& ee : elem.second.elems) {
				if (ee.rel) {
					rel.add = ee.add;
					Symbol elemRel = getSymbol(ee.symElemName);
					
					for (Symbol& sss : symbolTable) {
						if (sss.name == sym.name) {
							sss.section = elemRel.section;
							break;
						}
					}

						//	ako je simbol relokatibilan tj ima izraz
						//	proveriti da li je taj simbol iz izraza koji je relokatibilan
						//	definisan u okviru iste sekcije i pc rel je
					if (rel.section == elemRel.section && /*!elemRel.global &&*/ rel.jmpInstr && rel.pcRel)
					{
						//	jmp d		14: 74 ** **	pomeraj je 00 00 => 17 - (14 + 3)  
						//	d:			17:
						int pomeraj = elemRel.value - rel.offset - rel.size;
						return (rel.size == 1) ? Helper::byteToHexString(pomeraj) : Helper::wordToHexString(pomeraj);
					}

					int constOfs = sym.expression.numResult;
					
					rel.symbolName = (relname + ": R(" + ee.symElemName +")");
					if (rel.pcRel)				//	pc rel
					{
						if (!elemRel.global)	//lokalan simbol + pc rel 
												//vrednost simbola - size_of_symbol
						{
							rel.op = (rel.size == 1) ? Helper::byteToHexString(constOfs + elemRel.value - 1 - rel.valueForPcRel) : Helper::wordToHexString(constOfs + elemRel.value - 2 - rel.valueForPcRel);
							rel.ref = elemRel.section;
							relocationTable.push_back(rel);
						}
						else					//globalan simbol + pc rel 
												// - size_of_symbol
						{
							rel.op = (rel.size == 1) ? Helper::byteToHexString(constOfs - 1 - rel.valueForPcRel) : Helper::wordToHexString(constOfs - 2 - rel.valueForPcRel);
							rel.ref = elemRel.number;				//globalni simbol relokacija referise na sam simbol
							relocationTable.push_back(rel);
						}
					}
					else {						// abs rel

						if (!elemRel.global)	//lokalan simbol + abs rel 
						{
							rel.op = (rel.size == 1) ? Helper::byteToHexString(constOfs + elemRel.value) : Helper::wordToHexString(constOfs + elemRel.value);
							rel.ref = elemRel.section;
							relocationTable.push_back(rel);
						}
						else					//globalan simbol + abs rel 
												//asembler popuni nulama, linker ce da razresi
						{
							rel.op = (rel.size == 1) ? Helper::byteToHexString(constOfs) : Helper::wordToHexString(constOfs);
							rel.ref = elemRel.number;				//globalni simbol relokacija referise na sam simbol
							relocationTable.push_back(rel);
						}
					}
					
					
					return rel.op;
				}
			}
		}
	}
}


//equ

void Assembler::doEqus(){
	
	while (true)
	{
		if(doEquSymbols() && doExpressions()) break;
	}
	//provera nedefinisanih equ simbola
	if (!equSymbols.empty()) {
		throw "Assembler error: Undefined symbol " + equSymbols.at(0).symbol2;
	}

	for (Symbol& s : symbolTable) {
		s.expression.symbolName = s.name;
		if(!s.expression.elemList.empty()) doIndexExpression(s.expression);
		
		if (s.expression.constant) {
			s.constant = true;
			s.value = s.expression.numResult;
		}
		if (s.expression.elemList.empty()) {}	
	}

	for (Symbol& s : symbolTable) {
		for (auto& elem : s.expression.index)
		{
			s.expression.numResult += elem.second.offset;
		}
	}

}

bool Assembler::doEquSymbols() {
	//	na kraju obrade u listi ostaju samo oni sa nedefinisanim .equ simbolom
	
	bool end = true;
	int i = 0;
	int num = 1;
	while (num != 0) {
		num = 0;
		end = true;
		i = 0;
		for (EquSymbol& es : equSymbols) {
			if (!es.resolve && singleEquSymbol(es))
			{
				num++;
				es.resolve = true;
				end = false;
				
			}
		}
		for (EquSymbol& es : equSymbols) {
			if (es.resolve) equSymbols.erase(equSymbols.begin() + i);
			else i++;
		}
		if (end) return end;
	}

//	ne mora da znaci -> moraju ovi equ i expr da se izvrse vise puta
//	for (EquSymbol& es : equSymbols) {
//		if (es.resolve == false) throw "Assembler error: Undefined symbol " + es.symbol2;
//	}
	return false;
}

bool Assembler::singleEquSymbol(EquSymbol es) {
	if (isGlobal(es.symbol2)) { 
		addEquGlobalSymbol(es.symbol1, getSymbolValue(es.symbol2), es.symbol2, isConst(es.symbol2)); 
		return true;
	}

	else if (isDefined(es.symbol2)) {
		bool constSym = isConst(es.symbol2);
		if (isDeclared(es.symbol1)) { updateSymbol(es.symbol1, getSymbolValue(es.symbol2), false, constSym); }
		else { addSymbol(false, es.symbol1, getSymbolValue(es.symbol2),false,true,true,true,false,constSym); }

		Expression e(es.symbol1, 0);
		if (!getSymExpr(es.symbol2).elemList.empty()) {
			e = getSymExpr(es.symbol2);
		}
		//dodato ako nije konst
		else if(!constSym){
			e.elemList.push_back(ExpressionElem(es.symbol2, true));
		}
		for (Symbol& s : symbolTable) {
			if (s.name == es.symbol1) {
				s.expression = e;
				break;
			}
		}
		return true;
	}
	return false;
}

bool Assembler::doExpressions() { 
	
	bool end = true;
	int i = 0;
	int num = 1;
	while (num != 0) {
		num = 0;
		end = true;
		i = 0;
		for (Symbol& s : symbolTable) {
			if (!s.expression.resolve() && singleExpression(s.expression))
			{
				end = false;
				num++;
			}
			else i++;
		}
		if (end) return end;
	}
	return false;
}

bool Assembler::singleExpression(Expression& e){
	bool check = false;
	int i = 0;
	for (ExpressionElem& ee : e.elemList) {
		bool sign = ee.add;
		if (ee.symElemName == e.symbolName) throw "Assembler error: Cyclic dependency detected. Undefined symbol " + e.symbolName;
		
		if (isDefined(ee.symElemName) && getSymExpr(ee.symElemName).elemList.empty()) {
			ee.resolve = true;
		}
		else if (isDefined(ee.symElemName) && !getSymExpr(ee.symElemName).elemList.empty()) {
			Expression pom = getSymExpr(ee.symElemName);
			e.numResult += (ee.add ? pom.numResult : -pom.numResult);
			
			for (ExpressionElem eel : pom.elemList) {
				bool res = (sign) ? eel.add : (eel.add ? sign : true);
				ExpressionElem exprElem(eel.symElemName, res);
				e.elemList.push_back(exprElem);
			}
			check = true;
			e.elemList.erase(e.elemList.begin() + i);
			return true;
		}
		i++;
	}

	return check;
}


//index 

void Assembler::doIndexExpression(Expression& e) {
	for (ExpressionElem& elem : e.elemList) {
		Symbol s = getSymbol(elem.symElemName);

			//konstantu samo dodam na result i brisem je iz izraza
		if (s.constant) {
			e.numResult += (elem.add ? s.value : -s.value);
			elem.constant = true;
		}
		else if (e.index.count(s.section) == 0) {
				e.index.insert(pair<int, Index>(s.section, Index(s.ex ? 1 : (elem.add ? 1 : -1))));
				std::map<int, Index>::iterator it = e.index.find(s.section);
				elem.val = s.value;
				it->second.elems.push_back(elem);
			}
		else {
			std::map<int, Index>::iterator it = e.index.find(s.section);
			it->second.value += (s.ex ? 1 : (elem.add ? 1 : -1));
			elem.val = s.value;
			it->second.elems.push_back(elem);
		}
	}

	auto it = e.elemList.begin();
	while (it != e.elemList.end()) {
		if (it->constant) it = e.elemList.erase(it);
		else ++it;
	}
	
	bool check = false;
	bool wasExtern = false;
	int constValue = 0;
	for (auto& elem : e.index)
	{
		if (elem.second.value < 0 || elem.second.value > 1) 
			throw "Assembler error: Invalid expression for equ symbol " + e.symbolName + ". Index value must be 0 or 1.\n# classification index table\n# symbol             section   index\n" + e.indexToString();
		else if (elem.second.value != 0) {
			if (check) throw "Assembler error: Invalid expression for equ symbol " + e.symbolName + ". Only one index can be 1.\n# classification index table\n# symbol             section   index\n" + e.indexToString();
			check = true;
			bool plus = false;

			for (ExpressionElem& ee : elem.second.elems) {
				//ne za extern 
				if (elem.first != 0)
				{
					if (!plus && ee.add) { plus = true; ee.rel = true; }
					elem.second.offset += (ee.add ? ee.val : -ee.val);
				}
				else {
					wasExtern = true;
					ee.rel = true;
				}
			}

		}
		else {
			for (ExpressionElem& ee : elem.second.elems) {
				//ne za extern 
				if (elem.first != 0) elem.second.offset += (ee.add ? ee.val : -ee.val);
				else {
					ee.rel = true; wasExtern = true;
				}
			}
		}
		constValue += elem.second.offset;
	}
	
	int con = 0;
	for (auto& elem : e.index)
	{
		con += elem.second.value;
	}

	// ako nema ulaz za extern simbole wasExtern == false
	// ako su svi ulazi nula check == false
	// znaci offset ce biti value simbola + simbol je konstanta
	if (/*!wasExtern && !check*/ con == 0 /*&& !e.index.empty()*/) {
		e.constant = true;
		e.numResult += constValue;
		//cout << "OVAJ JE KONST " << e.symbolName << endl;
	}

}



void Assembler::Pass() {

	for (string line : lines) {

		if (line.find(".end") != string::npos)
		{
			currSection.size = locCnt;
			sections.insert(pair<int, Section>(currSection.num, currSection));
			sectionsAll.push_back(currSection);
			break;
		}
		int delimiter = line.find(":");

		if (delimiter != string::npos) {
			string label = line.substr(0, delimiter);

					//		dodaj simbol <label>
					//		ako je vec deklarisan i nije definisan onda updateSymbol (.global je bilo)
					//		inace addSymbol
			if (currentSection == 0) throw "Assembler error: Label: " + label + " cannot be defined out of section.";

			if (isDeclared(label)) updateSymbol(label, locCnt); 
			else addSymbol(false, label, locCnt);

			// !!!! novi definisani simbol
			// !!!! razresiti gde treba 
			// !!!! razresavanje u petlji -> vrti se dok ima promena 

			line = line.substr(delimiter + 1, line.size());
			Helper::trim(line);
		}

		if (line[0] == '.') {
			parseDirective(line);
		}
		else {
			if (currentSection == 0) throw "Assembler error: Instruction: " + line +" cannot be out of section.";
			parseInstruction(line);
		}
	}
	doEqus();
	addSymbolNumbers();
	doRelocations();
}



//directive

void Assembler::parseDirective(string line) {
	int delimiter = line.find_first_of(" ");

	if (delimiter == string::npos) {
		throw "Assembler Error: Directive " + line + " without value";
	}
	string value = line.substr(delimiter, line.size());
	Helper::trim(value);

	if (line.find(".section") != string::npos) {
		dirSection(value);
	}
	else if (line.find(".global") != string::npos) {
		dirGlobal(value);
	}
	else if (line.find(".extern") != string::npos) {
		dirExtern(value);
	}
	else if (line.find(".byte") != string::npos) {
		dirByte(value);
	}
	else if (line.find(".word") != string::npos) {
		dirWord(value);
	}
	else if (line.find(".skip") != string::npos) {
		dirSkip(value);
	}
	else if (line.find(".equ") != string::npos) {
		dirEqu(value);
	}
	else throw "Assembler Error: Unknown directive " + line;
}

void Assembler::dirSection(string value) {
	currSection.size = locCnt;
	locCnt = 0;
	Assembler::activeSection = ++currentSection;

	addSymbol(true, value, locCnt);
	
	sections.insert(pair<int, Section>(currSection.num, currSection));
	sectionsAll.push_back(currSection);

	Section newSec = Section(value, currentSection);
	currSection = newSec;
}

void Assembler::dirGlobal(string value) {
	int end = 0;
	do
	{
		end = value.find(",");
		string symbolName = value.substr(0, end);
		Helper::trim(symbolName);

		// moze da bude definisan
		// a: ...
		// .global a
		if (isDefined(value)) updateGlobalSym(value);

		// ne moze da bude samo deklarisan
		// .global x  -> deklarisan x
		// .global x  -> greska !!!
		else if (isDeclared(value)) throw "Assembler Error: Multiple definition of symbol " + symbolName;

		// dodajem ga u tabelu simbola kao samo deklarisanog
		//				sec,		name,    val,  gl, setsec,   def, decl
		else addSymbol(false, symbolName, locCnt, true, false, false, true);

		value = value.substr(end + 1, value.size());

	} while (end != string::npos);
}

void Assembler::dirExtern(string value) {
	int end = 0;
	do
	{
		end = value.find(",");
		string symbolName = value.substr(0, end);
		Helper::trim(symbolName);
		//addSymbol(sec, symbolName,   val,              gl,              setSection, def,  decl,  ex) {
		addSymbol(false, symbolName, locCnt /*ili 0*/, true, false /*sekcija undef*/, true, true, true);
		value = value.substr(end + 1, value.size());

	} while (end != string::npos);

}

void Assembler::dirSkip(string value) {
	if (!Helper::isNumber(value))
	{
		throw "Assembler Error: .skip " + value + " is not valid. Expected number.";
	}
	int num = Helper::strToInt(value);
	locCnt += num;
	string data;
	while (num)
	{
		data += "00"; num--;
	}
	currSection.data.push_back(DataSection(data));
}

void Assembler::dirByte(string value) {
	int end = 0;
	int num = 0;

	do
	{
		end = value.find(",");
		string s = value.substr(0, end);
		Helper::trim(s);
		value = value.substr(end + 1, value.size());

		// .byte <broj>
		if (Helper::isNumber(s))
		{
			num = Helper::strToInt(s);
			currSection.data.push_back(DataSection(Helper::byteToHexString(num)));
		}
		//	.byte <simbol>
		//	bilo da je simbol definisan ili ne pravim relokaciju
		//	kasnije ce se kod obrade relokacija visak obrisati 
		//  ***  //
		else
		{
			Relocation rel = Relocation(currentSection, locCnt, s);
			rel.size = 1;
			rel.wordOrByte = true;
			DataSection ds = DataSection("", true);
			ds.rel = rel;
			currSection.data.push_back(ds);
		}

		//na kraju uvecam lc jer je on offset za simbole
		locCnt++;

	} while (end != string::npos);
}

void Assembler::dirWord(string value) {
	int end = 0;
	int num = 0;

	do
	{
		end = value.find(",");
		string s = value.substr(0, end);
		Helper::trim(s);
		value = value.substr(end + 1, value.size());

		// .byte <broj>
		if (Helper::isNumber(s))
		{
			num = Helper::strToInt(s);
			currSection.data.push_back(DataSection(Helper::wordToHexString(num)));
		}
		// .byte <simbol>
		//  ***  //
		else
		{
			Relocation rel = Relocation(currentSection, locCnt, s);
			rel.wordOrByte = true;
			DataSection ds = DataSection("", true);
			ds.rel = rel;
			currSection.data.push_back(ds);
		}
	
		locCnt += 2;
	
	} while (end != string::npos);
}

void Assembler::dirEqu(string value) {
	
	string e = value;
	int end = value.find(",");
	if (end == string::npos) throw "Assembler error: Invalid format .equ " + value + ". Expected  .equ <symbol>, <expression>";

	string symbol = value.substr(0, end);
	Helper::trim(symbol);
	value = value.substr(end + 1, value.size());
	Helper::trim(value);


		// .equ <symbol>, broj		
		// dodaje se simbol u tabelu simbola kao konstanta!
	if (Helper::isNumber(value))
	{
		if (isDeclared(symbol)) updateSymbol(symbol, Helper::strToInt(value), false, true);
		else addSymbol(false, symbol, Helper::strToInt(value),false, true, true, true,false,true);
		return;
	}


	bool sign = (value.find("+") != string::npos || value.find("-") != string::npos) ? true : false;
	bool defined = isDefined(value);
	bool externSym = isExtern(value);
	bool globalSym = isGlobal(value);
	bool constSym = isConst(value);

		//	za lokalne
	if (!sign && defined && !globalSym)
	{				
		if (isDeclared(symbol)) updateSymbol(symbol, getSymbolValue(value), false, constSym);
		else addSymbol(false, symbol, getSymbolValue(value), false, true, true, true, false, constSym);
	
		if (!getSymExpr(value).elemList.empty()) {
			Expression e = getSymExpr(value);
			for (Symbol& s : symbolTable) {
				if (s.name == symbol) {
					s.expression = e;
					expressionList.push_back(e);
					break;
				}
			}
		}
		//ako je prazan dodaj sam taj simbol jer je relokatibilan! 
		else {
			for (Symbol& s : symbolTable) {
				if (s.name == symbol) {
					Expression e(symbol, 0);
					e.elemList.push_back(ExpressionElem( value,true));
					s.expression = e;
					break;
				}
			}

		}
		return;
	}
		//	za globalne
	if (!sign && defined && globalSym) 
	{
		addEquGlobalSymbol(symbol, locCnt, value, constSym);
		return;
	}
		//	za nedefinisane
	if (!sign && !defined)
	{
		if (symbol == value) throw "Assembler error: Symbol " + symbol + " not defined. Invalid format .equ " +symbol+", "+value;
		EquSymbol es = EquSymbol(symbol, value);
		equSymbols.push_back(es);
		return;
	}

	bool add = true;
	bool onlyNums = true;
	int plus = 0, minus = 0, result = 0;
	string elem = "";

	Expression expr(symbol, result);

//	value.erase(std::remove_if(value.begin(), value.end(), std::isspace), value.end());	//obrisi sve razmake 
	
//	std::string::iterator end_pos = std::remove(value.begin(), value.end(), ' ');

	value.erase(std::remove(value.begin(), value.end(), ' '), value.end());

	if (value.at(0) == '+') { add = true; value = value.substr(1, value.size());}
	else if (value.at(0) == '-') { add = false; value = value.substr(1, value.size());}
	
	do
	{
		plus = value.find_first_of("+");
		minus = value.find_first_of("-");
		if (plus == string::npos) { elem = value.substr(0, minus); end = minus; }
		else if (minus == string::npos) { elem = value.substr(0, plus); end = plus; }
		else { end = (minus < plus) ? minus : plus;  elem = value.substr(0, end); }
		
		if (elem[0] == '+' || elem[0] == '-' || elem.empty()) throw "Assembler error: Invalid format .equ " + e;


		if (Helper::isNumber(elem))			// +-<broj>
		{
			result += add ? Helper::strToInt(elem) : -Helper::strToInt(elem);
		}
		else								// +-<simbol>
		{
			onlyNums = false; 
			if (isDefined(elem) && !getSymExpr(elem).elemList.empty()) {
				Expression pom = getSymExpr(elem);
				expr.numResult += (add ? pom.numResult : -pom.numResult);
				for (ExpressionElem ee : pom.elemList) {
					ExpressionElem exprElem(ee.symElemName, !(ee.add ^ add));
					expr.elemList.push_back(exprElem);
				}
			}
			else {
				ExpressionElem exprElem(elem, add);
				expr.elemList.push_back(exprElem);
			}
		}

		if (end != string::npos) {
			if (value.at(end) == '+') add = true;
			else if (value.at(end) == '-') add = false;
			value = value.substr(end + 1, value.size());
		}
	} while (end != string::npos);

	if (onlyNums) 
	{
		if (isDeclared(symbol)) updateSymbol(symbol, result);
		else addSymbol(false, symbol, result);
		return;
	}
	else 
	{
		expr.numResult += result;
		addEquExprSymbol(symbol, locCnt, expr);
	}	
}



//instruction

void Assembler::parseInstruction(string line) {
	Instruction i = NULL;

	isInstrOk(line);

	int numOp = numOpInstr(line);
	int opCode = instrOpCode(line);
	bool isByte = isByteOp(line);
	string l = line;
	InstrDescrType instrType = static_cast<InstrDescrType>(opCode);

	unsigned char firstByte = getFirstByteOfInstruction(opCode, isByte);
	currSection.data.push_back(DataSection(Helper::byteToHexString(firstByte)));
	int numBytes = 1;

	//0 operanada
	if (numOp == 0)
	{
		if (line.find(' ') != string::npos || line.find(',') != string::npos)
		{
			throw "Assembler error: Invalid instruction format " + line + ". No operand expected.";
		}
		i = Instruction(locCnt, numBytes, numOp, isByte);
		i.firstByte = firstByte;
		i.instrType = instrType;
		i.line = l;
		instructionList.push_back(i);

	}

	//1 operand
	if (numOp == 1) {
		if (line.find(' ') == string::npos || line.find(',') != string::npos)
		{
			throw "Assembler error: Invalid instruction format " + line + ". One operand expected.";
		}
		string operand = line.substr(line.find(' ') + 1, line.size()); Helper::trim(operand);

		i = Instruction(locCnt, numBytes, numOp, isByte);
		i.firstByte = firstByte;
		i.instrType = instrType;
		i.line = l;

		i.firstOp = getOperand(operand, i, numBytes);

		instructionList.push_back(i);
	}

	//2 operanda
	if (numOp == 2) {
		if (line.find(',') == string::npos)
		{
			throw "Assembler error: Invalid instruction format " + line + ". Two operands expected.";
		}
		
		line = line.substr(line.find(' ') + 1, line.size());
		string first = line.substr(0, line.find(',')); Helper::trim(first);
		string sec = line.substr(line.find(',') + 1, line.size()); Helper::trim(sec);

		i = Instruction(locCnt, numBytes, numOp, isByte);
		i.firstByte = firstByte;
		i.instrType = instrType;
		i.line = l;

		i.firstOp = getOperand(first, i, numBytes);
		if (i.instrType == SHR && i.firstOp.addrType == IMM) throw "Assembler Error: Invalid instruction " + i.line + ". Immediate addressing mode not allowed for destination operand.";
		
	//	if (i.firstOp.pcRel) cout << i.line << endl;
											//prvi operand od dva!!!! ako je prvi pc rel
		i.secOp = getOperand(sec, i, numBytes, i.firstOp.pcRel);
	//	if (i.firstOp.pcRel) cout << to_string(currSection.data[currSection.data.size() - 1].rel.valueForPcRel) << " " << currSection.data[currSection.data.size() - 1].rel.toString() << endl;

		if (i.instrType >= XCHG && i.instrType <= SHL && i.secOp.addrType == IMM) throw "Assembler Error: Invalid instruction " + i.line + ". Immediate addressing mode not allowed for destination operand.";

		instructionList.push_back(i);
	}

	locCnt += numBytes;
	
}

void Assembler::isInstrOk(string s) {
	string  e = s;
	int end = (s.find(' ') == string::npos) ? s.size() : s.find(' ');
	s = s.substr(0, end);

	end = ((s.at(end - 1) == 'b' && s != "sub") || s.at(end - 1) == 'w') ? end - 1 : end;
	s = s.substr(0, end);
	if (Assembler::instructions.count(s) == 0)
	{
		throw "Assembler error: Invalid instruction " + s + ". Line: " + e;
	}
}

int Assembler::instrOpCode(string s) {
	int end = (s.find(' ') == string::npos) ? s.size() : s.find(' ');
	s = s.substr(0, end);

	end = ((s.at(end - 1) == 'b' && s != "sub") || s.at(end - 1) == 'w') ? end - 1 : end;
	s = s.substr(0, end);
	return Assembler::instructions.find(s)->second.first;
}

int Assembler::numOpInstr(string s) {
	int end = (s.find(' ') == string::npos) ? s.size() : s.find(' ');
	s = s.substr(0, end);

	end = ((s.at(end - 1) == 'b' && s != "sub") || s.at(end - 1) == 'w') ? end - 1 : end;
	s = s.substr(0, end);
	return Assembler::instructions.find(s)->second.second;
}

bool Assembler::isByteOp(string s) {
	int end = (s.find(' ') == string::npos) ? s.size() : s.find(' ');
	s = s.substr(0, end);
	//	if (s == "push" || s == "int" || s == "pushw" || s == "pushb" || s == "intb" || s == "intw") {
	//		Helper::pushOrInt = true;
	//	}
	if (s != "sub" && s.at(end - 1) == 'b') {
		return true;
	}
	if (s.at(end - 1) == 'w') {
		return false;
	}
	return false;
}

unsigned char Assembler::getFirstByteOfInstruction(int opCode, int isByteOp) {
	int last = opCode % 2;
	int sCode = isByteOp ? 0 : 1;
	unsigned char firstHex = (unsigned char)(opCode >> 1);
	firstHex <<= 4;
	unsigned char secHex = last * 8 + sCode * 4;
	return firstHex |= secHex;
}



Operand Assembler::getOperand(string operand, Instruction i, int& numBytes, bool fstOp) {
	Operand op;

	if (i.instrType >= INT && i.instrType <= JGT) 
	{
		if (operand.at(0) == '$' || operand.at(0) == '%' || operand.at(0) == '(' || operand.at(0) == ')') 
			throw "Assembler error: Invalid format " + operand + " in instruction: " + i.line + ". Expected '*' as first character.";

		op = jumpOperand(operand, i, numBytes);

		// mozda ipak moze skok i neposredno
		//if (op.addrType == IMM) throw "Assembler Error: Invalid instruction " + i.line + ". Immediate addressing mode not allowed for destination operand.";
	}
	else 
	{
		if (operand.at(0) == '*')
			throw "Assembler error: Invalid format " + operand + " in instruction: " + i.line + ". Unexpected '" + operand.at(0) + "' as first character.";

		op = dataOperand(operand, i, numBytes, fstOp);
	}

	//check errors
	if (i.instrType >= CALL && i.instrType <= JGT && i.isByte) throw "Assembler Error: Instruction " + i.line + " cannot be combined with 'b' extension.";
	if (i.instrType == XCHG && op.addrType == IMM) throw "Assembler Error: Invalid instruction " + i.line + ". Immediate addressing mode not allowed with xchg.";
	if (i.instrType == POP && op.addrType == IMM) throw "Assembler Error: Invalid instruction " + i.line + ". Immediate addressing mode not allowed for destination operand.";
//	if (op.addrType == REGIND_POM && op.isByte) throw "Assembler Error: Register indirect addressing mode " + i.line + " cannot be combined with 'b' extension.";
//	if (op.addrType == REGIND_POM && op.isByte) throw "Assembler Error: Memory address mode " + i.line + " cannot be combined with 'b' extension.";
	
	return op;
}

Operand Assembler::dataOperand(string o, Instruction i, int& numBytes, bool fstOp) {
	if (o.at(0) == '*') throw "Assembler error: Invalid format " + o + " in instruction: " + i.line + ". Unexpected character '*'.";

	Operand operand;
	string e = o;
	int offset = locCnt + numBytes + 1;
	int size = i.isByte ? 1 : 2;

	operand.offset = offset;
	operand.size = size;

	OpAddrType addrType;
	unsigned char regNum = 0x00;
	unsigned char lowHigh = 0x00;

		if (o.at(0) == '%')
	{
		numBytes++;
		if (fstOp) {
			currSection.data[currSection.data.size() - 1].rel.valueForPcRel = 1;
		}
		addrType = REGDIR;

		operand = regAdrr(e, o, addrType, i);
	}
	else
		if (o.at(0) == '(')
		{
			numBytes++;
			if (fstOp) currSection.data[currSection.data.size() - 1].rel.valueForPcRel = 1;
			addrType = REGIND;

			int last = o.length() - 1;
			if (o.at(last) != ')') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected (%r<num>)";
			if (o.at(1) != '%') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected (%r<num>)";

			operand = regAdrr(e, o.substr(1, last - 1), addrType, i);
		}
	else
		if (o.at(0) == '$')
		{
			if (o.at(1) == '$' || o.at(1) == '%' || o.at(1) == '(' || o.at(1) == ')' || o.at(1) == '*')
				throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Unexpected '" + o.at(1) + "' as second character.";

			numBytes += i.isByte ? 2 : 3;
			if (fstOp) currSection.data[currSection.data.size() - 1].rel.valueForPcRel = i.isByte ? 2 : 3;
			addrType = IMM;

			operand.isByte = i.isByte;
			operand.opDesc = (addrType << 5);

			//	prvi bajt operand nezavisno od relokacije
			currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));
			
			o = o.substr(1, o.size());
			if (Helper::isNumber(o))
			{
				operand.ImDiAd = Helper::strToInt(o);
				operand._useSym = false;

				currSection.data.push_back(DataSection(
					i.isByte ? Helper::byteToHexString(operand.ImDiAd) : Helper::wordToHexString(operand.ImDiAd)));
			}
			else
			{
				//	$<symbol>
				//  ***  //
				// stavljam da je isByte = false !!! //
				operand.symbol = o;
				operand._useSym = true;
				Relocation rel = Relocation(currentSection, offset, operand.symbol);
				rel.size = operand.size;
				DataSection ds = DataSection("", true);
				ds.rel = rel;
				currSection.data.push_back(ds);
			}
		}
	else
		if (o.find('(') != string::npos)
		{
			numBytes += i.isByte ? 2 : 3;
			if (fstOp) {
				currSection.data[currSection.data.size() - 1].rel.valueForPcRel = i.isByte ? 2 : 3;
			}
			addrType = REGIND_POM;

			int pom = o.find('(');
			int last = o.length() - 1;
			if (o.at(last) != ')') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected <offset>(%r<num>)";
			if (o.at(pom++ + 1) != '%') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected <offset>(%r<num>)";

			string pomeraj = o.substr(0, pom - 1);
			o = o.substr(pom, o.length());
			last = o.find(')');

			operand = regAdrr(e, o.substr(0, last), addrType, i);

			if (Helper::isNumber(pomeraj)) 		//	ako je pomeraj broj
			{
				operand.ImDiAd = Helper::strToInt(pomeraj);
				operand._useSym = false;
				currSection.data.push_back(DataSection(Helper::wordToHexString(operand.ImDiAd)));
			}
			else			
			{
				//	<symbol>(%r<num>)	ako je pomeraj simbol
				//  ***  //
				operand.symbol = pomeraj;
				operand._useSym = true;

				Relocation rel = Relocation(currentSection, offset, operand.symbol, operand.pcRel);
				rel.size = operand.size;
				DataSection ds = DataSection("", true);
				ds.rel = rel;
				currSection.data.push_back(ds);
			}

		}
	else
		if (Helper::isNumber(o))
		{
			numBytes += i.isByte ? 2 : 3;
			if (fstOp) currSection.data[currSection.data.size() - 1].rel.valueForPcRel = i.isByte ? 2 : 3;
			addrType = MEM;

			operand.isByte = i.isByte;
			operand.opDesc = (addrType << 5);
			operand.ImDiAd = Helper::strToInt(o);
			operand._useSym = false;

			currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));
			currSection.data.push_back(DataSection(
				i.isByte ? Helper::byteToHexString(operand.ImDiAd) : Helper::wordToHexString(operand.ImDiAd)));
		}
	else
		// <symbol> 
		//  ***  //
	{
		
		numBytes += i.isByte ? 2 : 3;
		if (fstOp) currSection.data[currSection.data.size() - 1].rel.valueForPcRel = i.isByte ? 2 : 3;
		addrType = MEM;

		operand.isByte = i.isByte;
		operand.opDesc = (addrType << 5);
		operand.symbol = o;
		operand._useSym = true;

		//	prvi bajt operand nezavisno od relokacije
		currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));


		Relocation rel = Relocation(currentSection, offset, operand.symbol);
		rel.size = operand.size;
		DataSection ds = DataSection("", true);
		ds.rel = rel;
		currSection.data.push_back(ds);
	}

	operand.addrType = addrType;

	return operand;
}

Operand Assembler::jumpOperand(string o, Instruction i, int& numBytes) {

	if (o.size() > 1 && o.at(1) == '*')
		throw "Assembler error: Invalid format " + o + " in instruction: " + i.line + ". Unexpected '" + o.at(1) + "' as second character. ";

	Operand operand;
	string e = o;
	int offset = locCnt + numBytes + 1;
	int size = i.isByte ? 1 : 2;

	operand.offset = offset;
	operand.size = size;

	OpAddrType addrType;
	unsigned char regNum = 0x00;
	unsigned char lowHigh = 0x00;


		// operand moze ili *<..> ili <symbol>/<broj> 
	switch (o.at(0)) {

	case '*':
	{
		o = o.substr(1, o.size());

		if (o.at(0) == '%')
		{
			numBytes++;
			addrType = REGDIR;

			operand = regAdrr(e, o, addrType, i);
		}
		else
			if (o.at(0) == '(')
			{
				numBytes++;
				addrType = REGIND;

				int last = o.length() - 1;
				if (o.at(last) != ')') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected *(%r<num>)";
				if (o.at(1) != '%') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected *(%r<num>)";

				operand = regAdrr(e, o.substr(1, last - 1), addrType, i);
			}
		else
			if (Helper::isNumber(o))
			{
				numBytes += i.isByte ? 2 : 3;
				addrType = MEM;

				operand.isByte = i.isByte;
				operand.opDesc = (addrType << 5);
				operand.ImDiAd = Helper::strToInt(o);
				operand._useSym = false;

				currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));
				currSection.data.push_back(DataSection(
					i.isByte ? Helper::byteToHexString(operand.ImDiAd) : Helper::wordToHexString(operand.ImDiAd)));
			}
		else
			if (o.find('(') != string::npos)
			{
				numBytes += i.isByte ? 2 : 3;
				addrType = REGIND_POM;

				int pom = o.find('(');
				int last = o.length() - 1;
				if (o.at(last) != ')') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected *<offset>(%r<num>)";
				if (o.at(pom++ + 1) != '%') throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected *<offset>(%r<num>)";

				string pomeraj = o.substr(0, pom - 1);
				o = o.substr(pom, o.length());
				last = o.find(')');

				operand = regAdrr(e, o.substr(0, last), addrType, i);

				if (Helper::isNumber(pomeraj)) 		//	ako je pomeraj broj
				{
					operand.ImDiAd = Helper::strToInt(pomeraj);
					operand._useSym = false;
					currSection.data.push_back(DataSection(Helper::wordToHexString(operand.ImDiAd)));
				}
				else		//		*<symbol>(%r<num>)	ako je pomeraj simbol
				{
					//  ***  //
					operand.symbol = pomeraj;
					operand._useSym = true;

					Relocation rel = Relocation(currentSection, offset, operand.symbol, operand.pcRel);
					rel.size = operand.size;
					rel.jmpInstr = true;
					DataSection ds = DataSection("", true);
					ds.rel = rel;
					currSection.data.push_back(ds);
				}

			}
		
		else
				// *<symbol> 
				//  ***  //
			{
				numBytes += i.isByte ? 2 : 3;
				addrType = MEM;

				//	prvi bajt operand nezavisno od relokacije
				currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));


				Relocation rel = Relocation(currentSection, offset, o);
				rel.size = operand.size;
				//rel.jmpInstr = true;
				DataSection ds = DataSection("", true);
				ds.rel = rel;
				currSection.data.push_back(ds);
			}

		break;
	}
	default:
	{
		numBytes += i.isByte ? 2 : 3;
		addrType = IMM;

		operand.isByte = i.isByte;
		operand.opDesc = (addrType << 5);

		//	prvi bajt operand nezavisno od relokacije
		currSection.data.push_back(DataSection(Helper::byteToHexString(operand.opDesc)));


		if (Helper::isNumber(o))
		{
			operand.ImDiAd = Helper::strToInt(o);
			operand._useSym = false;
			currSection.data.push_back(DataSection(
				i.isByte ? Helper::byteToHexString(operand.ImDiAd) : Helper::wordToHexString(operand.ImDiAd)));
		}
		else
		{
			//	<symbol>
			//  ***  //
			operand.symbol = o;
			operand._useSym = true;
																			//	aps rel
			Relocation rel = Relocation(currentSection, offset, operand.symbol, false);
			rel.size = operand.size;
			//	u ovom slucaju moze da bude:
			//	a: ...
			//	jmp a 
			//	skace se na adresu a, tj njenu vrednost iz tabele simbola -> ako su u istoj sekciji ne treba relokacija
			rel.jmpInstr = true;
			DataSection ds = DataSection("", true);
			ds.rel = rel;
			
			

			currSection.data.push_back(ds);
		}
		break;
	}
	}

	operand.addrType = addrType;

	return operand;
}

Operand Assembler::regAdrr(string e, string o, OpAddrType addrType, Instruction i) {
	unsigned char regNum = 0x00;
	unsigned char lowHigh = 0x00;

	int l = 0;
	char last = 0;

	if (o.length() < 3) throw "Assembler error: Invalid format " + e + " in instruction: " + i.line;

	if ((o.at(1) == 'r' && o.at(2) >= '0' && o.at(2) <= '7') || o.substr(1, 2) == "pc" || o.substr(1, 2) == "sp" || o.substr(1,3) == "psw")
	{
		l = o.size();
		last = o[l - 1];

		if (i.isByte && last != 'l' && last != 'h' && addrType == REGDIR)
		{
			throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ".\n" + string(17,' ') + "Expected l/h with register direct addressing mode.";
		}

//		if (i.isByte && o.size() == 4 && o.at(3) != 'l' && o.at(3) != 'h')
//		{
//			if (addrType != REGIND_POM) throw "Assembler error: Invalid format " + e + " in instruction: " + i.line + ". Expected l/h.";
//		}

//		if (!i.isByte && o.size() == 4)
//		{
//			throw "Assembler error: Invalid format " + e + " in instruction: " + i.line;
//		}
	}
	else throw "Assembler error: Invalid format " + e + " in instruction: " + i.line;

	if (i.isByte && addrType == REGDIR)
	{
		lowHigh = (o.at(l - 1) == 'h') ? 0x01 : 0x00;
		o = o.substr(0, l - 1);
	}
	if (registers.count(o.substr(1, l)) == 0) throw "Assembler error: Invalid register '" + e + "' in instruction: " + i.line;
	regNum = registers.find(o.substr(1, l))->second;
	unsigned char opDesc = (addrType << 5) | (regNum << 1) | lowHigh;

	Operand op = Operand();

	op.opDesc = opDesc;
	op.isByte = i.isByte;
	op.pcRel = (regNum == 0x07) ? true : false;
	op.size = i.isByte ? 1 : 2;

	//	prvi bajt operand nezavisno od relokacije
	currSection.data.push_back(DataSection(Helper::byteToHexString(op.opDesc)));

	return op;
}



//print

string Assembler::printSymbolTable() {
	string print = "\n=> symbol table\n# name" + string(18, ' ') + "sec  scope   value     num   size\n";

	for (Symbol s : symbolTable) {
		if (s.constant && !s.global) print += "# ";
		else 
		if (s.constant && s.global) print += "~ ";
		else print += "  ";
		print += s.toString() + (s.sec ? Helper::wordToHexStringEqu(sections.find(s.number)->second.size) : "") + "\n";
	} 

	return print;
}

string Assembler::printSectionData() {
	string print = "\n";
	for (Section s : sectionsAll) {
		if (s.num != 0)	print += "\n=> ." + s.name + "\n" + format(s.toString()) + "\n";
	}
	return print;
}

string Assembler::printInstructions() {
	string print = "\n...Instructions...\n" + string(27, ' ') + " opCode   locCnt     instrCode \n";

	for (Instruction i : instructionList) {
		print += i.line.append(30 - i.line.length(), ' ')
			+ to_string(i.instrType) + "  \t" + to_string(i.locCnt)
			+ "\t" + format(i.toString()) + "\n";
	}
	return print;
}

string Assembler::printRelocation() {
	int sec = 0;
	string print = "\n";
	for (Relocation rel : relocationTable) 
	{
		if (rel.section != sec) 
		{
			print += "\n=> .rel ." + sections.find(rel.section)->second.name + "\n";
			print += "#  offset    relType        ref   sign   size\n";// symName                 sign\n";
			sec = rel.section;
		}
		print += "   " + rel.toString() + "\n";
	}
	return print;
}

string Assembler::printExpression() {


	string print = "\n\n=> classification index table\n# symbol                     section   index   expression\n";
	/*
	for (Symbol& s : symbolTable) {
		for (auto& elem : s.expression.index)
		{
			s.expression.numResult += elem.second.offset;
		}
	}
	*/
	for (Symbol& s : symbolTable) {
		print += s.expression.indexToString();
	}

/*	design = "\n# Expressions:";
	for (Expression e : expressionList) {
		design += "\n " + e.toString();
	}
*/
	return print;
}

string Assembler::format(string str) {
	string result = "";
	int num = 0;
	for (unsigned i = 0; i < str.length(); i++)
	{
		if (num == 40) 
		{
			num = 0;
			result += "\n";
		}
		num++;
		result += str[i];
		if (i % 2) result += " ";
	}
	return result;
}
