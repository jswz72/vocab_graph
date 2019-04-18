from vocab_graph import graph_ops
WordMem = graph_ops.WordMem

# Location of test files made in this project
edge_list_fname = './data/edge-list.txt'
word_map_fname = './data/word-order.txt'
forgotten_threshold = 0.14

graph = graph_ops.VocabGraph(edge_list_fname, word_map_fname)

learned = [WordMem('flame', last_learned=0, strength=1),
        WordMem('shark', last_learned=1, strength=1),
        WordMem('possible', last_learned=1, strength=1),
        WordMem('idea', last_learned=1, strength=1),
        WordMem('the', last_learned=1, strength=3),
        WordMem('time', last_learned=2, strength=2),
        WordMem('of', last_learned=0, strength=2)]

show_results = lambda l: [print(x) for x in l]

res_words = graph.review(learned)
show_results(res_words)
print('\n')

# Choose to review a few words this round
reviewed_this_round = [wm.word for wm in res_words[:2]]
print("Going to review these words:")
print(reviewed_this_round)

for wm in learned:
    if wm.word in reviewed_this_round:
        # Update values for words learned this round
        wm.last_learned = 0
        wm.strength += 1
    else:
        # Increase time for all other words not learned
        wm.last_learned += 1

res_words = graph.review(learned, reviewed_this_round)
show_results(res_words)
print('\n')

# Check memory levels of words, use some threshold to determine if in danger of being forgotten
for wm in res_words:
    if wm.memory < forgotten_threshold:
        print('{} in danger of being forgotten'.format(wm.word))
