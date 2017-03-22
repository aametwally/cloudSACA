#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <getopt.h>
#include "mpi.h"
#include "mydefinitions.hpp"
#include "readFile.hpp"
#include "tuplesConst.hpp"
#include "psrs.hpp"
#include "helpers.hpp"
#include "NameingConstructMapper.hpp"
#include "utilities.hpp"

using namespace std;


	
int pDC3(int* text,int textLenLocal, int textLenGlobal,int rank, int size,bool verbose, STuple** S_Tuple_Sorted, int pDC3Level)
{
	/// Increment recursion level
	pDC3Level++; 
	
	int tuples12lenLocal, tuples12lenGlobal;
	long StartTime,TotalTime;
	int startIndex;
	TotalTime=clock();
	if (rank!=size-1)
	startIndex=textLenLocal*rank;
	else
	startIndex=(textLenGlobal/size)*rank;
	
	
	
	/// Calculate tuples12lenLocal
	if (textLenLocal%3==0)
	tuples12lenLocal=(textLenLocal*2)/3;
	else if (textLenLocal%3==1)
	{
		if (startIndex%3==0)
		tuples12lenLocal=(textLenLocal*2)/3;
		else
		tuples12lenLocal=(textLenLocal*2)/3 +1;	
	}
	else if (textLenLocal%3==2)
	{
		if (startIndex%3==1)
		tuples12lenLocal=(textLenLocal*2)/3 +1;
		else
		tuples12lenLocal=(textLenLocal*2)/3;
	}
	
	
	/// Calculate tuples12lenGlobal
	tuples12lenGlobal=(textLenGlobal*2)/3;
	

    /// Construct tuples12 for (i mode 3)=1 or 2 
    SAtuple* tuples12= (SAtuple *)malloc(tuples12lenLocal * sizeof(SAtuple));
	StartTime=clock();
    tuplesConst12(text, textLenLocal, tuples12,textLenGlobal,size,rank);
    
	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, StartTime,"Tuples12 construction",pDC3Level);
		printf("Memory After Tuple12 Construction, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}
	

	/// Sort Tuple12
	SAtuple * tuples12Sorted;
	StartTime=clock();
	int tuples12SortedLenLocal= all_sort_psrs_i(tuples12, tuples12lenLocal, &tuples12Sorted, size, rank, pDC3Level);
	MPI_Barrier(MPI_COMM_WORLD);

	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, StartTime,"PSRS Tuples12 Total",pDC3Level);
		printf("Memory After PSRS Tuple12, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}
	

	/// Gather the NodeTuplesCount for all nodes into NodeTuplesCount Array
	/// We use it for start naming Tuples Pi, could be removed if we changed our stategy. 
	int* NodeTuplesCount=(int*)malloc(size*sizeof(int));
	MPI_Allgather (&tuples12SortedLenLocal,1,MPI_INT,NodeTuplesCount,1,MPI_INT,MPI_COMM_WORLD) ;

	
	/// Prefix sum of NodeTuplesCount Array
	for(int i=0;i<size;i++)
	{
		if (i==0)
		NodeTuplesCount[i]=NodeTuplesCount[i];
		else
		NodeTuplesCount[i]=NodeTuplesCount[i]+NodeTuplesCount[i-1];
	}
	
	
		
	/// Check for Naming, Mapper construct (rank, index) then checking for duplications, if there are any duplications, go for recursion
	/// Return the size of the mappers per node 
	mapperDuple* mapper12Dummy= (mapperDuple*)malloc(tuples12SortedLenLocal* sizeof(mapperDuple));
	StartTime=clock();
	mapperDuple* MapperfromStuple;

	int NodeLen= NameingConstructMapper(tuples12Sorted, NodeTuplesCount, tuples12lenGlobal, mapper12Dummy,rank, size,verbose,pDC3Level,&MapperfromStuple);
	MPI_Barrier(MPI_COMM_WORLD);
	free(tuples12Sorted);
	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, StartTime,"recursion",pDC3Level);
		printf("Memory After Naming, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}

	
	/// Permute Mappers and sort them by index
	StartTime=clock();
	int *rankArray;
	int recvLen=all_sort_Mapper_i(MapperfromStuple, NodeLen, textLenLocal, textLenGlobal,rank, size,pDC3Level,&rankArray);	
	MPI_Barrier(MPI_COMM_WORLD);
	free(MapperfromStuple);
	
	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, StartTime,"Permute Mapper",pDC3Level);
		printf("Memory After Permute, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}

	

	/// Get the rank of the next 2 elements from the consecutive node to fill rank elemnts for S  
	int* gain=(int*)malloc(2*sizeof(int));
	int* reserve=(int*)malloc(2*sizeof(int));
	if (rank!=0)
	{
		for (int i=0;i<2;i++)
		reserve[i]=rankArray[i];
	}	
	/// Point to point communication
	int tag=0;
	int rc;
	MPI_Status Stat;

	MPI_Barrier(MPI_COMM_WORLD);
	if (rank!=0)
	{
		rc = MPI_Send(reserve, 2, MPI_INT, rank-1, tag, MPI_COMM_WORLD);
	}
	if (rank!=size-1)
	{
		rc = MPI_Recv(&rankArray[textLenLocal], 2, MPI_INT, rank+1, tag, MPI_COMM_WORLD, &Stat);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	

	/// Construct S0,S1,S2
	STuple* S=(STuple*)malloc(textLenLocal*sizeof(STuple));
	StartTime=clock();	
	S_Tuple_Construction(S,rankArray,text,textLenLocal,textLenGlobal,rank,size);
	
	free(rankArray);
	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, StartTime,"S construction",pDC3Level);
		printf("Memory After S Construction, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}

	/// Sort S0,S1,S2
	StartTime=clock();
	int SSortedLenLocal= all_sort_S_i(S, textLenLocal, S_Tuple_Sorted,size, rank,pDC3Level);
	MPI_Barrier(MPI_COMM_WORLD);
	
	if (verbose==1 &&rank==0)
	if (rank==0)
	{	
		elapsedTime(rank, StartTime,"PSRS S",pDC3Level);
		printf("Memory After PSRS S ToTal, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}


	/// Finish pDC3, Measure Total Time.	
	if (verbose==1 &&rank==0)
	{	
		elapsedTime(rank, TotalTime,"Total time=",pDC3Level);
		printf("Memory before collecting the output, pDC3Level=%d \n",pDC3Level);
		memory_usage(rank);
	}
	

	/// Return the size of SAper node 
	return SSortedLenLocal;
}
