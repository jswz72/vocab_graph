import vocab_graph

# Location of test files made in this project
edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'

graph = vocab_graph.VocabGraph(edge_list_fname, word_map_fname)

source_words = ['flame', 'shark']
res = graph.recommend(source_words, 20)
print('Recommendations:')
for word in res:
    print(word)


