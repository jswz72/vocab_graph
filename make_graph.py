import numpy as np
import time
import re

vectorfile = 'glove.6B.50d.txt'
graphfile = 'list.txt'
LIMIT = 5000
THRESHOLD = 0.9


def euclidean_dist(x, y):
    return np.sqrt(np.sum((np.subtract(x, y)) ** 2))

def collect_vectors(f):
    mat = [line.split(' ') for line in f.readlines()[:LIMIT]]
    print(f'Original num words: {len(mat)}')
    only_alpha = re.compile('[^a-zA-Z]')
    mat = [line for line in mat if only_alpha.sub('', line[0])]
    print(f'After filtering: {len(mat)}')
    return { row[0]: tuple([float(num) for num in row[1:]]) for row in mat }

def parse_vector_file(filename):
    words = None
    with open (filename, encoding="utf8") as vector_file:
        words = collect_vectors(vector_file)
    return words

def find_edges(words, add_func, threshold=None):
    for vtx1, vec1 in words.items():
        for vtx2, vec2 in words.items():
            if vtx1 == vtx2:
                continue
            dist = euclidean_dist(vec1, vec2)
            if threshold and dist > threshold:
                continue
            add_func((vtx1, vtx2, dist))


def write_edge_list(words, outfile, threshold=None):
    '''write edgelist to file at given path'''
    of = open(outfile, 'w')
    def add_func(edge_tuple):
        vtx1, vtx2, dist = edge_tuple
        of.write(f'{vtx1} {vtx2} {dist}')
    find_edges(words, add_func, threshold)
    of.close()

def create_edge_list(words, threshold=None):
    '''create and return edgelist in memory'''
    returnarr = []
    def add_func(edge_tuple):
        returnarr.append(edge_tuple)
    find_edges(words, add_func, threshold)
    print(len(returnarr))
    return returnarr

    
def sort_and_print_edges(edges):
    edges.sort(key=lambda x: x[2])
    print('Most related:')
    for els in edges[:10]:
        print(els)

    print('\nLeast related')
    for els in edges[-10:]:
        print(els)

start = time.time()
words = parse_vector_file(vectorfile)
parse_time = time.time()
print(f'Time to parse vectorfile: {parse_time - start}')
edges = create_edge_list(words, THRESHOLD)
edge_time = time.time()
print(f'Time to create edges: {edge_time - parse_time}')
sort_and_print_edges(edges)