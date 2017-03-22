#include <stdio.h>
#include <stdlib.h>
#include "readFile.hpp"
#include "mpi.h"

int* readF(char* fname, int** text,int size, int rank)
{
	FILE *fp;
	int n;
	MPI_Status stat;

    if((fp = fopen(fname, "r")) == NULL) 
    {
    	fprintf(stderr, "Cannot open file `%s': ", fname);
    	perror(NULL);
    	exit(EXIT_FAILURE);
    }

    /// Get the file size. 
    if(fseek(fp, 0, SEEK_END) == 0) 
    {
    	n = ftell(fp);
    	rewind(fp);
    	if(n < 0) 
    	{
     		fprintf(stderr, "Cannot ftell `%s': ", fname);
     		perror(NULL);
      		exit(EXIT_FAILURE);
	    }
	} 
	else 
	{
    	fprintf(stderr, "Cannot fseek `%s': ", fname);
    	perror(NULL);
    	exit(EXIT_FAILURE);
	}
	
	///	This to eliminate the last NULL element.
	n=n-1;     			
		
	int processSize;
	if (rank!=size-1)
	processSize=(n/size);
	else
	processSize=n-((size-1)*(n/size));
	
	

	/// Allocate memory. 
	*text = (int *)malloc((processSize+3) * sizeof(int));

	if((text == NULL)) 
	{
    	fprintf(stderr, "Cannot allocate memory for the input file.\n");
    	exit(EXIT_FAILURE);
  	}

	/// Read n bytes of data. 
	if (rank!=size-1)
	fseek(fp, rank*processSize, SEEK_SET);
	else
	fseek(fp, (rank*(n/size)), SEEK_SET);
	for(int j=0;j<processSize+2;j++)
	{
		(*text)[j]=getc(fp);
	}

  	fclose(fp);
  	
  	/// Padding the last node text with two zeros
  	if(rank==size-1)
  	{
		(*text)[processSize]=0;
		(*text)[processSize+1]=0; 
  	}
	
	
	/// return localLen and GlobalLen
	int* ret=(int*) malloc (2*sizeof(int));
	ret[0]=processSize;
	ret[1]=n;
  	return ret;
}
