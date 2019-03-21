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
#include "wtime.h"

using std::cout;
using std::endl;
using std::string;

const string ofname = "edge-list.txt";
const string woname = "word-order.txt";

__device__ euclidean_dist(double *x, double *y, int dims) {
    double sum = 0;
    for (unsigned int i = 0; i < dims; i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

__global__ void create_edge_list_kernel(double *word_vecs, size_t word_vecs_len, double threshold, 
        double *edge_list, int wv_dims, int *threadwork, size_t *edge_list_size) {
    int wv_num_cols = wv_dims + 1;

    int ie_max_size = word_vecs_len * word_vecs_len / blockDim.x * 3;
    double *inner_edge_list = (int *)malloc(sizeof(double) * ie_max_size);
    int ie_idx = 0;
 
    const int my_thread_id = threadIdx.x + blockIdx.x * blockDim.x;
    const int thread_count = blockDim.x * gridDim.x;

    int tid = my_thread_id;
    while (tid < word_vecs_len) {
        for (unsigned int j = 0; j < word_vecs_len; j++) {
            // No edges to self
            if (tid != j) { 
                double dist = euclidean_dist(word_vecs[tid * wv_num_cols + 1], word_vecs[j * wv_num_cols + 1], wv_dims); 
                if (!threshold || dist < threshold) {
                    double vtx1 = word_vecs[tid * wv_num_cols];
                    double vtx2 = word_vecs[j * wv_num_cols];
                    
                    int ie_base = ie_idx * 3;
                    inner_edge_list[ie_base] = vtx1;
                    inner_edge_list[ie_base + 1] = vtx2;
                    inner_edge_list[ie_base + 2] = dist;
                    ie_idx++;
                }
            }
        }
        tid += thread_count;
    }
    // Add size of inner edge list
    threadwork[my_thread_id + 1] = ie_idx + 1;

    // Single thread aggregates work
    if (threadIdx.x == 1) {
        for (int i = 1; i <= thread_count; i++) {
            threadwork[i] += threadwork[i - 1];
        }
    }

    // Give size back to caller
    *edge_list_size = threadwork[thread_count];

    // Copy thread's work to edge_list
    int base = thread_work[my_thread_id];
    for (int i = 0; i < thread_work[my_thread_id]; i++) {
        int el_row = (base + i) * 3;
        int ie_row = i * 3;
        for (int j = 0; j < 3; j++) {
            edge_list[el_row + j] = inner_edge_list[ie_row + j];
        }
    }
};

double *parse_word_vectors(const char *vecfilename, int limit, int dims, size_t word_size) {
    std::ifstream infile(vecfilename);
    string line;
    int i = 0;
    while (std::getline(infile, line))
        i++;

    double *words = (double *)malloc(sizeof(double) * i * (dims + 1));

    infile.clear();
    infile.seekg(0, std::ios::beg);

	std::ofstream outfile(ofname);
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
        int d_i = 0;
        while ((iss >> d)) {
            words[base + d_i] = d;
        }
        row++;
    }
    *word_size = row + 1;
    return words;
}

void write_edge_list(double *edge_list, size_t size) {
	std::ofstream outfile(ofname);
	for (unsigned int i = 0; i < size; i++) {
        base = i * 3;
		outfile << edge_list[base] << " " 
			<< edge_list[base + 1] << " " << edge_list[base + 2] << endl;
	}
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

	cout << "Word vec size: " << word_size << endl;

	double *words_d;
	HANDLE_ERR(cudaMalloc((void **) &words_d, sizeof(double) * word_size * 51));
	double *edge_list_d;
	HANDLE_ERR(cudaMalloc((void **) &edge_list, sizeof(double) * word_size * word_size * 3));
	double *threadwork_d;
	HANDLE_ERR(cudaMalloc((void **) &threadwork_d, sizeof(int) * 128 * 128));

    HANDLE_ERR(cudaMemcpy (words_d, words, sizeof (double) * word_size * 51, cudaMemcpyHostToDevice));

    double starttime = wtime();
    create_edge_list_kernel <<< 128, 128 >>> (words_d, word_size, threshold, edge_list_d, 50, threadwork_d, edge_list_size);
    double endtime = wtime() - starttime;

	double *edge_list = (double *)malloc(sizeof(double) * 3 * edge_list_size);

    HANDLE_ERR(cudaMemcpy (edge_list, edge_list_d, sizeof(double) * 3 * edge_list_size, cudaMemcpyDeviceToHost));

    write_edge_list(edge_list, edge_list_size);

    cout << "Num edges: " << edge_list.size() << endl;
    cout << "Time: " << endtime << endl;
}
