default: review recommend

recommend:
	g++ -Wall ../cpp/graph_ops/recommend_py.cpp ../cpp/graph_ops/utils.cpp ../cpp/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o recommend.dll

review:
	g++ -Wall ../cpp/graph_ops/review_py.cpp ../cpp/graph_ops/utils.cpp ../cpp/graph_ops/review_and_recommend.cpp -fPIC -shared -std=c++11 -fopenmp -o review.dll
