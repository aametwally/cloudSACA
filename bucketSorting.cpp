#include "bucketSorting.hpp"

/// Sort Buckets
int* BucketSorting(int SA2Size, int* SA2, bool verbose,int globalstarttime, int rank, vector<char> reference_array)
{
	/// Construct SA_ptr array that contains the address of the first character of each suffix, to be sent to the string sorting function
	char** SA2_Ptr=(char **)malloc(sizeof(char*)*SA2Size);
	for (int i=0;i<SA2Size;i++)
	{
		SA2_Ptr[i]=&reference_array[SA2[i]];			
	}
	
	/// Construct Index array which should be sorted to get the final SA
	int* index=(int*)malloc(sizeof(int)*SA2Size);
	for (int j=0;j<SA2Size;j++)
	{
		index[j]=SA2[j];
	}
	free(SA2);

	if (verbose==true)
	{
		elapsedTime(rank, globalstarttime,"Time before Sorting");
	}
	
	/// String Sorting
	int StartTime=clock();
	ssort1main(SA2_Ptr, SA2Size,index);
	if (verbose==true)
	elapsedTime(rank, StartTime,"Finish Sorting, Sorting Time");
	free(SA2_Ptr);
	return index;
}
