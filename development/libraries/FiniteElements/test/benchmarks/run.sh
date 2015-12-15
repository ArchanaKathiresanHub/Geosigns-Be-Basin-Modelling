#!/bin/bash

#set -x #echo on

export PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin:$PATH
export LD_LIBRARY_PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/gcc/gcc-4.8.1/Linux64x_26/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/lib:$LD_LIBRARY_PATH

source /apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/bin/compilervars.sh intel64
source /apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/mkl/bin/mklvars.sh intel64

compiler="g++ g++-4.8.1 icc"
#compiler="icc"

rm -rf results.txt *.jpg compl.*


for CCC in $compiler
do
   rm -f ./StiffMatrixBench

   CFLAGS="-O3 -fomit-frame-pointer -finline-functions -DNDEBUG -DEIGEN_USE_SPARSE_MATRIX"
#   CFLAGS="-O0 -g -DEIGEN_USE_SPARSE_MATRIX -DDEBUG_PRINT"
   
   if [ $CCC == "icc" ]
   then
      CFLAGS="$CFLAGS -no-ipo -ip -no-prec-div"
   fi
   
   $CCC $CFLAGS -o StiffMatrixBench -I../../../utilities/src \
                                -I/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/eigen/eigen-3.2-intel/LinuxRHEL64_x86_64_57WS/include/eigen3 \
                                -I/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/include \
                                -L/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/armadillo/armadillo-3.920.1-intel/LinuxRHEL64_x86_64_57WS/lib \
                                -I/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/boost/boost_1_54_0/Linux64x_26/include \
                 ElementAssembly.C ArmElementAssembly.C CldElementAssembly.C EgnElementAssembly.C \
                 MklElementAssembly.C SseElementAssembly.C BoostElementAssembly.C main.C \
                ../../src/FiniteElementTypes.C ../../src/FiniteElement.C ../../src/BasisFunction.C -larmadillo -liomp5 >> compl.out 2>> compl.err

   ./StiffMatrixBench >> results.txt
#   valgrind --tool=memcheck --log-file=vlg.txt ./StiffMatrixBench > results_${CCC}.txt
#   valgrind --tool=callgrind ./StiffMatrixBench
#   tv StiffMatrixBench
done

sed -f sed.txt results.txt  > results.m
octave plotresults.m

rm -f ./StiffMatrixBench
