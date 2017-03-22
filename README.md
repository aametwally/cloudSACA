pSACA: Parallel Suffix Array Construction Algorithm
=====

This package contains 4 implementations for suffix array construction algorithms:

1. Futamura-Aluru-Kurtz Algorithm.
2. Naive implementations for Kulla-Sanders Algorithm.
3. Merge implementations for Kulla-Sanders Algorithm.
4. Optimized implementations for Kulla-Sanders Algorithm.

### Publication:

* Metwally, Ahmed A., Ahmed H. Kandil, and Mohamed Abouelhoda. "Distributed suffix array construction algorithms: Comparison of two algorithms." In Biomedical Engineering Conference (CIBEC), 2016 8th Cairo International, pp. 27-30. IEEE, 2016.
* Abdelhadi, Ahmed, A. H. Kandil, and Mohamed Abouelhoda. "Cloud-based parallel suffix array construction based on MPI." In Biomedical Engineering (MECBME), 2014 Middle East Conference on, pp. 334-337. IEEE, 2014.
</br>


# Getting Started
This section details steps for installing and running pSACA. Current pSACA version only supports Linux. If you experience difficulty installing or running the software, please contact (Ahmed Metwally: ametwa2@uic.edu).

### Prerequisites
To install and run pSACA on local infrastrucrure, you should Install and configure the following on all Nodes:
* g++  
* Install and configure MPI on all Nodes 
* OpenMP  
* openmpi-bin 
* openmpi-common 
* libopenmpi1.3 
* libopenmpi-dev (Not actually needed for a slave node.)

</br>



## Installing, Testing, and Running

#### Clone the project to your local repository:
```
git clone https://github.com/aametwally/pSACA.git
```


#### Change directory to pSACA, then build pSACA from scratch:
```
cd pSACA
./install.sh
```


## Running pSACA on FASTA or txt reference file:
```
mpirun -np <no_cores> --hostfile <path-to-host-file> <executable> <reference-file> [options]
```


Output file format: binary file which contains the suffix array of the input file. 


#### Implemented options:
```
-w    Window size, Default=2
-v    Verbose
-c    Clean fasta file.
-r    Check correctness of the output file.
-o    Outputfile. The default is called fSA.txt and found in the same  directory. The output file is a binary file which contains the suffix array of the input file. 
```


#### Example:
* If you want to run on local machine:
```
mpirun -np 4 ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA
```

* If you want to run on computer cluster:
```
mpirun -np 16 --hostfile MPI.hosts ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA
```

* To run the cluster over AWS, uust download cloudSACA_AWS then follow with its README file and it would configure the MPI cluster for you very easily.
```
Use the AWS image (eu-west-1): ami-a5df1dd2
```
