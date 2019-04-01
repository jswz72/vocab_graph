import knowledge_graph

edge_list_fname = '../cpp/get_edges/edge-list.txt'
word_map_fname = '../cpp/get_edges/word-order.txt'

graph = knowledge_graph.GraphOps(edge_list_fname, word_map_fname)

learned = ['flame', 'shark', 'the', 'of']
reviewed = ['flame']
res = graph.review(learned, 2, reviewed, 2)

print("PYTHON: ")
print(res)
