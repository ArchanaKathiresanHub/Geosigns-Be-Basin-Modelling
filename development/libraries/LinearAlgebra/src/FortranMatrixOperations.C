//------------------------------------------------------------//

#include "FortranMatrixOperations.h"
#include "LAPACKFunctions.h"

//------------------------------------------------------------//

void Numerics::mvp ( const FortranMatrix& mat,
                     const Vector&        vec,
                           Vector&        result ) {

   BLAS::mvp ( false, mat.numberOfRows (), mat.numberOfColumns (), mat.data (), vec.data (), result.data ());

}

//------------------------------------------------------------//

void Numerics::luFactorise ( FortranMatrix& mat,
                             IntegerArray&  permutation ) {
  LAPACK::luFactorise ( mat.numberOfRows (), mat.data (), &permutation [ 0 ]);
}

//------------------------------------------------------------//

void Numerics::backSolve ( const FortranMatrix& mat,
                           const IntegerArray&  permutation,
                                 Vector&        result ) {
  LAPACK::backSolve ( mat.numberOfRows (), mat.data (), &permutation [ 0 ], result.data ());
}

//------------------------------------------------------------//

void Numerics::addOuterProduct ( FortranMatrix& mat,
                                 const Vector&  vec1,
                                 const Vector&  vec2 ) {
  LAPACK::addOuterProduct ( mat.numberOfRows (), mat.numberOfColumns (), vec1.data (), vec2.data (), mat.data ());
}

//------------------------------------------------------------//
