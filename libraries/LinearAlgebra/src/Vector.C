//------------------------------------------------------------//

#include "Vector.h"
#include "BLASFunctions.h"

#include <cmath>
#include <iostream>

//------------------------------------------------------------//

Numerics::Vector::Vector () {
  values = 0;
  vectorDimension = -1;
}

//------------------------------------------------------------//

Numerics::Vector::Vector ( const int size ) {

  if ( size <= 0 ) {
  }

  values = new FloatingPoint [ size ];
  vectorDimension = size;
}

//------------------------------------------------------------//

Numerics::Vector::Vector ( const Vector& vec ) { // : Vector ( vec.dimension ()) {

  values = new FloatingPoint [ vec.dimension () ];
  vectorDimension = vec.dimension ();

  copy ( vec );
}

//------------------------------------------------------------//

Numerics::Vector::~Vector () {

  if ( values != 0 ) {
    delete [] values;
    vectorDimension = -1;
  }

}

//------------------------------------------------------------//

void Numerics::Vector::copy ( const Vector& vec ) {

  BLAS::copy ( vectorDimension, vec.values, values );

  // Should replace this code with the appropriate BLAS function call

//   int i;

//   for ( i = 0; i < vectorDimension; i++ ) {
//     values [ i ] = vec.values [ i ];
//   }

}

//------------------------------------------------------------//

void Numerics::Vector::resize ( const int newSize ) {

  if ( vectorDimension != newSize ) {

    if ( values != 0 ) {
      delete [] values;
    }

    values = new FloatingPoint [ newSize ];
    vectorDimension = newSize;
  }

}

//------------------------------------------------------------//

void Numerics::Vector::fill ( const FloatingPoint withTheValue ) {

  int i;

  for ( i = 0; i < vectorDimension; i++ ) {
    values [ i ] = withTheValue;
  }

}

//------------------------------------------------------------//

Numerics::Vector& Numerics::Vector::operator += ( const Vector& vec ) {

  BLAS::axpy ( vectorDimension, 1.0, vec.data (), values );

  // Should replace this code with the appropriate BLAS function call

//   int i;

//   for ( i = 0; i < vectorDimension; ++i ) {
//     values [ i ] += vec.values [ i ];
//   }

  return *this;
}

//------------------------------------------------------------//

Numerics::Vector& Numerics::Vector::operator -= ( const Vector& vec ) {

  BLAS::axpy ( vectorDimension, -1.0, vec.data (), values );

//   // Should replace this code with the appropriate BLAS function call

//   int i;

//   for ( i = 0; i < vectorDimension; ++i ) {
//     values [ i ] -= vec.values [ i ];
//   }

  return *this;
}

//------------------------------------------------------------//

Numerics::Vector& Numerics::Vector::operator *= ( const FloatingPoint scalar ) {

  BLAS::scale ( vectorDimension, scalar, values );

  // Should replace this code with the appropriate BLAS function call

//   int i;

//   for ( i = 0; i < vectorDimension; ++i ) {
//     values [ i ] *= scalar;
//   }

  return *this;
}

//------------------------------------------------------------//

Numerics::Vector& Numerics::Vector::operator= ( const Vector& vec ) {

  if ( this != &vec ) {
    copy ( vec );
  }

  return *this;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::innerProduct ( const Vector& u,
                                                 const Vector& v ) {

  return BLAS::dot ( u.dimension (), u.data (), v.data ());

  // Should replace this code with the appropriate BLAS function call

//   FloatingPoint result = 0.0;
//   int i;

//   for ( i = 0; i < v1.dimension (); i++ ) {
//     result += v1 ( i ) * v2 ( i );
//   }

//   return result;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::vectorLength ( const Vector& v ) {

  return std::sqrt ( innerProduct ( v, v ));

}

//------------------------------------------------------------//

bool Numerics::isZeroVector ( const Vector& v ) {

  int i;

  for ( i = 0; i < v.dimension (); ++i ) {

    if ( v ( i ) != 0.0 ) {
      return false;
    }

  }

  return true;
}

//------------------------------------------------------------//

void Numerics::vectorAXPY ( const FloatingPoint A,
                            const Vector&       X,
                                  Vector&       Y ) {


  BLAS::axpy ( X.dimension (), A, X.data (), Y.data ());

  // Should replace this code with the appropriate BLAS function call

//   int i;

//   for ( i = 0; i < X.dimension (); ++i ) {
//     Y ( i ) = A * X ( i ) + Y ( i );
//   }

}

//------------------------------------------------------------//


void Numerics::vectorXMY ( const Vector& X,
                                 Vector& Y ) {

  int i;

  for ( i = 0; i < X.dimension (); ++i ) {
    Y ( i ) = X ( i ) - Y ( i );
  }

}

//------------------------------------------------------------//

void Numerics::vectorYMX ( const Vector& X,
                                 Vector& Y ) {

  int i;

  for ( i = 0; i < X.dimension (); ++i ) {
    Y ( i ) = Y ( i ) - X ( i );
  }

}

//------------------------------------------------------------//

void Numerics::vectorXMY ( const Vector& X,
                           const Vector& Y,
                                 Vector& Z ) {

  int i;

  for ( i = 0; i < X.dimension (); ++i ) {
    Z ( i ) = X ( i ) - Y ( i );
  }

}

//------------------------------------------------------------//

void Numerics::vectorScale ( const FloatingPoint a,
                                   Vector&       x ) {

  int i;

  if ( a != 1.0 ) {
    BLAS::scale ( x.dimension (), a, x.data());
  }

//   } else if ( A == -1.0 ) {

//     for ( i = 0; i < X.dimension (); ++i ) {
//       X ( i ) = -X ( i );
//     }

//   } else {

//     for ( i = 0; i < X.dimension (); ++i ) {
//       X ( i ) = A * X ( i );
//     }

//   }

}

//------------------------------------------------------------//

void Numerics::vectorScale ( const FloatingPoint a,
                             const Vector&       x,
                                   Vector&       y ) {

  int i;


  y = x;

  if ( a != 1.0 ) {
    BLAS::scale ( y.dimension (), a, y.data());
  }

//   if ( A == 1.0 ) {

//     Y = X;

//   } else if ( A == -1.0 ) {

//     for ( i = 0; i < X.dimension (); ++i ) {
//       Y ( i ) = -X ( i );
//     }

//   } else {

//     for ( i = 0; i < X.dimension (); ++i ) {
//       Y ( i ) = A * X ( i );
//     }

//   }

}

//------------------------------------------------------------//

void Numerics::Vector::print ( const std::string&  vectorName,
                                     std::ostream& output ) const {

  int i;

  for ( i = 0; i < dimension (); i++ ) {
    output <<  vectorName << " ( " << i + 1 << " ) = " << operator ()( i ) << ";" << std::endl;
  }

}

//------------------------------------------------------------//
