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

extern "C" int *recommend(const char *csr_filename, unsigned int *num_recs, 
        int *source_words, unsigned int num_source_words, bool use_rec_pool=false, 
        int *rec_pool_in=0, unsigned int num_rec_pool=0) 
{
    
    string base_filename(csr_filename);
    string beg_file = base_filename + "_beg_pos.bin";
    string csr_file = base_filename + "_csr.bin";
    string weight_file = base_filename + "_weight.bin";


    graph<long, long, double, long, long, double> *csr = 
        new graph <long, long, double, long, long, double>
        (beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

    std::vector<int> source_word_idxs(source_words, source_words + num_source_words);

    std::vector<WordDist*> closest_words;
    if (use_rec_pool) {
        std::unordered_set<int> rec_pool(rec_pool_in, rec_pool_in + num_rec_pool);
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

extern "C" int recommend_group(const char *csr_filename, int *source_words, 
        unsigned int num_source_words, int *word_groups,
        unsigned int num_groups, unsigned int *group_sizes)
{
    string base_filename(csr_filename);
    string beg_file = base_filename + "_beg_pos.bin";
    string csr_file = base_filename + "_csr.bin";
    string weight_file = base_filename + "_weight.bin";


    graph<long, long, double, long, long, double> *csr = 
        new graph <long, long, double, long, long, double>
        (beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

    std::vector<int> source_word_idxs(source_words, source_words + num_source_words);
    std::vector<std::unordered_set<int> > groups;
    
    for (int i = 0; i < num_groups; i++) {
        int beg = group_sizes[i];
        int end = group_sizes[i + 1];
        groups.push_back(std::unordered_set<int>(word_groups + beg, word_groups + end));
    }

    return ReviewAndRec::recommend_group(csr, source_word_idxs, groups);
}

int leven_dist(const char *a, const char *b) {
    int *dists = (int *)malloc(sizeof(int) * (strlen(a) + 1) * (strlen(b) + 1));
    int cols = strlen(a) + 1;
    for (int i = 0; i <= strlen(b); i++) {
        for (int j = 0; j < strlen(a); j++) {
            if (i == 0)
                dists[i * cols + j] = j;
            else if (j == 0)
                dists[i * cols + j] = i;
            else
                dists[i * cols + j] = 0;
        }
    }

    for (int j = 1; j <= strlen(b); j++) {
        for (int i = 1; i <= strlen(a); i++) {
            int indicator = a[i - 1] != b[j - 1];
            dists[j * cols + i] = std::min({dists[j * cols + i - 1] + 1,
                    dists[(j - 1) * cols + i] + 1,
                    dists[(j - 1) * cols + i - 1] + indicator});
        }
    }

    return dists[strlen(b) * cols + strlen(a)];
}

extern "C" int *recommend_spelling(const char **source_words, int num_source_words, 
        const char **all_words, int num_all_words) {
    cout << leven_dist(source_words[0], source_words[1]) << endl;
    return 0;
}



