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
#include "helpers.hpp"
using namespace std;

/// Trim string line
void trim(string &line, long &start, long &end) 
{
	/// Trim leading spaces. 
	for(long i = start; i < (int)line.length(); i++) 
	{ 
		if(line[i] != ' ') { start = i; break; } 
	}
	/// Trim trailing spaces.
	for(long i = line.length() - 1; i >= 0; i--) 
	{ 
		if(line[i] != ' ') { end = i; break; } 
		if(i == 0) break;
	}
}


/// Read Fasta file
void load_fasta(string filename, vector<char> &reference_array,vector<char> &AlphapetChar)
{
	string meta, line;
	long length = 0;
	char c;

	/// Everything starts at zero.
	ifstream data(filename.c_str());
	if(!data.is_open()) { cerr << "unable to open " << filename << endl; exit(1); } 
	while(!data.eof()) 
	{
		getline(data, line); // Load one line at a time.
		if(line.length() == 0) continue;
		long start = 0, end = line.length() - 1;
		/// Meta tag line and start of a new sequence.
		if(line[0] == '>') 
		{
			/// Reset parser state.
			start = 1; meta = ""; length = 0;
		}
		trim(line, start, end);
		/// Collect meta data.
		if(line[0] == '>') 
		{
			for(long i = start; i <= end; i++) { if(line[i] == ' ') break; meta += line[i]; }
		}
		else 
		{ /// Collect sequence data.
			length += end - start + 1;
			for(long i = start; i <= end; i++) 
			{
				c= std::tolower(line[i]);
				reference_array.push_back(c);
				if (!searchChar(c,AlphapetChar))
				{
					AlphapetChar.push_back(c);
					
				}
			}
		}
	}
}


/// search about certain character in array of character 
bool searchChar(char c,const std::vector<char> &AlphapetChar)
{
	for (int i=0;i<AlphapetChar.size();i++)
	if (c==AlphapetChar[i])
	return 1;
	return 0;
	
}


/// Search about character, return index of found charachter 
int SeqSearch (char x, const std::vector<char> &AlphapetChar)
{
	for (int i=0;i<AlphapetChar.size();i++)
	{
		if (x==AlphapetChar[i])
		return i;
	}
}


/// Merge generated files into one file
void collect_files(int tasks)
{
	char ch;
	FILE *file_ptr;
	FILE *aggr=fopen("Results/Total.txt","w");
	for (int i=0;i<tasks;i++)
	{
		stringstream ss;
		ss << i;
		string file_to_open = ss.str();
		file_to_open = "Results/"+file_to_open+".txt";	
		if((file_ptr = fopen(file_to_open.c_str(), "r")) != NULL)
		{
			while((ch = fgetc(file_ptr)) != EOF)
			{
				fprintf(aggr,"%c",ch);
			}
			fclose(file_ptr);
		}
		else
		{
			printf("Could not open %s\n", file_to_open.c_str());
		}
	}
	fclose(aggr);
	
}


/// Report Memory Info (MaximumMemoryUsage, ThreadsNum, PID)
void memory_usage(int rank) 
{
	ifstream proc("/proc/self/status");
	string s;
	int start=0;
	while(getline(proc, s), !proc.fail()) 
	{		
		if(s.substr(0, 5) == "VmHWM") {
				printf("Thread=%d, %s\n",rank,s.c_str());
		} 
		
	}
}


/// Measure the elapsed time 
void elapsedTime(int rank,double start,string message)
{
		int finish = clock();
		int total= finish-start;
    printf("Thread=%d, %s=%4.3f Sec\n",rank, message.c_str(), (double) total / CLOCKS_PER_SEC);
	
}

void CheckCorrectness (const std::vector<char> reference_array,string file)
{
	FILE *aggr=fopen(file.c_str(),"r");
	vector<int> Index;
	int temp;
	int returnValue=0;
	if (aggr==NULL)
		printf("Error while Opening File\n");
	else
	{
		while(!feof(aggr))
		{
			returnValue=fread(&temp,sizeof(int),1,aggr);
			Index.push_back(temp);			
		}
	}
	int TempIndex1;
	int TempIndex2;
	for (int i=0;i<Index.size()-1;i++)
	{
		TempIndex1=Index[i];
		TempIndex2=Index[i+1];	
		for (int j=0;(TempIndex1+j<reference_array.size())&&(TempIndex2+j<reference_array.size());j++)
		{	
			if(reference_array[TempIndex1+j]<reference_array[TempIndex2+j])
				break;
			if (reference_array[TempIndex1+j]>reference_array[TempIndex2+j])
			{	
				printf("Check Failed at index= %d, in line= %d\n",TempIndex1,i);
				return;
			}
		}
	}
	printf("Check Succeeded !!! \n");
}

void clean_load_fasta(string filename, vector<char> &reference_array,vector<char> &AlphapetChar)
{
	string meta, line;
	long length = 0;
	char c;
	int StartCountN=0;
	int DiffCounter=0;
	int SequenceCounter=-1;

	/// Everything starts at zero.
	///startpos.push_back(0);
	ifstream data(filename.c_str());

	FILE *diff_ptr=fopen("DifferenceN","w");
	if(!data.is_open()) { cerr << "unable to open " << filename << endl; exit(1); } 
	while(!data.eof()) 
	{
		getline(data, line); /// Load one line at a time.
		if(line.length() == 0) continue;
		long start = 0, end = line.length() - 1;
		/// Meta tag line and start of a new sequence.
		if(line[0] == '>') 
		{
			/// Reset parser state.
			start = 1; meta = ""; length = 0;
		}
		trim(line, start, end);
		/// Collect meta data.
		if(line[0] == '>') 
		{
			for(long i = start; i <= end; i++) { if(line[i] == ' ') break; meta += line[i]; }
		}
		else 
		{ /// Collect sequence data.
			length += end - start + 1;
			for(long i = start; i <= end; i++) 
			{
				c= std::tolower(line[i]);
				
				if (c=='a' || c=='c' || c=='g' || c=='t' || c=='n')
				{
					SequenceCounter++;
					if (c=='n')
					{
						if (StartCountN==0)
						{
							StartCountN=1;
							reference_array.push_back(c);
						}
						
						else
						{
							StartCountN=2;
							DiffCounter++;
							continue;
						}
						
					}
					else
					{
						if (StartCountN==2)
						{
							StartCountN=0;
							fprintf(diff_ptr,"%d\n",SequenceCounter);
						}
						reference_array.push_back(c);
					}
					
					if (!searchChar(c,AlphapetChar))
					{
						AlphapetChar.push_back(c);
						
					}
				}
			}
		}
	}
	fclose(diff_ptr);
}







void SaveOutputToFile(string OutputFile, int* SAFinal, bool verbose, int rank, int size)
{
	
	FILE *totalFile=fopen(OutputFile.c_str(),"wb");
	if(totalFile==NULL)
	{
		printf("Cannot Open: %s\n",OutputFile.c_str());
		exit(1);
	}
	else
	{
		for (int j=0;j<size;j++)
		{
			fwrite(&SAFinal[j], sizeof(int), 1, totalFile);
		}
			
		fclose(totalFile);
	}
}






void usage(string prog) 
{
	cerr << "Usage: mpirun -np <no_cores> --hostfile <host-file> " << prog << " <reference-file> [options]" << endl;
	cerr << "Implemented Options:" << endl;
	cerr << "-w      Window size, Default=2" << endl;
	cerr << "-v      Verbose" << endl;
	cerr << "-c      Clean fasta file" << endl;
	cerr << "-o      Outputfile --- The default is called filename_SA.txt and found in the same directory." << endl;
	exit(1);
}


void copyright()
{
	printf("****************************************************************\n");
	printf("********                     pSACA                      ********\n");
	printf("********  Parallel Suffix Array Construction Algorithm 	********\n");
	printf("********                  Based on MPI                 	********\n");
	printf("********  [Implementation of Futamura-Kurtz Algorithm]  ********\n");
	printf("****************************************************************\n");
	printf("*                  This is pSACA version 1.2                   *\n");
	printf("*                                                              *\n");
	printf("*       Copyright by Ahmed Abdelhadi Metwally (C) 2014         *\n");
	printf("*                                                              *\n");
	printf("*            Please report bugs & suggestions to               *\n");
	printf("*               <ahmed.abdelhady@eng.cu.edu.eg>                *\n");
	printf("****************************************************************\n");
}




