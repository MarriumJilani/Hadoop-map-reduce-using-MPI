# Hadoop-map-reduce-using-MPI
This is a C code that implements the functionality of Hadoop map reduce to multiply two matrices. The Hadoop map reduce is created from scratch using MPI.
It includes the generation of matrices and then their serial multiplication. The parallel multiplication will be done using mpi_map_reduce.c file. The results of both files can be compared to check correctness. This code requires a master slave cluster setup and will be run using mpicc and mpiexec commands on a number of processors. 
The aim is to parallelise matrix multiplication using Hadoop map reduce, and also to implement Hadoop map reduce itself
