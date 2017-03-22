#include "definitions.hpp"

void tuplesConst12(int* text, int textLen, SAtuple *tuples12,int textLenGlobal,int size, int rank);
void Const_S0(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex);
void Const_S1(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex);
void Const_S2(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex);
void S_Tuple_Construction(STuple* S,int* rankArray,int* text,int textLenLocal,int textLenGlobal,int _MYPROC, int _PROCS);
