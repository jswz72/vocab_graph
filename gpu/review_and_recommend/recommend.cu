#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include "review_and_recommend.h"
#include "utils.h"

using std::cout;
using std::endl;
using std::string;


int main(int argc, char **argv) {
	if (argc < 5) {
		cout << "Input: ./exe base_file mapping_file num_recs source_words..." << endl;
		return 1;
	}
	
	string base_filename (argv[1]);
	string beg_file = base_filename + "_beg_pos.bin";
	string csr_file = base_filename + "_csr.bin";
	string weight_file = base_filename + "_weight.bin";

	const char *mapping_file = argv[2];
	int num_recs = atoi(argv[3]);
	
	graph<long, long, double, long, long, double> *csr = 
		new graph <long, long, double, long, long, double>
		(beg_file.c_str(), csr_file.c_str(), weight_file.c_str());

	std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;

	int num_source_words = argc - 4;
	std::vector<int> source_word_idxs;
	std::vector<string> words = Utils::get_word_mapping(mapping_file);

    for (int i = 0; i < num_source_words; i++) {
		const char *source_word = argv[i + 4];
		auto it = std::find(words.begin(), words.end(), source_word);
		if (it == words.end()) {
			cout << "Not found in graph: " << source_word << endl;
			return 1;
		}
		int idx = std::distance(words.begin(), it);
		source_word_idxs.push_back(idx);
    }

    // Copy source word idxs to device arr
    // Copy csr beg_pos arry into device arry
    // Copy csr csr arry into device arry
    // Copy csr weight arry into device arry
    //Crate closest_words of size num_rec
    // Pass in beg_pos, csr.csr, weight arrays, source_word_idxs arrays, num_recs, and vert_count, closest_words

    // Copy back closest_words

    WordDist **closest_words;
    int *source_idxs_d, *beg_pos_d, *csr_d, *weight_d;
    WordDist **closest_words_d;
    int *num_recs_d;

    HANDLE_ERR(cudaMalloc((void **) &source_idxs_d, sizeof(int) * source_word_idxs.size()));
    HANDLE_ERR(cudaMalloc((void **) &beg_pos_d, sizeof(int) * csr->vert_count));
    HANDLE_ERR(cudaMalloc((void **) &csr_d, sizeof(int) * csr->edge_count));
    HANDLE_ERR(cudaMalloc((void **) &weight_d, sizeof(int) * csr->edge_count));
    HANDLE_ERR(cudaMalloc((void **) &closest_words_d, sizeof(WordDist*) * num_recs));
    HANDLE_ERR(cudaMalloc((void **) &num_recs_d, sizeof(int)));

    HANDLE_ERR(cudaMemcpy (source_idxs_d, source_words_idxs, sizeof(int) * source_words_idxs.size(), cudaMemcpyHostToDevice));
    HANDLE_ERR(cudaMemcpy (beg_pos_d, csr->beg_pos, sizeof(int) * csr->vert_count, cudaMemcpyHostToDevice));
    HANDLE_ERR(cudaMemcpy (csr_d, csr->csr, sizeof(int) * csr->edge_count, cudaMemcpyHostToDevice));
    HANDLE_ERR(cudaMemcpy (weight_d, csr->weight, sizeof(int) * csr->edge_count, cudaMemcpyHostToDevice));
    HANDLE_ERR(cudaMemcpy (num_recs_d, num_recs, sizeof(int), cudaMemcpyHostToDevice));

	ReviewAndRec::recommend_kernel(beg_pos_d, csr_d, weight_d, source_idxs_d, num_recs, csr->vert_count, num_recs_d);

    HANDLE_ERR(cudaMemcpy (num_recs, num_recs_d, sizeof(int), cudaMemcpyDeviceToHost));
    HANDLE_ERR(cudaMemcpy (closest_words, closest_words_d, sizeof(WordDist*) * num_recs, cudaMemcpyDeviceToHost));

	cout << "\nLearning recommendations :" << endl;
	for (int i = 0; i < num_recs; i++) {
		cout << words[closest_words[i]->word_id] << " (Value: "
			<< closest_words[i]->dist << ")" << endl;
	}
	return 0;	
}
