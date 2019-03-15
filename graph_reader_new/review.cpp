#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include "review_and_recommend.h"
#include "utils.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	if (argc < 4) {
		cout << "Input: ./exe base_file mapping_file num_to_review" << endl;
		return 1;
	}
	
	string base_filename (argv[1]);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";

	const char *mapping_file = argv[2];
	int num_to_review = atoi(argv[3]);
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	std::vector<string> learned_words;
	std::vector<string> reviewed_words;

	cout << "Enter learned words" << endl;
	string line;
	string word;
	std::getline(std::cin, line);
	std::istringstream islearned(line);
	while (islearned >> word)
		learned_words.push_back(word);

	cout << "Enter reviewed words" << endl;
	std::getline(std::cin, line);
	std::istringstream isreviewed(line);
	while (isreviewed >> word)
		reviewed_words.push_back(word);


	int num_source_words = argc - 6;
	std::vector<int> source_word_idxs;
	std::vector<string> words = Utils::get_word_mapping(mapping_file);

	std::vector<int> learned_words_idx;
	std::vector<int> reviewed_words_idx;

    for (int i = 0; i < learned_words.size(); i++) {
		string source_word = learned_words[i];
		auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		learned_words_idx.push_back(idx);
    }
    for (int i = 0; i < reviewed_words.size(); i++) {
		string source_word = reviewed_words[i];
		auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		reviewed_words_idx.push_back(idx);
    }

	// take random word from learned
	if (reviewed_words_idx.size() == 0)
		reviewed_words_idx.push_back(learned_words_idx[rand() % source_word_idxs.size()]);

	std::vector<int> to_review = ReviewAndRec::review(csr, reviewed_words_idx, learned_words_idx, num_to_review);

	cout << "\nRecommended review order " 
		"starting from: " << words[reviewed_words_idx[0]] << endl;
	for (int i = 0; i < to_review.size(); i++)
		cout << i + 1 << ": " << words[to_review[i]] << endl;
	return 0;	
}
