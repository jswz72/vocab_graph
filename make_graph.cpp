#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <vector>
#include <unordered_map>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Provide vecfile" << endl;
        return 1;
    }
    const char *vecfilename = argv[1];
    const int threshold = argc > 2 ? atoi(argv[2]) : 0;
    const int limit = argc > 3 ? atoi(argv[3]) : 0;
    
    std::ifstream infile(vecfilename);
    std::string line;

    std::unordered_map<std::string, std::vector<double> > words;
    int counter = 0;
    while (std::getline(infile, line)) {
        if (limit && ++counter > limit) {
            break;
        }
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        bool alphanum = true;
        for (std::string::iterator it=word.begin(); it != word.end(); it++) {
            if (!isalpha(*it)) {
                alphanum = false;
                break;
            }
        }
        if (!alphanum)
            continue;
        double d;
        std::vector<double> vec;
        while ((iss >> d)) {
            vec.push_back(d);
        }
        words[word] = vec;
    }
    std::vector<double> digs = words["the"];
    for (int i = 0; i < digs.size(); i++) {
        cout << digs[i] << ", ";
    }
}