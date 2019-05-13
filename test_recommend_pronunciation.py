import vocab_graph
import fuzzy
import sys

word1 = sys.argv[1]
word2 = sys.argv[2]

edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'

dmetaphone = fuzzy.DMetaphone()

for word in [word1, word2]:
    print(dmetaphone(word))

