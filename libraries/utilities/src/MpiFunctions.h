#ifndef _UTILITIES__MPI_FUNCTIONS_H_
#define _UTILITIES__MPI_FUNCTIONS_H_

#include "mpi.h"

namespace MpiFunctions {


   /// template struct to determine the mpi-type from the template typename.
   template<typename Scalar>
   struct MpiType;

   /// Specialisation of mpi-type for char.
   template<>
   struct MpiType<char> {
      static const MPI_Datatype type = MPI_CHAR;
   };

   /// Specialisation of mpi-type for const char.
   template<>
   struct MpiType<const char> {
      static const MPI_Datatype type = MPI_CHAR;
   };

   /// Specialisation of mpi-type for unsigned char.
   template<>
   struct MpiType<unsigned char> {
      static const MPI_Datatype type = MPI_UNSIGNED_CHAR;
   };

   /// Specialisation of mpi-type for const unsigned char.
   template<>
   struct MpiType<const unsigned char> {
      static const MPI_Datatype type = MPI_UNSIGNED_CHAR;
   };

   /// Specialisation of mpi-type for int.
   template<>
   struct MpiType<int> {
      static const MPI_Datatype type = MPI_INT;
   };

   /// Specialisation of mpi-type for const int.
   template<>
   struct MpiType<const int> {
      static const MPI_Datatype type = MPI_INT;
   };

   /// Specialisation of mpi-type for unsigned int.
   template<>
   struct MpiType<unsigned int> {
      static const MPI_Datatype type = MPI_UNSIGNED;
   };

   /// Specialisation of mpi-type for const unsigned int.
   template<>
   struct MpiType<const unsigned int> {
      static const MPI_Datatype type = MPI_UNSIGNED;
   };

   /// Specialisation of mpi-type for float.
   template<>
   struct MpiType<float> {
      static const MPI_Datatype type = MPI_FLOAT;
   };

   /// Specialisation of mpi-type for const float.
   template<>
   struct MpiType<const float> {
      static const MPI_Datatype type = MPI_FLOAT;
   };

   /// Specialisation of mpi-type for double.
   template<>
   struct MpiType<double> {
      static const MPI_Datatype type = MPI_DOUBLE;
   };

   /// Specialisation of mpi-type for const double.
   template<>
   struct MpiType<const double> {
      static const MPI_Datatype type = MPI_DOUBLE;
   };



   /// Get the maximum value from all processes in communicator.
   ///
   // Probably need some compile-time check so that this is 
   // not compiled for non-scalar types.
   template<typename Scalar>
   Scalar Maximum ( MPI_Comm& communicator, const Scalar& operand ) {

      Scalar localOp = operand;
      Scalar result;

      MPI_Allreduce ( &localOp, &result, 1, 
                      MpiType<Scalar>::type, MPI_MAX, communicator );

      return result;
   }

#if 0
   // Specialisation of Maximum for Boolean values.
   template<>
   bool Maximum<bool> ( MPI_Comm& communicator, const bool& operand );
#endif

   /// Get the minimum value from all processes.
   ///
   // Probably need some compile-time check so that this is 
   // not compiled for non-scalar types.
   template<typename Scalar>
   Scalar Minimum ( MPI_Comm& communicator, const Scalar& operand ) {

      Scalar localOp = operand;
      Scalar result;

      MPI_Allreduce ( &localOp, &result, 1, 
                      MpiType<Scalar>::type, MPI_MIN, communicator );

      return result;
   }

#if 0
   // Specialisation of Minimum for Boolean values.
   template<>
   bool Minimum<bool> ( MPI_Comm& communicator, const bool& operand );
#endif

   /// Get the sum of values from all processes.
   ///
   // Probably need some compile-time check so that this is 
   // not compiled for non-scalar types.
   template<typename Scalar>
   Scalar Sum ( MPI_Comm& communicator, const Scalar& operand ) {

      Scalar localOp = operand;
      Scalar result;

      MPI_Allreduce ( &localOp, &result, 1, 
                      MpiType<Scalar>::type, MPI_SUM, communicator );

      return result;
   }


}

//------------------------------------------------------------//



#if 0
template<>
bool MpiFunctions::Maximum<bool> ( MPI_Comm& communicator, const bool& operand ) {

   int localOperand = ( operand ? 1 : 0 );
   int result;

   MPI_Allreduce ( &localOperand, &result, 1, 
                   MpiType<int>::type, MPI_MAX, communicator );

   return result == 1;
}



template<>
bool MpiFunctions::Minimum<bool> ( MPI_Comm& communicator, const bool& operand ) {

   int localOperand = ( operand ? 1 : 0 );
   int result;

   MPI_Allreduce ( &localOperand, &result, 1, 
                   MpiType<int>::type, MPI_MIN, communicator );

   return result == 1;
}
#endif

#endif // _UTILITIES__MPI_FUNCTIONS_H_
