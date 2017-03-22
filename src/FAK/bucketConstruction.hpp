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
#include "helpers.hpp"
using namespace std;

int BucketConstruction(vector<char> AlphapetChar, vector<char> reference_array, int size, int numtasks, int rank, int w, bool verbose, int** SA2);
