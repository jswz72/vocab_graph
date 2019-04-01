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
    double memory = 0.0;  // Current memory of wor
    int last_learned = -1;   // how many cycles ago was learned
    unsigned int strength = 0;   // Strength of memory for specific word (how fast memory decays)
    WordMem(int word_id, unsigned int score): word_id(word_id), score(score) {};
    int word_id;    // ID of word in graph
    int score; // Score of original ordering via collective closest, lower is beter

    std::string to_string() {
        std::stringstream ss;
        ss << "ID: " << word_id << " Mem: " << memory << " Last learned: " << last_learned
            << " Strength: " << strength;
        return ss.str();
    }

    bool operator<(const WordMem& b) const {
        // Tie breaker is what is most alike to reviewed words
        if (memory == b.memory)
            return score < b.score;
        return memory < b.memory;
    }
};

void review_cycle(WordMem *word_mems, int n) {
    for (int i = 0; i < n; i++) {
        auto wm = &word_mems[i];
        if (!wm->strength)
            continue;
        wm->memory = exp(-wm->last_learned/(double) wm->strength);
        wm->last_learned++;
    }
}

void pick_word(WordMem &wm) {
    wm.memory = 1;
    wm.last_learned = 0;
    wm.strength++;
}

extern "C" int *review(const char *csr_filename, const char **words_in, 
        unsigned int num_words, const char **learned_words_in, 
        unsigned int num_learned_words, const char **reviewed_words_in, 
        unsigned int num_reviewed_words, unsigned int num_to_review, unsigned int num_cycles=1)
{
    // Get filename info and initialize graph
    string base_filename (csr_filename);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

    // Create words, learned, and reviewed vectors
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
    // Take random word from learned for source if not given any reviewed
    if (!num_reviewed_words) {
        reviewed_words_idx.push_back(learned_words_idx[rand() % learned_words_idx.size()]);
    }

    std::set<int> reviewed_set(reviewed_words_idx.begin(), reviewed_words_idx.end());
    std::set<int> learned_set(learned_words_idx.begin(), learned_words_idx.end());

    WordMem *word_mems = (WordMem*)malloc(sizeof(WordMem) * num_learned_words);
    // Words to review in order of relatedness to already-reviewed words
	std::vector<int> to_review(num_learned_words);
    // Review recommendation matrix, each row is review cycle
    int *review_recs = (int *)malloc(sizeof(int) * num_to_review * num_cycles);

    for (int cycle = 0; cycle < num_cycles; cycle++) {
        cout << "Cycle " << cycle << " review recs: " << endl;
        // If all learned words aren't in reviewed set
        if (!std::includes(reviewed_set.begin(), reviewed_set.end(), learned_set.begin(), learned_set.end())) {
            cout << "Doing collec closest computation" << endl;
            to_review = ReviewAndRec::review(csr, reviewed_words_idx, learned_words_idx, num_to_review);
            for (int i = 0; i < to_review.size(); i++) {
               word_mems[i] = WordMem(to_review[i], i); 
            }
        }
        // Take words to review this cycle
        for (int i = 0; i < num_to_review; i++) {
            // Debug
            cout << word_mems[i].to_string() << endl;

            pick_word(word_mems[i]);
            review_recs[cycle * num_to_review + i] = word_mems[i].word_id;
            // Add to reviewed set if not there
            if (reviewed_set.find(word_mems[i].word_id) == reviewed_set.end()) {
                reviewed_set.insert(word_mems[i].word_id);
                reviewed_words_idx.push_back(word_mems[i].word_id);
            }
        }
        cout << endl;
        // Increment memory
        review_cycle(word_mems, num_learned_words);
        // Sort words have worst memory of
        std::sort(word_mems, word_mems + num_learned_words);
    }

    return review_recs;
}

