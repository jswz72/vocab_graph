import re

only_alpha = re.compile('[^a-zA-Z]')

f = open('all-words.txt')

trimmed = [line for line in f.readlines() if only_alpha.sub('', line)]

f2 = open('all-words-alpha-num.txt', 'w')

for word in trimmed:
    f2.write(word)
