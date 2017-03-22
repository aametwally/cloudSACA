#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include "stringSorting.hpp"
#include "helpers.hpp"

using namespace std;

int* BucketSorting(int SA2Size, int* SA2, bool verbose,int globalstarttime, int rank, vector<char> reference_array);
