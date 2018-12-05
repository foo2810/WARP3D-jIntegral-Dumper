#include "jdomain.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>


static void normStr(char *str, int len) {
	int i;
	for (i = len - 1; i >= 0; i--) {
		if (str[i] != ' ') {
			break;
		}
	}

	if (i == len - 1) {
		str[len - 1] = '\0';
		return;
	}

	str[i+1] = '\0';
}

void DomainValue::print() {
	std::ios::fmtflags flagsSave = std::cout.flags();

	std::cout << "domain: " << std::setw(24) << domainID << "  structure: " << std::setw(8)
	          << stname << "  loading: " << std::setw(8) << lsldnm
			  << "  loading step number: " << std::setw(12) << std::right << ltmstp << "\n";
	
	std::cout << " domain     dm1         dm2         dm3         dm4         dm5         "
		      << "dm6         dm7         dm8        total J  killed ele" << std::endl;
	
	std::cout << std::fixed;
	std::cout << std::scientific;
	for (int i = 0; i < static_cast<int>(nList.size()); i++) {
		std::cout << " " << std::setw(3) << nList[i] << "    ";
		for (int j = 0; j < 9; j++) {
			std::cout << " " << std::setw(11) << std::setprecision(4) << domains[i][j];
		}
		std::cout << "   " << std::setw(3) << skList[i] << std::endl;
	}

	std::cout << " domain average   domain min   domain max" << "\n";
	std::cout << " " << std::setw(11) << std::setprecision(4) << diAvg;
	std::cout << "     " << std::setw(11) << std::setprecision(4) << diMin;
	std::cout << "  " << std::setw(11) << std::setprecision(4) << diMax << "\n" << std::endl;;

	std::cout << "stress intensity factors from J (single-mode loading):\n"
	          << " domain  KI pstrs    KI pstrn    KII pstrs   KII pstrn   KIII" << std::endl;

	for (int i = 0; i < static_cast<int>(nList2.size()); i++) {
		std::cout << " " << std::setw(3) << nList2[i] << "   ";
		for (int j = 0; j < 5; j++) {
			std::cout << " " << std::setw(11) << std::setprecision(4) << domains2[i][j];
		}
		std::cout << std::endl;
	}

	std::cout.flags(flagsSave);
}

bool DomainValue::dumpAll(std::string csv) {
	std::ofstream ofs;
	ofs.open(csv.c_str(), std::ios::out | std::ios::app);
	if (!ofs) return false;

	std::ios::fmtflags flagsSave = ofs.flags();

	ofs << "domain:," << domainID << ",structure:," << stname << ",loading:," << lsldnm
			  << ",loading step number:," << ltmstp << std::endl;
	
	ofs << "domain,dm1,dm2,dm3,dm4,dm5,dm6,dm7,dm8,total J,killed ele" << std::endl;
	
	ofs << std::fixed;
	ofs << std::scientific;
	for (int i = 0; i < static_cast<int>(nList.size()); i++) {
		ofs << nList[i] << "," << std::flush;
		for (int j = 0; j < 9; j++) {
			ofs << std::setw(11) << std::setprecision(4) << domains[i][j] << "," << std::flush;
		}
		ofs << skList[i] << std::endl;
	}

	ofs << "domain average, domain min,domain max" << std::endl;
	ofs << std::setw(11) << std::setprecision(4) << diAvg << "," << std::flush;
	ofs << std::setw(11) << std::setprecision(4) << diMin << "," << std::flush;
	ofs << std::setw(11) << std::setprecision(4) << diMax << "\n" << std::endl;;

	ofs << "stress intensity factors from J (single-mode loading):\n"
	          << " domain,KI pstrs,KI pstrn,KII pstrs,KII pstrn,KIII" << std::endl;

	for (int i = 0; i < static_cast<int>(nList2.size()); i++) {
		ofs << nList2[i] << std::flush;
		for (int j = 0; j < 5; j++) {
			ofs << "," << std::setw(11) << std::setprecision(4) << domains2[i][j] << std::flush;
		}
		ofs << std::endl;
	}

	ofs << std::defaultfloat;
	ofs << "\n\n" << std::endl;

	ofs.flags(flagsSave);

	ofs.close();

	return true;
}

bool DomainValue::dumpAvg(std::string csv) {
	std::ofstream ofs;
	ofs.open(csv.c_str(), std::ios::out | std::ios::app);
	if (!ofs) return false;

	std::ios::fmtflags flagsSave = ofs.flags();

	ofs << domainID << "," << ltmstp << "," << diAvg << std::endl;

	ofs.flags(flagsSave);

	ofs.close();

	return true;
}


DomainExtracter::DomainExtracter(std::string bFile) {
	fname = bFile;
	ifs.open(fname.c_str(), std::ios::binary | std::ios::in);
	if (!ifs.is_open()) {
		std::cerr << "Error: Can not open file" << std::endl;
		exit(1);
	}

	createMap();

	ifs.close();
}

void DomainExtracter::printDomainValues(std::string domainID, int step) {
	DomainValue jVal = readDomainVal(map[domainID][step]);
	jVal.print();
}


DomainValue DomainExtracter::getDomainValues(std::string domainID, int step) {
	return readDomainVal(map[domainID][step]);
}

DomainValue DomainExtracter::getDomainValues(int id, int step) {
	return readDomainVal(map[keyMap[id]][step]);
}

bool DomainExtracter::createMap() {
	int32_t typeNum = 0;
	int32_t numLines = 0;
	int32_t numStep = 0;
	int32_t numIter = 0;

	int32_t lenRecord = 0;
	unsigned int id = 1;


	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char *>(&lenRecord), 4);
		if (ifs.eof()) break;    // Need to check eof

		// Read thunk header
		ifs.read(reinterpret_cast<char *>(&typeNum), 4);
		ifs.read(reinterpret_cast<char *>(&numLines), 4);
		ifs.read(reinterpret_cast<char *>(&numStep), 4);
		ifs.read(reinterpret_cast<char *>(&numIter), 4);


		// Skip footer
		ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);


		if (ifs.eof()) {
			std::cerr << "Error: bpf file may be broken" << std::endl;
			exit(1);
		}

		// Domain Values: packet-type = 17
		if (typeNum != 17) {
			skipPackContent(numLines);
		} else {
			std::string domainID;
			int step;

			getDomainMeta(&domainID, &step);

			if (map.find(domainID) != map.end()) {
				map[domainID][step] = static_cast<int>(ifs.tellg());
			} else {
				keyMap[id] = domainID;
				map[domainID][step] = static_cast<int>(ifs.tellg());
				id++;
			}

			skipPackContent(numLines);
		}
	}

	return true;
}

void DomainExtracter::getDomainMeta(std::string *domainID, int *step) {
	uint32_t lenRec;
	char dID[24];
	int savePos;

	savePos = ifs.tellg();

	// read header
	ifs.read(reinterpret_cast<char *>(&lenRec), 4);

	ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);
	ifs.read(dID, 24);
	ifs.seekg(static_cast<int>(ifs.tellg()) + 8 + 8, std::ios_base::beg);
	ifs.read(reinterpret_cast<char *>(step), 4);

	// skip footer
	ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);

	ifs.seekg(savePos, std::ios_base::beg);

	normStr(dID, sizeof dID);
	*domainID = dID;
}

void DomainExtracter::skipPackContent(int numLines) {
	int lenRec;

	for (int i = 0; i < numLines; i++) {
		// read header
		ifs.read(reinterpret_cast<char *>(&lenRec), 4);
		//skip data
		ifs.seekg(static_cast<int>(ifs.tellg()) + lenRec, std::ios_base::beg);
		// skip footer
		ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);
	}
}

DomainValue DomainExtracter::readDomainVal(int pos) {
	int lenRec;
	char dID[25], st[9], ls[9];
	DomainValue jVal;

	ifs.open(fname, std::ios::in | std::ios::binary);
	if (!ifs.is_open()) {
		std::cerr << "Error: Cannot open file" << std::endl;
		exit(1);
	}

	ifs.seekg(pos, std::ios_base::beg);

	// read header
	ifs.read(reinterpret_cast<char *>(&lenRec), 4);

	ifs.read(reinterpret_cast<char *>(&jVal.numDomains), 4);
	ifs.read(reinterpret_cast<char *>(dID), 24);
	ifs.read(reinterpret_cast<char *>(st), 8);
	ifs.read(reinterpret_cast<char *>(ls), 8);
	ifs.read(reinterpret_cast<char *>(&jVal.ltmstp), 4);

	// [Caution] ex) If length of dID is 24, normStr 24th character is replaced with '/0'
	// dID, st, ls are string, but not terminated by null character -> size + 1
	normStr(dID, 25);  jVal.domainID = dID;
	normStr(st, 9);    jVal.stname = st;
	normStr(ls, 9);    jVal.lsldnm = ls;

	// skip footer
	ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);


	// Read J-integral values
	int nowring, sk;
	double val1;
	jVal.domains.resize(jVal.numDomains);
	for (int i = 0; i < jVal.numDomains; i++) {
		// read header
		ifs.read(reinterpret_cast<char *>(&lenRec), 4);

		ifs.read(reinterpret_cast<char *>(&nowring), 4);
		jVal.nList.push_back(nowring);

		for (int j = 0; j < 9; j++) {
			ifs.read(reinterpret_cast<char *>(&val1), 8);
			jVal.domains[i].push_back(val1);
		}

		ifs.read(reinterpret_cast<char *>(&sk), 4);
		jVal.skList.push_back(sk);

		// skip footer
		ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);
	}

	// Read max, min and average values
	if (jVal.numDomains > 1) {
		ifs.read(reinterpret_cast<char *>(&lenRec), 4);

		ifs.read(reinterpret_cast<char *>(&jVal.diAvg), 8);
		ifs.read(reinterpret_cast<char *>(&jVal.diMin), 8);
		ifs.read(reinterpret_cast<char *>(&jVal.diMax), 8);

		// skip footer
		ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);
	}

	// Read stress intensity factor values computed from J-values
	int nowring2;
	double val2;
	jVal.domains2.resize(jVal.numDomains);
	for (int i = 0; i < jVal.numDomains; i++) {

		// read header
		ifs.read(reinterpret_cast<char *>(&lenRec), 4);

		ifs.read(reinterpret_cast<char *>(&nowring2), 4);
		jVal.nList2.push_back(nowring2);

		for (int j = 0; j < 5; j++) {
			ifs.read(reinterpret_cast<char *>(&val2), 8);
			jVal.domains2[i].push_back(val2);
		}

		// skip footer
		ifs.seekg(static_cast<int>(ifs.tellg()) + 4, std::ios_base::beg);
	}

	ifs.close();

	return jVal;
}


//std::unordered_map< std::string, std::unordered_map<int, int> > DomainExtracter::getMap() {
std::map< std::string, std::map<int, int> > DomainExtracter::getMap() {
	return map;
}

std::map< int, std::string > DomainExtracter::getKeyMap() {
	return keyMap;
}

