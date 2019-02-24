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

double euclidean_dist(std::vector<double> const &x, std::vector<double> const &y) {
    double sum = 0;
    for (int i = 0; i < x.size(); i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

std::vector<std::tuple<std::string, std::string, double> > create_edge_list(
		std::vector<std::tuple<std::string, std::vector<double > > > const &word_vecs, double threshold) {

	std::vector<std::tuple<std::string, std::string, double> > edge_list;
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
        std::vector<std::tuple<std::string, std::string, double> > inner_edge_list;

        #pragma omp for schedule(static) nowait
        for (int i = 0; i < word_vecs.size(); i++) {
            for (int j = 0; j < word_vecs.size(); j++) {
                // No edges to self
                if (i != j) { 
                    double dist = euclidean_dist(std::get<1>(word_vecs[i]), std::get<1>(word_vecs[j])); 
                    if (!threshold || threshold && dist < threshold) {
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
            int sum_work = 0;
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

std::vector<std::tuple<std::string, std::vector<double> > > parse_word_vectors(
		const char *vecfilename, 
		int limit) {
    std::ifstream infile(vecfilename);
    std::string line;

	std::vector<std::tuple<std::string, std::vector<double > > > words;
    //only read until limit lines
    int counter = 0;
    while (std::getline(infile, line)) {
        if (limit && ++counter > limit) {
            break;
        }
        std::istringstream iss(line);
        std::string word;
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Provide vecfile" << endl;
        return 1;
    }
    const char *vecfilename = argv[1];
    const double threshold = argc > 2 ? atof(argv[2]) : 0;
    const int limit = argc > 3 ? atoi(argv[3]) : 0;

    auto words = parse_word_vectors(vecfilename, limit);

    double start_time = omp_get_wtime();
    auto edge_list = create_edge_list(words, threshold);
    double endtime = omp_get_wtime() - start_time;
    for (auto& edge: edge_list) {
	cout << std::get<0>(edge) << " " 
		<< std::get<1>(edge) << " " << std::get<2>(edge) << endl;
    }
    cout << "Num edges: " << edge_list.size() << endl;
    cout << "Time: " << endtime << endl;

}
