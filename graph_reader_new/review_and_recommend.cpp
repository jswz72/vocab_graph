#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>
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
    std::vector<WordDist*> word_dist;

    // Fill out dists to all vtxs (dist col) from word (dist row)
    for (int i = 0; i < n; i++)
        SSSP(csr, source_words[i], dist, i);

    // Get collective dist of vtx (col) to all source words (row)
    for (int i = 0; i < csr->vert_count; i++) {
		WordDist *wd = new WordDist(get_collective_dist(dist, n, csr->vert_count, i), i);
        word_dist.push_back(wd);
    }

    // Word has no relation to given set
    double no_relation = (1 / DOUBLE_MAX) * n;

    std::vector<WordDist*> related_words;

    // Filter out all dists that are 0 or not related
    std::copy_if(word_dist.begin(), word_dist.end(), 
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

void review_and_rec(CSR *csr, std::vector<int> &source_words, std::vector<string> word_mappings, int n) {

    std::vector<WordDist*> closest_words = collective_closest(source_words, source_words.size(), csr);
    cout << "\nClosest words:" << endl;
    for (int i = 0; i < n; i++) {
        if (i >= closest_words.size()) {
            cout << "End" << endl;
            break;
        }
        cout << word_mappings[closest_words[i]->word_id] << " (Dist: "
            << closest_words[i]->dist << ")" << endl;
    }
} 