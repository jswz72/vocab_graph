import vocab_graph

# Location of test files made in this project
edge_list_fname = 'vocab_graph/src/get_edges/edge-list.txt'
word_map_fname = 'vocab_graph/src/get_edges/word-order.txt'

graph = vocab_graph.VocabGraph(edge_list_fname, word_map_fname)

source_words = ['flame', 'shark']
res = graph.recommend(source_words, 20)
print("PYTHON:")
for word in res:
    print(word)
