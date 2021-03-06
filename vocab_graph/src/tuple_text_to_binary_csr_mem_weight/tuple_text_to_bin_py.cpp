/*
 * Copyright 2016 The George Washington University
 * Written by Hang Liu 
 * Directed by Prof. Howie Huang
 *
 * https://www.seas.gwu.edu/~howie/
 * Contact: iheartgraph@gmail.com
 *
 * 
 * Please cite the following paper:
 * 
 * Hang Liu and H. Howie Huang. 2015. Enterprise: breadth-first graph traversal on GPUs. In Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC '15). ACM, New York, NY, USA, Article 68 , 12 pages. DOI: http://dx.doi.org/10.1145/2807591.2807594

 *
 * This file is part of Enterprise.
 *
 * Enterprise is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Enterprise is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Enterprise.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include "wtime.h"

#define SIZE_LIMIT (1<<30)

#define INFTY int(1<<30)
using namespace std;

typedef long int vertex_t;
typedef long int index_t;
typedef double weight_t;

inline off_t fsize(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}


extern "C" void tuple_text_to_binary_csr(const char *tuple_file, 
        unsigned int is_reverse, int skip_head, int thread_count, int weight_flag){
    int fd;
    char* ss_head;
    char* ss;

    std::cout<<"Input: ./exe tuple_file(text) "
        <<"reverse_the_edge(1 reverse, 0 not reverse) lines_to_skip thread_count 0=noweight-1-weight\n";
    size_t file_size = fsize(tuple_file);

    fd=open(tuple_file,O_CREAT|O_RDWR,00666 );
    if(fd == -1)
    {
        printf("%s open error\n", tuple_file);
        perror("open");
        exit(-1);
    }

    ss_head = (char*)mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
    assert(ss_head != MAP_FAILED);
    madvise(ss_head, file_size, MADV_SEQUENTIAL);
    double time_beg = wtime();
    long progress = 1;

    size_t head_offset=0;
    int skip_count = 0;
    while(true)
    {
        if(skip_count == skip_head) break;
        if(head_offset == file_size &&
                skip_count < skip_head)
        {
            std::cout<<"Eorr: skip more lines than the file has\n\n\n";
            exit(-1);
        }

        head_offset++;
        if(ss_head[head_offset]=='\n')
        {
            head_offset++;
            skip_count++;
            if(skip_count == skip_head) break;
        }
        if(head_offset > progress)
        {
            printf("%ld lines processed, %f seconds elapsed\n", head_offset, wtime() - time_beg);
            progress <<=1;
        }
    }

    ss = &ss_head[head_offset];
    file_size -= head_offset;

    size_t curr=0;
    size_t next=0;

    //step 1. vert_count,edge_count,
    size_t edge_count=0;
    size_t vert_count;
    vertex_t v_max = 0;
    vertex_t v_min = INFTY;//as infinity
    vertex_t a;

    //int reg = 0;
    progress = 1;
    int checkwt=0;//checkwt=3 for weight
    while(next<file_size){
        char* sss=ss+curr;
        a = atol(sss);
        if(weight_flag!=0)
        {
            checkwt++;
        }
        //std::cout<<a<<"\n";
        //if(reg ++ > 10) break;
        if(checkwt!=3)//3 for the weights
        {
            if(v_max<a){   // its checking the weight also which should be skipped while updating the min and max vertex ID //anil
                v_max = a;
            }
            if(v_min>a){
                v_min = a;
            }
        }

        while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
            next++;
        }
        while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
            next++;
        }
        /*
           while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
           */
        curr = next;
        if(next > progress)
        {
            printf("%f%%, %f seconds elapsed\n", next*100.0/file_size, wtime() - time_beg);
            progress <<=1;
        }

        //one vertex is counted once
        if(checkwt!=3)// not considering weight as the vertex
        {
            edge_count++;
        }
        if(checkwt==3)
        {
            checkwt=0;
        }
}
//printf("%f seconds elapsed\n", wtime() - time_beg);

const index_t line_count=edge_count>>1;
if(!is_reverse) edge_count >>=1;

vert_count = v_max + 1;
assert(v_min<INFTY);
cout<<"edge count: "<<edge_count<<endl;
cout<<"max vertex id: "<<v_max<<endl;
cout<<"min vertex id: "<<v_min<<endl;

cout<<"edge count: "<<edge_count<<endl;
cout<<"vert count: "<<vert_count<<endl;

//step 2. each file size
char filename[256];
sprintf(filename,"%s_csr.bin",tuple_file);
//int fd4 = open(filename,O_CREAT|O_RDWR|O_LARGEFILE,00666 );
FILE *fd4 = fopen64(filename, "wb");
assert(fd4 != NULL);

//ftruncate(fd4, edge_count*sizeof(vertex_t));
//vertex_t* adj_file = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd4,0);
vertex_t* adj = (vertex_t*)mmap(NULL,
        edge_count*sizeof(vertex_t),
        PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
assert(adj != MAP_FAILED);
//	assert(adj_file != MAP_FAILED);
//madvise(adj, edge_count*sizeof(vertex_t), MADV_RANDOM); //-- NOT work!
//	msync(adj, edge_count*sizeof(vertex_t), MS_ASYNC); //-- NOT work!

//added by Hang to generate a weight file
sprintf(filename,"%s_weight.bin",tuple_file);
//int fd6 = open(filename,O_CREAT|O_RDWR|O_LARGEFILE,00666 );
FILE *fd6  = fopen64(filename, "wb");
assert(fd6 != NULL);

//ftruncate(fd6, edge_count*sizeof(vertex_t));
//vertex_t* weight_file= (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd6,0);
weight_t* weight= (weight_t*)mmap(NULL,
        edge_count*sizeof(weight_t),
        PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
assert(weight != MAP_FAILED);
//	assert(weight_file != MAP_FAILED);
//-End 
//	madvise(weight, edge_count*sizeof(vertex_t), MADV_RANDOM);	//-- NOT WORK
//msync(weight, edge_count*sizeof(vertex_t), MS_ASYNC); //-- NOT work!

//	sprintf(filename,"%s_head.bin",tuple_file);
//	int fd5 = open(filename,O_CREAT|O_RDWR,00666 );
//	ftruncate(fd5, edge_count*sizeof(vertex_t));
//	vertex_t* head = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd5,0);
//	assert(head != MAP_FAILED);

//-----------------------------
//Avoid mmapped memory space for degree because 
//**undirected** graph requires to random update degree array
//This is a big problem for LUSTRE filesystem.
//-------------------------------------------------
//	sprintf(filename,"%s_deg.bin",tuple_file);
//	int fd2 = open(filename,O_CREAT|O_RDWR,00666 );
//	ftruncate(fd2, vert_count*sizeof(index_t));
//	index_t* degree = (index_t*)mmap(NULL,vert_count*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd2,0);
index_t *degree = (index_t *)mmap (NULL,
        vert_count*sizeof(index_t),
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);
assert(degree != MAP_FAILED);

sprintf(filename,"%s_beg_pos.bin",tuple_file);
int fd3 = open(filename,O_CREAT|O_RDWR,00666 );
ftruncate(fd3, (vert_count+1)*sizeof(index_t));
index_t* begin  = (index_t*)mmap(NULL,(vert_count+1)*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd3,0);
assert(begin != MAP_FAILED);

//step 3. write degree
for(int i=0; i<vert_count;i++){
    degree[i]=0;
}

vertex_t index, dest;
size_t offset =0;
curr=0;
next=0;

printf("Getting degree progress ...\n");
progress = 1;

while(offset<line_count){
    char* sss=ss+curr;
    index = atol(sss);
    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    curr = next;

    char* sss1=ss+curr;
    dest=atol(sss1);

    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    //Considering extra weight item in the data file //21 June 018 Anil
  //  cout<<offset<<endl;
 if(weight_flag!=0)
 {
    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    //~Considering extra weight item in the data file //21 June 018 Anil
 }
    curr = next;
    degree[index]++;
    if(is_reverse) degree[dest]++;
    if(offset > progress)
    {
//        cout<<offset<<endl;
        printf("%f%%, %f seconds elapsed\n", offset*100.0/line_count, wtime() - time_beg);
        progress <<=1;
    }
    //		cout<<index<<" "<<degree[index]<<endl;

    offset++;
}
//	exit(-1);
begin[0]=0;
begin[vert_count]=edge_count;

printf("\nCalculate beg_pos ...\n");
for(size_t i=1; i<vert_count; i++){
    begin[i] = begin[i-1] + degree[i-1];
    //		cout<<begin[i]<<" "<<degree[i]<<endl;
    degree [i-1] = 0;
}
degree[vert_count-1] = 0;
//step 4: write adjacent list 
vertex_t v_id;
offset =0;
next = 0;
curr = 0;

progress = 1;
printf("\nConstructing CSR progress...\n");
while(offset<line_count){
    char* sss=ss+curr;
    index = atol(sss);
    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    curr = next;

    char* sss1=ss+curr;
    v_id = atol(sss1);
    adj[begin[index]+degree[index]] = v_id;
    //Added by Zhenlin
	if(is_reverse){
	   if(v_id==index) //self-loop
          adj[begin[v_id]+degree[v_id] + 1] = index;
       else // normal case
          adj[begin[v_id]+degree[v_id]] = index;
    }
    //if(is_reverse) adj[begin[v_id]+degree[v_id]] = index;
    //vertex_t wtvalue;
    weight_t wtvalue;
    
    if(weight_flag!=0)
    {
    //Cosidering the weight is present in the file as third column anil 21 June
    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    
    curr=next;
    char* sss2=ss+curr;
    //int wtvalue=atol(sss2);
    /*weight_t*/  wtvalue=atof(sss2);
    //~ /Cosidering the weight is present in the file as third column anil 21 June
    }
    else
    {
        wtvalue=(rand()%63+1);
    }
    //Added by Hang
    //weight_t wtvalue=
  //  adj[begin[index]+degree[index]] = wtvalue;
   // if(is_reverse) adj[begin[v_id]+degree[v_id]] = wtvalue;

//    	vertex_t rand_weight=(rand()%63+1);
    weight[begin[index]+degree[index]] = wtvalue;// rand_weight;
    if(is_reverse)
        weight[begin[v_id]+degree[v_id]] = wtvalue;//rand_weight;
    //-End

    //head[begin[index]+degree[index]]= index;
    while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
        next++;
    }
    while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
        next++;
    }
    curr = next;
    degree[index]++;
    if(is_reverse) degree[v_id]++;
    if(offset > progress)
    {
        printf("%f%%, %f seconds elapsed\n", offset*100.0/line_count, wtime() - time_beg);
        progress <<=1;
    }

    offset++;
}


long long int to_write_size = edge_count*sizeof(vertex_t);
//long long int to_write_off = 0;
std::cout<<"Dumping "<<to_write_size<<" bytes CSR and weight arrays to disk ...\n";	
//	memcpy(adj_file, adj, edge_count*sizeof(vertex_t));
//	memcpy(weight_file, weight, edge_count*sizeof(vertex_t));
//std::cout<<"Adj Write "<<write(fd4, adj, edge_count*sizeof(vertex_t))<<" bytes and expect "<< edge_count*sizeof(vertex_t)<<" bytes\n";
//std::cout<<"weight Write "<<write(fd6, weight, edge_count*sizeof(vertex_t))<<" bytes and expect "<< edge_count*sizeof(vertex_t)<<" bytes\n";

std::cout<<"Adj Write "<<fwrite(adj, edge_count, sizeof(vertex_t), fd4)<<" bytes and expect "<< edge_count*sizeof(vertex_t)<<" bytes\n";
std::cout<<"weight Write "<<fwrite(weight, edge_count, sizeof(vertex_t), fd6)<<" bytes and expect "<< edge_count*sizeof(vertex_t)<<" bytes\n";


//	while(to_write_size > 0)
//	{
//		if(to_write_size > SIZE_LIMIT)
//        {   
//            std::cout<<"Write "<<SIZE_LIMIT<<" bytes\n";
//            assert(pwrite(fd4, adj+to_write_off, SIZE_LIMIT, to_write_off) == SIZE_LIMIT);
//            assert(pwrite(fd6, weight+to_write_off, SIZE_LIMIT, to_write_off) == SIZE_LIMIT);
//        }
//        else
//        {
//            std::cout<<"Write "<<to_write_size<<" bytes\n";
//            assert(pwrite(fd4, adj+to_write_off, to_write_size, to_write_off) == to_write_size);
//            assert(pwrite(fd6, weight+to_write_off, to_write_size, to_write_off) == to_write_size);
//        }
//
//        to_write_size -= SIZE_LIMIT;
//        to_write_off += SIZE_LIMIT;
//        //- the last round maybe wrong to_write_size and to_write_off, but will already exit.
//	}

printf("%f seconds elapsed\n\n", wtime() - time_beg);


//step 5
//print output as a test
//	for(size_t i=0; i<vert_count; i++){
//	for(size_t i=0; i<(vert_count<8?vert_count:8); i++){
//		cout<<i<<" "<<begin[i+1]-begin[i]<<" ";
//		for(index_t j=begin[i]; j<begin[i+1]; j++){
//			cout<<adj[j]<<" ";
//		}
////		if(degree[i]>0){
//			cout<<endl;
////		}
//	}

//	for(int i=0; i<edge_count; i++){
//	for(int i=0; i<64; i++){
//		cout<<degree[i]<<endl;
//	}

munmap( ss,sizeof(char)*file_size );

//-Added by Hang
munmap( weight,sizeof(vertex_t)*edge_count );
//	munmap( weight_file,sizeof(vertex_t)*edge_count );
//-End

munmap( adj,sizeof(vertex_t)*edge_count );
//	munmap( adj_file,sizeof(vertex_t)*edge_count );
//	munmap( head,sizeof(vertex_t)*edge_count );
munmap( begin,sizeof(index_t)*vert_count+1 );
munmap( degree,sizeof(index_t)*vert_count );
//	close(fd2);
close(fd3);
//	close(fd4);
//	close(fd5);

//-Added by Hang
//	close(fd6);
fclose(fd4);
fclose(fd6);
//-End
}
