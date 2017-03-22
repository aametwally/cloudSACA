#include "duplicateChecker.hpp"

	
/// Mapper construct (rank, index) and Naming, then checking for duplications, if there are any duplications, go for recursion
/// Return the size of the mappers per node 
int duplicateChecker(SAtuple* arr, int* NodesBucketSize, int globalLen, mapperDuple* mapper12Dummy,int rank,int size,bool verbose,int pDC3Level, mapperDuple** MapperfromStuple)
{	
	MPI_Barrier(MPI_COMM_WORLD);
	int StartTime=clock();
	int localLen, localSum, start;
	if (rank!=0)
	localLen=NodesBucketSize[rank]-NodesBucketSize[rank-1];
	else
	localLen=NodesBucketSize[rank];

	if (rank==0)
	start=1;
	else
	start=NodesBucketSize[rank-1]+1;

	localSum=start;
	int SendCount=0;
	int ReceiveCount=0;
	
	
	/// Normal Naming
	for(int i=0; i<localLen; i++)
	{
		if(isEqual_SAtuple(&arr[i], &arr[i+1])) 
		{
			mapper12Dummy[i].index=arr[i].globalIndex;
			mapper12Dummy[i].rank=localSum;		
		}
		else 
		{	
			mapper12Dummy[i].index=arr[i].globalIndex;
			mapper12Dummy[i].rank=localSum;
			localSum+=1;
			SendCount++;
		}
	}

	
	/// Send naming info to all nodes to check if there are any duplicates
	MPI_Allreduce (&SendCount,&ReceiveCount,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);


	/// If no duplicates found
	if(ReceiveCount==NodesBucketSize[size-1])
	{
		*MapperfromStuple=mapper12Dummy;
		return localLen; 
	}
	/// If duplications found
	else 
	{
		/// Sort mappers according to (r,i) -> (i mod 3, i div 3)
		mapperDuple* SortedMapper;
		int recvLen= all_sort_Mapper_recursion(mapper12Dummy, &SortedMapper, localLen,rank, size,pDC3Level, verbose);
		MPI_Barrier(MPI_COMM_WORLD);
		if (verbose==1 && rank==0)
		elapsedTime(rank, StartTime,"Naming until sort_Mapper recursion",pDC3Level);
		MPI_Barrier(MPI_COMM_WORLD);


		/// gather all mappers to Oragnize the number of mapper per node to be (totalLen/Size)		
		/// Define MPI Struct
		MPI_Datatype MAPPER_DUPLE;
		MPI_Datatype type[2] = { MPI_INT, MPI_INT };
		int blocklen[2] = { 1, 1 };
		MPI_Aint disp[2];
		MPI_Get_address(&SortedMapper[0].index, &disp[0]);
		MPI_Get_address(&SortedMapper[0].rank, &disp[1]);
		for(int i=1; i>=0; i--) disp[i]-=disp[0];	
		MPI_Type_create_struct(2, blocklen, disp, type, &MAPPER_DUPLE);
		MPI_Type_commit(&MAPPER_DUPLE);
		
		mapperDuple* TotalMappers=(mapperDuple*)malloc((NodesBucketSize[size-1]+2)*sizeof(mapperDuple));
		int* recv_cnt= (int*)malloc((size)* sizeof(int)); 
		
		MPI_Allgather (&recvLen,1,MPI_INT,recv_cnt,1,MPI_INT,MPI_COMM_WORLD);
		
		int* recv_off= (int*)malloc((size)* sizeof(int));
		recv_off[0]=0;
		for (int i=1 ; i<size ; i++) 
		{
			recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
		}
		
		MPI_Allgatherv(SortedMapper, recvLen, MAPPER_DUPLE, TotalMappers, recv_cnt, recv_off, MAPPER_DUPLE, MPI_COMM_WORLD);
		
		TotalMappers[NodesBucketSize[size-1]].rank=0;
		TotalMappers[NodesBucketSize[size-1]+1].rank=0;
		
		int MapperGlobalLen=NodesBucketSize[size-1];
		int MapperLocalLen;
	
		if(rank!=size-1)
		MapperLocalLen=MapperGlobalLen/size;
		else
		MapperLocalLen=MapperGlobalLen-((MapperGlobalLen/size)*rank);
		
		free(SortedMapper);
		SortedMapper=&TotalMappers[(NodesBucketSize[size-1]/size)*rank];
		int* RecursiveArray=(int*)malloc((MapperLocalLen+2)*sizeof(int));		
		

		for (int i=0;i<MapperLocalLen+2;i++)
		{
			RecursiveArray[i]=SortedMapper[i].rank;
		}


		MPI_Barrier(MPI_COMM_WORLD);
		if (verbose==1 && rank==0)
		elapsedTime(rank, StartTime,"Naming until pDC3 recursion",pDC3Level);
		
		
		/// Recursive pDC3
		STuple* S_Tuple_Sorted;
		int SSortedLenLocal=pDC3(RecursiveArray, MapperLocalLen, MapperGlobalLen, rank, size, verbose, &S_Tuple_Sorted, pDC3Level);


		/// gather the SSortedLenLocal for all nodes in SSortedLenLocalArray
		/// We use this info to start ranking Mappers. 
		int* SSortedLenLocalArray=(int*)malloc(size*sizeof(int));
		MPI_Allgather (&SSortedLenLocal,1,MPI_INT,SSortedLenLocalArray,1,MPI_INT,MPI_COMM_WORLD) ;
		
		
		/// Prefix Sum of NodesBucketSize
		for(int i=0;i<size;i++)
		{
			if (i==0)
			SSortedLenLocalArray[i]=SSortedLenLocalArray[i];
			else
			SSortedLenLocalArray[i]=SSortedLenLocalArray[i]+SSortedLenLocalArray[i-1];
		}
		
		*MapperfromStuple=(mapperDuple*)malloc(SSortedLenLocal*sizeof(mapperDuple));

		for(int i=0;i<SSortedLenLocal;i++)
		{
			/// check that boundry, last element is out of boundry in the case of the recursion
			if(rank==0)
			(*MapperfromStuple)[i].rank=i+1;
			else
			(*MapperfromStuple)[i].rank=SSortedLenLocalArray[rank-1]+i+1;
			
			/// problem for the mapper12Dummy
			(*MapperfromStuple)[i].index=TotalMappers[S_Tuple_Sorted[i].index].index;
		}
		free(TotalMappers);
		free (S_Tuple_Sorted);
		
		return SSortedLenLocal;
	}
	
}
