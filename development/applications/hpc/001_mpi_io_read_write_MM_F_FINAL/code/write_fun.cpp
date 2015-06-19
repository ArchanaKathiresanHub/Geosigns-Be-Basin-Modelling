
#include "mpi.h"
#include "write_fun.h"
#include "parameters.h"

#include <iostream>
#include <fstream>
#include <string>

using std::ofstream;
using std::string;

// function definition

void write_fun(MPI::File the_file,
               const long int & dim,
               const int & ITER,
               const int & my_rank)
{
     static int counter = 0;
     ofstream out_file;

     MPI::Status status;

     double * array_local = new double [dim];

     for(int i = 0; i != dim; ++i) {
          array_local[i] = static_cast<double>(i);
     }

     // create the sentinel file

     string tmp_str = F_SENTINEL + std::to_string(static_cast<long long>(my_rank));

     out_file.open(tmp_str.c_str());
     out_file << counter;
     out_file.close();

     // do write data

     for (int it = 0; it != ITER; it++) {
          the_file.Write(array_local,
                         dim,
                         MPI::DOUBLE,
                         status);
     }

     // update sentinel file

     counter++;
     out_file.open(tmp_str.c_str());
     out_file << counter;

     // delete heap

     delete [] array_local;
}

// end
