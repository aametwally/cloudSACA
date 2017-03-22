#include "helpers.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "mydefinitions.hpp"

using namespace std;

/// Measure the elapsed time 
void elapsedTime(int rank,long start,string message,int pDC3Level)
{
	long finish = clock();
	long total= finish-start;
    printf("Thread=%d, %s=%ld Sec, pDC3LEVEL=%d\n",rank, message.c_str(), (long) total / CLOCKS_PER_SEC,pDC3Level);
	
}

void TestVector()
{
	vector<char> reference_array;
	reference_array.push_back('b');
	printf("refer=%c\n",reference_array[0]);
}


void swap_SAtuple(SAtuple* A, SAtuple* B)
{
   SAtuple temp;
   temp.X= (*A).X; temp.Y= (*A).Y; temp.Z= (*A).Z; temp.globalIndex= (*A).globalIndex; //temp.rank= (*A).rank;   
   (*A).X= (*B).X; (*A).Y= (*B).Y; (*A).Z= (*B).Z; (*A).globalIndex= (*B).globalIndex; //(*A).rank= (*B).rank;
   (*B).X= temp.X; (*B).Y= temp.Y; (*B).Z= temp.Z; (*B).globalIndex= temp.globalIndex; //(*B).rank= temp.rank;
}


void copyAtoB_SAtuple(SAtuple* A, SAtuple* B)
{
	(*B).X= (*A).X;
	(*B).Y= (*A).Y;
	(*B).Z= (*A).Z;
	(*B).globalIndex= (*A).globalIndex;
	//(*B).rank= (*A).rank;	
}

void copyArrayAtoB_SAtuple(SAtuple* A, SAtuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_SAtuple(&A[i], &B[i]);
}

int isEqual_SAtuple(SAtuple *A, SAtuple *B)
{
	if((*A).X==(*B).X && (*A).Y==(*B).Y && (*A).Z==(*B).Z) return 1;
	return 0;
}

int largerThan_SAtuple(SAtuple A, SAtuple B)
{
	if(A.X > B.X) return 1;
	else if( A.X < B.X ) return 0;
	else if (A.X == B.X) 
	{
		if(A.Y > B.Y) return 1;
		else if( A.Y < B.Y ) return 0;
		else if (A.Y == B.Y)
		{
			if(A.Z > B.Z) return 1;
			else
			return 0;
			//~ else if( A.Z < B.Z) return 0;
			//~ else if (A.Z == B.Z)
			//~ {
				//~ if(A.globalIndex > B.globalIndex) return 1;
				//~ //else return 0;
				//~ else if( A.globalIndex < B.globalIndex ) return 0;
				//~ else if (A.globalIndex == B.globalIndex)
				//~ {
					//~ if( A.rank > B.rank ) return 1;
					//~ else return 0;
				//~ }
			//~ }
		} 		
	}
}

int smallerThan_SAtuple(SAtuple A, SAtuple B)
{
	if(A.X > B.X) return 0;
	else if( A.X < B.X ) return 1;
	else if (A.X == B.X) 
	{
		if(A.Y > B.Y) return 0;
		else if( A.Y < B.Y ) return 1;
		else if (A.Y == B.Y)
		{
			if(A.Z > B.Z) return 0;
			
			/// **************
			/// Might be a problem here which affects the fastasort part
			/// *************************
			//~ else return 1;
			else if( A.Z < B.Z) return 1;
			else if (A.Z == B.Z)
			{
				if(A.globalIndex > B.globalIndex) return 0;
				else if( A.globalIndex < B.globalIndex ) return 1;
				else return 0;
				//~ else if (A.globalIndex == B.globalIndex)
				//~ {
					//~ if( A.rank < B.rank ) return 1;
					//~ else return 0;
				//~ }
			}
		} 		
	}
}

int smallerThan_DC3(mapperDuple A, mapperDuple B)
{
	if(A.index%3 > B.index%3) return 0;
	else if( A.index%3 < B.index%3 ) return 1;
	else if (A.index%3 == B.index%3) 
	{
		if(A.index/3 >= B.index/3) return 0;
		else return 1;
		//~ else if( A.index/3 < B.index/3) return 1;	
	}
}

int isEqual_DC3(mapperDuple *A, mapperDuple *B)
{
	if((*A).index==(*B).index && (*A).rank==(*B).rank) return 1;
	return 0;
}

int largerThan_DC3(mapperDuple A, mapperDuple B)
{
	if(A.index%3 < B.index%3) return 0;
	else if( A.index%3 > B.index%3 ) return 1;
	else if (A.index%3 == B.index%3) 
	{
		if(A.index/3 < B.index/3) return 0;
		else if( A.index/3 > B.index/3) return 1;	
	}
}

void swap_mapperDuple(mapperDuple* A, mapperDuple* B)
{
	mapperDuple temp;
	temp.index= (*A).index; temp.rank= (*A).rank;   
	(*A).index= (*B).index; (*A).rank= (*B).rank;
	(*B).index= temp.index; (*B).rank= temp.rank;
}

void copyAtoB_mapperDuple(mapperDuple* A, mapperDuple* B)
{
	(*B).index= (*A).index;
	(*B).rank= (*A).rank;
}

void copyArrayAtoB_mapperDuple(mapperDuple* A, mapperDuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_mapperDuple(&A[i], &B[i]);
}

int isEqual_mapperDuple(mapperDuple *A, mapperDuple *B)
{
	if((*A).index==(*B).index && (*A).rank==(*B).rank) return 1;
	return 0;
}

int largerThan_mapperDuple(mapperDuple A, mapperDuple B)
{
	if(A.index > B.index) return 1;
	else if( A.index < B.index ) return 0;
	else if (A.index == B.index) 
	{
		if( A.rank > B.rank ) return 1;
		else return 0;
	}
}

int smallerThan_index_mapperDuple(mapperDuple A, mapperDuple B)
{
	if(A.index >= B.index) return 0;
	else if( A.index < B.index ) return 1;
}

void assert_malloc(void *ptr, int rank) {
    if (ptr==NULL) {
        fprintf(stderr,"ERROR: PE%2d cannot malloc\n",rank);
        fflush(stderr);
        exit(1);
    }
}


/// Merge generated files into one file
void collect_files(int size)
{
	char ch;
	FILE *file_ptr;
	FILE *aggr=fopen("Results/Total.txt","w");
	for (int i=0;i<size;i++)
	{
		stringstream ss;
		ss << i;
		string file_to_open = ss.str();
		file_to_open = "Results/"+file_to_open+".txt";	
		if((file_ptr = fopen(file_to_open.c_str(), "r")) != NULL)
		{
			while((ch = fgetc(file_ptr)) != EOF)
			{
				fprintf(aggr,"%c",ch);
			}
			fclose(file_ptr);
		}
		else
		{
			printf("Could not open %s\n", file_to_open.c_str());
		}
	}
	fclose(aggr);
	
}



void swap_S(STuple* A, STuple* B)
{
   STuple temp;
   temp.type= (*A).type; temp.T1= (*A).T1; temp.T2= (*A).T2; temp.R1= (*A).R1; temp.R2= (*A).R2;  temp.index= (*A).index;
   (*A).type= (*B).type; (*A).T1= (*B).T1; (*A).T2= (*B).T2; (*A).R1= (*B).R1; (*A).R2= (*B).R2;  (*A).index= (*B).index;
   (*B).type= temp.type; (*B).T1= temp.T1; (*B).T2= temp.T2; (*B).R1= temp.R1; (*B).R2= temp.R2; (*B).index= temp.index;
}

void copyAtoB_S(STuple* A, STuple* B)
{
	(*B).type= (*A).type;
	(*B).T1= (*A).T1;
	(*B).T2= (*A).T2;
	(*B).R1= (*A).R1;
	(*B).R2= (*A).R2;
	(*B).index= (*A).index;	
}

void copyArrayAtoB_S(STuple* A, STuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_S(&A[i], &B[i]);
}

int isEqual_S(STuple *A, STuple *B)
{
	if((*A).type==(*B).type && (*A).T1==(*B).T1 && (*A).T2==(*B).T2 && (*A).R1==(*B).R1 && (*A).R2==(*B).R2 && (*A).index==(*B).index) return 1;
	return 0;
}


/// Bug: miss comparizon of all parts  (this is not largerthan or equal)
int largerThan_S(STuple A, STuple B)
{
	if(A.type==0 && B.type==0)
	{
		if (A.T1<B.T1) return 0;
		else if (A.T1>B.T1) return 1;
		else if (A.T1==B.T1)
		{
			if (A.R1<B.R1) return 0;
			else return 1;
		}
		
	}
	else if(A.type==0 && B.type==1)
	{
		if (A.T1<B.T1) return 0;
		else if (A.T1>B.T1) return 1;
		else if (A.T1==B.T1)
		{
			if (A.R1<B.R2) return 0;
			else return 1;
		}
		
	}
	else if(A.type==1 && B.type==0)
	{
		if (A.T1<B.T1) return 0;
		else if (A.T1>B.T1) return 1;
		else if (A.T1==B.T1)
		{
			if (A.R2<B.R1) return 0;
			else return 1;
		}
		
	}
	
	else if(A.type==0 && B.type==2 || A.type==2 && B.type==0)
	{
		if (A.T1<B.T1) return 0;
		else if (A.T1>B.T1) return 1;
		else if (A.T1==B.T1)
		{
			if (A.T2<B.T2) return 0;
			else if (A.T2>B.T2) return 1;
			else if (A.T2==B.T2)
			{
				if (A.R2<B.R2) return 0;
				else return 1;
			}
		}
		
	}
	else
	{
		if (A.R1>B.R1) return 1;
		else return 0;
		
	}
}

int smallerThan_S(STuple A, STuple B)
{		
	if(A.type==0 && B.type==0)
	{
		if (A.T1<B.T1) return 1;
		else if (A.T1>B.T1) return 0;
		else if (A.T1==B.T1)
		{
			if (A.R1<B.R1) return 1;
			else return 0;
		}
		
	}
	else if(A.type==0 && B.type==1)
	{
		if (A.T1<B.T1) return 1;
		else if (A.T1>B.T1) return 0;
		else if (A.T1==B.T1)
		{
			if (A.R1<B.R2) return 1;
			else return 0;
		}
		
	}
	else if(A.type==1 && B.type==0)
	{
		if (A.T1<B.T1) return 1;
		else if (A.T1>B.T1) return 0;
		else if (A.T1==B.T1)
		{
			if (A.R2<B.R1) return 1;
			else return 0;
		}
		
	}
	
	else if(A.type==0 && B.type==2 || A.type==2 && B.type==0)
	{
		if (A.T1<B.T1) return 1;
		else if (A.T1>B.T1) return 0;
		else if (A.T1==B.T1)
		{
			if (A.T2<B.T2) return 1;
			else if (A.T2>B.T2) return 0;
			else if (A.T2==B.T2)
			{
				if (A.R2<B.R2) return 1;
				else return 0;
			}
		}
		
	}
	else
	{
		if (A.R1<B.R1) return 1;
		else return 0;
		
	}	
}





