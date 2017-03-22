#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "helpers.hpp"
#include "definitions.hpp"
#include "seqSort.hpp"
#include "psrs.hpp"
#include "pDC3.hpp"

int duplicateChecker(SAtuple* arr, int* NodesBucketSize, int globalLen, mapperDuple* mapper12Dummy,int rank, int size,bool verbose, int pDC3Level,mapperDuple** MapperfromStuple);		
