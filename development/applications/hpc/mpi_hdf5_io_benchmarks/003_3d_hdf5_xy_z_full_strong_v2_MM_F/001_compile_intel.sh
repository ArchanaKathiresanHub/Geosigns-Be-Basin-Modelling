#!/bin/bash

  mpiicpc  -O3                           \
		 -std=c++11                    \
		 -Wall                         \
           driver_program.cpp            \
           /opt/hdf5/intel/1.8.9/lib/libhdf5.a \
		 -lz                           \
           -o x_intel

