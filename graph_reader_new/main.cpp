#include <iostream>
#include "graph.h"

using std::cout;
using std::endl;

template<typename G>
void print_edges(G *csr) {
	for(int i = 0; i < csr->vert_count; i++)
    {
        int beg = csr->beg_pos[i];
        int end = csr->beg_pos[i+1];
        cout << i << "'s neighor list: ";
        for(int j = beg; j < end; j++)
        {
            cout << csr->csr[j] << " ";
            cout <<"Dist :" << csr->weight[j] <<" ";
        }
        cout << endl;
    }
}

int main(int argc, char **argv)
{
	if (argc < 4) {
		cout << "Input: ./exe beg csr weight" << endl;
		return 1;
	}
	
	const char *beg_file=argv[1];
	const char *csr_file=argv[2];
	const char *weight_file=argv[3];
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file, csr_file, weight_file);

	int num_source_words = argc - 4;
	const char **source_words = new const char*[num_source_words];
    for (int i = 0; i < num_source_words; i++) {
        source_words[i] = argv[i + 4];
    }
    for (int i = 0; i < num_source_words; i++) {
		cout << source_words[i] << endl;
        /*if (!csr.word_in_graph(source_words[i])) {
            cout << "Not found in graph: " << source_words[i] << endl;
            return 1;
        }*/
    }
    cout << endl;

    std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;
	return 0;	
}
