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
    bool word_in_graph(std::string word) { return word_idx.find(word) != word_idx.end(); }
    std::string idx_to_word(int id) { return idx_words[id]; }

private:

    std::unordered_map<std::string, long> word_idx;
    std::unordered_map<long, std::string> idx_words;
    void get_counts(std::ifstream &edgefile);
    void add_edges(std::ifstream &edgefile);
};
