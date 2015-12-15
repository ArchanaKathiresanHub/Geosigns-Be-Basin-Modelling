#!/bin/bash

export LD_LIBRARY_PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/gcc/gcc-4.8.1/Linux64x_26/lib64:$LD_LIBRARY_PATH
export PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/gcc/gcc-4.8.1/Linux64x_26/bin:$PATH

compiler="icc g++-4.8.1"

for CCC in $compiler
do
    rm -f ./computeProps
    CFLAGS="-O3  -DNDEBUG -openmp -fopenmp -lpthread"
    
    $CCC $CFLAGS -o computeProps computeProperty.C main.C

    ./computeProps
done 