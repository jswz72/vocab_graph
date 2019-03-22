#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <vector>
#include <tuple>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include "error_handler.h"
#include "wtime.h"

using std::cout;
using std::endl;
using std::string;

const string ofname = "edge-list.txt";
const string woname = "word-order.txt";

double __device__ euclidean_dist(double *x, double *y, int dims) {
    double sum = 0;
    for (unsigned int i = 0; i < dims; i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

__global__ void create_edge_list_kernel(double *word_vecs, size_t word_vecs_len, 
        double threshold, double *edge_list, int wv_dims) {
    int wv_num_cols = wv_dims + 1;
 
    const int my_thread_id = threadIdx.x + blockIdx.x * blockDim.x;
    const int thread_count = blockDim.x * gridDim.x;

    int tid = my_thread_id;
    while (tid < word_vecs_len) {
        for (unsigned int j = 0; j < word_vecs_len; j++) {
            int write_idx = (tid * word_vecs_len * 3) + (j * 3);
            // No edges to self
            if (tid != j) { 
                double dist = euclidean_dist(word_vecs + 
                        (tid * wv_num_cols + 1), 
                        word_vecs + (j * wv_num_cols + 1), wv_dims); 
                if (!threshold || dist < threshold) {
                    double vtx1 = word_vecs[tid * wv_num_cols];
                    double vtx2 = word_vecs[j * wv_num_cols];
                    edge_list[write_idx] = vtx1;
                    edge_list[write_idx + 1] = vtx2;
                    edge_list[write_idx + 2] = dist;
                } else {
                    edge_list[write_idx] = -1;
                }
            } else {
                edge_list[write_idx] = -1;
            }
        }
        tid += thread_count;
    }
};

double *parse_word_vectors(const char *vecfilename, int limit, int dims, size_t *word_size) {
    std::ifstream infile(vecfilename);
    string line;
    int i = 0;
    while (std::getline(infile, line))
        i++;

    double *words = (double *)malloc(sizeof(double) * i * (dims + 1));

    infile.clear();
    infile.seekg(0, std::ios::beg);

	std::ofstream outfile(woname);
    int row = 0;

    //only read until limit lines
    int limit_counter = 0;
    while (std::getline(infile, line)) {
        if (limit && ++limit_counter > limit) {
            break;
        }
        std::istringstream iss(line);
        string word;
        iss >> word;
        // only take alphanumeric 
        bool alphanum = true;
        for (auto const& ch: word) {
            if (!isalpha(ch)) {
                alphanum = false;
                break;
            }
        }
        if (!alphanum)
            continue;
        // Write to word-order
        outfile << word << endl;

        int base = row * 51;
        words[base] = row;
        double d;
        int d_i = 1;
        while ((iss >> d)) {
            words[base + d_i] = d;
            d_i++;
        }
        row++;
    }
    *word_size = row;
    return words;
}

int write_edge_list(double *edge_list, size_t size) {
	std::ofstream outfile(ofname);
    int counter = 0;
	for (unsigned int i = 0; i < size; i++) {
        int base = i * 3;
        if (edge_list[base] == -1)
            continue;
		outfile << edge_list[base] << " " 
			<< edge_list[base + 1] << " " << edge_list[base + 2] << endl;
        counter++;
	}
    return counter;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "./exec vecfile outfile(optional) "
			<< "threshold(optional) limit(optional) "
			<< "to_nums(optional)" << endl;
        return 1;
    }
    const char *vecfilename = argv[1];
    const int write_file = argc > 2 ? atoi(argv[2]) : 0;
    const double threshold = argc > 3 ? atof(argv[3]) : 0;
    const int limit = argc > 4 ? atoi(argv[4]) : 0;
	const int to_nums = argc > 5 ? atoi(argv[5]) : 0;

    size_t word_size;
    double *words = parse_word_vectors(vecfilename, limit, 50, &word_size);
    
    size_t edge_list_size = word_size * word_size;

	cout << "Word vec size: " << word_size << endl;

	double *words_d;
	HANDLE_ERR(cudaMalloc((void **) &words_d, sizeof(double) * word_size * 51));
	double *edge_list_d;
	HANDLE_ERR(cudaMalloc((void **) &edge_list_d, sizeof(double) * edge_list_size *3));

    HANDLE_ERR(cudaMemcpy (words_d, words, sizeof(double) * word_size * 51, cudaMemcpyHostToDevice));

    double starttime = wtime();
    create_edge_list_kernel <<< 128, 128 >>> (words_d, word_size, threshold, edge_list_d, 50);
    cudaDeviceSynchronize();
    double endtime = wtime() - starttime;

	double *edge_list = (double *)malloc(sizeof(double) * 3 * edge_list_size);
    HANDLE_ERR(cudaMemcpy(edge_list, edge_list_d, sizeof(double) * 3 * edge_list_size, cudaMemcpyDeviceToHost));

    int num_edges = write_edge_list(edge_list, edge_list_size);

    cout << "Num edges: " << num_edges << endl;
    cout << "Time: " << endtime << endl;
}
