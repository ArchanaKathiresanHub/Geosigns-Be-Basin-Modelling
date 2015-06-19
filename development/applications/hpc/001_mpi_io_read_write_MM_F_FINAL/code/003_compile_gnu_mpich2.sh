#!/bin/bash

  # 1. compile

  mpic++.mpich2  -O3                             \
                 -Wall                           \
                 -std=c++0x                      \
			  write_fun.cpp                   \
			  fibonacci.cpp                   \
                 driver_read_write_mpi_final.cpp \
                 -o x_gnu_mpich2
