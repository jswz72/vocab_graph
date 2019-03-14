#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include "graph.h"
#include "review_and_recommend.h"

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

int main(int argc, char **argv) {
	if (argc < 5) {
		cout << "Input: ./exe beg csr weight mapping source_words..." << endl;
		return 1;
	}
	
	const char *beg_file = argv[1];
	const char *csr_file = argv[2];
	const char *weight_file = argv[3];
	const char *mapping_file = argv[4];
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file, csr_file, weight_file);

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	int num_source_words = argc - 5;
	std::vector<int> source_word_idxs;
	std::vector<string> words = get_word_mapping(mapping_file);

    for (int i = 0; i < num_source_words; i++) {
		const char *source_word = argv[i + 5];
		auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		source_word_idxs.push_back(idx);
    }

	ReviewAndRec::review_and_rec(csr, source_word_idxs, words, 10);
	return 0;	
}
