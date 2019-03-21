default: get_edge to_nums
get_edge:
	g++ get_edge_list.cpp --std=c++11 -fopenmp -Wall -o get_edge_list.bin

to_nums:
	g++ edge_list_to_nums.cpp --std=c++11 -fopenmp -Wall -o edge_list_to_nums.bin


