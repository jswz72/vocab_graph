from vocab_graph import graph_ops
WordMem = graph_ops.WordMem

# Location of test files made in this project
edge_list_fname = 'vocab_graph/src/get_edges/edge-list.txt'
word_map_fname = 'vocab_graph/src/get_edges/word-order.txt'

graph = graph_ops.VocabGraph(edge_list_fname, word_map_fname)

learned = [WordMem('flame', 0, 1),
        WordMem('shark', 1, 1),
        WordMem('the', 3, 2),
        WordMem('of', 0, 2)]

#reviewed = ['flame']

res = graph.review(learned)
for word in res:
    print(word)
