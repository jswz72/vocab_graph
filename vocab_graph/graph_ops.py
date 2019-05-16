from ctypes import cdll, POINTER, c_char_p, c_int, c_double, c_uint, byref, Structure
import fuzzy
import os

_REC_LIB = 'recommend.so'
_REVIEW_LIB = 'review.so'
_DIR = os.path.dirname(os.path.abspath(__file__))
_REC_FP = os.path.join(_DIR, _REC_LIB)
_REV_FP = os.path.join(_DIR, _REVIEW_LIB)

_to_bytes = lambda word: bytes(word, 'utf-8')

class VocabGraph:
    """ Operations to be done on the knowledge graph defined by the given filenames
    csr_base_filename: Base filename of the binary beg_pos, csr, and weight
    graph files

    word_map_filename: Filename of the text file of words in the graph,
    ordered by their index
    """
    def __init__(self, csr_base_filename, word_map_filename):
        self.csr_fname_b = _to_bytes(csr_base_filename)
        self.c_recommend = cdll.LoadLibrary(_REC_FP).recommend
        self.c_recommend_group = cdll.LoadLibrary(_REC_FP).recommend_group
        self.c_recommend_spelling = cdll.LoadLibrary(_REC_FP).recommend_spelling
        self.c_recommend_phonetic = cdll.LoadLibrary(_REC_FP).recommend_phonetic
        self.c_review = cdll.LoadLibrary(_REV_FP).review
        self._get_word_mapping(word_map_filename)


    def _get_word_mapping(self, fname):
        # Load word mapping file into indexable list
        with open(fname) as f:
            lines = f.readlines()
        self.word_map = [line.strip() for line in lines]
        self.word_map_b = [_to_bytes(word) for word in self.word_map]


    def _get_idx(self, word):
        idx = None
        for i, wmword in enumerate(self.word_map):
            if word == wmword:
                idx = i
        if idx is None:
            raise Exception('Given word not found in graph: {}'.format(word))
        return idx



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
        num_recs_t = POINTER(c_uint)
        source_words_t = POINTER(c_int)
        num_source_words_t = c_uint
        use_rec_pool_t = c_uint
        rec_pool_t = POINTER(c_int)
        num_rec_pool_t = c_uint

        self.c_recommend.argtypes = [csr_fname_t, num_recs_t, source_words_t,
                num_source_words_t, use_rec_pool_t, rec_pool_t, num_rec_pool_t]

        self.c_recommend.restype = POINTER(c_int)

        num_recs_ptr = c_uint(num_recs)
        source_words_arr = (c_int * len(source_words))()
        source_words_arr[:] = [self._get_idx(word) for word in source_words]
        rec_pool_arr = (c_int * len(rec_pool))()
        rec_pool_arr[:] = [self._get_idx(word) for word in rec_pool]

        ret_ptr = self.c_recommend(self.csr_fname_b, byref(num_recs_ptr),
                source_words_arr, len(source_words), bool(rec_pool),
                rec_pool_arr, len(rec_pool))

        return [self.word_map[idx] for idx in ret_ptr[:num_recs_ptr.value]]


    def recommend_group(self, source_words, rec_groups):
        """
        Recommend group of words, based on their collective closensess to source words.

        rec_groups: List of lists of words of arbitrary lengths
        """
        total_group_len = sum([len(g) for g in rec_groups])
        num_groups = len(rec_groups)

        csr_fname_t = c_char_p
        source_words_t = POINTER(c_int)
        num_source_words_t = c_uint
        groups_t = c_int * total_group_len
        num_groups_t = c_uint
        group_sizes_t = c_uint * (num_groups + 1)


        self.c_recommend_group.argtypes = [csr_fname_t, source_words_t, num_source_words_t,
                groups_t, num_groups_t, group_sizes_t]

        self.c_recommend.restype = c_int

        source_words_arr = (c_int * len(source_words))()
        source_words_arr[:] = [self._get_idx(word) for word in source_words]
        groups = (c_int * total_group_len)()
        groups[:] = [self._get_idx(word) for group in rec_groups for word in group]
        group_sizes = (c_uint * (num_groups + 1))()
        group_sizes[0] = 0
        for i in range(1, num_groups + 1):
            group_sizes[i] = group_sizes[i - 1] + len(rec_groups[i - 1])

        idx = self.c_recommend_group(self.csr_fname_b, source_words_arr, len(source_words),
                groups, len(rec_groups), group_sizes)
        return rec_groups[idx]


    def review(self, learned_words, reviewed_words=[]):
        """
        Return list of words to review (in order) realtive to the given
        learned words and already reviewed words.

        learned_words: List of WordMems representing all learned words

        reviewed_words: Optional list of word strings, representing which words have been reviewed
        Returned words will be ordered in terms of collective closest to these words

        Will return list of WordMems, includes given parameters and a calculated memory for each word
        """
        #self._check_words([wm.word for wm in learned_words])
        #self._check_words(reviewed_words)
        csr_fname_t = c_char_p
        learned_words_t = POINTER(c_int)
        num_learned_words_t = c_uint
        t_params_t = POINTER(c_int)
        s_params_t = POINTER(c_double)
        reviewed_words_t = POINTER(c_int)
        num_reviewed_words_t = c_uint

        self.c_review.argtypes = [csr_fname_t, learned_words_t,
                num_learned_words_t, t_params_t, s_params_t,
                reviewed_words_t, num_reviewed_words_t]

        self.c_review.restype = POINTER(CWordMem)

        learned_words_arr = (c_int * len(learned_words))()
        learned_words_arr[:] = [self._get_idx(wm.word) for wm in learned_words]
        t_params_arr = (c_int * len(learned_words))()
        t_params_arr[:] = [wm.last_learned for wm in learned_words]
        s_params_arr = (c_double * len(learned_words))()
        s_params_arr[:] = [wm.strength for wm in learned_words]

        reviewed_words_arr = (c_int * len(reviewed_words))()
        reviewed_words_arr[:] = [self._get_idx(word) for word in reviewed_words]

        ret_ptr = self.c_review(self.csr_fname_b, learned_words_arr, len(learned_words),
                t_params_arr, s_params_arr, reviewed_words_arr, len(reviewed_words))

        return [WordMem(self.word_map[wm.word_id], wm.last_learned, wm.strength, wm.memory)
            for wm in ret_ptr[:len(learned_words)]]


    def recommend_spelling(self, source_words, num_recs):
        '''
        Recommend words based on spellling similarity, 
        not semantic meaning

        num_recs recommendations are returned based on their collective 
        similarity to the spelling of source_words, 
        using Levenshtein distance
        '''
        source_words_t = c_char_p * len(source_words)
        num_source_words_t = c_uint
        word_map_t = c_char_p * len(self.word_map)
        num_word_map_t = c_uint
        num_recs_t = c_uint

        self.c_recommend_spelling.argtypes = [source_words_t, 
                num_source_words_t, word_map_t, num_word_map_t, 
                num_recs_t]

        self.c_recommend_spelling.restype = POINTER(c_char_p)

        source_words_arr = (c_char_p * len(source_words))()
        source_words_arr[:] = [_to_bytes(word) for word in source_words]
        word_map_arr = (c_char_p * len(self.word_map))()
        word_map_arr[:] = self.word_map_b

        ret_ptr = self.c_recommend_spelling(source_words_arr, len(source_words),
                word_map_arr, len(self.word_map), num_recs)

        return [word.decode('utf-8') for word in ret_ptr[:num_recs]]


    def recommend_phonetic(self, source_words, num_recs):
        dmetaph = fuzzy.DMetaphone()
        word_phonetics = [dmetaph(word) for word in self.word_map]
        # Account for 1-2 phonetic encodings for each word
        flat_word_phonetics = [ph for phonetics in 
                word_phonetics for ph in phonetics if ph]
        source_phonetics = [dmetaph(word) for word in source_words]
        flat_source_phonetics = [ph for phonetics in 
                source_phonetics for ph in phonetics if ph]

        source_phonetics_t = c_char_p * len(flat_source_phonetics)
        source_sums_t = c_uint * (len(source_words) + 1)
        source_sums_size_t = c_uint
        word_sums_t = c_uint * (len(word_phonetics) + 1)
        word_sums_size_t = c_uint
        word_phonetics_t = c_char_p * len(flat_word_phonetics)

        self.c_recommend_phonetic.argtypes = [source_phonetics_t,
                source_sums_t, source_sums_size_t, word_phonetics_t,
                word_sums_t, word_sums_size_t]

        self.c_recommend_phonetic.restype = POINTER(c_int)

        source_phonetics_arr = (c_char_p * len(flat_source_phonetics))()
        source_phonetics_arr[:] = [ph for ph in 
                flat_source_phonetics]
        source_sums_arr = (c_uint * (len(source_words) + 1))()
        source_sums_arr[0] = 0
        for i in range(1, len(source_words) + 1):
            source_sums_arr[i] = source_sums_arr[i - 1] + len([ph for ph in source_phonetics[i - 1] if ph])
        word_sums_arr = (c_uint * (len(word_phonetics) + 1))()
        word_sums_arr[0] = 0
        for i in range(1, len(word_phonetics) + 1):
            word_sums_arr[i] = word_sums_arr[i - 1] + len([ph for ph in word_phonetics[i - 1] if ph])

        word_phonetics_arr = (c_char_p * len(flat_word_phonetics))()
        word_phonetics_arr[:] = [ph for ph in flat_word_phonetics]

        ret_ptr = self.c_recommend_phonetic(source_phonetics_arr,
                source_sums_arr, len(source_sums_arr), word_phonetics_arr,
                word_sums_arr, len(word_sums_arr))

        ret_arr = []
        i = 0
        while len(ret_arr) < num_recs and i < len(self.word_map):
            rec_word = self.word_map[ret_ptr[i]]
            if rec_word not in source_words:
                ret_arr.append(rec_word)
            i += 1

        return ret_arr

class CWordMem(Structure):
    """ C Struct of word memories """
    _fields_ = [("word_id", c_int), ("memory", c_double),
            ("last_learned", c_int), ("strength", c_uint)]

class WordMem:
    """ Class holding memory/forgetting information of word
    word: word string

    last_learned: how many time units ago was learned (positive number)

    strength: how quickly memory of this word decays

    memory: resulting memory of the word given the above parametrs (initialized to zero)
    """
    def __init__(self, word, last_learned, strength, memory=0):
        self.word = word
        self.last_learned = last_learned
        self.strength = strength
        self.memory = memory

    def __str__(self):
        return "Word: {}, memory: {}, strength: {}, last learned: {}".format(self.word,
                self.memory, self.strength, self.last_learned)



