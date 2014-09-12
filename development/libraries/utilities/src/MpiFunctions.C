#include "MpiFunctions.h"

namespace MpiFunctions {
   /// Specialisation of mpi-type
   template <> const MPI_Datatype MpiType<char> :: type = MPI_CHAR;
   template <> const MPI_Datatype MpiType<int> :: type = MPI_INT;
   template <> const MPI_Datatype MpiType<float> :: type = MPI_FLOAT;
   template <> const MPI_Datatype MpiType<double> :: type = MPI_DOUBLE;
}
