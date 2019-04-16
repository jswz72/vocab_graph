from ctypes import cdll, POINTER, c_char_p, c_int, c_double, c_uint, byref
import os

_DIR = os.path.dirname(os.path.abspath(__file__))
_EDGE_LIB = 'get_edges.so'
_TTTB_LIB = 'tuple_text_to_bin.so'
_EDGE_FP = os.path.join(_DIR, _EDGE_LIB)
_TTTB_FP = os.path.join(_DIR, _TTTB_LIB)

_to_bytes = lambda word: bytes(word, 'utf-8')


def get_edge_list(vecfilename, edge_file_out, threshold=0, limit=0, to_nums=False,
        word_file_out=None):
    """ Write an edge list file given the filename of a vectorfile, and output edgefile.

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


def tuple_text_to_bin(tuple_file, reverse_edges=False, lines_to_skip=0, weight=True):
    """ Write binary CSR graph file from an edge list.
    To be used in later operations
    """
    c_tttb = cdll.LoadLibrary(_TTTB_FP).tuple_text_to_binary_csr

    tuple_file_t = c_char_p
    reverse_edges_t = c_uint
    lines_to_skip_t = c_uint
    thread_count_t = c_uint
    weight_t = c_uint

    c_tttb.argtypes = [tuple_file_t, reverse_edges_t, lines_to_skip_t,
            thread_count_t, weight_t]

    tuple_file_b = _to_bytes(tuple_file)

    c_tttb(tuple_file_b, reverse_edges, lines_to_skip, 1, weight)


def make_graph(vecfilename, edge_file_out, threshold=0, limit=0, to_nums=False,
        word_file_out=None, reverse_edges=False, lines_to_skip=0, weight=True):
    """Runs get_edge_list and tuple_text_to_bin in order, with same parameters
    as noted in respective functions."""
    get_edge_list(vecfilename, edge_file_out, threshold, limit, to_nums,
            word_file_out)
    tuple_text_to_bin(edge_file_out, reverse_edges, lines_to_skip, weight)



