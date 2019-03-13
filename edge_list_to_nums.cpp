#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <vector>
#include <tuple>
#include <cmath>
#include <unordered_map>

int main (int argc, char *argv[]) {
	if (argc < 3) {
		std::cout << "./exec edgefilename outfilename" << std::endl;
		return 1;
	}
    const char *edgefilename = argv[1];
	std::string outfilename(argv[2]);
	std::string word_order_fname = "word-order.txt";

    std::ifstream infile(edgefilename);
    std::ofstream outfile(outfilename);
	std::string word_order_filename;

	size_t pos = outfilename.find_last_of('/');
	if (pos != std::string::npos) {
		word_order_filename.assign(outfilename.begin(), outfilename.begin() + pos + 1);
		word_order_filename += word_order_fname;
	}
	else
		word_order_filename = word_order_fname;

    std::ofstream word_order(word_order_filename);
    std::unordered_map<std::string, int> words;
    std::string line;
    int counter = 0;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string vtx1, vtx2;
        double weight;
        iss >> vtx1 >> vtx2 >> weight;
        //weight *= 10000;
        int vtx1val, vtx2val;
        if (words.find(vtx1) == words.end()) {
            words[vtx1] = counter++;
            word_order << vtx1 << std::endl;
        }
        if (words.find(vtx2) == words.end()) {
            words[vtx2] = counter++;
            word_order << vtx2 << std::endl;
        }
        vtx1val = words[vtx1];
        vtx2val = words[vtx2];
        outfile << vtx1val << " " << vtx2val << " " << weight << "\n";
    }
    return 0;
}
