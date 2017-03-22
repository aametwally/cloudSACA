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
#include "pDC3.hpp"

using namespace std;


main(int argc, char *argv[]) 
{
	if (argc<2)
	{
		usage(argv[0]);
		exit(1);
	}	
	char* fname=argv[1];
	bool verbose=false;
	bool clean=false;
	bool review=false;
	string OutputFile="SA.txt";



	while (1) 
	{	
		/// Collecting Parameters 
		static struct option long_options[] = 
		{			 
			{"v", 0, 0, 0},
			{"r", 0, 0, 0},
			{"c", 0, 0, 0},
			{"o", 1, 0, 0}  
		};
		int longindex = -1;
		int c = getopt_long_only(argc, argv, "", long_options, &longindex);
		if(c == -1) break; /// Done parsing flags.
		else if(c == '?') 
		{
			/// If the user entered junk, let him know. 
			cerr << "Invalid parameters." << endl;
			usage(argv[0]);
		}
		else 
		{
			/// Branch on long options.
			switch(longindex) 
			{ 
				case 0: verbose=true; break;
				case 1: review=true; break;
				case 2: clean=true; break;
				case 3: OutputFile=optarg; break;		
				default: break; 
			}
		}
	}
	
	
	int* text=NULL;
	int size, rank, textLenLocal, textLenGlobal, tuples12lenLocal, tuples12lenGlobal;
	int* delta;
	MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	long StartTime,TotalTime;
	TotalTime=clock();
	StartTime=clock();
	
	if (rank==0)
	copyright();
	
	
	
    ///STEP #1: Open file for reading
   	int* readFret= readF(fname, &text,size,rank);
   	textLenLocal=readFret[0];
   	textLenGlobal= readFret[1];
   	int pDC3Level =0;
   	if (verbose&&rank==0)
   	{
		elapsedTime(rank, StartTime,"Read time",pDC3Level);
		printf("Memory After Sequence read\n");
		memory_usage(rank);
	}
   	
	/// pDC3 Algorithm
	STuple* S_Tuple_Sorted;
	
	int SSortedLenLocal=pDC3(text,textLenLocal,textLenGlobal,rank,size,verbose,&S_Tuple_Sorted,pDC3Level);
	free(text);
	
	int* SA=(int*)malloc(SSortedLenLocal*sizeof(int));
	for (int i=0;i<SSortedLenLocal;i++)
	{
		SA[i]=S_Tuple_Sorted[i].index;
	}
	
	
	
	free(S_Tuple_Sorted);
	int* SAFinal;
	int* IndexDisplacement=(int*)malloc((size)*sizeof(int));
	int* IndexReceive=(int*)malloc((size)*sizeof(int));
	MPI_Gather(&SSortedLenLocal,1,MPI_INT,&IndexReceive[rank],1,MPI_INT,0,MPI_COMM_WORLD);
	if (rank==0)
	{
		SAFinal=(int*)malloc(textLenGlobal*sizeof(int));
		IndexDisplacement[0]=0;
		for(int i=1;i<size;i++)
		{
			IndexDisplacement[i]=IndexDisplacement[i-1]+IndexReceive[i-1];
		}
	}
	
	MPI_Gatherv(SA,SSortedLenLocal,MPI_INT,SAFinal,&IndexReceive[rank],&IndexDisplacement[rank],MPI_INT,0,MPI_COMM_WORLD);
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	/// Save the Output SA to external File
	
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank==0)
	{
		/// Save SA to an output file
		SaveOutputToFile(OutputFile, rank,textLenGlobal,SAFinal);
		
		/// Review the Output SA for Correctness
		if (review==1)
		CheckCorrectness(fname,OutputFile);
	}
	free(SAFinal);
	MPI_Finalize ();
}




