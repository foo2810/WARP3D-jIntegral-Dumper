// J-Domain Extracter

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>


class DomainValue {
public:
	int numDomains;
	std::string domainID;
	std::string stname;
	std::string lsldnm;
	int ltmstp;

	std::vector<int> nList;
	std::vector< std::vector<double> > domains;
	std::vector<int> skList;

	double diAvg;
	double diMin;
	double diMax;

	std::vector<int> nList2;
	std::vector< std::vector<double> > domains2;

	void print();
	bool dumpAll(std::string csv);
	bool dumpAvg(std::string csv);
};

// ************
// * Caution! *
// ************
//
// bpf file is wrote using "Sequential access" of fortran.
//
// + Fortran binary file
// How to access disk files in fortran depends on parameter, "access" in opening disk files.
// [Sequential access]
//   <header><record><footer><header><record><footer>...
// 	   header: data size (4bytes ?)
//     record: data
//     footer: data size. the value is same with header (4bytes ?)
// 
// [Direct access]
//   <record><record><record>...
//   record: data (all records are same size!)
// 
// [Stream access]
//   Same with high levele I/O system of C lang

class DomainExtracter {
	std::string fname;
	std::ifstream ifs;
	int curPos;
	std::map< std::string, std::map<int, int> > map;
	std::map< int, std::string > keyMap;

	bool createMap();
	void getDomainMeta(std::string *domainID, int *step);
	DomainValue readDomainVal(int pos);
	void skipPackContent(int numLines);

public:
	DomainExtracter(std::string bFile);
	void printDomainValues(std::string domainID, int step);
	DomainValue getDomainValues(std::string domainID, int step);
	DomainValue getDomainValues(int id, int step);
	std::map< std::string, std::map<int, int> > getMap();
	std::map< int, std::string > getKeyMap();
};
