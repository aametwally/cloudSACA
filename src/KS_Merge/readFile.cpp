#include <stdio.h>
#include <stdlib.h>
#include "readFile.hpp"
#include "mpi.h"

int* readF(char* fname, int** text,int size, int rank)
{
	int ProcessorsDataSize[size];
	int* RowData;
	int TotalLength;
	if (rank==0)
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
		
		RowData=(int*)malloc((n+3)*sizeof(int));
		for(int j=0;j<n;j++)
		{
			RowData[j]=getc(fp);
		}
		
		fclose(fp);
		//~ printf("Thread=%d,Rowdata= ",rank);
		//~ for(int j=0;j<n;j++)
		//~ {
			//~ printf("%d ",RowData[j]);
		//~ }
		//~ printf("\n");
		
		//// ************ calculate the size of each processors.
		
		for(int i=0;i<size;i++)
		{
			if (i!=size-1)
			ProcessorsDataSize[i]=(n/size);
			else
			ProcessorsDataSize[i]=n-((size-1)*(n/size));
			
		}
		TotalLength=n;
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast (&TotalLength,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	
	int displs[size];
	if (rank==0)
	{
		displs[0]=0;
		for (int i=1; i<size; i++)
		displs[i]= displs[i-1]+ ProcessorsDataSize[i-1];
	}
	
	
	for(int i=0;i<size;i++)
	{
		if (i!=size-1)
		ProcessorsDataSize[i]=(TotalLength/size)+2;
		else
		ProcessorsDataSize[i]=TotalLength-((size-1)*(TotalLength/size));	
	}

	//~ printf("Thread=%d,ToTalLength=%d\n",rank,TotalLength);
	int processSize;
	if (rank!=size-1)
	processSize=(TotalLength/size)+2;
	else
	processSize=TotalLength-((size-1)*(TotalLength/size));
	
	//~ printf("Thread=%d,processSize=%d\n",rank,processSize);
	

	/// Allocate memory. 
	*text = (int *)malloc((processSize+3) * sizeof(int));

	for (int i=0;i<processSize;i++)
	(*text)[i]=0;
	
	
	//~ printf("thread=%d, text= ",rank);
	//~ for (int i=0;i<processSize;i++)
	//~ {
		//~ printf("%d ",(*text)[i]);
	//~ }
	//~ printf("\n");
	
	
	if((text == NULL)) 
	{
		fprintf(stderr, "Cannot allocate memory for the input file.\n");
		exit(EXIT_FAILURE);
	}

	MPI_Scatterv (RowData, ProcessorsDataSize, displs, MPI_INT, (*text), processSize, MPI_INT,0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank!=size-1)
	processSize=processSize-2;
		
  	/// Padding the last node text with two zeros
  	if(rank==size-1)
  	{
		(*text)[processSize]=0;
		(*text)[processSize+1]=0; 
  	}

	
	/// return localLen and GlobalLen
	int* ret=(int*) malloc (2*sizeof(int));
	ret[0]=processSize;	
	ret[1]=TotalLength;
  	return ret;
}
