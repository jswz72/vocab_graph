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
#include <unordered_set>
#include <omp.h>

using std::cout;
using std::endl;
using std::string;

typedef std::vector<std::tuple<string, string, double> > EdgeList;
typedef std::tuple<string, std::vector<double > > WordVec;


// Get the Euclidean distance between two word vectors
double euclidean_dist(std::vector<double> const &x, std::vector<double> const &y) {
    double sum = 0;
    for (unsigned int i = 0; i < x.size(); i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Obtain an edge list from a given vector file
EdgeList create_edge_list(std::vector<WordVec> const &word_vecs, double threshold) {

	EdgeList edge_list;
	cout << "Word vec size: " << word_vecs.size() << endl;
 
	size_t *threadwork;
    #pragma omp parallel
    {
        int threadcount = omp_get_num_threads();
        int thread_idx = omp_get_thread_num();
        #pragma omp single
        {
			threadwork = new size_t[threadcount + 1];
			cout << "Using " << threadcount << " threads" << endl;
        }
        std::vector<std::tuple<string, string, double> > inner_edge_list;

        #pragma omp for schedule(static) nowait
        for (unsigned int i = 0; i < word_vecs.size(); i++) {
            for (unsigned int j = 0; j < word_vecs.size(); j++) {
                // No edges to self
                if (i != j) { 
                    double dist = euclidean_dist(std::get<1>(word_vecs[i]), std::get<1>(word_vecs[j])); 
                    if (!threshold || dist < threshold) {
                        auto edge = std::make_tuple(std::get<0>(word_vecs[i]), std::get<0>(word_vecs[j]), dist); 
                        inner_edge_list.push_back(edge);
                    }
                }
            }
        }
        threadwork[thread_idx + 1] = inner_edge_list.size();
        #pragma omp barrier
        #pragma omp single
        {
            for(int i = 1; i <= threadcount; i++) {
				threadwork[i] += threadwork[i - 1];
            }
            edge_list.resize(edge_list.size() + threadwork[threadcount]);
        }
        std::copy(inner_edge_list.begin(), inner_edge_list.end(), edge_list.begin() + threadwork[thread_idx]);
    }
    delete[] threadwork;
	return edge_list;
};
std::vector<WordVec> parse_word_vectors(const char *vecfilename, int limit) {
    std::ifstream infile(vecfilename);
    cout << "vec: " << vecfilename << endl;
    string line;

	std::vector<WordVec> words;
    //only read until limit lines
    int counter = 0;
    while (std::getline(infile, line)) {
        if (limit && ++counter > limit) {
            break;
        }
        std::istringstream iss(line);
        string word;
        iss >> word;
	// only take alphanumeric 
        bool alphanum = true;
        for (auto const& ch: word) {
            if (!isalpha(ch)) {
                alphanum = false;
                break;
            }
        }
        if (!alphanum)
            continue;
        double d;
        std::vector<double> vec;
        while ((iss >> d))
            vec.push_back(d);
		auto tup = std::make_tuple(word, vec);
		words.push_back(tup);
    }
    return words;
}

// Combine directory name with filename, lookout for possible slash separator.
// Appending unix only right now.
string combine_dir(string dir, string name) {
    return dir + ((dir.back() == '/' || dir.back() == '\\') ? name : '/' + name);
}

// Write edge list to file
void write_edge_list(std::vector<std::tuple<string, string, double> >&edge_list, string ofpath) {
	std::ofstream outfile(ofpath);
	for (auto& edge: edge_list) {
		outfile << std::get<0>(edge) << " " 
			<< std::get<1>(edge) << " " << std::get<2>(edge) << endl;
	}
}

// Write edge list to file, translating words to numerical indices.
// Also write word-order file to get back word meaning from indices
void write_edge_list_to_nums(std::vector<std::tuple<string, string, double> > &edge_list, 
        string ofpath, string wopath) {
	std::ofstream outfile(ofpath);
	std::ofstream word_order(wopath);
    std::unordered_map<string, int> words;
    int counter = 0;

	for (auto& edge: edge_list) {
		string vtx1 = std::get<0>(edge);
		string vtx2 = std::get<1>(edge);
		double weight = std::get<2>(edge);
		if (words.find(vtx1) == words.end()) {
			words[vtx1] = counter++;
			word_order << vtx1 << endl;
		}
		if (words.find(vtx2) == words.end()) {
			words[vtx2] = counter++;
			word_order << vtx2 << endl;
		}
        outfile << words[vtx1] << " " << words[vtx2] << " " << weight << "\n";
	}
}

extern "C" void get_edge_list(const char *vecfilename, const char *edge_file,
        double threshold, unsigned int limit, const int to_nums, const char *word_file) {

    auto words = parse_word_vectors(vecfilename, limit);

    string edge_fname(edge_file);
    string word_fname(word_file);
    cout << edge_fname << " " << word_fname << endl;

    double start_time = omp_get_wtime();
    auto edge_list = create_edge_list(words, threshold);
    double endtime = omp_get_wtime() - start_time;
    if (to_nums)
        write_edge_list_to_nums(edge_list, edge_fname, word_fname);
    else
        write_edge_list(edge_list, edge_fname);
    cout << "Num edges: " << edge_list.size() << endl;
    cout << "Time: " << endtime << endl;
}

