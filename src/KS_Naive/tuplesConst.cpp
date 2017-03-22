#include <stdio.h>
#include "mydefinitions.hpp"
#include "mpi.h"


/// Construct tuples12 for (i mode 3)=1 or 2
void tuplesConst12(int* text, int textLen, SAtuple *tuples12,int textLenGlobal,int size, int rank)
{	
	int i=0, j=0;

	int global;
	for(i=0; i < textLen; i++)
	{
		if (rank!=size-1)
		global=(textLen*rank) + i;
		else
		global=(textLenGlobal/size)*(rank)+i;
		
		if(global % 3 !=0)
		{
			tuples12[j].X= text[i];
			tuples12[j].Y= text[i+1];
			tuples12[j].Z= text[i+2];
			tuples12[j].globalIndex= global;
			j++;
		}
	}
}

/// Construct S0
void Const_S0(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex)
{
	S[start].type=0;
	S[start].T1=text[index];
	S[start].T2=text[index+1];
	S[start].R1=rankArray[index+1];
	S[start].R2=rankArray[index+2];
	S[start].index=GlobalIndex;	
}

/// Construct S1
void Const_S1(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex)
{	
	S[start].type=1;
	S[start].T1=text[index];
	S[start].T2=' ';
	S[start].R1=rankArray[index];
	S[start].R2=rankArray[index+1];
	S[start].index=GlobalIndex;	
}

/// Construct S2
void Const_S2(STuple* S,int* rankArray,int* text,int start,int index,int GlobalIndex)
{	
	S[start].type=2;
	S[start].T1=text[index];
	S[start].T2=text[index+1];
	S[start].R1=rankArray[index];
	S[start].R2=rankArray[index+2];
	S[start].index=GlobalIndex;	
}

/// Construct S0, S1, S2
void S_Tuple_Construction(STuple* S,int* rankArray,int* text,int textLenLocal,int textLenGlobal,int rank, int size)
{	
	int GlobalIndex=0;
	int start=0;
	for(int i=0;i<textLenLocal;i++)
	{
		GlobalIndex=(rank*(textLenGlobal/size))+i;
		
		if (GlobalIndex%3==0)
		{
			Const_S0(S,rankArray,text,start,i,GlobalIndex);
			start++;
		}
		else if(GlobalIndex%3==1)
		{
			Const_S1(S,rankArray,text,start,i,GlobalIndex);
			start++;
		}
		else
		{
			Const_S2(S,rankArray,text,start,i,GlobalIndex);;
			start++;
		}
	}
}
