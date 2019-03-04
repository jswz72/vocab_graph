#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "CSR.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Enter edgelist filename" << endl;
        return 1;
    }
    const char *edgefilename = argv[1];

    CSR *csr = new CSR(edgefilename);
    std::cout << "Num verticies " << csr->vertex_count << endl;
    std::cout << "Num edges " << csr->edge_count << endl;

    for (long i = 0; i < csr->vertex_count; i++) {
        long beg = csr->beg_pos[i];
        long end = csr->beg_pos[i + 1];
        //cout << csr->words[i] << " neighbor list" << endl;
        for (int j = beg; j < end; j++) {
            //cout << csr->adj_list[j] << ", weight: " << csr->weight_list[j] << endl;
        }
        //cout << endl;
    }
    cout << csr->word_to_idx("to");
} 
