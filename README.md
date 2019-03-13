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
`./executable vectorfile euclidean_distance_threshold limit`  
 Same params as above  
 Writes to console  
 Uses openmp multithreading, use env params to specify num threads 

### To process edgelist:
Need to convert `string string num` representation to `num num num` for easier processing:  
Use `edge_list_to_nums.cpp`  
`./executable vector_file_input vector_file_output`  
Will also create `word-order.txt` file in output directory, for use in later processing back to string words

----
Use `tuple_text_to_binary_csr_mem_weight` to write a binary CSR graph of the previous numeric edgelist  
`./executable edge_list_file reverse_edges(1/0) lines_to_skip(0+ thread_count(1+) write_weights(1/0)`  
Used from, and explained at [graph_project_start](https://github.com/asherliu/graph_project_start)

----
Use `graph_reader_new` to read binary CSR and perform any operations defined in its main function (`review_and_recommend`, etc.)    
`./executable beg_pos_file csr_file weight_file`  
Used from, and explained at [graph_project_start](https://github.com/asherliu/graph_project_start)  
Can use `read_and_exec.sh` to facillitate this by just passing the base name of the `*beg_pos/*csr/*weight` files  

