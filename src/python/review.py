import knowledge_graph

edge_list_fname = '../cpp/get_edges/edge-list.txt'
word_map_fname = '../cpp/get_edges/word-order.txt'

graph = knowledge_graph.GraphOps(edge_list_fname, word_map_fname)

learned = ['flame', 'shark', 'the', 'of', 'following', 'put', 'although', 'making', 'movie', 'order', 'prove']
reviewed = ['flame']
res = graph.review(learned, 3, reviewed, 10)

print("PYTHON: ")
for rev in res:
    print(rev)
