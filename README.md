cloud-based Parallel Suffix Array Construction Algorithm based on MPI 
=====

The suffix array is currently the best choice for indexing genomics data, because of its efficiency and a large number of applications. cloudSACA is a solution that automates the establishment of a computer cluster in a cloud and automatically constructs the suffix array in a distributed fashion over the cluster nodes. This has the advantage of encapsulating all set-up details and execution of the algorithm. The distributed nature of the algorithm we use overcomes the problem that arises when the user wishes, due to cost issues, to use small memory machines in the cloud. cloudSACA is used to construct the suffix array on the cloud in a distributed manner. Currently, we support only two providers; AWS and Azure. In the package we provide 4 implementations for suffix array construction algorithms:

1. Futamura-Aluru-Kurtz Algorithm.
2. Naive implementations for Kulla-Sanders Algorithm.
3. Merge implementations for Kulla-Sanders Algorithm.
4. Optimized implementations for Kulla-Sanders Algorithm.

### Publication:

* Metwally, Ahmed A., Ahmed H. Kandil, and Mohamed Abouelhoda. "Distributed suffix array construction algorithms: Comparison of two algorithms." In Biomedical Engineering Conference (CIBEC), 2016 8th Cairo International, pp. 27-30. IEEE, 2016.
* Abdelhadi, Ahmed, A. H. Kandil, and Mohamed Abouelhoda. "Cloud-based parallel suffix array construction based on MPI." In Biomedical Engineering (MECBME), 2014 Middle East Conference on, pp. 334-337. IEEE, 2014.


# Getting Started
This section details steps for installing and running pSACA. Current pSACA version only supports Linux. If you experience difficulty installing or running the software, please contact (Ahmed Metwally: ametwa2@uic.edu).

### Prerequisites
To install and run pSACA on local infrastructure, you should Install and configure the following on all Nodes:
* g++  
* Install and configure MPI on all Nodes 
* OpenMP  
* openmpi-bin 
* openmpi-common 
* libopenmpi1.3 
* libopenmpi-dev (Not actually needed for a slave node.)



## Installing and Running

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

#### Implemented options:
```
-w    Window size, Default=2
-v    Verbose
-c    Clean fasta file.
-r    Check correctness of the output file.
-o    Outputfile. The default is called fSA.txt and found in the same directory. The output file is a binary file which contains the suffix array of the input file. 
```


#### Example:
* If you want to run on a local machine:
```
mpirun -np 4 ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA
```

* If you want to run on computer cluster:
```
mpirun -np 16 --hostfile MPI.hosts ./FAK  Hgchr22.fa -w 4 -v -o Hgchr22_SA
```

* To run the cluster over AWS, just download cloudSACA_AWS then follow with its README file to onfigure the MPI cluster.
```
Use the AWS image (eu-west-1): ami-a5df1dd2
```
