#!/bin/bash

  mpicxx  -O3                           \
          -std=c++0x                    \
		-Wall                         \
          driver_program.cpp            \
          /opt/hdf5/gnu/1.8.9/lib/libhdf5.a \
          -lz                           \
          -o x_gnu

