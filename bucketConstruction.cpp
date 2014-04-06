#include "bucketConstruction.hpp"

/// Bucket table construction 
int BucketConstruction(vector<char> AlphapetChar, vector<char> reference_array, int size, int numtasks, int rank, int w, bool verbose, int ** SA2)
{
	/// Calculate number of buckets
	int buckets=pow(AlphapetChar.size(),w);
	
	/// Sort Alphabet
	std::sort (AlphapetChar.begin(), AlphapetChar.end());
	
	/// Initializing the Array that contains number of allocated buckets for each processor
	int* ProcessorBuckets=(int *)malloc(numtasks*sizeof(int));
	for(int i=0;i<numtasks;i++)
	{
		ProcessorBuckets[i]=0;	
	}

	int StartIndex=0;
	int EndIndex=0;
	int TaskWidth=0;
	
	/// Construct StartIndex, EndIndex and TaskWidth Arrays
	if (rank!=numtasks-1)
	{
		StartIndex=(rank*(size/numtasks));
		EndIndex=((rank+1)*(size/numtasks));
		TaskWidth=EndIndex-StartIndex;
	}
	else
	{
		StartIndex=(rank*(size/numtasks));
		EndIndex=size;
		TaskWidth=EndIndex-StartIndex;
	}
		
	/// B  		>> array that contains number of occrrences of each bucket per process
	/// BTotal		>> array that contains number of occrrences of each bucket for all sequence
	/// BPrefixsum>> prefix sum of Btotal
	
	int *B=(int *)malloc(buckets*sizeof(int));
	int *BTotal=(int *)malloc(buckets*sizeof(int));
	int *BPrefixSum=(int *)malloc(buckets*sizeof(int));
	
	/// Initialization of B,BTotal and BPrefixsum
	for (int i=0;i<buckets;i++)
	{
		B[i]=0;
		BTotal[i]=0;
		BPrefixSum[i]=0;
	}
	
	/// Find the corresponding bucket for each prefix (using shift operation for power calculation)
	/// SA  >> Array that contains the corresponding bucket for each suffix
	int* SA=(int *)malloc(TaskWidth*sizeof(int));
	int Map=0;
	int MapStart=0;
	int SigmaPowerW=pow(AlphapetChar.size(),w-1);
	int Factor=0;
	int StartTime=clock();
	
	for(int i=StartIndex;i<EndIndex;i++)
	{
		Map=0;
		if (MapStart==0)
		{
			for (int j=0;j<w;j++)
			{
					
					Map=Map+(pow(AlphapetChar.size(),(w-1-j)))*(SeqSearch(reference_array[i+j],AlphapetChar));
			}
			MapStart++;
			Factor=(SeqSearch(reference_array[i],AlphapetChar));
		}
		else
		{
			Map = (AlphapetChar.size())*(SA[i-StartIndex-1] - SigmaPowerW*Factor) + (SeqSearch(reference_array[i+w-1],AlphapetChar));
			Factor=(SeqSearch(reference_array[i],AlphapetChar));
	
		}

		SA[i-StartIndex]=Map;
		B[Map]++;
	}
	if (verbose==true)
	elapsedTime(rank, StartTime,"Mapping");

	/// Collect number of occurrance of bucket in Btotal array
	MPI_Allreduce (B,BTotal,buckets,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
	
	/// Prefix Sum for the BTotal array
	for (int i=0;i<buckets;i++)
	{
		if (i==0)
		BPrefixSum[i]=BTotal[i];
		
		BPrefixSum[i]=BPrefixSum[i-1]+BTotal[i];
	}
	
	/// Calculate number of allocated buckets for each processor (Prefix Sum)
	for(int i=0;i<numtasks;i++)
	{
		for (int j=0;j<buckets;j++)
		{
			if (BPrefixSum[j]>=(((i+1)*(size/numtasks))-1))
			{
				if (abs(BPrefixSum[j]-((i+1)*(size/numtasks)))<=abs(BPrefixSum[j-1]-((i+1)*(size/numtasks))))
				ProcessorBuckets[i]=j;
				else
				ProcessorBuckets[i]=j-1;
				
				if (i==numtasks-1)
				ProcessorBuckets[i]=buckets-1;
				
				break;	
			}			
		}
	}

	/// T   >> Array that contains bucket number of each suffix sorted by the buckets (derived from SA array)
	int* T=(int *)malloc(TaskWidth*sizeof(int));
	int *SACount=(int *)malloc(buckets*sizeof(int));
	int *BIndivPrefix=(int *)malloc(buckets*sizeof(int));
	
	for (int i=0;i<TaskWidth;i++)
	{
			T[i]=0;	
	}
	for (int i=0;i<buckets;i++)
	SACount[i]=0;
	
	/// Prefix Sum for B array for each processor 
	for (int i=0;i<buckets;i++)
	{
		if (i==0)
		BIndivPrefix[i]=B[i];
		BIndivPrefix[i]=BIndivPrefix[i-1]+B[i];
	}
	
	
	/// Permute the suffixes according to bucket number to the appropiate position within each processor
	StartTime=clock();
	for (int i=StartIndex;i<EndIndex;i++)
	{
			if (SA[i-StartIndex]!=0)
			{
				T[BIndivPrefix[SA[i-StartIndex]-1]+SACount[SA[i-StartIndex]]]=i;
				SACount[SA[i-StartIndex]]++;
			}
			else
			{
				T[SACount[SA[i-StartIndex]]]=i;
				SACount[SA[i-StartIndex]]++;
			}	
	}
	if (verbose==true)
	elapsedTime(rank, StartTime,"Sort suffix in appropriate");
	free(SA);	

	int* ReceiveCount=(int*)malloc(numtasks*sizeof(int));
	int* Displacement=(int*)malloc(numtasks*sizeof(int));	
	
	/// SA2  >> Array that contains the indices of the suffixes after permutation between processors according to bucket number. 
	//~ int* SA2;
	int SA2Size=0;
	
	/// calculate needed size for SA2 Array
	if (rank==0)
	SA2Size=BPrefixSum[ProcessorBuckets[rank]];
	else
	SA2Size=(BPrefixSum[ProcessorBuckets[rank]]-BPrefixSum[ProcessorBuckets[rank-1]]);
	
	if (verbose==true)
	
	*SA2=(int*) malloc(sizeof(int)*SA2Size);
	
	/// Initialization of SA2 Array
	for (int i=0;i<SA2Size;i++)
	(*SA2)[i]=-1;
	
	int j=0;
	int start=0;
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	/// Suffixes permutation between processors
	StartTime=clock(); 
	for (int i=0;i<numtasks;i++)
	{
		for (j=j;j<buckets;j++)
		{
			MPI_Gather (&B[j],1,MPI_INT,ReceiveCount,1,MPI_INT,i,MPI_COMM_WORLD);			
			
			Displacement[0]=0;
			for (int m=1;m<numtasks;m++)
			{
				Displacement[m]=Displacement[m-1]+ReceiveCount[m-1];
			}
			
			if (i==0)
			{
				if (start==0)
				MPI_Gatherv(&T[0],B[j],MPI_INT,(*SA2), ReceiveCount, Displacement,MPI_INT,i,MPI_COMM_WORLD);
				else
				MPI_Gatherv(&T[BIndivPrefix[j-1]],B[j],MPI_INT,&((*SA2)[BPrefixSum[j-1]]), ReceiveCount, Displacement,MPI_INT,i,MPI_COMM_WORLD);
			}
			else
			{	
				if (start==0)
				MPI_Gatherv(&T[BIndivPrefix[j-1]],B[j],MPI_INT,(*SA2), ReceiveCount, Displacement,MPI_INT,i,MPI_COMM_WORLD);
				else
				MPI_Gatherv(&T[BIndivPrefix[j-1]],B[j],MPI_INT,&((*SA2)[BPrefixSum[j-1]-BPrefixSum[ProcessorBuckets[i-1]]]), ReceiveCount, Displacement,MPI_INT,i,MPI_COMM_WORLD);
			}
			
			start++;
			if(j==ProcessorBuckets[i])
			break;	
		}
		j++;
		start=0;
	}
	if (verbose==true)
	elapsedTime(rank, StartTime,"1st Permutation Phase");
	free(B);
	free(BTotal);
	free(BPrefixSum);
	free(SACount);
	free(BIndivPrefix);
	free(ReceiveCount);
	free(Displacement);
	free(T);
	
	return SA2Size;
}


	
