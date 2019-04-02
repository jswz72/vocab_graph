#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <cstdlib>
#include "review_and_recommend.h"
#include "utils.h"

using std::string;
using std::cout;
using std::endl;

extern "C" int *review(const char *csr_filename, const char **words_in, 
        unsigned int num_words, const char **learned_words_in, 
        unsigned int num_learned_words, const char **reviewed_words_in, 
        unsigned int num_reviewed_words, unsigned int num_to_review)
{
    string base_filename (csr_filename);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";

	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

    std::vector<string> words(words_in, words_in + num_words);

    std::vector<int> learned_words_idx;
    for (unsigned int i = 0; i < num_learned_words; i++) {
        string lw(learned_words_in[i]);
        learned_words_idx.push_back(Utils::find_word(lw, words));
    }

    std::vector<int> reviewed_words_idx;
    for (unsigned int i = 0; i < num_reviewed_words; i++) {
        string rw(reviewed_words_in[i]);
        reviewed_words_idx.push_back(Utils::find_word(rw, words));
    }
    
    // Take random from learned if not given any reviewed
    if (!num_reviewed_words) {
        reviewed_words_idx.push_back(learned_words_idx[rand() 
                % learned_words_idx.size()]);
    }

	std::vector<int> to_review = ReviewAndRec::review(csr, reviewed_words_idx, learned_words_idx, num_to_review);
    int *data = (int *)malloc(sizeof(int) * to_review.size());
    for (int unsigned i = 0 ; i < to_review.size(); i++)
        data[i] = to_review[i];
    return data;
}


