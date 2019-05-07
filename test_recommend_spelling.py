import vocab_graph
import sys

word1 = sys.argv[1]
word2 = sys.argv[2]

edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'

graph = vocab_graph.VocabGraph(edge_list_fname, word_map_fname)

recommendations = graph.recommend_spelling([word1, word2], 3)
print('Recommended words based on spelling similarity:')
for word in recommendations:
    print(word)



