#!/bin/bash

export LD_LIBRARY_PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/gcc/gcc-4.8.1/Linux64x_26/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/lib:$LD_LIBRARY_PATH

source /apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/bin/compilervars.sh intel64
source /apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/mkl/bin/mklvars.sh intel64

compiler="g++ g++-4.8.1 icc"
#compiler="g++-4.8.1"
#compiler="icc"

rm -rf results.txt

for CCC in $compiler
do
   rm -f ./StiffMatrixBench

   $CCC -O3 -o StiffMatrixBench -I../../../utilities/src \
                                -I/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/eigen/eigen-3.2-intel/LinuxRHEL64_x86_64_57WS/include/eigen3 \
                                -I/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/include \
                                -L/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/lib \
                                -DNOT_USE_SPARSE_MATRIX \
                ElementAssembly.C ArmElementAssembly.C CldElementAssembly.C EgnElementAssembly.C MklElementAssembly.C main.C \
                ../../src/FiniteElementTypes.C ../../src/FiniteElement.C ../../src/BasisFunction.C -larmadillo -liomp5

   ./StiffMatrixBench >> results.txt
   #valgrind --tool=memcheck --log-file=vlg.txt ./StiffMatrixBench > results_${CCC}.txt
   #tv StiffMatrixBench
done
sed -f sed.txt results.txt  > results.m
octave plotresults.m

#rm -f ./StiffMatrixBench
#valgrind --tool=callgrind ./StiffMatrixBench
