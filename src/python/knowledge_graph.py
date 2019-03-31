from ctypes import cdll, POINTER, c_char_p, c_int, c_uint, byref

class GraphOps:
    """
    Operations to be done on the knowledge graph defined by the given filenames
    Init Args:
        csr_base_filename: Base filename of the binary beg_pos, csr, and weight
        graph files

        word_map_filename: Filename of the text file of words in the graph,
        ordered by their index
    """
    def __init__(self, csr_base_filename, word_map_filename):
        self.csr_fname_b = bytes(csr_base_filename, 'utf-8')
        self.c_recommend = cdll.LoadLibrary('./recommend.dll').recommend
        self.c_review = cdll.LoadLibrary('./review.dll').review
        self._get_word_mapping(word_map_filename)


    def _get_word_mapping(self, fname):
        # Load word mapping file into indexable list
        with open(fname) as f:
            lines = f.readlines()
        self.word_map = [line.strip() for line in lines]
        self.word_map_b = [bytes(word, 'utf-8') for word in self.word_map]


    def recommend(self, source_words, num_recs, rec_pool=[]):
        """
        Return list of recommended words to learn, relative to given
        source (learned) words.

        num_recs: the number of recommendations to be made, output list may
        be shorter depending on how many recomendations are found in graph

        Optional rec_pool: if given, recommendtions will only be made from
        words present in this recommendation pool list
        """
        csr_fname_t = c_char_p
        words_t = c_char_p * len(self.word_map)
        num_words_t = c_uint
        num_recs_t = POINTER(c_uint)
        source_words_t = c_char_p * len(source_words)
        num_source_words_t = c_uint
        use_rec_pool_t = c_uint
        rec_pool_t = c_char_p * len(rec_pool)
        num_rec_pool_t = c_uint

        self.c_recommend.argtypes = [csr_fname_t, words_t, num_words_t, num_recs_t,
                source_words_t, num_source_words_t, use_rec_pool_t,
                rec_pool_t, num_rec_pool_t]

        self.c_recommend.restype = POINTER(c_int)

        num_recs_ptr = c_uint(num_recs)
        words_arr = (c_char_p * len(self.word_map))()
        words_arr[:] = self.word_map_b
        source_words_arr = (c_char_p * len(source_words))()
        source_words_arr[:] = [bytes(word, 'utf-8') for word in source_words]
        rec_pool_arr = (c_char_p * len(rec_pool))()
        rec_pool_arr[:] = [bytes(word, 'utf-8') for word in rec_pool]

        ret_ptr = self.c_recommend(self.csr_fname_b, words_arr, len(self.word_map),
                byref(num_recs_ptr), source_words_arr, len(source_words),
                bool(rec_pool), rec_pool_arr, len(rec_pool))

        return [self.word_map[idx] for idx in ret_ptr[:num_recs_ptr.value]]


    def review(self, learned_words, num_to_review, reviewed_words=[]):
        csr_fname_t = c_char_p
        words_t = c_char_p * len(self.word_map)
        num_words_t = c_uint
        learned_words_t = c_char_p * len(learned_words)
        num_learned_words_t = c_uint
        reviewed_words_t = c_char_p * len(reviewed_words)
        num_reviewed_words_t = c_uint
        num_to_review_t = c_uint

        self.c_review.argtypes = [csr_fname_t, words_t, num_words_t,
                learned_words_t, num_learned_words_t, reviewed_words_t,
                num_reviewed_words_t, num_to_review_t]

        self.c_review.restype = POINTER(c_int)

        words_arr = (c_char_p * len(self.word_map))()
        words_arr[:] = self.word_map_b
        learned_words_arr = (c_char_p * len(learned_words))()
        learned_words_arr[:] = [bytes(word, 'utf-8') for word in learned_words]
        reviewed_words_arr = (c_char_p * len(reviewed_words))()
        reviewed_words_arr[:] = [bytes(word, 'utf-8') for word in reviewed_words]

        ret_ptr = self.c_review(self.csr_fname_b, words_arr, len(self.word_map),
                learned_words_arr, len(learned_words), reviewed_words_arr,
                len(reviewed_words), num_to_review)

        return [self.word_map[idx] for idx in ret_ptr[:num_to_review]]
