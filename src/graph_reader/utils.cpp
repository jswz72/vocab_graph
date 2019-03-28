#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using std::cout;
using std::endl;
using std::string;

template<typename G>
void print_edges(G *csr) {
	for(int i = 0; i < csr->vert_count; i++)
    {
        int beg = csr->beg_pos[i];
        int end = csr->beg_pos[i+1];
        cout << i << "'s neighor list: ";
        for(int j = beg; j < end; j++)
        {
            cout << csr->csr[j] << " ";
            cout <<"Dist :" << csr->weight[j] <<" ";
        }
        cout << endl;
    }
}

std::vector<string> get_word_mapping(const char *mapping_file) {
	std::ifstream infile(mapping_file);
	std::vector<string> words;
	string line;
	while (std::getline(infile, line))
		words.push_back(line);
	return words;
}
