#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <limits.h>
#include "GraphOps.hpp"

using std::cout;
using std::endl;

typedef graph<long, long, long, long, long, long> long_graph;

// Breadth first search for target in graph
long BFS(long_graph *g, long target)
{
    std::queue<long> q;
    long start = g->beg_pos[0];
    q.push(start);
    std::unordered_set<long> visited;
    visited.insert(start);
    long depth = 0;
    long nodes_in_level = 1;
    long nodes_in_next_level = 0;
    while (!q.empty())
    {
        long cur = q.front();
        q.pop();
        if (cur == target) 
            return depth;
        long beg = g->beg_pos[cur];
        long end = g->beg_pos[cur+1];
        long initQLen = q.size();
        for(long i = beg; i < end; i++)
        {
            long next = g->csr[i];
            if (visited.find(next) == visited.end())
            {
                q.push(next);
                visited.insert(next);
            }
        }
        nodes_in_next_level += q.size() - initQLen;
        nodes_in_level--;
        if (nodes_in_level == 0) {
            depth++;
            nodes_in_level = nodes_in_next_level;
            nodes_in_next_level = 0;
        }
    }
    return -1;
}

long min_dist(long distances[], unsigned int path[], long verticies)
{
    long min = LONG_MAX;
    long min_idx;
    for (long i = 0; i < verticies; i++)
    {
        if (!path[i] && distances[i] <= min)
        {
            min = distances[i];
            min_idx = i;
        }
    }
    return min_idx;
}

// Use Dijsktra's algorithm to find the shortest path to the given target
long shortest_path_weights(long_graph *g, long target)
{
    long verticies = g->vert_count;
    // distance from start to vertix (index of arr)
    long distances[verticies];
    // bitset: 1 if vertex included in path
    unsigned int path[verticies];
    for (long i = 0; i < verticies; i++)
    {
        distances[i] = LONG_MAX;
        path[i] = 0;
    }
    distances[g->beg_pos[0]] = 0;
    for (long count = 0; count < verticies - 1; count++)
    {
        long cur = min_dist(distances, path, verticies);
        cout << " cur: " << cur << endl;
        path[cur] = true;

        // Update distances of remaining verticies
        for (long i = g->beg_pos[cur]; i < g->beg_pos[cur+1]; i++)
        {
            long neighbor = g->csr[i];
            cout << "neighbor: " << neighbor << endl;
            if (!path[neighbor] && distances[cur] != LONG_MAX
                    && distances[cur] + g->weight[i] < distances[neighbor])
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
