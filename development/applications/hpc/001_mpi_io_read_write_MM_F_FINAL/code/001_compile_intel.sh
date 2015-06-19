#!/bin/bash

  # 1. compile

  mpiicpc  -O3                             \
           -Wall                           \
           -std=c++11                      \
           write_fun.cpp                   \
		 fibonacci.cpp                   \
           driver_read_write_mpi_final.cpp \
           -o x_intel
