#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <set>

bool inclue(std::set<unsigned int> &a, std::set<unsigned int> &b) {
	// a \subseteq b

	for(auto e: a) {
		if(b.count(e) == 0) {
			return false;
		}
	}

	return true;
}

unsigned int nombre(std::vector< std::set<unsigned int> > &V) {
	unsigned int result=0;

	for(auto &s: V) {
		for(auto i: s) {
			result++;
		}
	}

	return result;
}

int main(int argc, char const *argv[])
{
	if(argc != 4) {
		std::cout << "Utilisation: " << argv[0] << " data.dat solution.txt k" << std::endl;
		return -1;
	}
	unsigned int k = atoi(argv[3]);

	std::ifstream infile(argv[1]);

	unsigned int totalOne=0;
	std::string line;
	std::vector<std::set<unsigned int>> data;
	while(std::getline(infile, line)) {
		int a;
		data.push_back( std::set<unsigned int>() );
	    std::istringstream iss(line);
	    while(iss>>a) {
			data.back().insert(a);
			totalOne++;
	    }

	}

	std::ifstream infile2(argv[2]);
	std::vector<std::set<unsigned int>> result;
	while(std::getline(infile2, line)) {
		if(line[0] >= '0' && line[0] <= '9') {

			auto pos = line.find(':');
			if(pos != std::string::npos) {
				pos++;
				line = line.substr( pos );
			}

			int a;
			result.push_back( std::set<unsigned int>() );
		    std::istringstream iss(line);
		    while(iss>>a) {
				result.back().insert(a);
		    }
	    }
	}

	std::vector< std::set<unsigned int> > Mres(data.size());

unsigned nb=1;
	for(auto &p: result) {
		for(unsigned int i=0; i<data.size(); i++) {
			if( inclue(p, data[i]) ) {
				for(auto e: p) {
					Mres[i].insert(e);
				}
			}
		}

		if(nb==k) {
			std::cout << argv[1] << "\t" << k << "\t" << (totalOne-nombre(Mres)) << "\t";
			return 0;
		}
		//std::cout << nb << ": " << nombre(Mres) << "\tError: " << (totalOne-nombre(Mres)) << std::endl;
		nb++;
	}



	return 0;
}
