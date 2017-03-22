#include "helpers.hpp"
using namespace std;


/// Measure the elapsed time 
void elapsedTime(int rank,long start,string message,int pDC3Level)
{
	long finish = clock();
	long total= finish-start;
    printf("Thread=%d, %s=%ld Sec, pDC3LEVEL=%d\n",rank, message.c_str(), (long) total / CLOCKS_PER_SEC,pDC3Level);
	
}


/// Swap struct A of tuple12 to struct B
void swap_SAtuple(SAtuple* A, SAtuple* B)
{
   SAtuple temp;
   temp.X= (*A).X; temp.Y= (*A).Y; temp.Z= (*A).Z; temp.globalIndex= (*A).globalIndex; //temp.rank= (*A).rank;   
   (*A).X= (*B).X; (*A).Y= (*B).Y; (*A).Z= (*B).Z; (*A).globalIndex= (*B).globalIndex; //(*A).rank= (*B).rank;
   (*B).X= temp.X; (*B).Y= temp.Y; (*B).Z= temp.Z; (*B).globalIndex= temp.globalIndex; //(*B).rank= temp.rank;
}


/// Copy struct A of tuple12 to struct B
void copyAtoB_SAtuple(SAtuple* A, SAtuple* B)
{
	(*B).X= (*A).X;
	(*B).Y= (*A).Y;
	(*B).Z= (*A).Z;
	(*B).globalIndex= (*A).globalIndex;
	//(*B).rank= (*A).rank;	
}


/// Copy array A of tuple12 struct to array B
void copyArrayAtoB_SAtuple(SAtuple* A, SAtuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_SAtuple(&A[i], &B[i]);
}


/// check if struct tuple12 A is equal struct tuple12 B, return 1 if equal 
int isEqual_SAtuple(SAtuple *A, SAtuple *B)
{
	if((*A).X==(*B).X && (*A).Y==(*B).Y && (*A).Z==(*B).Z) return 1;
	return 0;
}

/// check if struct tuple12 A is larger than struct tuple12 B, return 1 if A is larger 
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
		} 		
	}
}


/// check if struct tuple12 A is smaller than struct tuple12 B, return 1 if A is smaller 
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
			else if( A.Z < B.Z) return 1;
			else if (A.Z == B.Z)
			{
				if(A.globalIndex > B.globalIndex) return 0;
				else if( A.globalIndex < B.globalIndex ) return 1;
				else return 0;
			}
		} 		
	}
}


/// check if struct mapper A is smaller than struct mapper B, return 1 if A is smaller. "this function needed in recursion part"
int smallerThan_DC3(mapperDuple A, mapperDuple B)
{
	if(A.index%3 > B.index%3) return 0;
	else if( A.index%3 < B.index%3 ) return 1;
	else if (A.index%3 == B.index%3) 
	{
		if(A.index/3 >= B.index/3) return 0;
		else return 1;
	}
}


/// check if struct mapper A is equal to struct mapper B, return 1 if equal. "this function needed in recursion part"
int isEqual_DC3(mapperDuple *A, mapperDuple *B)
{
	if((*A).index==(*B).index && (*A).rank==(*B).rank) return 1;
	return 0;
}


/// check if struct mapper A is larger than struct mapper B, return 1 if A is larger. "this function needed in recursion part"
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


/// swap mapper A with mapper B
void swap_mapperDuple(mapperDuple* A, mapperDuple* B)
{
	mapperDuple temp;
	temp.index= (*A).index; temp.rank= (*A).rank;   
	(*A).index= (*B).index; (*A).rank= (*B).rank;
	(*B).index= temp.index; (*B).rank= temp.rank;
}


/// copy mapper A to mapper B
void copyAtoB_mapperDuple(mapperDuple* A, mapperDuple* B)
{
	(*B).index= (*A).index;
	(*B).rank= (*A).rank;
}


/// copy array of mappers A to Array of mappers B
void copyArrayAtoB_mapperDuple(mapperDuple* A, mapperDuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_mapperDuple(&A[i], &B[i]);
}


/// check if mapper A is equal to mapper B, return 1 if equal
int isEqual_mapperDuple(mapperDuple *A, mapperDuple *B)
{
	if((*A).index==(*B).index && (*A).rank==(*B).rank) return 1;
	return 0;
}


/// check if mapper A is larger than mapper B, return 1 is A is larger 
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


/// check if mapper A is smaller than mapper B, return 1 if A is smaler
int smallerThan_index_mapperDuple(mapperDuple A, mapperDuple B)
{
	if(A.index >= B.index) return 0;
	else if( A.index < B.index ) return 1;
}


/// check if the allocated memory has been done successfully 
void assert_malloc(void *ptr, int rank) {
    if (ptr==NULL) {
        fprintf(stderr,"ERROR: PE%2d cannot malloc\n",rank);
        fflush(stderr);
        exit(1);
    }
}


/// Merge partial SA files into one file
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


/// Swap S truct A wit Struct B
void swap_S(STuple* A, STuple* B)
{
   STuple temp;
   temp.type= (*A).type; temp.T1= (*A).T1; temp.T2= (*A).T2; temp.R1= (*A).R1; temp.R2= (*A).R2;  temp.index= (*A).index;
   (*A).type= (*B).type; (*A).T1= (*B).T1; (*A).T2= (*B).T2; (*A).R1= (*B).R1; (*A).R2= (*B).R2;  (*A).index= (*B).index;
   (*B).type= temp.type; (*B).T1= temp.T1; (*B).T2= temp.T2; (*B).R1= temp.R1; (*B).R2= temp.R2; (*B).index= temp.index;
}


/// copy S struct A to B
void copyAtoB_S(STuple* A, STuple* B)
{
	(*B).type= (*A).type;
	(*B).T1= (*A).T1;
	(*B).T2= (*A).T2;
	(*B).R1= (*A).R1;
	(*B).R2= (*A).R2;
	(*B).index= (*A).index;	
}


/// copy array fo S struct A to B
void copyArrayAtoB_S(STuple* A, STuple* B, int size)
{
	int i;
	for(i=0; i<size; i++) copyAtoB_S(&A[i], &B[i]);
}


/// check if S truct A is ual to B, return 1 if equal
int isEqual_S(STuple *A, STuple *B)
{
	if((*A).type==(*B).type && (*A).T1==(*B).T1 && (*A).T2==(*B).T2 && (*A).R1==(*B).R1 && (*A).R2==(*B).R2 && (*A).index==(*B).index) return 1;
	return 0;
}


/// Check f S truct A is larger than B, return 1 if A is larger
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


/// check if S struct A is smaller than B, return 1 if A is smaller
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


bool compTuple12(const SAtuple &A, const SAtuple &B)
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
			else if( A.Z < B.Z) return 1;
			else if (A.Z == B.Z)
			{
				if(A.globalIndex > B.globalIndex) return 0;
				else if( A.globalIndex < B.globalIndex ) return 1;
				else return 0;
			}
		} 		
	}
}

bool compMapper(mapperDuple A, mapperDuple B)
{
	if(A.index%3 > B.index%3) return 0;
	else if( A.index%3 < B.index%3 ) return 1;
	else if (A.index%3 == B.index%3) 
	{
		if(A.index/3 >= B.index/3) return 0;
		else return 1;
	}
}




bool compS(const STuple &A, const STuple &B)
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
