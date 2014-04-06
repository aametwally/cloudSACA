#include <vector>
using namespace std;


void trim(string &line, long &start, long &end);
void load_fasta(string filename, vector<char> &reference_array,vector<char> &AlphapetChar);
bool searchChar(char c,const std::vector<char> &AlphapetChar);
int SeqSearch (char x, const std::vector<char> &AlphapetChar);
void collect_files(int tasks);
void memory_usage(int rank);
void elapsedTime (int rank, double start,string message);
void CheckCorrectness (const std::vector<char> reference_array,string file);
void clean_load_fasta(string filename, vector<char> &reference_array,vector<char> &AlphapetChar);
void usage(string prog);
void SaveOutputToFile(string OutputFile, int* SAFinal, bool verbose, int rank, int size);
void copyright();

