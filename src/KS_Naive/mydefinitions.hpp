#ifndef MYDEF_INCLUDED
#define MYDEF_INCLUDED


/// Define the Tuple12 Struct
typedef struct
{
	int X;
	int Y;
	int Z;
	int globalIndex;
}SAtuple;



/// Define the Mapper Struct
typedef struct
{
	int index;
	int rank;
}mapperDuple;



/// Define S Struct
typedef struct
{
	int type;
	int T1;
	int T2;
	int R1;
	int R2;
	int index;
}STuple;

#endif
