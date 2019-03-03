#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>

using std::cout;
using std::endl;
using std::string;

/*struct Vertex {
    long id;
    string val;
    Vertex(long id, string val): id(id), val(val) {};
    Vertex() {};
};*/

struct CSR {
    long *beg_pos;
    string *words;

    string *adj_list;
    double *weight_list;

    long vertex_count;
    long edge_count;

    CSR(long num_vtx, long num_edge): vertex_count(num_vtx), edge_count(num_edge) {
        beg_pos = new long[num_vtx];
        words = new string[num_vtx];
        adj_list = new string[num_edge];
        weight_list = new double[num_edge];
    };
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Enter edgelist filename" << endl;
        return 1;
    }
    const char *edgefilename = argv[1];

    std::ifstream edgefile(edgefilename);
    std::string line;

    std::unordered_map<std::string, long> word_idxs;

    long num_lines = 0;
    string cur_vtx = "";
    long num_vtx = 0;
    while (std::getline(edgefile, line)) {
        string vtx = line.substr(0, line.find(" "));
        if (vtx != cur_vtx) {
            cur_vtx = vtx;
            num_vtx++;
        }
        num_lines++;
    }

    std::cout << "Num edges " << num_lines << endl;
    std::cout << "Num verticies " << num_vtx << endl;

    CSR *csr = new CSR(num_vtx, num_lines);

    cur_vtx = "";

    edgefile.clear();
    edgefile.seekg(0, std::ios::beg);
    long vtx_idx = 0;
    long edge_idx = 0;
    while (std::getline(edgefile, line)) {
        string vtx1;
        string vtx2;
        double weight;
        std::istringstream iss(line);
        iss >> vtx1 >> vtx2 >> weight;

        if (vtx1 != cur_vtx) {
            csr->beg_pos[vtx_idx] = edge_idx;
            csr->words[vtx_idx] = vtx1;
            cur_vtx = vtx1;
            vtx_idx++;
        }
        csr->adj_list[edge_idx] = vtx2;
        csr->weight_list[edge_idx] = weight;
        cout << vtx1 << "- " << edge_idx << endl;
        edge_idx++;
    }
    csr->beg_pos[vtx_idx + 1] = edge_idx;


    for (long i = 0; i < csr->vertex_count; i++) {
        long beg = csr->beg_pos[i];
        long end = csr->beg_pos[i + 1];
        cout << "DEBUG: " << beg << ", " << end << endl;
        //cout << csr->words[i] << " neighbor list" << endl;
        for (int j = beg; j < end; j++) {
            //cout << csr->adj_list[j] << ", weight: " << csr->weight_list[j] << endl;
        }
        cout << endl;
    }
} 
