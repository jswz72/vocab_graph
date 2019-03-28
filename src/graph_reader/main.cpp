#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include "graph.h"

using std::cout;
using std::endl;
using std::string;

const int num_recs = 100;
string REV_CMD = "rev";
string REC_CMD = "rec";

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
		cout << "Input: ./exe beg csr weight mapping cmd source_words..." << endl;
		return 1;
	}
	
	const char *beg_file = argv[1];
	const char *csr_file = argv[2];
	const char *weight_file = argv[3];
	const char *mapping_file = argv[4];
	string cmd (argv[5]);
	if (cmd != REV_CMD && cmd != REC_CMD) {
		cout << "Invalid cmd" << endl;
		cout << cmd << endl;
		return 1;
	}
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file, csr_file, weight_file);

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	int num_source_words = argc - 6;
	std::vector<int> source_word_idxs;
	std::vector<string> words = get_word_mapping(mapping_file);

    for (int i = 0; i < num_source_words; i++) {
		const char *source_word = argv[i + 6];
		auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		source_word_idxs.push_back(idx);
    }

	if (cmd == REC_CMD) {
		std::vector<WordDist*> closest_words = ReviewAndRec::recommend(csr, source_word_idxs, num_recs);
		cout << "\nLearning recommendations :" << endl;
		for (int i = 0; i < closest_words.size(); i++) {
			cout << words[closest_words[i]->word_id] << " (Dist: "
				<< closest_words[i]->dist << ")" << endl;
		}
		if (closest_words.size() < num_recs)
			cout << "End" << endl;
	} else {
		// Currently just taking random word from learned
		std::vector<int> reviewed;
		reviewed.push_back(source_word_idxs[rand() % source_word_idxs.size()]);
		std::vector<int> to_review = ReviewAndRec::review(csr, reviewed, source_word_idxs, num_recs);
		cout << "\nRecommended review order " 
			"starting from: " << words[reviewed[0]] << endl;
		for (int i = 0; i < to_review.size(); i++)
			cout << words[to_review[i]] << endl;
	}

	return 0;	
}
