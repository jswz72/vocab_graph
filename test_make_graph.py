import vocab_graph

vocab_graph.make_graph(vecfilename='./data/tv.txt',
        edge_file_out='./data/edge-list.txt',
        threshold=3.0,
        limit=10000,
        to_nums=True,
        word_file_out='./data/word-order.txt')

