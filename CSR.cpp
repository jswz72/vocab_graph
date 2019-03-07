#include "CSR.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>

using std::string;

void CSR::get_counts(std::ifstream &edgefile) {
    std::string line;

    string cur_vtx = "";
    while (std::getline(edgefile, line)) {
        string vtx = line.substr(0, line.find(" "));
        if (vtx != cur_vtx) {
            cur_vtx = vtx;
            vertex_count++;
        }
        edge_count++;
    }
}

void CSR::add_edges(std::ifstream &edgefile) {
    string cur_vtx = "";
    std::string line;
    long vtx_idx = 0;
    long edge_idx = 0;
    while (std::getline(edgefile, line)) {
        string vtx1;
        string vtx2;
        double weight;
        std::istringstream iss(line);
        iss >> vtx1 >> vtx2 >> weight;

        if (vtx1 != cur_vtx) {
            beg_pos[vtx_idx] = edge_idx;
            words[vtx_idx] = vtx1;
            word_idx[vtx1] = vtx_idx;
            idx_words[vtx_idx] = vtx1;
            cur_vtx = vtx1;
            vtx_idx++;
        }
        adj_list[edge_idx] = vtx2;
        weight_list[edge_idx] = weight;
        edge_idx++;
    }
    beg_pos[vtx_idx] = edge_idx;
}

CSR::CSR(const char *edgefilename) {
    std::ifstream edgefile(edgefilename);
    get_counts(edgefile);

    beg_pos = new long[vertex_count];
    words = new string[vertex_count];
    adj_list = new string[edge_count];
    weight_list = new double[edge_count];

    edgefile.clear();
    edgefile.seekg(0, std::ios::beg);
    add_edges(edgefile);
}
