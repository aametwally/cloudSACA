#include <string>

using namespace std;
void CheckCorrectness (string ReferenceFile, string SuffixArrayFile);
void usage(string prog);
void memory_usage(int rank);
void SaveOutputToFile(string str, int rank,int textLenGlobal, int* SAFinal);
void SaveOutputToFile_pDC3(int rank,int SSortedLenLocal, STuple* S_Tuple_Sorted);
void CheckCorrectness_pDC3 (int* ReferenceFile, int ReferenceSize ,string SuffixArrayFile, int pDC3Level);
void copyright();
