#include <string>
using namespace std;

void CheckCorrectness (string ReferenceFile, string SuffixArrayFile);
void usage(string prog);
void memory_usage(int rank, string label, int pDC3Level);
void SaveOutputToFile(int rank,int SSortedLenLocal, STuple* S_Tuple_Sorted);
void SaveOutputToFile(string str,int rank,int textLenGlobal, int* SAFinal);
void CheckCorrectness_pDC3 (int* ReferenceFile, int ReferenceSize ,string SuffixArrayFile, int pDC3Level);
void copyright();
