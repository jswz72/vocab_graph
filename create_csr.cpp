#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include "CSR.h"

using std::cout;
using std::endl;
using std::string;

const double DOUBLE_MAX = std::numeric_limits<double>::max();
const double DOUBLE_INF = std::numeric_limits<double>::infinity();
double NO_RELATION;

struct WordDist {
    double dist;
    int word_id;
    WordDist(double dist, int id): dist(dist), word_id(id) {};
};

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
double shortest_path_weights(CSR &csr, const char *source, int target)
{
    int verticies = csr.vertex_count;
    // distance from start to vertex 
    double distances[verticies];
    // bitset true if included in path
    unsigned int path[verticies];
    for (int i = 0; i < verticies; i++)
    {
        distances[i] = DOUBLE_MAX;
        path[i] = 0;
    }

    int source_idx = csr.word_to_idx(source);
    distances[source_idx] = 0;
    for (int count = 0; count < verticies - 1; count++)
    {
        int cur = min_dist(distances, path, verticies);
        path[cur] = true;

        // Update distances
        for (int i = csr.beg_pos[cur]; i < csr.beg_pos[cur+1]; i++)
        {
            int neighbor = csr.word_to_idx(csr.adj_list[i]);
            if (!path[neighbor] && 
                    distances[cur] != DOUBLE_MAX &&
                     distances[cur] + csr.weight_list[i] < distances[neighbor])
            {
                distances[neighbor] = distances[cur] + csr.weight_list[i];
            }
        }
    }
    return distances[target];
}

// Single Shortest Path from Source... Source is given source word
void SSSP(CSR &csr, const char *source_word, double *dist, int row) {

    int cols = csr.vertex_count;
    for (int i = 0; i < cols; i++) {
        double tmp = shortest_path_weights(csr, source_word, i);
        dist[row * cols + i]  = tmp;
    }
}

std::vector<WordDist*> collective_closest(const char **source_words, int n, CSR &csr) {
    // Row for each source word, col for each vtx
    double *dist = (double *)malloc(sizeof(double) * n * csr.vertex_count);

    // All vtxs, sorted in terms of closest
    std::vector<WordDist*> word_dist;

    // Fill out dists to all vtxs (dist col) from word (dist row)
    for (int i = 0; i < n; i++)
        SSSP(csr, source_words[i], dist, i);

    // Get collective dist of vtx (col) to all source words (row)
    for (int i = 0; i < csr.vertex_count; i++) {
        word_dist.push_back(new WordDist(get_collective_dist(dist, n, csr.vertex_count, i), i));
    }

    // Word has no relation to given set
    double no_relation = (1 / DOUBLE_MAX) * n;

    std::vector<WordDist*> related_words;

    // Filter out all dists that are 0 or not related
    std::copy_if(word_dist.begin(), word_dist.end(), 
            std::back_inserter(related_words), [DOUBLE_INF, no_relation](WordDist *a) -> bool {
                    return a->dist != DOUBLE_INF && a->dist != no_relation;
    });
    
    // Sort in terms of collect closest
    sort(related_words.begin(), related_words.end(), [](WordDist *a, WordDist *b) -> bool
    {
        return a->dist < b->dist;
    });
    return related_words;
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
        source_words[i] = argv[i + 2];
    }


    CSR csr(edgefilename);
    std::cout << "Num verticies " << csr.vertex_count << endl;
    std::cout << "Num edges " << csr.edge_count << endl;

    for (int i = 0; i < num_source_words; i++) {
        if (!csr.word_in_graph(source_words[i])) {
            cout << "Not found in graph: " << source_words[i] << endl;
            return 1;
        }
    }
    cout << endl;

    //csr.show_verticies();
    std::vector<WordDist*> closest_words = collective_closest(source_words, num_source_words, csr);
    
    cout << "Closest words:" << endl;
    for (int i = 0; i < 10; i++) {
        if (i >= closest_words.size()) {
            cout << "End" << endl;
            break;
        }
        cout << csr.idx_to_word(closest_words[i]->word_id) << " (Dist: "
            << closest_words[i]->dist << ")" << endl;
    }

} 
