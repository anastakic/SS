#include "Helper.h"

void Helper::removeExtraBlanks(string& s) {
	string output;
	unique_copy(s.begin(), s.end(), back_insert_iterator<string>(output), [](char a, char b) { return isspace(a) && isspace(b); });
	s = output;
}

void Helper::removeComment(string& s) {
	string output = s.substr(0, s.find("#", 0));
	s = output;
};

void Helper::ltrim(string& s) {
	std::replace(s.begin(), s.end(), '\t', ' ');	//Sve tabove menjam razmakom
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !isspace(c); }));

}

void Helper::rtrim(string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {return !isspace(ch); }).base(), s.end());
}

bool Helper::endsWith(string const& s, string const& ending) {
	if (s.length() >= ending.length()) {
		return (0 == s.compare(s.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

string Helper::charToHexString(unsigned char c) {
	stringstream stream;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << (unsigned int)c;
	return stream.str();
}

string Helper::intToHexString(int i) {
	stringstream stream;
	stream << setfill('0') << std::setw(sizeof(int) * 2) << std::hex << i;
	return stream.str();
};

string Helper::wordToHexString(int i) {
	stringstream stream;
	short pom1 = i & 0x00ff;
	short pom2 = i & 0xff00;
	pom2 >>= 8;
	pom2 &= 0x00ff;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << pom1;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << pom2;

	return stream.str();
};

string Helper::wordToHexStringEqu(int i) {
	stringstream stream;
	short pom1 = i & 0x00ff;
	short pom2 = i & 0xff00;
	pom2 >>= 8;
	pom2 &= 0x00ff;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << pom2;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << pom1;

	return stream.str();
};

string Helper::byteToHexString(int i) {
	stringstream stream;
	short pom = i & 0xff;
	stream << setfill('0') << std::setw(sizeof(char) * 2) << std::hex << pom;
	return stream.str();
};

int Helper::strToInt(string s) {

	if (s.find("0x") != string::npos) {
		return stoi(s, 0, 16);
	}
	else {
		return stoi(s);
	}
};

bool Helper::isNumber(string s)
{
	if (s.size() == 0) {
		return false;
	}

	if (s.at(0) == '-') {
		s = s.substr(1, s.size() - 1);
		std::string::const_iterator it = s.begin();
		while (it != s.end() && isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}
	if (s.size() > 1) {
		if (s.at(1) == 'x') {
			s = s.substr(2, s.size() - 1);
			std::string::const_iterator it = s.begin();
			while (it != s.end() && (isdigit(*it) || ((*it >= 'a') && (*it <= 'f')) || ((*it >= 'A') && (*it <= 'F')))) ++it;
			return !s.empty() && it == s.end();
		}
		else {
			std::string::const_iterator it = s.begin();
			while (it != s.end() && isdigit(*it)) ++it;
			return !s.empty() && it == s.end();
		}
	}
	else {
		if (isdigit(s[0])) return true;
		else return false;
	}

}
