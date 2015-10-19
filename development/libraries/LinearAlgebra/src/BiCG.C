//------------------------------------------------------------//

#include "BiCG.h"
#include <iostream>

//------------------------------------------------------------//


void Numerics::BiCG::operator ()
   ( const Vector&         RHS,
           Vector&         Solution ) {


  const int dimension = RHS.dimension ();

  int Iteration_Count = 1;
  FloatingPoint Residual;
  FloatingPoint Rho_1;
  FloatingPoint Rho_2 = 0.0; // just to stop the compiler complaining about use before assignment
  FloatingPoint Alpha = 0.0; // just to stop ...
  FloatingPoint Beta;
  FloatingPoint Omega = 0.0; // just to stop ...

  Vector P    ( dimension );
  Vector PHat ( dimension );
  Vector S    ( dimension );
  Vector SHat ( dimension );
  Vector T    ( dimension );
  Vector V    ( dimension );

  Vector Work_Space ( dimension );


  FloatingPoint NormB = vectorLength ( RHS );
  Vector        R ( dimension );
  Vector        RTilde ( dimension );
  
  //
  // Begin
  //
  if ( isZeroVector ( Solution )) {
    R = RHS;
  } else {
    theMatrix->matrixVectorProduct ( Solution, Work_Space );
    vectorXMY ( RHS, Work_Space, R );
  }

  RTilde = R;

  if ( NormB == 0.0 ) {
    NormB = 1.0;
  }

  NormB = 1.0;

  Residual = vectorLength ( R ) / NormB;

  if ( Residual < solverTolerance ) {
    numberOfIterations = 0;
    return;
  }

  while ( Iteration_Count <= maximumNumberOfIterations ) {
    Rho_1 = innerProduct ( RTilde, R );

    if ( Rho_1 == 0.0 ) {
      // throw an exception
    }

    if ( Iteration_Count == 1 ) {
      P = R;
    } else {
      Beta = ( Rho_1 / Rho_2 ) * ( Alpha / Omega );

      //
      // P = R + Beta * ( P - Omega  * V );
      //
      vectorScale ( Omega, V, Work_Space );
      vectorXMY   ( P, Work_Space, Work_Space );
      vectorScale ( Beta, Work_Space );

      Work_Space += R;

      P = Work_Space;
    }

    thePreconditioner->solve ( P, PHat );
    theMatrix->matrixVectorProduct ( PHat, V );

    Alpha = Rho_1 / innerProduct ( RTilde, V );

    //
    // S = R - Alpha * V;
    //
    vectorScale ( Alpha, V, Work_Space );
    vectorXMY   ( R, Work_Space, Work_Space );

    S = Work_Space;

    Residual = vectorLength ( S ) / NormB;

    if ( Residual < solverTolerance ) {

      if ( printResidual ) {
        std::cout << " resid " << Iteration_Count << "  " << Residual << std::endl;
      }

      vectorAXPY ( Alpha, PHat, Solution );
      break;
    }

    thePreconditioner->solve ( S, SHat );
    theMatrix->matrixVectorProduct ( SHat, T );
    Omega = innerProduct ( T, S ) / innerProduct ( T, T );

    //
    // Solution = Solution + Alpha * PHat + Omega * SHat;
    //
    vectorAXPY ( Alpha, PHat, Solution );
    vectorAXPY ( Omega, SHat, Solution );

    //
    // R = S - Omega * T;
    //
    vectorScale ( Omega, T, Work_Space );

    vectorXMY   ( S, Work_Space );
    R = Work_Space;

    Rho_2 = Rho_1;
    Residual = vectorLength ( R ) / NormB;

    if ( printResidual ) {
      std::cout << " resid " << Iteration_Count << "  " << Residual << std::endl;
    }

    if ( Residual < solverTolerance ) {
      break;
    }

    if ( Omega == 0.0 ) {
      // throw an exception
    }

    Iteration_Count = Iteration_Count + 1;
  }

  numberOfIterations = Iteration_Count;
}
