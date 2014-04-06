#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <getopt.h>
#include "helpers.hpp"
#include "bucketSorting.hpp"
#include "bucketConstruction.hpp"

using namespace std;



int main (int argc, char *argv[])
{
	/// Fetch the commandline
	if (argc<2)
	{
		usage(argv[0]);
		printf("less than required minimum number of options\n");
		exit(1);
	}
	string file=argv[1];
	int size=0;	
	vector<char> reference_array;
	int  numtasks, rank, len, rc,threads; 
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int ProcessSize=0;
	char c;	
	int w=2;
	bool verbose=false;
	bool clean=false;
	int hash=0;
	vector<char> AlphapetChar;
	double StartTime=0;
	bool review=false;
	string OutputFile=file+"_SA";
	while (1) 
	{	
		/// Collecting Options
		static struct option long_options[] = 
		{			 
			{"w", 1, 0, 0},
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
			case 0: w = atol(optarg); break;
			case 1: verbose=true; break;
			case 2: review=true; break;
			case 3: clean=true; break;
			case 4: OutputFile=optarg; break;
			
			default: break; 
			}
		}
	}
	

	/// Initializing MPI Processes + Initializing the time counter
	rc = MPI_Init(&argc,&argv);
    if (rc != MPI_SUCCESS) 
    {
		printf ("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
    }
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Get_processor_name(hostname, &len);
	
	if (rank==0)
	copyright();

	/// Read data from the file
	StartTime=clock();

	if (clean)
	clean_load_fasta(file,reference_array,AlphapetChar);
	else
	load_fasta(file,reference_array,AlphapetChar);
	

	if(verbose&&rank==0)
	{
		elapsedTime(rank, StartTime,"Reading file");
		printf("output file is=%s\n",OutputFile.c_str());
	}	
	size=reference_array.size();

	/// add (w-2) of '$' to the end of the sequence array 
	for (int i=0;i<w-1;i++)
	{
		reference_array.push_back('$');
	}
	AlphapetChar.push_back('$');
	
	if (verbose&&rank==0)
	{
		printf("Thread=%d, AlphabetSize=%ld\n",rank,AlphapetChar.size());
	}
	int globalstarttime = clock();
	
	
	
	///Phase 1: Bucket table construction 
	int *SA2;
	int SA2Size= BucketConstruction(AlphapetChar, reference_array, size, numtasks, rank, w, verbose, &SA2);
	
	
	///Phase2:  Sort Buckets
	int* index=	BucketSorting(SA2Size, SA2, verbose, globalstarttime, rank, reference_array);


	/// send partial SAs to the master node:
	int* SAFinal;
	if (rank==0);
	SAFinal=(int*)malloc((size+1)*sizeof(int));
	StartTime=clock();
	int* IndexDisplacement=(int*)malloc((numtasks)*sizeof(int));
	int* IndexReceive=(int*)malloc((numtasks)*sizeof(int));
	MPI_Gather(&SA2Size,1,MPI_INT,&IndexReceive[rank],1,MPI_INT,0,MPI_COMM_WORLD);
	if (rank==0)
	{
		IndexDisplacement[0]=0;
		for(int i=1;i<numtasks;i++)
		{
			IndexDisplacement[i]=IndexDisplacement[i-1]+IndexReceive[i-1];
		}
	}
	MPI_Gatherv(index,SA2Size,MPI_INT,SAFinal,&IndexReceive[rank],&IndexDisplacement[rank],MPI_INT,0,MPI_COMM_WORLD);
	free(index);
	
	/// Measure Total Time
	MPI_Barrier(MPI_COMM_WORLD);
	if(verbose&&rank==0)
	elapsedTime(rank, globalstarttime,"Total Time");
	
	
	/// Save output file
	if (rank==0)
	{
		StartTime=clock();
		SaveOutputToFile(OutputFile, SAFinal, verbose, rank, size);
		if (verbose==true)
		{
			elapsedTime(rank, StartTime,"Saving Output");
			elapsedTime(rank, globalstarttime,"Total Time After Output Saving");
			memory_usage(rank);
		}
	}
	
	free (SAFinal);
    
    /// Check the correctness of the generated SA
    MPI_Barrier(MPI_COMM_WORLD);
	if (review && rank==0)
	CheckCorrectness (reference_array, OutputFile.c_str());
	
	MPI_Finalize();
}


