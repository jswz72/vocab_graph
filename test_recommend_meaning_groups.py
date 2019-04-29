import random
import vocab_graph

# Location of test files made in this project
edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'

graph = vocab_graph.VocabGraph(edge_list_fname, word_map_fname)

# Simple recommendations
print('Simple rec:')
source_words = ['flame', 'shark']
groups = [['the', 'of', 'other', 'learn'], ['whale', 'torch', 'as'], ['flame', 'shark']]

rec_group = graph.recommend_group(source_words, groups)
print('Recommended group:')
print(rec_group)

# More groups (250)
print('\nLarger groups:');
source_words = ['flame', 'shark', 'idea', 'explain', 'week', 'lines', 'company']
# Make 250 random groups of random length
with open(word_map_fname) as f:
    words = [l.strip() for l in f]
groups = []
for i in range(250):
    group = []
    for i in range(random.randint(5, 50)):
        group.append(words[random.randint(0, len(words) - 1)])
    groups.append(group)

rec_group = graph.recommend_group(source_words, groups)
print(rec_group)

