#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <cstdlib>
#include <climits>
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

// Return Levenshtein edit distance between two words
int leven_dist(const char *a, const char *b) {
    int *dists = (int *)malloc(sizeof(int) * (strlen(a) + 1) * (strlen(b) + 1));
    int cols = strlen(a) + 1;
    for (int i = 0; i <= strlen(b); i++)
        for (int j = 0; j <= strlen(a); j++)
            dists[i * cols + j] = 0;

    for (int i = 0; i <= strlen(a); i++)
        dists[i] = i;

    for (int i = 0; i <= strlen(b); i++)
        dists[i * cols] = i;

    for (int i = 1; i <= strlen(b); i++) {
        for (int j = 1; j <= strlen(a); j++) {
            int indicator = a[j - 1] != b[i - 1];
            dists[i * cols + j] = std::min({dists[i * cols + j - 1] + 1,
                    dists[(i - 1) * cols + j] + 1,
                    dists[(i - 1) * cols + j - 1] + indicator});
        }
    }

    return dists[strlen(b) * cols + strlen(a)];
}

struct EditDist {
    const char *word;
    int edit_dist;
    EditDist(const char *word): word(word) {};
};

extern "C" const char **recommend_spelling(const char **source_words, int num_source_words, 
        const char **all_words, int num_all_words, int num_recs) 
{
    EditDist *dists = (EditDist *)malloc(sizeof(EditDist) * num_all_words);
    for (int i = 0; i < num_all_words; i++) {
        dists[i] = EditDist(all_words[i]);
        dists[i].edit_dist = 0;
    }
    for (int i = 0; i < num_source_words; i++) {
        #pragma omp for
        for (int j = 0; j < num_all_words; j++) {
           int dist = leven_dist(source_words[i], all_words[j]);
            dists[j].edit_dist += dist;
        }
    }
    std::sort(dists, dists + num_all_words, [](EditDist a, EditDist b) -> bool
    {
        return a.edit_dist < b.edit_dist;
    });
    std::unordered_set<std::string> source_words_set;
    for (int i = 0; i < num_source_words; i++) {
        source_words_set.insert(std::string(source_words[i]));
    }
    const char **ret_arr = (const char **)malloc(sizeof(const char *) * num_recs);
    int added = 0;
    for (int i = 0; added < num_recs && i < num_all_words; i++) {
        if (source_words_set.find(std::string(dists[i].word)) == source_words_set.end()) {
            ret_arr[added++] = dists[i].word;
        }
    }
    return ret_arr;
}

struct PHEditDist {
    int id;
    int edit_dist;
    PHEditDist(int id): id(id), edit_dist(0) {};
};

extern "C" int *recommend_phonetic(const char **source_phonetics,
		unsigned int *source_sums, unsigned int source_sums_size,
		const char **word_phonetics, unsigned int *word_sums,
		unsigned int word_sums_size)
{
	int num_words = word_sums_size - 1;
	int num_source_words = source_sums_size - 1;
	PHEditDist *dists = (PHEditDist *)malloc(sizeof(PHEditDist) * num_words);

	for (int i = 0; i < num_words; i++)
        dists[i] = PHEditDist(i);

	for (int i = 0; i < num_source_words; i++) {
		for (int j = 0; j < num_words; j++) {
			int min_dist = INT_MAX;
			for (int sub_i = source_sums[i]; sub_i < source_sums[i + 1];
				   	sub_i++) {
				for (int sub_j = word_sums[j]; sub_j < word_sums[j + 1];
						sub_j++) {
					int dist = leven_dist(source_phonetics[sub_i],
							word_phonetics[sub_j]);
					if (dist < min_dist)
						min_dist = dist;
				}
			}
			dists[j].edit_dist += min_dist;
		}
	}

	std::sort(dists, dists + num_words, [](PHEditDist a, PHEditDist b) -> bool
    {
        return a.edit_dist < b.edit_dist;
    });
	int *ret_arr = (int *)malloc(sizeof(int) * (num_words));
	for (int i = 0; i < num_words; i++) {
		ret_arr[i] = dists[i].id;
	}
	return ret_arr;
}



