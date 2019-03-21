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
#include "wtime.h"

using std::cout;
using std::endl;
using std::string;

const string ofname = "edge-list.txt";
const string woname = "word-order.txt";

double euclidean_dist(std::vector<double> const &x, std::vector<double> const &y) {
    double sum = 0;
    for (unsigned int i = 0; i < x.size(); i++) {
        double diff = x[i] - y[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

std::vector<std::tuple<string, string, double> > create_edge_list(
		std::vector<std::tuple<string, std::vector<double > > > const &word_vecs, double threshold) {

	std::vector<std::tuple<string, string, double> > edge_list;
	cout << "Word vec size: " << word_vecs.size() << endl;
 
    for (unsigned int i = 0; i < word_vecs.size(); i++) {
        for (unsigned int j = 0; j < word_vecs.size(); j++) {
            // No edges to self
            if (i != j) { 
                double dist = euclidean_dist(std::get<1>(word_vecs[i]), std::get<1>(word_vecs[j])); 
                if (!threshold || dist < threshold) {
                    auto edge = std::make_tuple(std::get<0>(word_vecs[i]), std::get<0>(word_vecs[j]), dist); 
                    edge_list.push_back(edge);
                }
            }
        }
    }
	return edge_list;
};

std::vector<std::tuple<string, std::vector<double> > > parse_word_vectors(
		const char *vecfilename, 
		int limit) {
    std::ifstream infile(vecfilename);
    string line;

	std::vector<std::tuple<string, std::vector<double > > > words;
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

void write_edge_list(std::vector<std::tuple<string, string, double> >&edge_list) {
	std::ofstream outfile(ofname);
	for (auto& edge: edge_list) {
		outfile << std::get<0>(edge) << " " 
			<< std::get<1>(edge) << " " << std::get<2>(edge) << endl;
	}
}


void write_edge_list_to_nums(std::vector<std::tuple<string, string, double> > &edge_list) {
	std::ofstream outfile(ofname);
	std::ofstream word_order(woname);
    std::unordered_map<std::string, int> words;
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "./exec vecfile outfile(optional) "
			<< "threshold(optional) limit(optional) "
			<< "to_nums(optional)" << endl;
        return 1;
    }
    const char *vecfilename = argv[1];
    const int write_file = argc > 2 ? atoi(argv[2]) : 0;
    const double threshold = argc > 3 ? atof(argv[3]) : 0;
    const int limit = argc > 4 ? atoi(argv[4]) : 0;
	const int to_nums = argc > 5 ? atoi(argv[5]) : 0;

    auto words = parse_word_vectors(vecfilename, limit);

    double starttime = wtime();
    auto edge_list = create_edge_list(words, threshold);
    double endtime = wtime() - starttime;
	if (write_file) {
		if (to_nums)
			write_edge_list_to_nums(edge_list);
		else
			write_edge_list(edge_list);
	} else {
		for (auto& edge: edge_list) {
			cout << std::get<0>(edge) << " " 
				<< std::get<1>(edge) << " " << std::get<2>(edge) << endl;
		}
	}
    cout << "Num edges: " << edge_list.size() << endl;
    cout << "Time: " << endtime << endl;

}
