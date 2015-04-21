#!/bin/bash

  mpicxx  -O3                            \
	      -Wall                          \
          -std=c++0x                     \
          driver_program.cpp             \
          /opt/hdf5/1.8.11/lib/libhdf5.a \
          -lz                            \
          -o x_gnu

