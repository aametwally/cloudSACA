#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "seq_sort.hpp"
#include "helpers.hpp"

#define BREAKPT 100


int partition_DC3(mapperDuple *A, int left, int right, int pivot)
{

	int i;
	swap_mapperDuple(&A[pivot], &A[right]); ///moving the pivot the end of the array (consider other pivot selection techniques)
	pivot=right;
	int storeIndex=left;

	for(i=left; i<=right; i++)
	{
			if(smallerThan_DC3(A[i], A[pivot])) 
			{
				swap_mapperDuple(&A[i], &A[storeIndex]);
				storeIndex++;
			}
	}
	swap_mapperDuple(&A[storeIndex], &A[pivot]);
	return storeIndex;
}


void quicksort_DC3(mapperDuple* A, int left, int right)
{	
	int i;
	if(left<right)
	{
		int pivotIndex= ((right-left)/2)+left;
		int pivotNewIndex= partition_DC3(A, left, right, pivotIndex);
		quicksort_DC3(A, left, pivotNewIndex-1);
		quicksort_DC3(A, pivotNewIndex+1, right);		
	}	
}

void insertsort_DC3(mapperDuple *A, int n) 
{
    mapperDuple* item;
    register int i,j;
    
    for (i=1 ; i<n ; i++) 
    {
		j = i-1;
		
		while ((j>=0)&& (smallerThan_DC3(A[j+1], A[j]))) 
		{
		    swap_mapperDuple(&A[j+1], &A[j]);
		    j--;
		}
 	}
}

void fastsort_DC3(mapperDuple* arr, int nel) {
    if (nel>=BREAKPT)
		quicksort_DC3(arr, 0, nel-1);
    else
		insertsort_DC3(arr,nel);
}


int partition_SAtuple(SAtuple *A, int left, int right, int pivot)
{
	int i;
	swap_SAtuple(&A[pivot], &A[right]); ///moving the pivot the end of the array (consider other pivot selection techniques)
	pivot=right;
	int storeIndex=left;

	for(i=left; i<=right; i++)
	{
			if(smallerThan_SAtuple(A[i], A[pivot])) 
			{
				swap_SAtuple(&A[i], &A[storeIndex]);
				storeIndex++;
			}
	}
	swap_SAtuple(&A[storeIndex], &A[pivot]);
	return storeIndex;
}

void quicksort_SAtuple(SAtuple* A, int left, int right)
{	
	int i;
	if(left<right)
	{
		int pivotIndex= ((right-left)/2)+left;
		int pivotNewIndex= partition_SAtuple(A, left, right, pivotIndex);
		quicksort_SAtuple(A, left, pivotNewIndex-1);
		quicksort_SAtuple(A, pivotNewIndex+1, right);		
	}	
}

void insertsort_SAtuple(SAtuple *A, int n) 
{
    SAtuple* item;
    register int i,j;
    
    for (i=1 ; i<n ; i++) 
    {
		j = i-1;
		
		while ((j>=0)&& (smallerThan_SAtuple(A[j+1], A[j]))) 
		{
		    swap_SAtuple(&A[j+1], &A[j]);
		    j--;
		}
 	}
}

void fastsort_SAtuple(SAtuple* arr, int nel) {
    if (nel>=BREAKPT)
		quicksort_SAtuple(arr, 0, nel-1);
    else
		insertsort_SAtuple(arr,nel);
}




int partition_mapperDuple(mapperDuple *A, int left, int right, int pivot)
{
	int i;
	swap_mapperDuple(&A[pivot], &A[right]);/// moving the pivot the end of the array (consider other pivot selection techniques)
	pivot=right;
	int storeIndex=left;

	for(i=left; i<=right; i++)
	{
		if(smallerThan_index_mapperDuple(A[i], A[pivot])) 
		{
			swap_mapperDuple(&A[i], &A[storeIndex]);
			storeIndex++;
		}
	}
	swap_mapperDuple(&A[storeIndex], &A[pivot]);
	return storeIndex;
}

void quicksort_mapperDuple(mapperDuple* A, int left, int right)
{	
	int i;
	if(left<right)
	{
		int pivotIndex= ((right-left)/2)+left;
		int pivotNewIndex= partition_mapperDuple(A, left, right, pivotIndex);
		quicksort_mapperDuple(A, left, pivotNewIndex-1);
		quicksort_mapperDuple(A, pivotNewIndex+1, right);		
	}	
}

void insertsort_mapperDuple(mapperDuple *A, int n) 
{
    mapperDuple* item;
    register int i,j;
    
    for (i=1 ; i<n ; i++) 
    {
		j = i-1;		
		while ((j>=0)&& (smallerThan_index_mapperDuple(A[j+1], A[j]))) 
		{
		    swap_mapperDuple(&A[j+1], &A[j]);
		    j--;
		}
 	}
}

void fastsort_mapperDuple(mapperDuple* arr, int len) ///Currently sort by index if needed will implement by rank
{
    if (len>=BREAKPT)
		quicksort_mapperDuple(arr, 0, len-1);
    else
		insertsort_mapperDuple(arr,len);
}





int partition_S(STuple *A, int left, int right, int pivot)
{
	int i;
	swap_S(&A[pivot], &A[right]);//moving the pivot the end of the array (consider other pivot selection techniques)
	pivot=right;
	int storeIndex=left;

	for(i=left; i<=right; i++)
	{
			if(smallerThan_S(A[i], A[pivot])) 
			{
				swap_S(&A[i], &A[storeIndex]);
				storeIndex++;
			}
	}
	swap_S(&A[storeIndex], &A[pivot]);
	return storeIndex;
}

void quicksort_S(STuple* A, int left, int right)
{	
	int i;
	if(left<right)
	{
		int pivotIndex= ((right-left)/2)+left;
		int pivotNewIndex= partition_S(A, left, right, pivotIndex);
		quicksort_S(A, left, pivotNewIndex-1);
		quicksort_S(A, pivotNewIndex+1, right);		
	}	
}

void insertsort_S(STuple *A, int n) 
{
    STuple* item;
    register int i,j;
    
    for (i=1 ; i<n ; i++) 
    {
		j = i-1;
		
		while ((j>=0)&& (smallerThan_S(A[j+1], A[j]))) 
		{
		    swap_S(&A[j+1], &A[j]);
		    j--;
		}
 	}
}

void fastsort_S(STuple* arr, int nel) {
    if (nel>=BREAKPT)
		quicksort_S(arr, 0, nel-1);
    else
		insertsort_S(arr,nel);
}

