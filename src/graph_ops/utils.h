#ifndef Utils
#define Utils
std::vector<std::string> get_word_mapping(const char *mapping_file);

template<typename G>
void print_edges(G *csr);

int find_word(std::string &word, std::vector<std::string> &words);
#endif
