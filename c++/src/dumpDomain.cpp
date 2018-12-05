#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <cstdlib>
#include <ctime>
#include "jdomain.h"

#define PRINT_MAP
//#define PRINT_JVAL
#define MODE 0
//#define DEBUG

void usage(std::string name) {
	std::cerr << "Usage: " << name << " <input file> <output file>" << std::endl;
}

void printTitle(std::string end) {
	std::cout << "**************************************************" << std::endl;
	std::cout << "* Domain Dumper                                  *" << std::endl;
	std::cout << "*                                                *" << std::endl;
	std::cout << "* Last modified: 2018/10/2                       *" << std::endl;
	std::cout << "**************************************************" << std::flush;
	std::cout << end << std::endl;
}

void printMap(DomainExtracter &reader) {
	int cnt = 0;
	std::map< std::string, std::map< int, int > > map(reader.getMap());
	std::map< int, std::string > keyMap(reader.getKeyMap());
	std::string domainID;
	int step;

	std::cout << "Map" << std::endl;
	std::cout << "+--------------------------+----------+" << std::endl;
	std::cout << "| Domain ID                | step     |" << std::endl;
	for (auto i = keyMap.begin(); i != keyMap.end(); i++) {
		domainID = i->second;
		for (auto j = map[domainID].begin(); j != map[domainID].end(); j++) {
			std::cout << "+--------------------------+----------+--------------------------+----------+" << std::endl;

			cnt++;
			step = j->first;
			std::cout << "| " << std::setw(24) << domainID << " | " << std::setw(8) << step << " |" << std::flush;
			
			j++;
			if (j != map[domainID].end()) {
				cnt++;
				step = j->first;
				std::cout << " " << std::setw(24) << domainID << " | " << std::setw(8) << step << " |" << std::endl;
			} else {
				std::cout << " " << std::setw(24) << "" << " | " << std::setw(8) << "" << " |" << std::endl;
				break;
			}

		}
	}

	if (cnt % 2 != 0) {
		std::cout << " " << std::setw(24) << "" << " | " << std::setw(8) << "" << " |" << std::endl;
	}

	std::cout << "+--------------------------+----------+--------------------------+----------+" << std::endl;
}

std::string extStem(std::string path) {
	int len = path.length();
	int p = len - 1, ep = len - 1;

	if (len > 1) {
		if (path.back() == '/') p--, ep--;
	} else if (len == 1) {
		if (path.back() == '/') return "";
		else return path;
	} else {
		return "";
	}

	for (;p >= 0; p--) {
		if (path.c_str()[p] == '/') break;
	}

	if (p+1 <= ep) {
		return path.substr(p+1, ep-p);
	} else {
		std::cerr << ">>> Error: Unexpected error" << std::endl;
		exit(1);
	}
}

int main(int argc, char **argv) {
	std::string iFile, oFile;
	std::ofstream ofs;

	bool flg = true;
	std::string domainID;
	int step;
	DomainValue jVal;
	std::string buf;
	std::string yn;
	

	if (argc != 3) {
		usage(argv[0]);
		exit(1);
	}

	printTitle("\n");
	std::cout << std::endl;

	iFile = argv[1];
	oFile = argv[2];

	DomainExtracter reader(iFile);
	std::map< std::string, std::map<int, int> > mapping(reader.getMap());
	std::map< int, std::string > keyMap(reader.getKeyMap());


	if (mapping.size() == 0) {
		std::cerr << ">>> Error: No domain value in " << iFile << "\n" << std::endl;
		exit(1);
	}

	// write timestamp and item name
	std::time_t t = time(NULL);
	std::tm *now = localtime(&t);

	ofs.open(oFile);
	if (!ofs) {
		std::cerr << ">>> Error: Cannot open file (" << oFile << ")" << std::endl;
		exit(1);
	}
	ofs << "Created in " << 1900 + now->tm_year << "/" << 1 + now->tm_mon << "/" << now->tm_mday
	    << " " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "\n";
	ofs << "load file," << extStem(iFile) << "\n\n";
	ofs << "step";
	for (auto itr = keyMap.begin(); itr != keyMap.end(); itr++) {
		ofs << "," << itr->second;
	}
	ofs << std::endl;
	ofs.close();

	// Main loop
	while (flg) {

#ifdef PRINT_MAP
		printMap(reader);
		std::cout << std::endl;
#endif

#if MODE == 1
		// Input domain ID
		while (true) {
			std::cout << "> Enter domain ID: " << std::flush;
			std::cin >> domainID;
			std::cout << std::endl;

			if (mapping.find(domainID) != mapping.end()) break;

			std::cout << ">>> Error: Domain, \"" << domainID << "\" not found. Try again" << std::endl;
			std::cout << std::endl;
		}
#endif

		// Input stem number
		while (true) {
			std::cout << "> Enter stem number: " << std::flush;
			std::cin >> buf;
			std::cout << std::endl;

			try {
				step = std::stoi(buf);
			}
			catch (std::exception &e) {
				std::cerr << ">>> Error: Your input is not a number. Try again\n" << std::endl;
				continue;
			}

			for (auto k = mapping.begin(); k != mapping.end(); k++) {
				if (k->second.find(step) != k->second.end()) {
					goto INPUT_STEP_END;
				} else {
					std::cout << "Error: step, " << step << " not found\n" << std::endl;
					break;
				}
			}
		}
INPUT_STEP_END:

#if MODE == 0
		ofs.open(oFile, std::ios::app);
		if (!ofs) {
			std::cerr << ">>> Error: Cannot open file (" << oFile << ")" << std::endl;
			exit(1);
		}

		ofs << step;

		for (auto itr = keyMap.begin(); itr != keyMap.end(); itr++) {
			domainID = itr->second;
			if (mapping.find(domainID) != mapping.end()) {
				jVal = reader.getDomainValues(domainID, step);
				ofs << "," << jVal.diAvg;
			} else {
				ofs << ",";
			}
		}

		ofs << std::endl;
		ofs.close();

#else

		jVal = reader.getDomainValues(domainID, step);

#ifdef PRINT_JVAL
			jVal.print();
#endif

		if (!jVal.dumpAll(oFile)) {
			std::cerr << ">>> Error: Failed to dump" << std::endl;
			exit(1);
		}

#endif
		

		// Continue or not
		while (true) {
			std::cout << "> Continue? (yes or no): " << std::flush;
			std::cin >> yn;
			std::cout << std::endl;

			if (yn == "yes" || yn == "YES" || yn == "y" || yn == "Y" ||
				yn == "no" || yn == "NO" || yn == "n" || yn == "N") {
				 break;
			}

			std::cerr << ">>> Error: Try again\n" << std::endl;
		}

		if (yn == "no" || yn == "NO" || yn == "n" || yn == "N") flg = false;
	}

	return 0;
}

