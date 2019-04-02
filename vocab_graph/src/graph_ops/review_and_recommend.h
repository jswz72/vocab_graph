#include <unordered_set>
#include "graph.h"
#ifndef ReviewAndRec
#define ReviewAndRec

struct WordDist {
    double dist; // Inverse collective dist to all source words 
    int word_id; // ID of source word corresponding to idx in graph
    WordDist(double dist, int id): dist(dist), word_id(id) {};
};
const double DOUBLE_MAX = std::numeric_limits<double>::max();
const double DOUBLE_INF = std::numeric_limits<double>::infinity();

typedef graph<long, long, double, long, long, double> CSR;

/**
 * Given souce words (known), graph, and number to recommend,
 * Recommend new words to learn based of their collective closeness
 * to aready known words.
 * Optionally takes a recommendation pool - output recommendations will only be given if
 * they are present in this pool
 */
std::vector<WordDist*> recommend(CSR *csr, std::vector<int> &source_words, unsigned int num_recs, bool use_rec_pool = false, std::unordered_set<int> const &rec_pool = std::unordered_set<int>());

/**
 * Given list of reviewed words, learned words, graph, and number of words to recommend to review,
 * Return order to review learned words based on collctive closeness
 * to already reviewed words
 */
std::vector<int> review (CSR *csr, std::vector<int> &reviewed, std::vector<int> &learned, unsigned int rev_count);
#endif
