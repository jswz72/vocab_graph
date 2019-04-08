default: get_edges review recommend 

recommend:
	g++ -Wall src/graph_ops/recommend_py.cpp src/graph_ops/utils.cpp src/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o recommend.so

review:
	g++ -Wall src/graph_ops/review_py.cpp src/graph_ops/utils.cpp src/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o review.so

get_edges:
	g++ -Wall src/get_edges/get_edge_list_py.cpp -fPIC -shared -std=c++11 -fopenmp -o get_edges.so
