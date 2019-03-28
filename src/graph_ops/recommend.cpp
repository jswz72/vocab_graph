#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include "review_and_recommend.h"
#include "utils.h"

using std::cout;
using std::endl;
using std::string;


int main(int argc, char **argv) {
	if (argc < 5) {
		cout << "Input: ./exe base_file mapping_file num_recs source_word_file rec_pool_file" << endl;
		return 1;
	}
	
	string base_filename (argv[1]);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";

	const char *mapping_file = argv[2];
	int num_recs = atoi(argv[3]);
	const char *source_word_file = argv[4];
	const char *rec_pool_file = argv[5];
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	std::vector<int> source_word_idxs;
	std::vector<string> words = Utils::get_word_mapping(mapping_file);

    std::ifstream infile(source_word_file);
    string source_word;
    int counter = 0;
    while (std::getline(infile, source_word)) {
        auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		source_word_idxs.push_back(idx);
        counter++;
    }
	int num_source_words = counter;

	std::vector<WordDist*> closest_words = ReviewAndRec::recommend(csr, source_word_idxs, num_recs);
	cout << "\nLearning recommendations :" << endl;
	for (int i = 0; i < closest_words.size(); i++) {
		cout << words[closest_words[i]->word_id] << " (Value: "
			<< closest_words[i]->dist << ")" << endl;
	}
	if (closest_words.size() < num_recs)
		cout << "End" << endl;
	return 0;	
}
