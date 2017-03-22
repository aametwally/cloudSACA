How to run the FAK algorithm:
------------------------------
A) make
B) mpirun -np <no_cores> --hostfile <path-to-host-file> ./FAK <reference-file> [options]



Implemented options:
----------------------
1) -w      Window size, Default=2
2) -v      Verbose
3) -o      Outputfile --- The default is called SA.txt and found in the same directory.

Examples:
----------
A) If you want to run on local machine:
mpirun -np 4 ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA

B) If you want to run on computer cluster:
mpirun -np 16 --hostfile MPI.hosts ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA
