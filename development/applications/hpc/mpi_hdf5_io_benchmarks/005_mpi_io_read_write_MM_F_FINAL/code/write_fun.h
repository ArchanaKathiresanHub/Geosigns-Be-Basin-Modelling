
#ifndef WRITE_FUN_H
#define WRITE_FUN_H

#include "mpi.h"

// function declaration

void write_fun(MPI::File,        // mpi type file 
			const long int &, // dimen of the array
			const int &,      // iterarion
			const int &);     // mpi rank

#endif // WRITE_FUN_H
