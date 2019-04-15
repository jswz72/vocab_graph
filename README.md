#### API for creating and operating on Vocab/Knowledge Graph

Python API accessible via `vocab_graph/graph_ops`:

__get_edge_list__: Use to create an edge-list from a given vectorfile name.  
- Will create edgelist on Euclidean distance relationship between word vectors.  
- Provide filepath of output.
- Can optionally give threshold parameters for max Euclidean distance to include, and limit for how many lines of input file to read..
- Provide *to_nums*= 1 in order to output an edge-list of numerical indices, with accompanying word_mapping file.  Otherwise, will output edgelist of form *string string number*  
__NOTE__: In parsing the vectorfile, non alpha-containing words (i.e. just numbers or just puncutation) are skipped over.  

After you have an edge-list.  Use *vocab_graph/src/tuple_text_to_binary_csr_mem_weight* to translate this to a graph format (Python API in progress). 


---

__VocabGraph__ class: Holds vocab graph constructed above, and has methods for operations on the graph.
- Initialize with the base filename created via the above *tuple_text_to_binary_csr_mem_weight*, and the filename of the word mapping file produced from *get_edge_list*.  

__VocabGraph.recommend__: Recommend words to learn based on given source words, calculated via collective closeness to source words.  
- Provide number of recommendations to make.  
- Additionally, can provide recomendation pool in form of a list.  If this is given, recommendations will only be made out of this pool.  

__VocabGraph.review__: Recommend a reviewing order of given *learned_words* given a list of *reviewed_words*, and update memories of learned words.  
- Output review recomendation order is determined by collecitve closeness to *reviewed_words*.
- If *reviewed_words* are not given, then will chose random word from *learned_words* as an initial review word.  
- Input *learned_words* must be in form of *WordMem* object, with *last_learned* and *strenght* fields populated. 
- Will calculate the memory of each word from the given WordMem parameters, and the returned recommended review list will contain updated WordMems.
- This can be called cyclically, with the caller updating the *reviewed_words* and WordMem parameters between calls to show the reiviewing of new words, and the resulting memories. 

__WordMem__ class:  Class to aggregate memory/forgetting parameters for reviewing.  
- *word* is the actual word string.  
- *last_learned* is the number of time units since the last time the word has been learned/reviewed.  
- *strength* is the strength/stability of memory for the specific word (higher strength means memory will decay slower).
