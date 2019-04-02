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

extern "C" int *recommend(const char *csr_filename, const char **words_in, 
        unsigned int num_words, unsigned int *num_recs, const char **source_words,
        unsigned int num_source_words, bool use_rec_pool=false, 
        const char **rec_pool_in=0, unsigned int num_rec_pool=0) 
{
    
    string base_filename(csr_filename);
    string beg_file = base_filename + "_beg_pos.bin";
    string csr_file = base_filename + "_csr.bin";
    string weight_file = base_filename + "_weight.bin";


    graph<long, long, double, long, long, double> *csr = 
        new graph <long, long, double, long, long, double>
        (beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

    std::vector<string> words(words_in, words_in + num_words);
    std::vector<int> source_word_idxs;
    for (unsigned int i = 0; i < num_source_words; i++) {
        string source_word (source_words[i]);
        source_word_idxs.push_back(Utils::find_word(source_word, words));
    }

    std::vector<WordDist*> closest_words;
    if (use_rec_pool) {
        std::unordered_set<int> rec_pool;
        for (unsigned int i = 0; i < num_rec_pool; i++) {
            string rec_word(rec_pool_in[i]);
            int idx = Utils::find_word(rec_word, words);
            if (idx >= 0)
                rec_pool.insert(idx);
        }
        closest_words = ReviewAndRec::recommend(csr, source_word_idxs, *num_recs, true, rec_pool);
    } else {
            closest_words = ReviewAndRec::recommend(csr, source_word_idxs, *num_recs);
    }
    int *word_ids = (int *)malloc(sizeof(int) * closest_words.size());
    for (unsigned int i = 0; i < closest_words.size(); i++)
        word_ids[i] = closest_words[i]->word_id;
    *num_recs = closest_words.size();
    return word_ids;
}
