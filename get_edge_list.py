import numpy as np
import time
import re
import sys
import csv

vectorfile = threshold = len(sys.argv) > 1 and sys.argv[1] or 'word-vectors.txt'
threshold = len(sys.argv) > 2 and int(sys.argv[2]) or None
outfile = len(sys.argv) > 3 and sys.argv[3] or None
LIMIT = 5000


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
    pairs = set()
    for vtx1, vec1 in words.items():
        for vtx2, vec2 in words.items():
            if vtx1 == vtx2 or (vtx2, vtx1) in pairs:
                continue
            dist = euclidean_dist(vec1, vec2)
            if threshold and dist > threshold:
                continue
            add_func((vtx1, vtx2, dist))
            pairs.add((vtx1, vtx2))


def write_edge_list(words, outfile, threshold=None):
    '''write edgelist to file at given path'''
    of = open(outfile, 'w')
    writer = csv.writer(of)
    def add_func(edge_tuple):
        writer.writerow(list(edge_tuple))
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

    
def print_edges(edges, all=False):
    if all:
        for els in edges:
            print(els)
        return

    print('Most related:')
    for els in edges[:10]:
        print(els)

    print('\nLeast related')
    for els in edges[-10:]:
        print(els)

start = time.time()
words = parse_vector_file(vectorfile)
print(f'Word vec size: {len(words)}')
parse_time = time.time()
print(f'Time to parse vectorfile: {parse_time - start}')

if outfile:
    edges = write_edge_list(words, outfile, threshold)
    edge_time = time.time()
    print(f'Time to create/write edge list: {edge_time - parse_time}')

edges = create_edge_list(words, threshold)
edge_time = time.time()
print(f'Time to create edges: {edge_time - parse_time}')
edges.sort(key=lambda x: x[2])
print_edges(edges, all=True)
print(f'Num edges: {len(edges)}')
filename, ext = outfile.split('.')
with open(f'{filename}_sorted.{ext}', 'w') as of:
    writer = csv.writer(of)
    for edge in edges:
        writer.writerow(list(edge))
    of.close()

