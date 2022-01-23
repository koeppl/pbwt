#include <cstdio>
#include <vector>
#include <fstream>
#include "/scripts/code/dcheck.hpp"

using namespace std;


#include <iostream>
using namespace std;

int main(const int argc, const char *const argv[]) {
	ifstream f(argv[1]);

	string s;
	std::getline(f, s);
	std::getline(f, s);
	size_t heightcount = 0;
	while(f.good() && !f.eof()) {
		getline(f, s);
		++heightcount;
	}
	++heightcount;
	// std::cout << "height: " << heightcount << std::endl;
	f.close();
	f.clear();
	f.open(argv[1]);
	std::getline(f, s);
	std::getline(f, s);
	std::cout << heightcount << std::endl;
	while(f.good() && !f.eof()) {
		getline(f, s);

		size_t column = 0;
		size_t i = 0;
		for(; column < 4 && i < s.length(); ++i) {
			if(s[i] == ' ' || s[i] == '\t') {
				while(s[i] == ' ' || s[i] == '\t') {
					++i;
				}
				++column;
			}
		}
		for(; i < s.length(); ++i) {
			if(s[i] != '1' && s[i] != '0') {
				std::cerr << "could not parse " << s[i] << s << std::endl;
				return 1;
			}
			cout << s[i];
			if(i+1 != s.length()) cout << ' ';
		}
		cout << std::endl;
	}


	return 0;
}

