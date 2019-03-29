from ctypes import *

recommend = cdll.LoadLibrary('./recommend.dll').recommend

csr_filename = '../cpp/get_edges/edge-list.txt'
with open('../cpp/get_edges/word-order.txt') as f:
    content = f.readlines()
words = [line.strip() for line in content]
num_words = len(words)
num_recs = 3
source_words = ['flame', 'shark']
num_source_words = len(source_words)
use_rec_pool = 0
rec_pool = ['']
num_rec_pool = 0

recommend.argtypes = [c_char_p, c_char_p * num_words, c_int,
        POINTER(c_int), c_char_p * num_source_words, c_int, c_int, c_char_p * 1, c_int]
recommend.restype = POINTER(c_int)


num_recs_ptr = c_int(num_recs)
csr_filename_bytes = bytes(csr_filename, 'utf-8')
words_arr = (c_char_p * num_words)()
words_arr[:] = [bytes(word, 'utf-8') for word in words]
source_words_arr = (c_char_p * num_source_words)()
source_words_arr[:] = [bytes(word, 'utf-8') for word in source_words]
rec_pool_arr = (c_char_p * 1)()
rec_pool_arr[:] = [bytes('', 'utf-8')]

ret_ptr = recommend(csr_filename_bytes, words_arr, num_words, byref(num_recs_ptr),
        source_words_arr, num_source_words, use_rec_pool, rec_pool_arr,
        num_rec_pool)

num_recs_made = num_recs_ptr.value
for i in range(num_recs_made):
    print(words[ret_ptr[i]])

