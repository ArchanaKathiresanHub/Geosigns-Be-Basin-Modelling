//------------------------------------------------------------//

#include <cmath>

#include "FortranMatrix.h"
#include "BLASFunctions.h"

//------------------------------------------------------------//

Numerics::FortranMatrix::FortranMatrix ( const int rowSize,
                                         const int colSize ) {

  initialise ( rowSize, colSize );

}

//------------------------------------------------------------//

Numerics::FortranMatrix::FortranMatrix () {

  allColumns = 0;
  allValues  = 0;
  rows = -1;
  columns = -1;

}

//------------------------------------------------------------//

Numerics::FortranMatrix::FortranMatrix ( const FortranMatrix& mat ) {

  int i;
  int j;

  initialise ( mat.numberOfRows (), mat.numberOfColumns ());

  for ( i = 0; i < rows * columns; i++ ) {
    allValues [ i ] = mat.allValues [ i ];
  }

}

//------------------------------------------------------------//

Numerics::FortranMatrix::~FortranMatrix () {

  if ( allColumns != 0 ) {
    delete [] allColumns;
    allColumns = 0;
  }

  if ( allValues != 0 ) {
    delete [] allValues;
    allValues  = 0;
  }

  rows = -1;
  columns = -1;
}

//------------------------------------------------------------//

void Numerics::FortranMatrix::initialise ( const int rowSize,
                                           const int colSize ) {

  if ( rowSize < 0 || colSize < 0 ) {
  }

  int i;

  allColumns = new FloatingPoint* [ colSize ];
  allValues  = new FloatingPoint  [ rowSize * colSize ];

  rows = rowSize;
  columns = colSize;

  for ( i = 0; i < colSize; i++ ) {
    allColumns [ i ] = &allValues [ i * rowSize ];
  }

}

//------------------------------------------------------------//

void Numerics::FortranMatrix::fill ( const FloatingPoint withTheValue ) {

  int i;

  for ( i = 0; i < rows * columns; i++ ) {
    allValues [ i ] = withTheValue;
  }

}

//------------------------------------------------------------//

void Numerics::FortranMatrix::matrixVectorProduct ( const Vector& vec,
                                                          Vector& result ) const {

  BLAS::mvp ( false,
              rows,
              columns,
              allValues,
              vec.data (),
              result.data ());

}

//------------------------------------------------------------//

void Numerics::FortranMatrix::getSubMatrix ( const IntegerArray&  inds,
                                                   FloatingPoint*  subMatrixBuffer ) const {


  int i;
  int j;
  int count = 0;

  for ( i = 0; i < inds.size (); i++ ) {

    for ( j = 0; j < inds.size (); ++j, ++count ) {
      subMatrixBuffer [ count ] = allColumns [ inds [ j ]][ inds [ i ]];
    }

  }

}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::FortranMatrix::maxValue () const {

  FloatingPoint max = allValues [ 0 ];
  int i;

  for ( i = 1; i < rows * columns; ++i ) {
    max = floatingPointMax ( max, allValues [ i ]);
  }

  return max;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::FortranMatrix::maxAbsValue () const {

  FloatingPoint max = std::fabs ( allValues [ 0 ]);
  int i;

  for ( i = 1; i < rows * columns; ++i ) {
    max = floatingPointMax ( max, std::fabs ( allValues [ i ]));
  }

  return max;
}


//------------------------------------------------------------//
