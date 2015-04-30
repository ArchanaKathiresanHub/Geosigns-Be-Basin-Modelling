#!/bin/bash

  mpiicpc  -O3                            \
           -Wall                          \
	   -std=c++11                     \
           driver_program.cpp             \
           /opt/hdf5/1.8.11/lib/libhdf5.a \
	   -lz                            \
           -o x_intel

