#include "graph.h"
#ifndef ReviewAndRec
#define ReviewAndRec
struct WordDist {
    double dist;
    int word_id;
    WordDist(double dist, int id): dist(dist), word_id(id) {};
};
const double DOUBLE_MAX = std::numeric_limits<double>::max();
const double DOUBLE_INF = std::numeric_limits<double>::infinity();

typedef graph<long, long, double, long, long, double> CSR;
std::vector<WordDist*> recommend(CSR *csr, std::vector<int> &source_words, int num_recs);

std::vector<int> review (CSR *csr, std::vector<int> &reviewed, std::vector<int> &learned, int rec_count);
#endif
