#!/bin/bash

  mpiicpc  -O0                                  \
	      -Wall                                \
	      -std=c++11                           \
           driver_program.cpp                   \
           /opt/hdf5/parallel/v16/lib/libhdf5.a \
	      -lz                                  \
           -o x_intel

