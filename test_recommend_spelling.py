import vocab_graph
import sys

def leven_dist(a, b):
    dists = [[0 for _ in range(len(a) + 1)] for _ in range(len(b) + 1)]

    for i in range(len(a) + 1):
        dists[0][i] = i

    for i in range(len(b) + 1):
        dists[i][0] = i

    for j in range(1, len(b) + 1):
        for i in range(1, len(a) + 1):
            indicator = a[i - 1] != b[j - 1]
            dists[j][i] = min(dists[j][i - 1] + 1,
                    dists[j - 1][i] + 1,
                    dists[j - 1][i - 1] + indicator)

    return dists[len(b)][len(a)]


word1 = sys.argv[1]
word2 = sys.argv[2]

edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'

graph = vocab_graph.VocabGraph(edge_list_fname, word_map_fname)

#print('Dist is {}'.format(leven_dist(word1, word2)))
graph.recommend_spelling([word1, word2])



