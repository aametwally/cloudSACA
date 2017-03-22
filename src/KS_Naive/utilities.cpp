#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include "mydefinitions.hpp"
#include "utilities.hpp"

void trim(string &line, long &start, long &end);
void load_fasta(string filename, vector<char> &reference_array,vector<char> &AlphapetChar);
bool searchChar(char c,const std::vector<char> &AlphapetChar);


/// Check Correctness of the Output SA
void CheckCorrectness (string ReferenceFile, string SuffixArrayFile)
{
	vector<char> reference_array;
	vector<char> AlphapetChar;
	load_fasta(ReferenceFile, reference_array,AlphapetChar);
	FILE *aggr=fopen(SuffixArrayFile.c_str(),"r");
	vector<int> Index;
	int returnValue=0;
	int temp;
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
	printf("\n");
	int TempIndex1;
	int TempIndex2;
	int count=0;
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
				count++;
				printf("Check Failed at index= %d, in line= %d\n",TempIndex1,i+1);
				break;

			}
		}
	}
	if (count==0)
	printf("Check Succeed !!! \n");
}

/// Trim string line
void trim(string &line, long &start, long &end) 
{
	// Trim leading spaces. 
	for(long i = start; i < (int)line.length(); i++) 
	{ 
		if(line[i] != ' ') { start = i; break; } 
	}
	// Trim trailing spaces.
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

	// Everything starts at zero.
	///startpos.push_back(0);
	ifstream data(filename.c_str());
	if(!data.is_open()) { cerr << "unable to open " << filename << endl; exit(1); } 
	while(!data.eof()) 
	{
		getline(data, line); // Load one line at a time.
		if(line.length() == 0) continue;
		long start = 0, end = line.length() - 1;
		// Meta tag line and start of a new sequence.
		if(line[0] == '>') 
		{
			// Save previous sequence and meta data.
			if(length > 0) 
			{
				///descr.push_back(meta);
				///reference_array.push_back('>'); //character used to separate strings
				///if (!searchChar('>',AlphapetChar))
				///{
				///	AlphapetChar.push_back('>');
				///}
				//S += '`'; // ` character used to separate strings
				///startpos.push_back(reference_array.size());
				//lengths.push_back(length+1);
			}
			// Reset parser state.
			start = 1; meta = ""; length = 0;
		}
		trim(line, start, end);
		// Collect meta data.
		if(line[0] == '>') 
		{
			for(long i = start; i <= end; i++) { if(line[i] == ' ') break; meta += line[i]; }
		}
		else 
		{ // Collect sequence data.
			length += end - start + 1;
			for(long i = start; i <= end; i++) 
			{
				c= std::tolower(line[i]);
				reference_array.push_back(c);
				if (!searchChar(c,AlphapetChar))
				{
					AlphapetChar.push_back(c);
					
				}
				//S += std::tolower(line[i]);
			}
		}
	}
}


/// Search about certain character inside array of characters 
bool searchChar(char c,const std::vector<char> &AlphapetChar)
{
	for (int i=0;i<AlphapetChar.size();i++)
	if (c==AlphapetChar[i])
	return 1;
	return 0;
	
}

/// How to use Kulla Algorithm, this message shows up when the user enters wrong command line
void usage(string prog) 
{
	cerr << "Usage: mpirun -np <no_cores> --hostfile <host-file> " << prog << " <reference-file> [options]" << endl;
	cerr << "Implemented Options:" << endl;
	cerr << "-v      Verbose" << endl;
	cerr << "-o      Outputfile --- The default is called SA.txt and found in the same directory." << endl;
	exit(1);
}


/// Measure Memory Usage
void memory_usage(int rank) 
{
	//ostringstream mem;
	ifstream proc("/proc/self/status");
	string s;
	int start=0;
	while(getline(proc, s), !proc.fail()) 
	{		
		if(s.substr(0, 6) == "VmPeak") {
			   printf("Thread=%d, %s\n",rank,s.c_str());
		}
		if(s.substr(0, 5) == "VmHWM") {
				printf("Thread=%d, %s\n",rank,s.c_str());
		} 
		 if(s.substr(0, 5) == "VmRSS") {
				printf("Thread=%d, %s\n",rank,s.c_str());
		}
	}
}


/// Save The Output SA to an external file le 
void SaveOutputToFile(string str, int rank,int textLenGlobal, int* SAFinal)
{
	FILE *out=fopen(str.c_str(),"w");
    for (int j=0;j<textLenGlobal;j++)
    {
				fwrite(&SAFinal[j], sizeof(int), 1, out);
	}
    fclose(out);
}

/// Save The Output of the recursion part to an external file 
void SaveOutputToFile_pDC3(int rank,int SSortedLenLocal, STuple* S_Tuple_Sorted)
{
	stringstream ss;
	ss << rank;
	string str = ss.str();
	str="Results/"+str+".txt";
	FILE *out=fopen(str.c_str(),"w");
    for (int j=0;j<SSortedLenLocal;j++)
    {
		fprintf(out,"%d\n",(S_Tuple_Sorted)[j].index);	
	}
    fclose(out);
}





/// Check Correctness of the output of the recursion Part
void CheckCorrectness_pDC3 (int* ReferenceFile, int ReferenceSize ,string SuffixArrayFile,int pDC3Level)
{
	FILE *aggr=fopen(SuffixArrayFile.c_str(),"r");
	vector<int> Index;
	int temp;
	if (aggr==NULL)
		printf("Error while Opening File\n");
	else
	{
		while(fscanf(aggr,"%d",&temp)!=EOF)
		{
			Index.push_back(temp);			
		}
	}
	printf("\n");
	int TempIndex1;
	int TempIndex2;
	int count=0;
	for (int i=0;i<ReferenceSize-1;i++)
	{
		TempIndex1=Index[i];
		TempIndex2=Index[i+1];	
		for (int j=0;(TempIndex1+j<ReferenceSize)&&(TempIndex2+j<ReferenceSize);j++)
		{	
			if(ReferenceFile[TempIndex1+j]<ReferenceFile[TempIndex2+j])
				break;
			if (ReferenceFile[TempIndex1+j]>ReferenceFile[TempIndex2+j])
			{	
				count++;
				printf("Check Failed at index= %d, in line= %d, pDC3Level=%d\n",TempIndex1,i+1,pDC3Level);
				break;
			}
		}
	}
	if (count==0)
	printf("Check Succeed, pDC3Level=%d !!! \n",pDC3Level);
}

void copyright()
{
	printf("****************************************************************\n");
	printf("********                     pSACA                      ********\n");
	printf("********  Parallel Suffix Array Construction Algorithm 	********\n");
	printf("********                  Based on MPI                 	********\n");
	printf("********  [Implementation of Kulla-Sanders Algorithm]   ********\n");
	printf("*                     Naive Implementation                     *\n");
	printf("****************************************************************\n");
	printf("*                  This is pSACA version 1.2                   *\n");
	printf("*                                                              *\n");
	printf("*       Copyright by Ahmed Abdelhadi Metwally (C) 2014         *\n");
	printf("*                                                              *\n");
	printf("*            Please report bugs & suggestions to               *\n");
	printf("*               <ahmed.abdelhady@eng.cu.edu.eg>                *\n");
	printf("****************************************************************\n");
}
