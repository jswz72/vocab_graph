#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
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


/**
 * Inverse sum rule of distances to source words
 */
double get_collective_dist(double *dist, int rows, int cols, int col) {
    double sum = 0;
    for (int i = 0; i < rows; i++) {
        sum += (1 / dist[i * cols + col]);
    }
    return sum;
}

/**
 * Get minimum distance of indicies not included in path
 */
int min_dist(double *distances, unsigned int *path, int verticies)
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
 * Find shortest weighted path to all nodes from source using djikstra's algorithm
 */
double *shortest_path_weights(CSR *csr, int source)
{
    int verticies = csr->vert_count;
    // distance from start to vertex 
    double *distances = new double[verticies];
    // bitset true if included in path
    unsigned int path[verticies];
    for (int i = 0; i < verticies; i++)
    {
        distances[i] = DOUBLE_MAX;
        path[i] = 0;
    }

    distances[source] = 0;
    for (int count = 0; count < verticies - 1; count++)
    {
        int cur = min_dist(distances, path, verticies);
        path[cur] = true;

        // Update distances
        for (int i = csr->beg_pos[cur]; i < csr->beg_pos[cur+1]; i++)
        {
			int neighbor = csr->csr[i];
            if (!path[neighbor] && 
                    distances[cur] != DOUBLE_MAX &&
                     distances[cur] + csr->weight[i] < distances[neighbor])
            {
                distances[neighbor] = distances[cur] + csr->weight[i];
            }
        }
    }
    return distances;
}

/**
 * Return word indices that are collectively closest to all given source word indices,
 * using aggregated shortest path calculations from given graph.
 */
WordDist** collective_closest(std::vector<int> &source_words, int n, CSR *csr, bool use_rec_pool = false, std::unordered_set<int> const &rec_pool = std::unordered_set<int>()) {
    // Row for each source word, col for each vtx
    double *dist = (double *)malloc(sizeof(double) * n * csr->vert_count);

    // All vtxs, sorted in terms of closest
	WordDist ** word_dist = (WordDist **)malloc(sizeof(WordDist*) * csr->vert_count);

    // Fill out dists to all vtxs (dist col) from word (dist row)
	#pragma omp parallel 
	{
		int threadcount = omp_get_num_threads();
		#pragma omp single
		{
			cout << "Using " << threadcount << " threads" << endl;
		}
		#pragma omp for schedule(static)
		for (int i = 0; i < n; i++) {
			int cols = csr->vert_count;
			double *shortest_paths = shortest_path_weights(csr, source_words[i]);
			for (int j = 0; j < cols; j++) {
				dist[i * cols + j] = shortest_paths[j];
			}
		}

		// Get collective dist of vtx (col) to all source words (row)
		#pragma omp for schedule(static)
		for (int i = 0; i < csr->vert_count; i++) {
            if (!use_rec_pool || rec_pool.find(i) != rec_pool.end()) {
                WordDist *wd = new WordDist(get_collective_dist(dist, n, csr->vert_count, i), i);
                word_dist[i] = wd;
            } else {
                word_dist[i] = new WordDist(0, -1);
            }
        }
	}
    free(dist);

    WordDist **wd = word_dist;
    int wd_size = csr->vert_count;
    if (use_rec_pool) {
        WordDist **condensed_word_dist = new WordDist*[rec_pool.size()];
        int idx = 0;
        for (int i = 0; i < csr->vert_count; i++) {
            if (word_dist[i]->word_id != -1)
                condensed_word_dist[idx++] = word_dist[i];
        }
        free(word_dist);
        wd = condensed_word_dist;
        wd_size = rec_pool.size();
    }
    
    // Sort in terms of collect closest
	std::sort(wd, wd + wd_size, [](WordDist *a, WordDist *b) -> bool
    {
        return a->dist > b->dist;
    });

	return wd;
}

int collective_closest_group(std::vector<int> &source_words, int n, CSR *csr, 
        std::vector<std::unordered_set<int> > &groups) {
    // Row for each source word, col for each vtx
    double *dist = (double *)malloc(sizeof(double) * n * csr->vert_count);

    std::vector<double> sums(groups.size(), 0);

    // Fill out dists to all vtxs (dist col) from word (dist row)
	#pragma omp parallel 
	{
		int threadcount = omp_get_num_threads();
		#pragma omp single
		{
			cout << "Using " << threadcount << " threads" << endl;
		}
		#pragma omp for schedule(static)
		for (int i = 0; i < n; i++) {
			int cols = csr->vert_count;
			double *shortest_paths = shortest_path_weights(csr, source_words[i]);
			for (int j = 0; j < cols; j++) {
				dist[i * cols + j] = shortest_paths[j];
			}
		}
    }

    // Get collective dist of vtx (col) to all source words (row)
    for (int i = 0; i < csr->vert_count; i++) {
        // Skip source words
        if (std::find(source_words.begin(), source_words.end(), i) != source_words.end())
            continue;
        double dist_val = get_collective_dist(dist, n, csr->vert_count, i);
        for (int j = 0; j < groups.size(); j++) {
            if (groups[j].find(i) != groups[j].end()) {
                sums[j] += dist_val;
            }
        }
	}

    // Average sums for size of group
    for (int i = 0; i < sums.size(); i++) {
        sums[i] /= groups[i].size();
    }

	return std::distance(sums.begin(), std::max_element(sums.begin(), sums.end()));
}


std::vector<WordDist*> recommend(CSR *csr, std::vector<int> &source_words, unsigned int num_recs, bool use_rec_pool, std::unordered_set<int> const &rec_pool) {
	double start_time = omp_get_wtime();
    WordDist ** word_dist;
    int wd_size;
    if (use_rec_pool) {
        word_dist = collective_closest(source_words, source_words.size(), csr, use_rec_pool, rec_pool);
        wd_size = rec_pool.size();
    } else {
        word_dist = collective_closest(source_words, source_words.size(), csr);
        wd_size = csr->vert_count;
    }
	cout << "Algo Time: " << omp_get_wtime() - start_time << endl;

	std::vector<WordDist*> related_words;
	
    // Word has no relation to given set
    double no_relation = (1 / DOUBLE_MAX) * source_words.size();
	
    // TODO look at resizing to idx of first no_relation (since in-order), while filtering out DOUBLE_INFs
    // Filter out all dists that are 0 (source word) or not related to any source words
    std::copy_if(word_dist, word_dist + wd_size, std::back_inserter(related_words), 
			[no_relation] (WordDist *a) -> bool {
                    return a->dist != DOUBLE_INF && a->dist != no_relation;
    });

    
	if (num_recs < related_words.size())
		related_words.resize(num_recs);

	double final_time = omp_get_wtime() - start_time;
	cout << "Final Time: " << final_time << endl;

	return related_words;
}

int recommend_group(CSR *csr, std::vector<int> &source_words, std::vector<std::unordered_set<int> > &groups) {
    double start_time = omp_get_wtime();
    int to_review = collective_closest_group(source_words, source_words.size(), csr, groups);
    cout << "Algo Time: " << omp_get_wtime() - start_time << endl;
    return to_review;
}

std::vector<int> review (CSR *csr, std::vector<int> &reviewed, std::vector<int> &learned) {
	double start_time = omp_get_wtime();
	WordDist** word_dist = collective_closest(reviewed, reviewed.size(), csr);
	std::vector<int> cur_review_set;
	double collec_closest_time = omp_get_wtime();
	cout << "Algo Time: " << collec_closest_time - start_time << endl;

	// Get intersection of recommended words (word_dist) and already leared words (in sorted order)
	for (int i = 0; i < csr->vert_count; i++) {
		int cur_id = word_dist[i]->word_id;
		bool is_learned = std::find(learned.begin(), learned.end(), cur_id) != learned.end();
		bool is_in_cur_rev= std::find(cur_review_set.begin(), cur_review_set.end(), cur_id) != cur_review_set.end();
		
		// Skip already reviewed words
		if (word_dist[i]->dist == DOUBLE_INF)
			continue;
		if (is_learned && !is_in_cur_rev)
			cur_review_set.push_back(cur_id);
        if (cur_review_set.size() == learned.size() - reviewed.size())
			break;
	}
	double final_time = omp_get_wtime() - start_time;
	cout << "Final Time: " << final_time << endl;

	return cur_review_set;
}
