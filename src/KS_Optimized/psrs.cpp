#include "psrs.hpp"
#include <algorithm>
#define OVERSMP 1



/// find position of Tuple12 key (or next larger) in sorted array
int getPosition(SAtuple *arr, int size, SAtuple *key, int rank)
{
	int i=0;
	for (i=0;i<size;i++)
	{
		if (isEqual_SAtuple(&arr[i], key))
		{
			return i;
		}
		else if (!largerThan_SAtuple(*key, arr[i]))
		return i;
	}
	return size;
}


///  Binary Search: find position of Tuple12 key (or next larger) in sorted array
int binary_search(SAtuple *arr, int size, SAtuple *key, int rank)
{
	/// continue searching while [imin,imax] is not empty
	int m;
	int imin=0,imax=size-1;
	while (imax >= imin)
	{
		/// calculate the midpoint for roughly equal partition
		m = (imin+ imax)/2;
		if(isEqual_SAtuple(&arr[m], key))
		{
			/// key found at index m
			return m;
		} 
		/// determine which subarray to search
		else if (largerThan_SAtuple(*key, arr[m]))
		/// change min index to search upper subarray
		imin = m + 1;
		else         
		/// change max index to search lower subarray
		imax = m - 1;
	}

	while (m <= (size-1) && largerThan_SAtuple(*key, arr[m])) ++m;
	/// key not found
	return m;
}




/// Sort Tuple12 using PSRS Method
int all_sort_psrs_i(SAtuple *A, int A_size, SAtuple **B,int size, int rank, int pDC3Level, int verbose) 
{
	int i, B_size;
	int *recv_cnt,     /// number of keys to receive from each PN 
      *send_cnt,     /// number of keys to send to PNs 
      *recv_off,
      *send_off;

	int no_samples,    /// number of samples to take from sorted data 
      pivdist,       /// distance between pivots in set of samples 
      possmp,        /// position of pivot in set of all samples 
      smpdist,       /// distance between consecutive samples 
      trecv;
      
	SAtuple  *pivbuffer,    /// array of pivots 
	         *recv_buf,     /// incoming data to merge
     	     *smpbuffer,    /// array of local samples (array of pointers)
      	     *smpallbuf,    /// array of global samples 
	         **prtbound;    /// boundaries for partitioning local data 

	MPI_Status stat;

	prtbound = (SAtuple **)malloc((size+1)*sizeof(SAtuple *));
	assert_malloc(prtbound, rank);

	send_cnt = (int *)malloc(size*sizeof(int));
	assert_malloc(send_cnt, rank);
  
	send_off = (int *)malloc(size*sizeof(int));
	assert_malloc(send_off, rank);
  
	recv_off = (int *)malloc(size*sizeof(int));
	assert_malloc(recv_off, rank);
	
	recv_cnt = (int *)malloc(size*sizeof(int));
	assert_malloc(recv_cnt, rank);
	
	pivbuffer = (SAtuple *)malloc((size-1)*sizeof(SAtuple));
	assert_malloc(pivbuffer, rank);

	/// Define MPI Struct
	MPI_Datatype tupleType;
    MPI_Datatype type[5] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    int blocklen[5] = { 1, 1, 1, 1 };
    MPI_Aint disp[5];

    MPI_Get_address(&A[0].X, &disp[0]);
    MPI_Get_address(&A[0].Y, &disp[1]);
    MPI_Get_address(&A[0].Z, &disp[2]);
    MPI_Get_address(&A[0].globalIndex, &disp[3]);


	for(i=4; i>=0; i--) disp[i]-=disp[0];	

    MPI_Type_create_struct(4, blocklen, disp, type, &tupleType);
    MPI_Type_commit(&tupleType);
	
	/// Local sorting of Tuple12
	MPI_Barrier(MPI_COMM_WORLD);
	int StartTime=clock();
	std::sort(&A[0], &A[A_size], compTuple12);
	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS Tuple12 First Fasta Sort ",pDC3Level);

	/// Filling out the samples buffer 
	no_samples = OVERSMP*size - 1; /// OVERSMP=1, then no of samples= no of processors -1  
	smpdist = A_size / (no_samples + 1);    ///  ////////////Take care, try to change this criteria
	
	smpbuffer = (SAtuple *)malloc(no_samples*sizeof(SAtuple));
	assert_malloc(smpbuffer, rank);
	
	for (i=0; i<no_samples; i++) copyAtoB_SAtuple(&A[(i+1)*smpdist], &smpbuffer[i]);
	
	/// Gather all samples into the Master node to sort then broadcast the selected pivot
	smpallbuf = (SAtuple *)malloc(size*no_samples*sizeof(SAtuple));
	assert_malloc(smpallbuf, rank);

	MPI_Gather(smpbuffer, no_samples, tupleType, smpallbuf, no_samples, tupleType, 0, MPI_COMM_WORLD);

	
	/// Forming final pivots 
	if (rank==0) 
	{
		fastsort_SAtuple(smpallbuf, size*no_samples);
		pivdist = no_samples;
		for (i=0; i<size-1; i++) 
		{
			copyAtoB_SAtuple(&smpallbuf[(i+1)*pivdist], &pivbuffer[i]);
		}
	}
  

	/// STEP 5: Broadcast pivots 
	MPI_Bcast(pivbuffer, size-1, tupleType, 0, MPI_COMM_WORLD);

	/// Filling prtbound, containing indexes of boundaries for paritioning local data. Each node should contain elements up to the pivot
	prtbound[0] = &A[0];
	
	int start = clock();
	for (i=1; i<size; i++)
	{	
		int needed_index=getPosition(A, A_size, &pivbuffer[i-1],rank);
		prtbound[i] = &A[needed_index];
	} 
	prtbound[size] = &A[A_size-1];
	int finish=clock();
	int total= finish-start;

	free (smpbuffer);
	free (smpallbuf);
	free (pivbuffer);

	/// Filling Send_count array
	for (i=0; i<size; i++)
	{
		if(i!=size-1) 
		send_cnt[i] = maxx(0, prtbound[i+1] - prtbound[i]); 
		else
		send_cnt[i] = maxx(0, prtbound[i+1] - prtbound[i]+1); 
	}
 	
	/// All to All Communcation, Send the send and receive info to all nodes
	MPI_Alltoall(send_cnt, 1, MPI_INT, recv_cnt, 1, MPI_INT, MPI_COMM_WORLD);
	
	/// Calculating the Offset Arrays for both the send and receive buffers
	send_off[0] = 0;
	recv_off[0] = 0;
	for (i=1 ; i<size ; i++) 
	{
		send_off[i] = send_off[i-1] + send_cnt[i-1];
		recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
	}
   	
	/// Allocate space for receive buffer
	trecv=0;

	for(i=0; i< size; i++) trecv=trecv + recv_cnt[i];

	if (trecv>0) 
	{
		recv_buf = (SAtuple *)malloc(trecv*sizeof(SAtuple));
		assert_malloc(recv_buf, rank);
	}
	else 
	{
		recv_buf = NULL;
	}
  	
  	/// All to All Communcation, to permute the Tuples between the nodes to be sorted
	MPI_Alltoallv(A, send_cnt, send_off, tupleType, recv_buf, recv_cnt, recv_off, tupleType, MPI_COMM_WORLD);
	
	free(A);
	MPI_Barrier(MPI_COMM_WORLD);
	StartTime=clock();

	SAtuple* result;
	SAtuple* listOfIntegers[size];
	for (int i=0;i<size;i++)
	{
		listOfIntegers[i]=&(recv_buf[recv_off[i]]);
	}
	
	/// Multimerge of sorted buckets of Tuple12 struct
	mergeAll_SA(listOfIntegers, &result, trecv, recv_cnt, size);

	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS Tuple12 Last Fasta Sort ",pDC3Level);
	
	/// Assign the pointer B to rec_buf 
	B_size = trecv;
	*B=result;
	
	free(recv_off);
	free(send_off);
	free(send_cnt);
	free(recv_cnt);
	free(prtbound);
	
	return (B_size);
}




/// MultiMerge of sorted Buckets to merge the Sorted array of tuple12 struct after permutation between the processes 
void mergeAll_SA(SAtuple** listOfIntegers,  SAtuple** result,int TotalSize,int* ListLen,int size)
{
	*result=(SAtuple*)malloc(TotalSize*sizeof(SAtuple));
    int totalNumbers = size;
    int curpos[size];
    int currow = 0, foundMinAt = 0;
    SAtuple minElement;

    for ( int i = 0; i < size; ++i)
    {
		curpos[i]=0;
    }
	
	int count=0;
    for ( ; ; )
    {
        for ( currow = 0 ; currow < size ; ++currow)
        {
            if ( curpos[currow] < ListLen[currow] )
            {
                minElement = listOfIntegers[currow][curpos[currow]];
                foundMinAt = currow;
                break;
            }
        }
        /// If all the elements were traversed in all the arrays, then no further work needs to be done
        if ( !(currow < size ) )
            break;
        
        ///    Traverse each of the array and find out the first available minimum value
        for (currow=0 ;currow < size; ++currow)
        {
            if ( smallerThan_SAtuple(listOfIntegers[currow][curpos[currow] ],minElement) && curpos[currow]<ListLen[currow])
            {
                minElement = listOfIntegers[currow][curpos[currow] ];
                foundMinAt = currow;
            }
        }
         
        ///    Store the minimum into the resultant array 
        ///    and increment the element traversed
        (*result)[count]=minElement;
        count++;
        ++curpos[foundMinAt];
    }
}



/// Binary Search: find position of mapper key (or next larger) in sorted array
int binary_search_mapperDuple(mapperDuple *arr, int size, int key, int rank)
{
	/// continue searching while [imin,imax] is not empty
	int m;
	int imin=0,imax=size-1;
	while (imax >= imin)
	{
	  /// calculate the midpoint for roughly equal partition
	  m = (imin+ imax)/2;
	  if(arr[m].index == key)
		/// key found at index m
		return m; 
	  /// determine which subarray to search
	  else if (key > arr[m].index)
		/// change min index to search upper subarray
		imin = m + 1;
	  else         
		/// change max index to search lower subarray
		imax = m - 1;
	}

	while (m <= (size-1) && key > arr[m].index) ++m;
	/// key not found
	return m;
}



/// Sort Mappers using PSRS Method based on their index
int all_sort_Mapper_i(mapperDuple* mapper, int mapperSize, int localTextLen, int globalTextLen,int rank, int size,int pDC3Level,int** rankArray)
{
	double StartTime=clock();
	int i, recvLen;
    int *pivots,     ///array of values of indices of the pivots
        *partitions, ///array of positions of the pivots in the mapper duple array
        *recv_cnt,   /// number of keys to receive from each PN 
        *send_cnt,   /// number of keys to send to PNs 
        *recv_off,
        *send_off;
        
        
    mapperDuple *recv_buf;

	pivots= (int*)malloc((size-1)* sizeof(int));
	assert_malloc(pivots, rank);

	partitions= (int*)malloc((size+1)* sizeof(int));
	assert_malloc(partitions, rank);

    recv_cnt= (int*)malloc((size)* sizeof(int)); 
	assert_malloc(recv_cnt, rank);

    send_cnt= (int*)malloc((size)* sizeof(int)); 
	assert_malloc(send_cnt, rank);

    recv_off= (int*)malloc((size)* sizeof(int));
	assert_malloc(recv_off, rank);

    send_off= (int*)malloc((size)* sizeof(int));
	assert_malloc(send_off, rank);
	
	
	/// Define MPI Struct
	MPI_Datatype MAPPER_DUPLE;
    MPI_Datatype type[2] = { MPI_INT, MPI_INT };
    int blocklen[2] = { 1, 1 };
    MPI_Aint disp[2];

    MPI_Get_address(&mapper[0].index, &disp[0]);
    MPI_Get_address(&mapper[0].rank, &disp[1]);

	for(i=1; i>=0; i--) disp[i]-=disp[0];	

    MPI_Type_create_struct(2, blocklen, disp, type, &MAPPER_DUPLE);
    MPI_Type_commit(&MAPPER_DUPLE);

	/// Local Sort of mappers by based on their index
	fastsort_mapperDuple(mapper, mapperSize);
	
    /// Fill pivots array
    int divion=globalTextLen/size;
   
	for (i=0; i<size-1; i++) 
	{
		pivots[i]= (divion * (i+1));
	}

	/// Fill partitions array
    partitions[0]= 0;  
	
    for (i=1; i<size; i++) partitions[i]= binary_search_mapperDuple(mapper, mapperSize, pivots[i-1],rank);
    partitions[size]= mapperSize-1;
    
    /// Filling Send count array
    for (i=0; i<size; i++)
    {
		if(i!=size-1) 
		send_cnt[i] = max(0, partitions[i+1] - partitions[i]); /// get size of elements to be sent between boundaries
		else 
		send_cnt[i] = max(0, partitions[i+1] - partitions[i]+1); //Other than node 0, include the pivot at the beginning
    }

    MPI_Alltoall(send_cnt, 1, MPI_INT,recv_cnt, 1, MPI_INT, MPI_COMM_WORLD);

	
	/// Calculating offsets for both the send and receive buffers
	send_off[0] = 0;
	recv_off[0] = 0;
	for (i=1 ; i<size ; i++) 
	{
		send_off[i] = send_off[i-1] + send_cnt[i-1];
		recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
	}

	/// Allocate space for receive buffer
	recvLen=0;

	for(i=0; i< size; i++) recvLen=recvLen + recv_cnt[i];

	if (recvLen>0) {
		recv_buf = (mapperDuple* )malloc(recvLen*sizeof(mapperDuple));
		assert_malloc(recv_buf, rank);
	}
	else recv_buf = NULL;
	MPI_Alltoallv(mapper, send_cnt, send_off, MAPPER_DUPLE, recv_buf, recv_cnt, recv_off, MAPPER_DUPLE, MPI_COMM_WORLD);
				
	
	*rankArray=(int*)malloc((localTextLen+2)*sizeof(int));
	for (i=0;i<localTextLen+2;i++)
	{
		(*rankArray)[i]=-1;
	}
		
	int relativeIndex=0;

	for (i=0;i<recvLen;i++)
	{
		if(recv_buf[i].index>=(globalTextLen/size))
		relativeIndex=recv_buf[i].index-(rank*(globalTextLen/size));
		else
		relativeIndex=recv_buf[i].index;

		(*rankArray)[relativeIndex]=recv_buf[i].rank;
	}

	free(pivots);
	free(partitions);
	free(recv_buf);
	free(recv_off);
	free(send_off);
	free(send_cnt);
	free(recv_cnt);
	
	return recvLen; 
}



/// Binary Search: find position of Mapper key (or next larger)
int binary_search_mapperDuple_recursion(mapperDuple *arr, int size, mapperDuple *key, int rank)
{
	/// continue searching while [imin,imax] is not empty
	int m;
	int imin=0,imax=size-1;
	while (imax >= imin)
	{
	  /// calculate the midpoint for roughly equal partition
	  m = (imin+ imax)/2;
	  if(isEqual_DC3(&arr[m],key))
		/// key found at index m
		return m; 
	  /// determine which subarray to search
	  else if (largerThan_DC3(*key, arr[m]))
		/// change min index to search upper subarray
		imin = m + 1;
	  else         
		/// change max index to search lower subarray
		imax = m - 1;
	}

	while (m <= (size-1) && largerThan_DC3(*key, arr[m])) ++m;
	/// key not found
	return m;
}





/// Sort Mappers (r,i) at the recursion using PSRS Method based on the (i mod 3, i div 3)
int  all_sort_Mapper_recursion(mapperDuple* mapper, mapperDuple **SortedMapper, int mapperSize, int rank, int size,int pDC3Level, int verbose)
{
	int i, recvLen;
    int *pivots,     ///array of values of indices of the pivots
        *partitions, ///array of positions of the pivots in the mapper duple array
        *recv_cnt,   /// number of keys to receive from each PN 
        *send_cnt,   /// number of keys to send to PNs 
        *recv_off,
        *send_off;
        
   
   	int no_samples,    /// number of samples to take from sorted data 
		pivdist,       /// distance between pivots in set of samples 
		possmp,        /// position of pivot in set of all samples 
		smpdist,       /// distance between consecutive samples 
		trecv;
        
    
    mapperDuple *pivbuffer,    /// array of pivots 
				*recv_buf,     /// incoming data to merge
				*smpbuffer,    /// array of local samples (array of pointers)
				*smpallbuf,    /// array of global samples 
				**prtbound;    /// boundaries for partitioning local data 

	pivots= (int*)malloc((size-1)* sizeof(int));
	assert_malloc(pivots, rank);

	partitions= (int*)malloc((size+1)* sizeof(int));
	assert_malloc(partitions, rank);

    recv_cnt= (int*)malloc((size)* sizeof(int)); 
	assert_malloc(recv_cnt, rank);

    send_cnt= (int*)malloc((size)* sizeof(int)); 
	assert_malloc(send_cnt, rank);

    recv_off= (int*)malloc((size)* sizeof(int));
	assert_malloc(recv_off, rank);

    send_off= (int*)malloc((size)* sizeof(int));
	assert_malloc(send_off, rank);
	
	pivbuffer = (mapperDuple *)malloc((size-1)*sizeof(mapperDuple));
	assert_malloc(pivbuffer, rank);
	    
	   
	/// Define MPI Struct
	MPI_Datatype MAPPER_DUPLE;
    MPI_Datatype type[2] = { MPI_INT, MPI_INT };
    int blocklen[2] = { 1, 1 };
    MPI_Aint disp[2];

    MPI_Get_address(&mapper[0].index, &disp[0]);
    MPI_Get_address(&mapper[0].rank, &disp[1]);

	for(i=1; i>=0; i--) disp[i]-=disp[0];	

    MPI_Type_create_struct(2, blocklen, disp, type, &MAPPER_DUPLE);
    MPI_Type_commit(&MAPPER_DUPLE);



	/// Local Sort of the mappers based on (i mod 3, i div 3)
	MPI_Barrier(MPI_COMM_WORLD);
	int StartTime=clock();
	std::sort(&mapper[0], &mapper[mapperSize], compMapper);
	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS Mapper_Recursion First Fasta Sort ",pDC3Level);	
	
	/// Fill pivots array
	no_samples = size - 1; /// OVERSMP=1, then no of samples= no of processors -1  
	smpdist = mapperSize / (no_samples + 1);    ///  ////////////Take care, try to change this criteria
	
	smpbuffer = (mapperDuple *)malloc(no_samples*sizeof(mapperDuple));
	assert_malloc(smpbuffer, rank);
	
	for (i=0; i<no_samples; i++) copyAtoB_mapperDuple(&mapper[(i+1)*smpdist], &smpbuffer[i]);
	
	/// Gather samples from other nodes 
	smpallbuf = (mapperDuple *)malloc(size*no_samples*sizeof(mapperDuple));
	assert_malloc(smpallbuf, rank);
	
	/// MPI_Gather: Note that the recvcount argument at the root indicates the number of items
	/// it receives from each process, not the total number of items it receives.
	MPI_Gather(smpbuffer, no_samples, MAPPER_DUPLE,smpallbuf, no_samples, MAPPER_DUPLE,0, MPI_COMM_WORLD);

	if (rank==0) 
	{
		/// STEP 4: Forming final pivots 
		fastsort_DC3(smpallbuf,size*no_samples);
		pivdist = no_samples;
		for (i=0; i<size-1; i++) 
		{
			copyAtoB_mapperDuple(&smpallbuf[(i+1)*pivdist], &pivbuffer[i]);
		}
	}

	
	/// Broadcast pivots 
	MPI_Bcast(pivbuffer, size-1, MAPPER_DUPLE, 0, MPI_COMM_WORLD);
	
	/// Fill partitions array
    partitions[0]= 0;  

    for (i=1; i<size; i++) partitions[i]= binary_search_mapperDuple_recursion(mapper, mapperSize, &pivbuffer[i-1],rank);
    partitions[size]= mapperSize-1;
    
    /// Filling Send count array
    for (i=0; i<size; i++)
    {
		if(i!=size-1) 
		send_cnt[i] = max(0, partitions[i+1] - partitions[i]); /// get size of elements to be sent between boundaries
		else 
		send_cnt[i] = max(0, partitions[i+1] - partitions[i]+1); ///Other than node 0, include the pivot at the beginning
    }
	
    MPI_Alltoall(send_cnt, 1, MPI_INT,recv_cnt, 1, MPI_INT, MPI_COMM_WORLD);	       
	
	
	/// Calculating offsets for both the send and receive buffers
	send_off[0] = 0;
	recv_off[0] = 0;
	for (i=1 ; i<size ; i++) 
	{
		send_off[i] = send_off[i-1] + send_cnt[i-1];
		recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
	}

	/// Allocate space for receive buffer
	recvLen=0;

	for(i=0; i< size; i++) recvLen=recvLen + recv_cnt[i];

	if (recvLen>0) {
		recv_buf = (mapperDuple* )malloc(recvLen*sizeof(mapperDuple));
		assert_malloc(recv_buf, rank);
	}
	else recv_buf = NULL;

	
	MPI_Alltoallv(mapper, send_cnt, send_off, MAPPER_DUPLE,
				recv_buf, recv_cnt, recv_off, MAPPER_DUPLE,
				MPI_COMM_WORLD);

	free(mapper);
	
	MPI_Barrier(MPI_COMM_WORLD);
	StartTime=clock();
	mapperDuple* result;
	mapperDuple* listOfIntegers[size];
	
	for (int i=0;i<size;i++)
	{
		listOfIntegers[i]=&(recv_buf[recv_off[i]]);
	}
	
	/// Multimerge of sorted buckets of mappers "used in recursion part"
	mergeAll_Mapper_Recursion(listOfIntegers, &result, recvLen, recv_cnt, size);

	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS Mapper_Recursion Last Fasta Sort ",pDC3Level);
	
	/// Assign the pointer B to rec_buf 
	*SortedMapper=result;
	
	free(pivots);
	free(partitions);
	free(recv_off);
	free(send_off);
	free(send_cnt);
	free(recv_cnt);
	
	return recvLen;
}


/// MultiMerge of sorted Buckets to merge the Sorted array of mappers after permutation between the processes 
void mergeAll_Mapper_Recursion(mapperDuple** listOfIntegers,  mapperDuple** result,int TotalSize,int* ListLen,int size)
{
	*result=(mapperDuple*)malloc(TotalSize*sizeof(mapperDuple));
    int totalNumbers = size;
    int curpos[totalNumbers];
    int currow = 0, foundMinAt = 0;
    mapperDuple minElement;
	
    /// Set the current position that was travered to 0 in all the array elements
    for ( int i = 0; i < totalNumbers; ++i)
    {
		curpos[i]=0;
    }
	
	int count=0;
	
	
    for ( ; ; )
    {

        for ( currow = 0 ; currow < totalNumbers ; ++currow)
        {
            if ( curpos[currow] < ListLen[currow] )
            {
                minElement = listOfIntegers[currow][curpos[currow]];
                foundMinAt = currow;
                break;
            }
        }
        /// If all the elements were traversed in all the arrays, then no further work needs to be done
        if ( !(currow < totalNumbers ) )
            break;
        
        ///    Traverse each of the array and find out the first available minimum value
        
        for (currow=0 ;currow < totalNumbers; ++currow)
        {
            if ( smallerThan_DC3(listOfIntegers[currow][curpos[currow] ],minElement) && curpos[currow]<ListLen[currow])
            {
                minElement = listOfIntegers[currow][curpos[currow] ];
                foundMinAt = currow;
            }
        }
        
        ///    Store the minimum into the resultant array 
        ///    and increment the element traversed
        
        (*result)[count]=minElement;
        count++;
        ++curpos[foundMinAt];
    }
    
}



/// Binary Search: find position of S key (or next larger) in sorted array
int binary_search_S(STuple *arr, int size, STuple *key, int rank)
{
  /// continue searching while [imin,imax] is not empty
  int m;
  int imin=0,imax=size-1;
  while (imax >= imin)
    {
      /// calculate the midpoint for roughly equal partition
      m = (imin+ imax)/2;
      if(isEqual_S(&arr[m], key))
     {  
		  /// key found at index m
          return m; 
      }
      /// determine which subarray to search
      else if (largerThan_S(*key, arr[m]))
        /// change min index to search upper subarray
        imin = m + 1;
      else         
        /// change max index to search lower subarray
        imax = m - 1;
    }
    
    while (m <= (size-1) && largerThan_S(*key, arr[m])) ++m;
  /// key not found
  return m;
}



/// Sort S using PSRS Method based on the comparison criteria stated on the paper
int all_sort_S_i(STuple *A,int A_size, STuple **B,int size, int rank,int pDC3Level, int verbose)
{
	
	int i, B_size;
	int *recv_cnt,   	  /// number of keys to receive from each PN 
      *send_cnt,   		  /// number of keys to send to PNs 
      *recv_off,
      *send_off;

	int no_samples,    /// number of samples to take from sorted data 
      pivdist,       /// distance between pivots in set of samples 
      possmp,        /// position of pivot in set of all samples 
      smpdist,       /// distance between consecutive samples 
      trecv;
      
	STuple  *pivbuffer,    /// array of pivots 
	         *recv_buf,     /// incoming data to merge
     	     *smpbuffer,    /// array of local samples (array of pointers)
      	     *smpallbuf,    /// array of global samples 
	         **prtbound;    /// boundaries for partitioning local data 

	MPI_Status stat;



	prtbound = (STuple **) malloc((size+1)*sizeof(STuple *));
	assert_malloc(prtbound, rank);
  
	send_cnt = (int *)malloc(size*sizeof(int));
	assert_malloc(send_cnt, rank);
  
	send_off = (int *)malloc(size*sizeof(int));
	assert_malloc(send_off, rank);
  
	recv_off = (int *)malloc(size*sizeof(int));
	assert_malloc(recv_off, rank);
	
	recv_cnt = (int *)malloc(size*sizeof(int));
	assert_malloc(recv_cnt, rank);
	
	pivbuffer = (STuple *)malloc((size-1)*sizeof(STuple));
	assert_malloc(pivbuffer, rank);

	/// Define MPI Struct
	MPI_Datatype tupleType;
    MPI_Datatype type[6] = { MPI_INT,MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    int blocklen[6] = { 1, 1, 1, 1, 1, 1};
    MPI_Aint disp[6];

    MPI_Get_address(&A[0].type, &disp[0]);
    MPI_Get_address(&A[0].T1, &disp[1]);
    MPI_Get_address(&A[0].T2, &disp[2]);
    MPI_Get_address(&A[0].R1, &disp[3]);
    MPI_Get_address(&A[0].R2, &disp[4]);
    MPI_Get_address(&A[0].index, &disp[5]);

	for(i=5; i>=0; i--) disp[i]-=disp[0];	

    MPI_Type_create_struct(6, blocklen, disp, type, &tupleType);
    MPI_Type_commit(&tupleType);
	
	/// Local sorting of S Structure 
	MPI_Barrier(MPI_COMM_WORLD);
	int StartTime=clock();
	std::sort(&A[0], &A[A_size], compS);
	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS S first Fasta Sort ",pDC3Level);
		
    
    /// Filling out the samples buffer 
	/// OVERSMP=1, then no of samples= no of processors -1  
	no_samples = OVERSMP*size - 1; 
	
	smpdist = A_size / (no_samples + 1);    ///  ////////////Take care, try to change this criteria

	smpbuffer = (STuple *)malloc(no_samples*sizeof(STuple));
	assert_malloc(smpbuffer, rank);

	for (i=0; i<no_samples; i++) copyAtoB_S(&A[(i+1)*smpdist], &smpbuffer[i]);
	

	/// Gather samples from other nodes 
	smpallbuf = (STuple *)malloc(size*no_samples*sizeof(STuple));
	assert_malloc(smpallbuf, rank);
	
	/// MPI_Gather: Note that the recvcount argument at the root indicates the number of items
	/// it receives from each process, not the total number of items it receives.
	MPI_Gather(smpbuffer, no_samples, tupleType, smpallbuf, no_samples, tupleType,0, MPI_COMM_WORLD);

	
	/// Forming final pivots 
	fastsort_S(smpallbuf, size*no_samples);

	if (rank==0) 
	{
		pivdist = size*no_samples/(size-1);
		possmp = pivdist/2;
		for (i=0; i<size-1; i++) 
		{
			copyAtoB_S(&smpallbuf[possmp], &pivbuffer[i]);
			possmp += pivdist;
		}
	}
	
	/// Broadcast pivots 
	MPI_Bcast(pivbuffer, size-1, tupleType, 0, MPI_COMM_WORLD);
	
	/// Filling prtbound, containing indexes of boundaries for paritioning local data in a way that each node contains up to the pivot
	prtbound[0] = &A[0];
	for (i=1; i<size; i++) prtbound[i] = &A[binary_search_S (A, A_size, &pivbuffer[i-1],rank)];
	prtbound[size] = &A[A_size-1]; /// + A_size;

	free (smpbuffer);
	free (smpallbuf);
	free (pivbuffer);



	/// Fill send_count array  
	for (i=0; i<size; i++)
	{
		if(i!=size-1) 
		send_cnt[i] = maxx(0, prtbound[i+1] - prtbound[i]); 
		else
		send_cnt[i] = maxx(0, prtbound[i+1] - prtbound[i]+1); 
	}
	
	MPI_Alltoall(send_cnt, 1, MPI_INT, recv_cnt, 1, MPI_INT, MPI_COMM_WORLD);

	/// Calculating offsets for both the send and receive buffers
	send_off[0] = 0;
	recv_off[0] = 0;
	for (i=1 ; i<size ; i++) 
	{
		send_off[i] = send_off[i-1] + send_cnt[i-1];
		recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
	}
	
	/// Allocate space for receive buffer
	trecv=0;

	for(i=0; i< size; i++) trecv=trecv + recv_cnt[i];

	if (trecv>0) 
	{
		recv_buf = (STuple *)malloc(trecv*sizeof(STuple));
		assert_malloc(recv_buf, rank);
	}
	else 
	{
		recv_buf = NULL;
	}
  
	MPI_Alltoallv(A, send_cnt, send_off, tupleType, recv_buf, recv_cnt, recv_off, tupleType, MPI_COMM_WORLD);
	free(A);
	
	
	/// Local Sort of S
	MPI_Barrier(MPI_COMM_WORLD);
	StartTime=clock();

	STuple* result; 
	STuple* listOfIntegers[size];
	for (int i=0;i<size;i++)
	{
		listOfIntegers[i]=&(recv_buf[recv_off[i]]);
	}
	
	/// Multimerge of sorted buckets of S struct
	mergeAll_S(listOfIntegers, &result, trecv, recv_cnt, size);
	
	
	/// Assign the pointer B to rec_buf 
	B_size = trecv;
	*B=result;
	
	
	MPI_Barrier(MPI_COMM_WORLD);
	if (verbose==1 && rank==0)
	elapsedTime(rank, StartTime,"PSRS S last Fasta Sort ",pDC3Level);
	
	free(recv_off);
	free(send_off);
	free(send_cnt);
	free(recv_cnt);
	free(prtbound);
	

	return (B_size);
}


/// MultiMerge of sorted Buckets to merge the Sorted array of S struct after permutation between the processes  
void mergeAll_S(STuple** listOfIntegers,  STuple** result,int TotalSize,int* ListLen,int size)
{
	*result=(STuple*)malloc(TotalSize*sizeof(STuple));
    int totalNumbers = size;
    int curpos[totalNumbers];
    int currow = 0, foundMinAt = 0;
    STuple minElement;

    /// Set the current position that was travered to 0 in all the array elements
    for ( int i = 0; i < totalNumbers; ++i)
    {
		curpos[i]=0;
    }
	
	int count=0;
    for ( ; ; )
    {
        ///  Find the first minimum 
        ///    Which is basically the first element in the array that hasn't been fully traversed
        

        for ( currow = 0 ; currow < totalNumbers ; ++currow)
        {
            if ( curpos[currow] < ListLen[currow] )
            {
                minElement = listOfIntegers[currow][curpos[currow]];
                foundMinAt = currow;
                break;
            }
        }
        /// If all the elements were traversed in all the arrays, then no further work needs to be done
        if ( !(currow < totalNumbers ) )
            break;
         
        ///    Traverse each of the array and find out the first available minimum value
        
        for (currow=0 ;currow < totalNumbers; ++currow)
        {
            if ( smallerThan_S(listOfIntegers[currow][curpos[currow] ],minElement) && curpos[currow]<ListLen[currow])
            {
                minElement = listOfIntegers[currow][curpos[currow] ];
                foundMinAt = currow;
            }
        }
         
        ///    Store the minimum into the resultant array 
        ///    and increment the element traversed
        
        (*result)[count]=minElement;
        count++;
        ++curpos[foundMinAt];
    }
}
