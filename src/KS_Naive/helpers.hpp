//#ifndef HELPERS
//#define HELPERS


#include <stdio.h>
#include <string>
#include "mydefinitions.hpp"
#include <iostream>
#include <stdlib.h>
#include <cmath>
//#include <fstream>

using namespace std;

void elapsedTime(int rank,long start,string message,int pDC3Level);

void swap_SAtuple(SAtuple* A, SAtuple* B);
void copyAtoB_SAtuple(SAtuple* A, SAtuple* B);
void copyArrayAtoB_SAtuple(SAtuple* A, SAtuple* B, int size);
int isEqual_SAtuple(SAtuple *A, SAtuple *B);
int largerThan_SAtuple(SAtuple A, SAtuple B);
int smallerThan_SAtuple(SAtuple A, SAtuple B);
int smallerThan_DC3(mapperDuple A, mapperDuple B);
int isEqual_DC3(mapperDuple *A, mapperDuple *B);
int largerThan_DC3(mapperDuple A, mapperDuple B);
void swap_mapperDuple(mapperDuple* A, mapperDuple* B);
void copyAtoB_mapperDuple(mapperDuple* A, mapperDuple* B);
void copyArrayAtoB_mapperDuple(mapperDuple* A, mapperDuple* B, int size);
int isEqual_mapperDuple(mapperDuple *A, mapperDuple *B);
int largerThan_mapperDuple(mapperDuple A, mapperDuple B);
int smallerThan_index_mapperDuple(mapperDuple A, mapperDuple B);

void assert_malloc(void *ptr, int _MYPROC);
void collect_files(int tasks);
void TestVector();



void swap_S(STuple* A, STuple* B);
void copyAtoB_S(STuple* A, STuple* B);
void copyArrayAtoB_S(STuple* A, STuple* B, int size);
int isEqual_S(STuple *A, STuple *B);
int largerThan_S(STuple A, STuple B);
int smallerThan_S(STuple A, STuple B);




