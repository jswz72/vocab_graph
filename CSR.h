#include <string>
#include <fstream>
#include <unordered_map>
#include <iostream>

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

    std::string idx_to_word(int id) { return words[id]; }

    void show_edges() { 
        for (int i = 0; i < vertex_count; i++) {
            int beg = beg_pos[i];
            int end = beg_pos[i + 1];
            std::cout << words[i] << " neighbor list" << std::endl;
            for (int j = beg; j < end; j++) {
                std::cout << adj_list[j] << ", weight: " << weight_list[j] << std::endl;
            }
        }
    }

    void show_verticies() {
        for (int i = 0 ;i < vertex_count; i++) {
            std::cout << words[i] << std::endl;
        }
    }
private:

    std::unordered_map<std::string, long> word_idx;
    void get_counts(std::ifstream &edgefile);
    void add_edges(std::ifstream &edgefile);
};
