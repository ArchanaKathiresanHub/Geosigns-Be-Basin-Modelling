//------------------------------------------------------------//

#include <cmath>
#include "GMRes.h"

//------------------------------------------------------------//


void Numerics::GMRes::operator ()
   ( const Vector& rhs,
           Vector& Solution ) {

  const int MaximumDimension = maximumNumberOfIterations + 2;
  const int dimension = theMatrix->numberOfRows ();

  int K;
  int iterationCount = 1;

  Vector R  ( dimension );
  Vector W  ( dimension );
  Vector workSpace2 ( dimension );
  Vector workSpace  ( dimension );
  Vector SN ( MaximumDimension );
  Vector S  ( MaximumDimension );
  Vector CS ( MaximumDimension );

  FloatingPoint residual;
  FloatingPoint normB = vectorLength ( rhs );
  FloatingPoint beta;

  VectorArray vectorSequence ( MaximumDimension + 3 );
  VectorArray hessenbergMatrix ( MaximumDimension + 3 );


  numberOfIterations = 0;

  if (normB == 0.0) {
    normB = 1.0;
  }

  normB = 1.0;

  //
  // This will save on a matrix vector product, if solution = 0.0 initially
  //
  if ( isZeroVector ( Solution )) {
    thePreconditioner->solve ( rhs, R );
  } else {
    theMatrix->matrixVectorProduct ( Solution, workSpace2 );
    vectorXMY ( rhs, workSpace2, workSpace );
    thePreconditioner->solve ( workSpace, R );
  }

  beta = vectorLength ( R );
  residual = beta / normB;

  if ( printResidual ) {
    std::cout << " resid ( " << iterationCount << " ) = " << residual << ";" << std::endl;
  }

  if ( residual <= solverTolerance ) {
    numberOfIterations = 0;
    return;
  }

  fill ( vectorSequence, 0 );
  fill ( hessenbergMatrix, 0 );

  vectorSequence   [ 1 ] = new Vector ( dimension );
  hessenbergMatrix [ 1 ] = new Vector ( 2 );

  vectorScale ( 1.0 / beta, R, *vectorSequence [ 1 ]);

  S.fill ( 0.0 );
  S ( 1 ) = beta;

  while (( residual >= solverTolerance ) && ( iterationCount <= maximumNumberOfIterations )) {

    vectorSequence   [ iterationCount + 1 ] = new Vector ( dimension );
    hessenbergMatrix [ iterationCount + 1 ] = new Vector ( iterationCount + 2 );

    theMatrix->matrixVectorProduct ( *vectorSequence [ iterationCount ], workSpace );
    thePreconditioner->solve ( workSpace, W );

    for ( K = 0; K < iterationCount; K++ ) {
      (*hessenbergMatrix [ iterationCount ])( K ) = innerProduct ( *vectorSequence [ K + 1 ], W );
      vectorAXPY ( -(*hessenbergMatrix [ iterationCount ])( K ), *vectorSequence [ K + 1 ], W );
    }

    (*hessenbergMatrix [ iterationCount ])( iterationCount ) = vectorLength ( W );

    vectorScale ( 1.0 / (*hessenbergMatrix [ iterationCount ])( iterationCount ), W, *vectorSequence [ iterationCount + 1 ]);

    for ( K = 0; K < iterationCount - 1; K++ ) {
      applyPlaneRotation ( (*hessenbergMatrix [ iterationCount ])( K ),
                           (*hessenbergMatrix [ iterationCount ])( K + 1 ),
                             CS ( K + 1 ), SN ( K + 1 ));
    }

    generatePlaneRotation ((*hessenbergMatrix  [ iterationCount ])( iterationCount - 1 ),
                           (*hessenbergMatrix  [ iterationCount ])( iterationCount ),
                            CS ( iterationCount ), SN ( iterationCount ));

    applyPlaneRotation    ((*hessenbergMatrix [ iterationCount ])( iterationCount - 1 ),
                           (*hessenbergMatrix [ iterationCount ])( iterationCount ),
                            CS ( iterationCount ), SN ( iterationCount ));

    applyPlaneRotation    ( S  ( iterationCount ),
                            S  ( iterationCount + 1 ),
                            CS ( iterationCount ),
                            SN ( iterationCount ));

    residual = std::fabs ( S ( iterationCount + 1 )) / normB;

    iterationCount = iterationCount + 1;

    if ( printResidual ) {
      std::cout << " resid ( " << iterationCount << " ) = " << residual << ";" << std::endl;
    }

  }

  update ( Solution, iterationCount - 1, hessenbergMatrix, S, vectorSequence, workSpace );

  for ( size_t I = 1; I < vectorSequence.size (); I++ ) {

    if ( vectorSequence [ I ] != 0 ) {
      delete vectorSequence [ I ];
      vectorSequence [ I ] = 0;
    }

    if ( hessenbergMatrix [ I ] != 0 ) {
      delete hessenbergMatrix [ I ];
      hessenbergMatrix [ I ] = 0;
    }

  }

  numberOfIterations = iterationCount;
}


//------------------------------------------------------------//

void Numerics::GMRes::update
   ( Vector&      solution,
     int          iterationCount,
     VectorArray& hessenbergMatrix,
     Vector&      S,
     VectorArray& vectorSequence,
     Vector&      workSpace ) {


  Vector y ( S );

  int i;
  int j;

  for ( i = iterationCount; i >= 1; i--) {
    y ( i ) = y ( i ) / (*hessenbergMatrix [ i ])( i - 1 );

    for ( j = i - 1; j >= 1; j-- ) {
      y ( j ) = y ( j ) - (*hessenbergMatrix [ i ])( j - 1 ) * y ( i );
    }

  }

  for ( j = 1; j <= iterationCount + 1; j++ ) {
    vectorScale ( y ( j ), *vectorSequence [ j ], workSpace );
    solution += workSpace;
  }

}


//------------------------------------------------------------//


void Numerics::GMRes::generatePlaneRotation
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


void Numerics::GMRes::applyPlaneRotation
   ( FloatingPoint& dx,
     FloatingPoint& dy,
     FloatingPoint  cs,
     FloatingPoint  sn ) {

  FloatingPoint temp  =  cs * dx + sn * dy;

  dy = -sn * dx + cs * dy;
  dx = temp;
}

//------------------------------------------------------------//

void Numerics::GMRes::fill ( VectorArray& vecs, 
                             Vector*      value ) const {

  size_t i;

  for ( i = 0; i < vecs.size (); i++ ){
    vecs [ i ] = value;
  }

}

//------------------------------------------------------------//
