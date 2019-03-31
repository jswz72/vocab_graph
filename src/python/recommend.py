import knowledge_graph

edge_list_fname = '../cpp/get_edges/edge-list.txt'
word_map_fname = '../cpp/get_edges/word-order.txt'

graph = knowledge_graph.GraphOps(edge_list_fname, word_map_fname)

source_words = ['flame', 'shark']
res = graph.recommend(source_words, 3)
for word in res:
    print(word)
