from ctypes import cdll, POINTER, c_char_p, c_int, c_double, c_uint, byref, Structure
import os

_EDGE_LIB = 'get_edges.so'
_REC_LIB = 'recommend.so'
_REVIEW_LIB = 'review.so'
_DIR = os.path.dirname(os.path.abspath(__file__))
_REC_FP = os.path.join(_DIR, _REC_LIB)
_REV_FP = os.path.join(_DIR, _REVIEW_LIB)
_EDGE_FP = os.path.join(_DIR, _EDGE_LIB)

class VocabGraph:
    """
    Operations to be done on the knowledge graph defined by the given filenames
    Init Args:
        csr_base_filename: Base filename of the binary beg_pos, csr, and weight
        graph files

        word_map_filename: Filename of the text file of words in the graph,
        ordered by their index
    """
    def __init__(self, csr_base_filename, word_map_filename):
        self.csr_fname_b = _to_bytes(csr_base_filename)
        self.c_recommend = cdll.LoadLibrary(_REC_FP).recommend
        self.c_review = cdll.LoadLibrary(_REV_FP).review
        self._get_word_mapping(word_map_filename)


    def _get_word_mapping(self, fname):
        # Load word mapping file into indexable list
        with open(fname) as f:
            lines = f.readlines()
        self.word_map = [line.strip() for line in lines]
        self.word_map_b = [_to_bytes(word) for word in self.word_map]


    def _check_words(self, words):
        for word in words:
            if word not in self.word_map:
                raise Exception('Given word not found in graph: {}'.format(word))


    def recommend(self, source_words, num_recs, rec_pool=[]):
        """
        Return list of recommended words to learn, relative to given
        source (learned) words.

        num_recs: the number of recommendations to be made, output list may
        be shorter depending on how many recomendations are found in graph

        Optional rec_pool: if given, recommendtions will only be made from
        words present in this recommendation pool list
        """
        self._check_words(source_words)
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
        source_words_arr[:] = [_to_bytes(word) for word in source_words]
        rec_pool_arr = (c_char_p * len(rec_pool))()
        rec_pool_arr[:] = [_to_bytes(word) for word in rec_pool]

        ret_ptr = self.c_recommend(self.csr_fname_b, words_arr, len(self.word_map),
                byref(num_recs_ptr), source_words_arr, len(source_words),
                bool(rec_pool), rec_pool_arr, len(rec_pool))

        return [self.word_map[idx] for idx in ret_ptr[:num_recs_ptr.value]]


    def review(self, learned_words, reviewed_words=[]):
        """
        Return list of words to review (in order) realtive to the given
        learned words and already reviewed words.
        """
        self._check_words([wm.word for wm in learned_words])
        self._check_words(reviewed_words)
        csr_fname_t = c_char_p
        words_t = c_char_p * len(self.word_map)
        num_words_t = c_uint
        learned_words_t = c_char_p * len(learned_words)
        num_learned_words_t = c_uint
        t_params_t = POINTER(c_int)
        s_params_t = POINTER(c_double)
        reviewed_words_t = c_char_p * len(reviewed_words)
        num_reviewed_words_t = c_uint

        self.c_review.argtypes = [csr_fname_t, words_t, num_words_t,
                learned_words_t, num_learned_words_t, t_params_t, s_params_t,
                reviewed_words_t, num_reviewed_words_t]

        self.c_review.restype = POINTER(CWordMem)

        words_arr = (c_char_p * len(self.word_map))()
        words_arr[:] = self.word_map_b
        learned_words_arr = (c_char_p * len(learned_words))()
        learned_words_arr[:] = [_to_bytes(wm.word) for wm in learned_words]
        t_params_arr = (c_int * len(learned_words))()
        t_params_arr[:] = [wm.last_learned for wm in learned_words]
        s_params_arr = (c_double * len(learned_words))()
        s_params_arr[:] = [wm.strength for wm in learned_words]

        reviewed_words_arr = (c_char_p * len(reviewed_words))()
        reviewed_words_arr[:] = [_to_bytes(word) for word in reviewed_words]

        ret_ptr = self.c_review(self.csr_fname_b, words_arr, len(self.word_map),
                learned_words_arr, len(learned_words), t_params_arr, s_params_arr,
                reviewed_words_arr, len(reviewed_words))

        #to_review = []
        return [WordMem(self.word_map[wm.word_id], wm.last_learned, wm.strength, wm.memory)
            for wm in ret_ptr[:len(learned_words)]]


_to_bytes = lambda word: bytes(word, 'utf-8')


class CWordMem(Structure):
    _fields_ = [("word_id", c_int), ("memory", c_double),
            ("last_learned", c_int), ("strength", c_uint)]

class WordMem:
    def __init__(self, word, last_learned, strength, memory=0):
        self.word = word
        self.last_learned = last_learned
        self.strength = strength
        self.memory = memory

    def __str__(self):
        return "Word: {}, memory: {}, strength: {}, last learned: {}".format(self.word,
                self.memory, self.strength, self.last_learned)


def get_edge_list(vecfilename, edge_file_out, threshold=0, limit=0, to_nums=False,
        word_file_out=None):
    """
    Write an edge list file given the filename of a vectorfile, and output edgefile.

        Optionally provide unsigned values threshold and limit.

        Optionally provide to_nums to translate produced edge-list into numerical/index form.
        If doing so, must provide word_file_out to write word indices to

        All filenames must be absolute paths
    """
    if to_nums and not word_file_out:
        raise ValueError("Must give word_file_out if to_nums is True")

    c_get_edge_list = cdll.LoadLibrary(_EDGE_FP).get_edge_list

    vecfilename_t = c_char_p
    edge_file_out_t = c_char_p
    threshold_t = c_double
    limit_t = c_uint
    to_nums_t = c_uint
    word_file_out_t = c_char_p

    c_get_edge_list.argtypes = [vecfilename_t, edge_file_out_t, threshold_t, limit_t, to_nums_t,
            word_file_out_t]

    vecfilename_b = _to_bytes(vecfilename)
    edge_file_out_b= _to_bytes(edge_file_out)
    word_file_out_b= _to_bytes(word_file_out)

    c_get_edge_list(vecfilename_b, edge_file_out_b, threshold, limit, to_nums, word_file_out_b)



