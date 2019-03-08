#include <iostream>
#include "GraphOps.hpp"
#include "graph.h"

int main(int args, char **argv)
{
	std::cout<<"Input: ./exe beg csr weight target\n";
	if(args!=4){std::cout<<"Wrong input\n"; return -1;}
	
	const char *beg_file=argv[1];
	const char *csr_file=argv[2];
	const char *weight_file=argv[3];
	
	graph<long, long, long, long, long, long>
	*csr = new graph
        <long, long, long, long, long, long>
	(beg_file,csr_file,weight_file);
    
    for(int i = 0; i < 10; i++)
    {
        int beg = csr->beg_pos[i];
        int end = csr->beg_pos[i+1];
        std::cout<<i<<"'s neighor list: ";
         //   std::cout<<i<<"'s outgoing money: ";
        for(int j = beg; j < end; j++)
        {
            std::cout<<csr->csr[j]<<" ";
            std::cout<<"Money:"; std::cout<<csr->weight[j]<<" ";
        }
        std::cout<<"\n";
    }
    std::cout << "Edges: " << csr->edge_count << std::endl;
    std::cout << "Verticies: " << csr->vert_count << std::endl;
	return 0;	
}
