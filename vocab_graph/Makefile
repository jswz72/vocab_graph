default: get_edges text_to_bin review recommend 

recommend:
	g++ -Wall -Wno-sign-compare src/graph_ops/recommend_py.cpp src/graph_ops/utils.cpp src/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o recommend.so

review:
	g++ -Wall -Wno-sign-compare src/graph_ops/review_py.cpp src/graph_ops/utils.cpp src/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o review.so

get_edges:
	g++ -Wall -Wno-sign-compare src/get_edges/get_edge_list_py.cpp -fPIC -shared -std=c++11 -fopenmp -o get_edges.so

text_to_bin:
	g++ -Wall -Wno-sign-compare src/tuple_text_to_binary_csr_mem_weight/tuple_text_to_bin_py.cpp -fPIC -shared -std=c++11 -fopenmp -o tuple_text_to_bin.so
