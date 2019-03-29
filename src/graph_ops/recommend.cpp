#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
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
	
	string base_filename(argv[1]);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";

	const char *mapping_file = argv[2];
	int num_recs = atoi(argv[3]);
	const char *source_word_file = argv[4];
	const char *rec_pool_file = argv[5];
    bool use_rec_pool = argc == 6;
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	std::vector<string> words = Utils::get_word_mapping(mapping_file);

    std::ifstream source_word_in(source_word_file);
    string source_word;
	std::vector<int> source_word_idxs;
    while (std::getline(source_word_in, source_word)) {
        int idx = Utils::find_word(source_word, words);
        if (idx < 0) {
            cout << "Not found in graph: " << source_word << endl;
            return -1;
        }
        source_word_idxs.push_back(idx);
    }

    std::vector<WordDist*> closest_words;
    if (use_rec_pool) {
        std::unordered_set<int> rec_pool;
        std::ifstream rec_pool_in(rec_pool_file);
        string rec_word;

        int missing_counter = 0;
        while (std::getline(rec_pool_in, rec_word)) {
            int idx = Utils::find_word(rec_word, words);
            if (idx < 0) {
                missing_counter++;
                continue;
            }
            rec_pool.insert(idx);
        }
        cout << missing_counter << " recommendation pool words not in graph" << endl;

        closest_words = ReviewAndRec::recommend(csr, source_word_idxs, num_recs, true, rec_pool);
    } else {
        closest_words = ReviewAndRec::recommend(csr, source_word_idxs, num_recs);
    }

	cout << "\nLearning recommendations :" << endl;
	for (int i = 0; i < closest_words.size(); i++) {
		cout << words[closest_words[i]->word_id] << " (Value: "
			<< closest_words[i]->dist << ")" << endl;
	}
	if (closest_words.size() < num_recs)
		cout << "End" << endl;
	return 0;	
}
