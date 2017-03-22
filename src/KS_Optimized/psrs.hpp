#include "mpi.h"
#include "definitions.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "seqSort.hpp"
#include "helpers.hpp"
#include "utilities.hpp"

int all_sort_psrs_i(SAtuple *A, int A_size, SAtuple **B, int size, int rank, int pDC3Level, int verbose);
int all_sort_S_i(STuple *A,int A_size, STuple **B, int size, int rank, int pDC3Level, int verbose);
int all_sort_Mapper_i(mapperDuple* mapper, int mapperSize, int localTextLen, int globalTextLen,int rank, int size,int pDC3Level,int** rankArray);
int all_sort_Mapper_recursion(mapperDuple* mapper, mapperDuple **SortedMapper, int mapperSize, int rank, int size,int pDC3Level, int verbose);

void mergeAll_S(STuple** listOfIntegers,  STuple** result,int TotalSize,int* ListLen,int size);
void mergeAll_SA(SAtuple** listOfIntegers,  SAtuple** result,int TotalSize,int* ListLen,int size);
void mergeAll_Mapper_Recursion(mapperDuple** listOfIntegers,  mapperDuple** result,int TotalSize,int* ListLen,int size);



#ifndef maxx
#define maxx(a,b) ((a)>(b)?(a):(b))
#endif


