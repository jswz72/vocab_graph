#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <set>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include "review_and_recommend.h"
#include "utils.h"


using std::string;
using std::cout;
using std::endl;

struct WordMem {
    int word_id;    // ID of word in graph
    double memory = 0.0;  // Current memory of wor
    int last_learned = -1;   // How many time units ago was learned
    unsigned int strength = 0;   // Strength of memory for specific word (how fast memory decays)

    WordMem(int word_id, int last_learned, unsigned int strength): word_id(word_id), 
        last_learned(last_learned), strength(strength) {};

    std::string to_string() {
        std::stringstream ss;
        ss << "ID: " << word_id << " Mem: " << memory << " Last learned: " << last_learned
            << " Strength: " << strength;
        return ss.str();
    }
};

// Calculate memory/forgetfullness for each word
void memory_cycle(WordMem *word_mems, int n) {
    for (int i = 0; i < n; i++) {
        auto wm = &word_mems[i];
        wm->memory = exp(-wm->last_learned/(double) wm->strength);
    }
}

extern "C" WordMem *review(const char *csr_filename, const char **words_in, 
        unsigned int num_words, const char **learned_words_in, 
        unsigned int num_learned_words, int *t_params, double *s_params, 
        char **reviewed_words_in, unsigned int num_reviewed_words)
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
    for (int i = 0; i < num_learned_words; i++) {
        string lw(learned_words_in[i]);
        learned_words_idx.push_back(Utils::find_word(lw, words));
    }

    std::vector<int> reviewed_words_idx;
    for (int i = 0; i < num_reviewed_words; i++) {
        string rw(reviewed_words_in[i]);
        reviewed_words_idx.push_back(Utils::find_word(rw, words));
    }
    
    // Take random from learned if not given any reviewed
    if (!num_reviewed_words) {
        reviewed_words_idx.push_back(learned_words_idx[rand() 
                % learned_words_idx.size()]);
    }

    WordMem *word_mems = (WordMem*)malloc(sizeof(WordMem) * num_learned_words);

	std::vector<int> to_review = ReviewAndRec::review(csr, reviewed_words_idx, 
            learned_words_idx);

    /*for (int i = 0 ; i < learned_words_idx.size() - reviewed_words_idx.size(); i++) {
        int word_id = to_review[i];
        word_mems[i] = WordMem(word_id, t_params[word_id], s_params[word_id]);
    }
    // Add reviewed words to back
    for (int i = 0; i < reviewed_words_idx.size(); i++) {
        int word_id = reviewed_words_idx[i];
        word_mems[i] = WordMem(word_id, t_params[word_id], s_params[word_id]);
    }
    memory_cycle(word_mems, num_learned_words);*/

    return word_mems;
}


