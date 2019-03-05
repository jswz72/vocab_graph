#include <string>
#include <fstream>
#include <unordered_map>

struct CSR {
    long *beg_pos;
    std::string *words;

    std::string *adj_list;
    double *weight_list;

    long vertex_count = 0;
    long edge_count = 0;

    CSR(const char *edgefilename);
    
    long word_to_idx(std::string word) { return word_idx[word]; }

private:

    std::unordered_map<std::string, long> word_idx;
    void get_counts(std::ifstream &edgefile);
    void add_edges(std::ifstream &edgefile);
};
