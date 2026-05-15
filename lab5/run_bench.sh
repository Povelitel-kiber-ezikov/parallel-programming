#!/bin/bash
#SBATCH --job-name=main
#SBATCH --time=0:05:00
#SBATCH --ntasks-per-node=8
#SBATCH --partition batch

module load intel/mpi4

SIZES=(200 400 800 1200 1600 2000)

for size in "${SIZES[@]}"
do
	echo "--- Size: $size ---"
	./create_mat $size mat_A.txt $size mat_B.txt
	mpirun -r ssh ./mult_mat mat_A.txt mat_B.txt mat_C.txt 

done
