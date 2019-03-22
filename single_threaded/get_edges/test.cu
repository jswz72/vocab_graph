#include <stdio.h>

int main(int argc, char **argv) {
    int nDevices;
    cudaGetDeviceCount(&nDevices);
    for (int i =0; i < nDevices; i++) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);
        printf("Device name: %s\n", prop.name);
        printf("Capabilities: %d.%d\n", prop.major, prop.minor);
    }
    return 0;
}
