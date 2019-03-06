#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "CSR.h"

using std::cout;
using std::endl;
using std::string;

struct WordDist {
    float dist;
    int word_id;
    WordDist(float dist, int id): dist(dist), word_id(id) {};
};

// Inverse sum rule
float get_collective_dist(float *dist, int rows, int cols, int col) {
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum += (1 / dist[i * cols + col]);
    return sum;
}

long shortest_path_weights(CSR *csr, string source, long target)
{
    int verticies = csr.vert_count;
    // distance from start to vertix (index of arr)
    long distances[verticies];
    // bitset true if included in path
    unsigned int path[verticies];
    for (long i = 0; i < verticies; i++)
    {
        distances[i] = INT_MAX;
        path[i] = 0;
    }
    distances[g->beg_pos[0]] = 0;
    for (long count = 0; count < verticies - 1; count++)
    {
        long cur = min_dist(distances, path, verticies);
        cout << " cur: " << cur << endl;
        path[cur] = true;

        // Update distances
        for (long i = g->beg_pos[cur]; i < g->beg_pos[cur+1]; i++)
        {
            long neighbor = g->csr[i];
            cout << "neighbor: " << neighbor << endl;
            if (!path[neighbor] && 
                    distances[cur] != INT_MAX &&
                     distances[cur] + g->weight[i] < distances[neighbor])
            {
                distances[neighbor] = distances[cur] + g->weight[i];
            }
        }
        cout << " count: " << count << endl;
        for (long i = 0; i < verticies; i++) {
            cout << i << " tt " << distances[i] << endl;
        }
        cout << endl;
    }
    return distances[target];
}

// Single Shortest Path from Source - Source is given source word
void SSSP(CSR &csr, string &source_word, float *dist, int row) {
    int cols = csr.vertex_count;
}

std::vector<WordDist*> collective_closest(string *source_words, int n, CSR &csr) {
    // Row for each source word, col for each vtx
    float *dist = (float *)malloc(sizeof(float) * n * csr.vertex_count);

    // All vtxs, sorted in terms of closest
    std::vector<WordDist*> word_dist;

    // Fill out dists to all vtxs (dist col) from word (dist row)
    for (int i = 0; i < n; i++)
        SSSP(csr, source_words[i], dist, i);

    // Get collective dist of vtx (col) to all source words (row)
    for (int i = 0; i < csr.vertex_count; i++)
        word_dist.push_back(new WordDist(get_collective_dist(dist, n, csr.vertex_count, i), i));


    // Sort in terms of collect closest
    sort(word_dist.begin(), word_dist.end(), [](WordDist *a, WordDist *b) -> bool
    {
        return a->dist > b->dist;
    });
    return word_dist;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Enter edgelist filename" << endl;
        if (argc < 3)
            cout << "Enter source words" << endl;
        return 1;
    }

    const char *edgefilename = argv[1];

    int num_source_words = argc - 2;
    const char **source_words = new const char*[num_source_words];
    for (int i = 0; i < num_source_words; i++) {
        source_words[i] = argv[i + 3];
    }


    CSR *csr = new CSR(edgefilename);
    std::cout << "Num verticies " << csr->vertex_count << endl;
    std::cout << "Num edges " << csr->edge_count << endl;

    for (int i = 0; i < num_source_words; i++) {
        cout << source_words[i] << endl;
    }

    /*int i = csr->word_to_idx("april");
    long beg = csr->beg_pos[i];
    long end = csr->beg_pos[i + 1];
    cout << csr->words[i] << " neighbor list" << endl;
    for (int j = beg; j < end; j++) {
        cout << csr->adj_list[j] << ", weight: " << csr->weight_list[j] << endl;
    }
    cout << endl;*/
} 
