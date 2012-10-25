//------------------------------------------------------------//

#include "DenseMatrix.h"
#include <iostream>

//------------------------------------------------------------//

void Numerics::DenseMatrix::getSubMatrix ( const IntegerArray&  inds,
                                                 FloatingPoint*  subMatrixBuffer ) const {


  int i;
  int j;
  int count = 0;

  for ( i = 0; i < inds.size (); i++ ) {

    for ( j = 0; j < inds.size (); ++j, ++count ) {
      subMatrixBuffer [ count ] = operator ()( inds [ i ], inds [ j ]);
    }

  }

}

//------------------------------------------------------------//

void Numerics::DenseMatrix::print ( const std::string&  matrixName,
                                          std::ostream& output ) const {

  int i;
  int j;

  for ( i = 0; i < numberOfRows (); i++ ) {

    for ( j = 0; j < numberOfColumns (); j++ ) {
      output <<  matrixName << " ( " << i + 1 << ", " << j + 1 << " ) = " << operator ()( i, j ) << ";" << std::endl;
    }

  }

}

//------------------------------------------------------------//
