#ifndef _SORT_PSRS_H
#define _SORT_PSRS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include "mydefinitions.hpp"


int all_sort_psrs_i(SAtuple *A, int A_size, SAtuple **B, int size, int rank);
int all_sort_S_i(STuple *A,int A_size, STuple **B, int size, int rank, int pDC3Level);
int all_sort_Mapper_i(mapperDuple* mapper, int mapperSize, int localTextLen, int globalTextLen,int rank, int size,int pDC3Level,int** rankArray);
int all_sort_Mapper_recursion(mapperDuple* mapper, mapperDuple **SortedMapper, int mapperSize, int rank, int size,int pDC3Level);

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#endif
