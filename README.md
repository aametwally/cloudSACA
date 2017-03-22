pSACA
=====

Parallel Suffix Array Construction Algorithm

How to run this algorithm:
A) make
B) mpirun -np <no_cores> --hostfile <path-to-host-file> ./FAK <reference-file> [options]

-----------------------------------------------------------
Implemented options:
1) -w      Window size, Default=2
2) -v      Verbose
3) -c      Clean fasta file, the differences are stored in DifferenceN file.
4) -o      Outputfile >> The default is called filename_SA.txt and found in the same directory.

-----------------------------------------------------------
Examples:
* If you want to run on local machine:
mpirun -np 4 ./FAK Hg22.fa -w 4 -v -c -o SA


* If you want to run on computer cluster:
mpirun -np 16 --hostfile MPI.hosts ./FAK Hg22.fa -w 4 -v -c -o SA
