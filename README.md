### To create edgelist from vector file:  
Use `get_edge_list{{.cpp | .py}}`  
Takes input vector file in form `word(string) dist(num) dist(num)`...  
Creates output edgelist of form `vtx(string) vtx(string) dist(num)`  
Tested with [Stanford Glove's](https://nlp.stanford.edu/projects/glove/) 50-dimensional wikipedia word vector of 400,000 words: `glove.6b.50d`

----
Python version:
`python3 vectorfile euclidean_distance_threshold outfile limit`
- vectorfile: input file of word vectors
- `euclidean_distance_threshold`: all edges added to the list must have weights less than this value (optional)
- outfile: where to write edgelist (optional)
- limit: how many lines to read from vectorfile (optional)

Single threaded.  
Can output to console or file.  

----
Cpp version:
`./executable vectorfile outfile euclidean_distance_threshold limit to_nums`  
- vectorfile: input file of word vectors
- outfile: 1 to write `edge-list.txt` file, 0 or ommitted to print to stdout
- `euclidean_distance_threshold`: all edges added to the list must have weights less than this value (optional)
- limit: how many lines to read from vectorfile (omit or enter 0 to read all)  
- `to_nums`: 1 to translate output vector to `int int double` form (also creates `word-order.txt` to transfer back to string), 0 to leave output as `string string double`  
 Uses openmp multithreading, use env params to specify num threads 

### To process edgelist:
If in `string string double` representation, need to convert to `int int double` for easier processing:  
Use `edge_list_to_nums.cpp`  
`./executable vector_file_input vector_file_output`  
Will also create `word-order.txt` file in output directory, for use in use in mapping back to string words  
If already in `int int double`, carry on

----
Use `tuple_text_to_binary_csr_mem_weight` directory to write a binary CSR graph representation of the previous numeric edgelist  
`./executable edge_list_file reverse_edges(1/0) lines_to_skip(0+) thread_count(1+) write_weights(1/0)`  
Used and modified from, and explained at [graph_project_start](https://github.com/asherliu/graph_project_start)

----
`graph_reader_new` directory is used to read previously created binary CSR and perform any operations defined (`review_and_recommend`, etc.)    
Used from (with modifications), and explained at [graph_project_start](https://github.com/asherliu/graph_project_start)  
Used for the below algorithms  

### New Word Recommendations and Recommendation Reviewing Order
#### New Word Recommendations
Given a list of known words, new words in the graph will be recommended on a basis of collective closeness to given words.  These recommendations are sorted in order of most alike (or least collective distance).
`./recommend.bin base_file mapping_file num_rec source_words...`  
- `base_file`: base filename of `*_beg_pos`, `*_csr*`, and `*_weight` files that were created by previous steps  
- `mapping_file`: file of word strings that correspond to indicies of the graph (likely `word-order.txt` output by the above steps  
- `num_rec`: number of new words to recommend
- `source_words`: any number of given source words
#### Review Order Recommendations
Given a list of known/learned words, and a list of already reviewed words, learned words will be recommended to be reviewed in order of their collective closeness to already reviewed words.  If no reviewed words are given, a random word is chosen from learned words to be the base reviewed word. 
`./review.bin base_file mapping_file num_to_review`
- `base_file`: same as above
- `mapping_file`: same as above
- `num_to_review`: number of sorted review items to recommend this round  
  
Learned and reviewed words will be prompted for after running program

### Demo
Order of operations for test run would be as follows:  
#### If starting from edgelist in (string string double) form
(tg.txt file is included)
```
make
./edge_list_to_nums.bin ./data/tg.txt ./data/tg-nums.txt
./tuple_text_to_binary_csr_mem_weight/text_to_bin.bin ./data/tg-nums.txt 0 0 1 1
./graph_reader_new/recommend.bin ./data/tg-nums.txt ./data/word-order.txt 10 sox earthquake manchester abc
**OUTPUT**:
Learning recommendations :
yankees (Value: 0.829524)
nbc (Value: 0.79526)
cbs (Value: 0.779539)
quake (Value: 0.601341)
liverpool (Value: 0.585569)
End
***
```  
End is printed if number of available recommendations is lower than given recommendation count

#### If starting from vectorfile
(vecfile.txt is not included)  
`./get_edge_list.bin vecfile.txt edgefile.txt 3 0 1`  
then use output `edgefile.txt` in above operations

