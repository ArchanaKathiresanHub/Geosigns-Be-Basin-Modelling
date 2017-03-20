//------------------------------------------------------------//

#include "GMResRestart.h"
#include <cmath>
#include <iostream>

//------------------------------------------------------------//

Numerics::GMResRestart::GMResRestart () {
  restartValue = DefaultRestartValue;
}

//------------------------------------------------------------//

void Numerics::GMResRestart::fill ( VectorArray& vecs, 
                                    Vector*      value ) const {

  size_t i;

  for ( i = 0; i < vecs.size (); i++ ){
    vecs [ i ] = value;
  }

}

//------------------------------------------------------------//


void Numerics::GMResRestart::generatePlaneRotation
   ( FloatingPoint  dx,
     FloatingPoint  dy,
     FloatingPoint& cs,
     FloatingPoint& sn ) {

  FloatingPoint temp;

  if (dy == 0.0) {
    cs = 1.0;
    sn = 0.0;
  } else if (std::fabs(dy) > std::fabs(dx)) {
    temp = dx / dy;
    sn = 1.0 / sqrt( 1.0 + temp*temp );
    cs = temp * sn;
  } else {
    temp = dy / dx;
    cs = 1.0 / sqrt( 1.0 + temp*temp );
    sn = temp * cs;
  }

}


//------------------------------------------------------------//


void Numerics::GMResRestart::applyPlaneRotation
   ( FloatingPoint& dx,
     FloatingPoint& dy,
     FloatingPoint  cs,
     FloatingPoint  sn ) {

  FloatingPoint temp =  cs * dx + sn * dy;

  dy = -sn * dx + cs * dy;
  dx = temp;
}


//------------------------------------------------------------//


void Numerics::GMResRestart::update
   ( Vector&        solution,
     int            iterationCount,
     FortranMatrix& hessenbergMatrix,
     Vector&        s,
     VectorArray&   vectorSequence,
     Vector&        workSpace ) {

  Vector y ( s );
  int i;
  int j;


  for ( i = iterationCount; i >= 1; i-- ) {
    y ( i ) = y ( i ) / hessenbergMatrix ( i, i );

    for ( j = i - 1; j >= 1; j-- ) {
      y ( j ) = y ( j ) - hessenbergMatrix ( i, j ) * y ( i );
    }

  }

  for ( j = 1; j <= iterationCount + 1; j++ ) {
    vectorScale ( y ( j ), *vectorSequence [ j ], workSpace );
    solution += workSpace;
  }

}

//------------------------------------------------------------//

void Numerics::GMResRestart::setRestartValue ( const int newRestartValue ) {
  restartValue = newRestartValue;
}

//------------------------------------------------------------//

void Numerics::GMResRestart::operator ()
   ( const Vector& rhs,
           Vector& solution ) {

  const int dimension = theMatrix->numberOfRows ();

  int  I, K;
  int  iterationCount = 1;

  Vector S  ( restartValue + 2 );
  Vector CS ( restartValue + 1 );
  Vector SN ( restartValue + 1 );
  Vector W  ( dimension );
  Vector workSpace  ( dimension );
  Vector workSpace2 ( dimension );
  Vector R  ( dimension );

  FloatingPoint residual;
  FloatingPoint normB = vectorLength ( rhs );
  FloatingPoint Beta;

  VectorArray vectorSequence ( restartValue + 2 );
  int restartCount;
  bool Converged = false;

//    UpperTriangularMatrix  hessenbergMatrix ( restartValue + 1, restartValue + 2 );
  FortranMatrix  hessenbergMatrix ( restartValue + 1, restartValue + 2 );

  if (normB == 0.0) {
    normB = 1.0;
  } // end if

  normB = 1.0;

  // This will save on a matrix vector product, if solution = 0.0 initially
  if ( isZeroVector ( solution )) {
    thePreconditioner->solve ( rhs, R );
  } else {
    theMatrix->matrixVectorProduct ( solution, workSpace2 );
    vectorXMY ( rhs, workSpace2, workSpace );
    thePreconditioner->solve ( workSpace, R );
  } // end if;

  Beta = vectorLength ( R );
  residual = Beta / normB;

  if ( printResidual ) {
    std::cout << " resid ( " << iterationCount << " ) = " << residual << ";" << std::endl;
  }

  if ( residual <= solverTolerance ) {
    numberOfIterations = 1;
    return;
  } // end if

  fill ( vectorSequence, 0 );
  vectorSequence [ 1 ] = new Vector ( dimension );

  while (( not Converged ) and ( iterationCount <= maximumNumberOfIterations )) {
    vectorScale ( 1.0 / Beta, R, *vectorSequence [ 1 ]);
    S.fill ( 0.0 );
    S ( 1 ) = Beta;
    restartCount = 1;

    while (( restartCount <= restartValue ) and ( iterationCount <= maximumNumberOfIterations )) {

      if ( vectorSequence [ restartCount + 1 ] == 0 ) {
        vectorSequence [ restartCount + 1 ] = new Vector ( dimension );
      } // end if

      theMatrix->matrixVectorProduct ( *vectorSequence [ restartCount ], workSpace );
      thePreconditioner->solve ( workSpace, W );

      for ( K = 1; K <= restartCount; K++ ) {
        hessenbergMatrix ( restartCount, K ) = innerProduct ( *vectorSequence [ K ], W );
        vectorAXPY ( -hessenbergMatrix ( restartCount, K ), *vectorSequence [ K ], W );
      } // end loop

      hessenbergMatrix ( restartCount, restartCount + 1 ) = vectorLength ( W );
      vectorScale ( 1.0 / hessenbergMatrix ( restartCount, restartCount + 1 ), W, *vectorSequence [ restartCount + 1 ]);

      for ( K = 1; K <= restartCount - 1; K++ ) {
        applyPlaneRotation ( hessenbergMatrix ( restartCount, K ),
                             hessenbergMatrix ( restartCount, K + 1 ),
                             CS ( K ), SN ( K ));
      } // end loop

      generatePlaneRotation
            ( hessenbergMatrix  ( restartCount, restartCount ),
              hessenbergMatrix  ( restartCount, restartCount + 1 ),
              CS ( restartCount ),
              SN ( restartCount ));

      applyPlaneRotation
            ( hessenbergMatrix  ( restartCount, restartCount ),
              hessenbergMatrix  ( restartCount, restartCount + 1 ),
              CS ( restartCount ),
              SN ( restartCount ));

      applyPlaneRotation
            ( S  ( restartCount ),
              S  ( restartCount + 1 ),
              CS ( restartCount ),
              SN ( restartCount ));

      residual = std::fabs ( S ( restartCount + 1 )) / normB;

      if ( residual < solverTolerance ) {
        Converged = true;
        break;
      } // end if


      restartCount   = restartCount + 1;
      iterationCount = iterationCount + 1;

      if ( printResidual ) {
        std::cout << " resid ( " << iterationCount << " ) = " << residual << ";" << std::endl;
      }

    } // end while loop

    update ( solution, restartCount - 1, hessenbergMatrix, S, vectorSequence, workSpace );

    if ( not Converged ) { 
      theMatrix->matrixVectorProduct ( solution, workSpace );
      vectorXMY ( rhs, workSpace );
      thePreconditioner->solve ( workSpace, R );
      Beta = vectorLength ( R );
    } // end if

  } // end while loop

  for ( I = 1; I <= restartValue + 1; I++ ) {

    if ( vectorSequence [ I ] != 0 ) {
      delete vectorSequence [ I ];
    } // end if

  } // end loop

  numberOfIterations = iterationCount;
}

//------------------------------------------------------------//
