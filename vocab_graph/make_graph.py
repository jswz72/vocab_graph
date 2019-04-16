import os

from ctypes import cdll, POINTER, c_char_p, c_int, c_double, c_uint, byref

_DIR = os.path.dirname(os.path.abspath(__file__))
_EDGE_LIB = 'get_edges.so'
_EDGE_FP = os.path.join(_DIR, _EDGE_LIB)

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



