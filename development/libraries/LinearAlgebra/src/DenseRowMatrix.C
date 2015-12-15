//------------------------------------------------------------//

#include "DenseRowMatrix.h"
#include <iostream>

//------------------------------------------------------------//

Numerics::DenseRowMatrix::DenseRowMatrix ( const int rowCount,
                                           const int colCount ) : allRows ( rowCount ) {

  int i;

  rows = rowCount;
  columns = colCount;

  for ( i = 0; i < rows; i++ ) {
    allRows [ i ] = new Vector ( columns );
  }

}

//------------------------------------------------------------//

Numerics::DenseRowMatrix::~DenseRowMatrix () {

  int i;


  for ( i = 0; i < rows; i++ ) {
    delete allRows [ i ];
  }

}

//------------------------------------------------------------//

void Numerics::DenseRowMatrix::fill ( const FloatingPoint withTheValue ) {

  int i;

  for ( i = 0; i < rows; i++ ) {
    allRows [ i ]->fill ( withTheValue );
  }

}

//------------------------------------------------------------//

void Numerics::DenseRowMatrix::matrixVectorProduct ( const Vector& vec,
                                                           Vector& result ) const {

  int i;

  for ( i = 0; i < rows; i++ ) {
    result ( i ) = innerProduct ( vec, *allRows [ i ]);
  }

}

//------------------------------------------------------------//

void Numerics::DenseRowMatrix::getSubMatrix ( const IntegerArray&  inds,
                                                    FloatingPoint*  subMatrixBuffer ) const {


  int i;
  int j;
  int count = 0;
  Vector* row;

  for ( i = 0; i < inds.size (); i++ ) {
    row = allRows [ inds  [ i ]];

    for ( j = 0; j < inds.size (); ++j, ++count ) {
      subMatrixBuffer [ count ] = (*row)( inds [ j ]);
    }

  }

}

// //------------------------------------------------------------//

// Numerics::DenseRowMatrix::ResizableVector::ResizableVector () {
//   values = 0;
//   vectorDimension = -1;
// }

// //------------------------------------------------------------//

// void Numerics::DenseRowMatrix::ResizableVector::setSize ( const int size ) {
//   values = new FloatingPoint [ size ];
//   vectorDimension = size;
// }

// //------------------------------------------------------------//

// bool Numerics::DenseRowMatrix::ResizableVector::isNull () const {
//   return values == 0;
// }

// //------------------------------------------------------------//
