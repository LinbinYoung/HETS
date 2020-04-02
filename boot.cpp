// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "common.h"

using namespace std;
using namespace seal;

int main(int argc, char** argv){
#ifdef SEAL_VERSION
    cout << "Microsoft SEAL version: " << SEAL_VERSION << endl;
#endif
    if (argc < 1){
        printf("Usage:: ./clustarexamples [valid degree_size: ]");
        return -1;
    }
    bfv_performance_custom(atoi(argv[1]));
    return 0;
}