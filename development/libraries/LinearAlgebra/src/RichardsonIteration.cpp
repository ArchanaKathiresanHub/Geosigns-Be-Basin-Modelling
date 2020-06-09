//------------------------------------------------------------//

#include "RichardsonIteration.h"
#include <iostream>

//------------------------------------------------------------//


void Numerics::RichardsonIteration::operator ()
   ( const Vector& RHS,
           Vector& Solution ) {

  int N = theMatrix->numberOfRows ();
  FloatingPoint Residual;
  FloatingPoint Norm_B = vectorLength ( RHS );
  Vector Z  ( N );
  Vector R  ( N );
  Vector WS ( N );
  int Iteration_Count = 1;

  bool Converged = false;

  if ( isZeroVector ( Solution )) {
    R = RHS;
  } else {
    theMatrix->matrixVectorProduct ( Solution, WS );
    vectorXMY ( RHS, WS, R );
  } // end if;

  if ( Norm_B == 0.0 ) {
    Norm_B = 1.0;
  } // end if

  Residual = vectorLength ( R ) / Norm_B;

  while (( !Converged ) && ( Iteration_Count <= maximumNumberOfIterations )) {
    thePreconditioner->solve ( R, Z );

    Solution += Z;

    theMatrix->matrixVectorProduct ( Solution, WS );
    vectorXMY ( RHS, WS, R );
    Residual = vectorLength ( R ) / Norm_B;

    Converged = Residual < solverTolerance;

    if ( printResidual ) {
      std::cout << " resid " << Iteration_Count << "  " << Residual << std::endl;
    }

    Iteration_Count = Iteration_Count + 1;
  } // end loop

  numberOfIterations = Iteration_Count;
} // end Numerics::LinearAlgebra::RISolver::Solve


//------------------------------------------------------------//
