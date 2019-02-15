import numpy as np

vectorfile = 'glove.6B.50d.txt'
graphfile = 'list.txt'
LIMIT = 1000
THRESHOLD = 3

def euclidean_dist(x, y):
    return np.sqrt(np.sum((np.subtract(x, y)) ** 2))

with open (vectorfile) as vf:
    mat = [line.split(' ') for line in vf.readlines()[:LIMIT]]
    words = { row[0]: tuple([float(num) for num in row[1:]]) for row in mat }

gf = open(graphfile, 'w')
#retarr = []
matches = set()
for vtx1, val in words.items():
    for vtx2, val2 in words.items():
        if vtx1 == vtx2 or (vtx2, vtx1) in matches:
            continue
        matches.add((vtx1, vtx2))
        dist = euclidean_dist(val, val2)
        if dist > THRESHOLD:
            continue
        gf.write(f'{vtx1} {vtx2} {dist}\n')
        #retarr.append((key, key2, dist))

# retarr.sort(key=lambda x: x[2])
# print('Most related:')
# for els in retarr[:10]:
#     print(els)

# print('\nLeast related')
# for els in retarr[-10:]:
#     print(els)