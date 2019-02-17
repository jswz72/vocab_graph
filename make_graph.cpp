#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <vector>
#include <tuple>
#include <cmath>
#include <unordered_map>

using std::cout;
using std::endl;

std::unordered_map<std::string, std::vector<double> > parse_word_vectors(const char *vecfilename, int limit) {
    std::ifstream infile(vecfilename);
    std::string line;

    std::unordered_map<std::string, std::vector<double> > words;
    int counter = 0;
    while (std::getline(infile, line)) {
        if (limit && ++counter > limit) {
            break;
        }
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        bool alphanum = true;
        for (std::string::iterator it=word.begin(); it != word.end(); it++) {
            if (!isalpha(*it)) {
                alphanum = false;
                break;
            }
        }
        if (!alphanum)
            continue;
        double d;
        std::vector<double> vec;
        while ((iss >> d)) {
            vec.push_back(d);
        }
        words[word] = vec;
    }
    return words;
}

double euclidean_dist(std::vector<double> x, std::vector<double> y) {
    double sum = 0;
    for (int i = 0; i < x.size(); i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Provide vecfile" << endl;
        return 1;
    }
    const char *vecfilename = argv[1];
    const int threshold = argc > 2 ? atoi(argv[2]) : 0;
    const int limit = argc > 3 ? atoi(argv[3]) : 0;

    std::unordered_map<std::string, std::vector<double> > words = parse_word_vectors(vecfilename, limit);

    std::vector<std::tuple<std::string, std::string, double> > edge_list;
    size_t *prefix;
    #pragma omp parallel
    {
        int threadcount = omp_get_num_threads();
        int thread_idx = omp_get_thread_num();
        #pragma omp single
        {
            prefix = new size_t[nthreads+1];
            prefix[0] = 0;
        }
        std::vector<std::tuple<std::string, std::string, double> > inner_edge_list;
        #pragma omp for
        for (auto const& vtx1_pair: words) {
            for (auto const& vtx2_pair: words) {
                if (vtx1_pair.first == vtx2_pair.first)
                    continue
                double dist = euclidean_dist(vt1_pair.second, vtx2_pair.second);
                if (dist < threshold)
                    std::tuple<std::string, std::string, double> edge(vtx1_pair.first, vtx2_pair.first, dist);
                    inner_edge_list.push_back(edge);
            }
        }
        prefix[thread_idx + 1] = inner_edge_list.size();
        #pragma omp barrier
        #pragma omp single
        {
            for(int i = 0; i <= threadcount; i++) {
                prefix[i] += prefix[i - 1];
            }
            edge_list.resize(inner_edge_list.begin(), inner_edge_list.end(), edge_list.begin() + prefix[thread_idx]);
        }
        std::copy(inner_edge_list.begin(), inner_edge_list.end(), edge_list.begin() + prefix[thread_idx]);
    }
    delete[] prefix;

    std::vector<double> digs = words["the"];
    for (int i = 0; i < digs.size(); i++) {
        cout << digs[i] << ", ";
    }
}