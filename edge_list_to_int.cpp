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

    const char *edgefilename = argv[1];
    const char *outfilename = argv[2];

    std::ifstream infile(edgefilename);
    std::ofstream outfile(outfilename);
    std::ofstream word_order("word-order.txt");
    std::unordered_map<std::string, int> words;
    std::string line;
    int counter = 0;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string vtx1, vtx2;
        double weight;
        iss >> vtx1 >> vtx2 >> weight;
        weight *= 10000;
        int vtx1val, vtx2val;
        if (words.find(vtx1) == words.end()) {
            words[vtx1] = counter;
            word_mappings << vtx1 << std::endl;
        }
        if (words.find(vtx2) == words.end()) {
            words[vtx2] = counter;
            word_mappings << vtx2 << std::endl;
        }
        vtx1val = words[vtx1];
        vtx2val = words[vtx2];
        outfile << vtx1val << " " << vtx2val << " " << weight << "\n";
    }
    return 0;
}
