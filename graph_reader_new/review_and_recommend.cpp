#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>
#include <omp.h>
#include "graph.h"
#include "review_and_recommend.h"

using std::cout;
using std::endl;
using std::string;


// Inverse sum rule
double get_collective_dist(double *dist, int rows, int cols, int col) {
    double sum = 0;
    for (int i = 0; i < rows; i++) {
        sum += (1 / dist[i * cols + col]);
    }
    return sum;
}

int min_dist(double distances[], unsigned int path[], int verticies)
{
    double min = DOUBLE_MAX;
    int min_idx;
    for (int i = 0; i < verticies; i++)
    {
        if (!path[i] && distances[i] <= min)
        {
            min = distances[i];
            min_idx = i;
        }
    }
    return min_idx;
}

/**
 * Find shortest weighted path using djikstra's algorithm
 */
double shortest_path_weights(CSR *csr, int source, int target)
{
    int verticies = csr->vert_count;
    // distance from start to vertex 
    double distances[verticies];
    // bitset true if included in path
    unsigned int path[verticies];
    for (int i = 0; i < verticies; i++)
    {
        distances[i] = DOUBLE_MAX;
        path[i] = 0;
    }

    //int source_idx = csr.word_to_idx(source); TODO?
    distances[source] = 0;
    for (int count = 0; count < verticies - 1; count++)
    {
        int cur = min_dist(distances, path, verticies);
        path[cur] = true;

        // Update distances
        for (int i = csr->beg_pos[cur]; i < csr->beg_pos[cur+1]; i++)
        {
            //int neighbor = csr.word_to_idx(csr.adj_list[i]); TODO
			int neighbor = csr->csr[i];
            if (!path[neighbor] && 
                    distances[cur] != DOUBLE_MAX &&
                     distances[cur] + csr->weight[i] < distances[neighbor])
            {
                distances[neighbor] = distances[cur] + csr->weight[i];
            }
        }
    }
    return distances[target];
}

// Single Shortest Path from Source... Source is given source word
void SSSP(CSR *csr, int source_word, double *dist, int row) {

    int cols = csr->vert_count;
    for (int i = 0; i < cols; i++) {
        double tmp = shortest_path_weights(csr, source_word, i);
        dist[row * cols + i]  = tmp;
    }
}

std::vector<WordDist*> collective_closest(std::vector<int> source_words, int n, CSR *csr) {
    // Row for each source word, col for each vtx
    double *dist = (double *)malloc(sizeof(double) * n * csr->vert_count);

    // All vtxs, sorted in terms of closest
	WordDist ** word_dist = (WordDist **)malloc(sizeof(WordDist*) * csr->vert_count);
    //std::vector<WordDist*> word_dist;

    // Fill out dists to all vtxs (dist col) from word (dist row)
	#pragma omp parallel 
	{
		int threadcount = omp_get_num_threads();
		int threadid = omp_get_thread_num();
		#pragma omp single
		{
			cout << "Using " << threadcount << " threads" << endl;
		}
		#pragma omp for schedule(static)
		for (int i = 0; i < n; i++) {
			SSSP(csr, source_words[i], dist, i);
			cout << "SSSP done for " << threadid << endl;
		}

		// Get collective dist of vtx (col) to all source words (row)
		#pragma omp for schedule(static)
		for (int i = 0; i < csr->vert_count; i++) {
			WordDist *wd = new WordDist(get_collective_dist(dist, n, csr->vert_count, i), i);
			word_dist[i] = wd;
		}
	}

    // Word has no relation to given set
    double no_relation = (1 / DOUBLE_MAX) * n;

    std::vector<WordDist*> related_words;

    // Filter out all dists that are 0 or not related
    std::copy_if(word_dist, word_dist + csr->vert_count, 
            std::back_inserter(related_words), [no_relation](WordDist *a) -> bool {
                    return a->dist != DOUBLE_INF && a->dist != no_relation;
    });
    
    // Sort in terms of collect closest
    sort(related_words.begin(), related_words.end(), [](WordDist *a, WordDist *b) -> bool
    {
        return a->dist < b->dist;
    });
    return related_words;
}

std::vector<WordDist*> recommend(CSR *csr, std::vector<int> &source_words, int num_recs) {

	double start_time = omp_get_wtime();
    std::vector<WordDist*> closest_words = collective_closest(source_words, source_words.size(), csr);
	cout << "Time: " << omp_get_wtime() - start_time << endl;
	if (num_recs < closest_words.size())
		closest_words.resize(num_recs);
	return closest_words;
} 

std::vector<int> review (CSR *csr, std::vector<int> &reviewed, std::vector<int> &learned, int rev_count) {
	double start_time = omp_get_wtime();
	std::vector<WordDist*> word_dists = collective_closest(reviewed, reviewed.size(), csr);
	std::vector<int> cur_review_set;
	double collec_closest_time = omp_get_wtime();
	cout << "Algo Time: " << collec_closest_time - start_time << endl;
	
	for (int i = 0; i < word_dists.size(); i++) {
		int cur_id = word_dists[i]->word_id;
		bool is_learned = std::find(learned.begin(), learned.end(), cur_id) != learned.end();
		bool is_in_cur_rev= std::find(cur_review_set.begin(), cur_review_set.end(), cur_id) != cur_review_set.end();
		if (is_learned && !is_in_cur_rev)
			cur_review_set.push_back(cur_id);
		if (cur_review_set.size() == rev_count)
			break;
	}
	double final_time = omp_get_wtime() - start_time;
	cout << "Final Time: " << final_time << endl;

	return cur_review_set;
}
