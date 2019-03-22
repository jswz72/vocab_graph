#ifndef ERRH
#define ERRH

#include <iostream>

void HandleErr(cudaError_t error, const char *file, int line) {
    if (error != cudaSuccess) {
        std::cout << cudaGetErrorString(error)
            << " in " << file << ", line: " << line << std::endl;
        exit(EXIT_FAILURE);
    }
}

#define HANDLE_ERR(err) HandleErr(err, __FILE__, __LINE__)

#endif
