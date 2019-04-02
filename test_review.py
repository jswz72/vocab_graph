from vocab_graph import graph_ops

# Location of test files made in this project
edge_list_fname = 'vocab_graph/src/get_edges/edge-list.txt'
word_map_fname = 'vocab_graph/src/get_edges/word-order.txt'

graph = graph_ops.VocabGraph(edge_list_fname, word_map_fname)

learned = ['flame', 'shark', 'the', 'of']
reviewed = ['flame']
res = graph.review(learned, 3, reviewed)
for word in res:
    print(word)
