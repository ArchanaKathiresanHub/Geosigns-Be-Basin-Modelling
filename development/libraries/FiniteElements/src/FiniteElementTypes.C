#include "FiniteElementTypes.h"

#include <string>
#include <stdlib.h>

#include "NumericFunctions.h"
#include "FiniteElement.h"

void FiniteElementMethod::ThreeVector::put ( const std::string& name,
                                             std::ostream&      o  ) const {

   o << name << " ( 1 ) = " << m_entries [ 0 ] << std::endl;
   o << name << " ( 2 ) = " << m_entries [ 1 ] << std::endl;
   o << name << " ( 3 ) = " << m_entries [ 2 ] << std::endl;
}


void FiniteElementMethod::Matrix3x3::put ( const std::string& name,
                                     std::ostream&      o ) const {

   o << name << " ( 1, 1 ) = " << m_entries [ 0 ][ 0 ] << std::endl;
   o << name << " ( 1, 2 ) = " << m_entries [ 0 ][ 1 ] << std::endl;
   o << name << " ( 1, 3 ) = " << m_entries [ 0 ][ 2 ] << std::endl;

   o << name << " ( 2, 1 ) = " << m_entries [ 1 ][ 0 ] << std::endl;
   o << name << " ( 2, 2 ) = " << m_entries [ 1 ][ 1 ] << std::endl;
   o << name << " ( 2, 3 ) = " << m_entries [ 1 ][ 2 ] << std::endl;

   o << name << " ( 3, 1 ) = " << m_entries [ 2 ][ 0 ] << std::endl;
   o << name << " ( 3, 2 ) = " << m_entries [ 2 ][ 1 ] << std::endl;
   o << name << " ( 3, 3 ) = " << m_entries [ 2 ][ 2 ] << std::endl;

}


void FiniteElementMethod::invert ( const Matrix3x3& Mat,
                                         Matrix3x3& Inverse ) {

  double Det_Inverse = 1.0 / determinant ( Mat );

  double A11 = Mat ( 1, 1 );
  double A12 = Mat ( 1, 2 );
  double A13 = Mat ( 1, 3 );

  double A21 = Mat ( 2, 1 );
  double A22 = Mat ( 2, 2 );
  double A23 = Mat ( 2, 3 );

  double A31 = Mat ( 3, 1 );
  double A32 = Mat ( 3, 2 );
  double A33 = Mat ( 3, 3 );


  Inverse ( 1, 1 ) = ( A22 * A33 - A23 * A32 ) * Det_Inverse;
  Inverse ( 1, 2 ) = ( A13 * A32 - A12 * A33 ) * Det_Inverse;
  Inverse ( 1, 3 ) = ( A12 * A23 - A13 * A22 ) * Det_Inverse;

  Inverse ( 2, 1 ) = ( A23 * A31 - A21 * A33 ) * Det_Inverse;
  Inverse ( 2, 2 ) = ( A11 * A33 - A13 * A31 ) * Det_Inverse;
  Inverse ( 2, 3 ) = ( A13 * A21 - A11 * A23 ) * Det_Inverse;

  Inverse ( 3, 1 ) = ( A21 * A32 - A22 * A31 ) * Det_Inverse;
  Inverse ( 3, 2 ) = ( A12 * A31 - A11 * A32 ) * Det_Inverse;
  Inverse ( 3, 3 ) = ( A11 * A22 - A12 * A21 ) * Det_Inverse;

}


//------------------------------------------------------------//

FiniteElementMethod::Matrix3x3& FiniteElementMethod::Matrix3x3::operator*= ( const double scalar ) {

   m_entries [ 0 ][ 0 ] *= scalar;
   m_entries [ 0 ][ 1 ] *= scalar;
   m_entries [ 0 ][ 2 ] *= scalar;

   m_entries [ 1 ][ 0 ] *= scalar;
   m_entries [ 1 ][ 1 ] *= scalar;
   m_entries [ 1 ][ 2 ] *= scalar;

   m_entries [ 2 ][ 0 ] *= scalar;
   m_entries [ 2 ][ 1 ] *= scalar;
   m_entries [ 2 ][ 2 ] *= scalar;

   return *this;
}

//------------------------------------------------------------//

void FiniteElementMethod::matrixVectorProduct ( const Matrix3x3&   Mat,
                                                const ThreeVector& Vec,
                                                      ThreeVector& Result ) {

  Result ( 1 ) = Mat ( 1, 1 ) * Vec ( 1 ) + Mat ( 1, 2 ) * Vec ( 2 ) + Mat ( 1, 3 ) * Vec ( 3 );
  Result ( 2 ) = Mat ( 2, 1 ) * Vec ( 1 ) + Mat ( 2, 2 ) * Vec ( 2 ) + Mat ( 2, 3 ) * Vec ( 3 );
  Result ( 3 ) = Mat ( 3, 1 ) * Vec ( 1 ) + Mat ( 3, 2 ) * Vec ( 2 ) + Mat ( 3, 3 ) * Vec ( 3 );

}

//------------------------------------------------------------//

FiniteElementMethod::ThreeVector FiniteElementMethod::matrixVectorProduct ( const Matrix3x3&   mat,
                                                                            const ThreeVector& vec ) {

   ThreeVector result;

   result ( 1 ) = mat ( 1, 1 ) * vec ( 1 ) + mat ( 1, 2 ) * vec ( 2 ) + mat ( 1, 3 ) * vec ( 3 );
   result ( 2 ) = mat ( 2, 1 ) * vec ( 1 ) + mat ( 2, 2 ) * vec ( 2 ) + mat ( 2, 3 ) * vec ( 3 );
   result ( 3 ) = mat ( 3, 1 ) * vec ( 1 ) + mat ( 3, 2 ) * vec ( 2 ) + mat ( 3, 3 ) * vec ( 3 );

   return result;
}

//------------------------------------------------------------//

void FiniteElementMethod::matrixTransposeVectorProduct ( const Matrix3x3&   Mat,
                                                   const ThreeVector& Vec,
                                                         ThreeVector& Result ) {

  Result ( 1 ) = Mat ( 1, 1 ) * Vec ( 1 ) + Mat ( 2, 1 ) * Vec ( 2 ) + Mat ( 3, 1 ) * Vec ( 3 );
  Result ( 2 ) = Mat ( 1, 2 ) * Vec ( 1 ) + Mat ( 2, 2 ) * Vec ( 2 ) + Mat ( 3, 2 ) * Vec ( 3 );
  Result ( 3 ) = Mat ( 1, 3 ) * Vec ( 1 ) + Mat ( 2, 3 ) * Vec ( 2 ) + Mat ( 3, 3 ) * Vec ( 3 );

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixMatrixProduct ( const Matrix3x3& Left,
                                                const Matrix3x3& Right,
                                                      Matrix3x3& Result ) {

  Result ( 1, 1 ) = Left ( 1, 1 ) * Right ( 1, 1 ) + Left ( 1, 2 ) * Right ( 2, 1 ) + Left ( 1, 3 ) * Right ( 3, 1 );
  Result ( 1, 2 ) = Left ( 1, 1 ) * Right ( 1, 2 ) + Left ( 1, 2 ) * Right ( 2, 2 ) + Left ( 1, 3 ) * Right ( 3, 2 );
  Result ( 1, 3 ) = Left ( 1, 1 ) * Right ( 1, 3 ) + Left ( 1, 2 ) * Right ( 2, 3 ) + Left ( 1, 3 ) * Right ( 3, 3 );

  Result ( 2, 1 ) = Left ( 2, 1 ) * Right ( 1, 1 ) + Left ( 2, 2 ) * Right ( 2, 1 ) + Left ( 2, 3 ) * Right ( 3, 1 );
  Result ( 2, 2 ) = Left ( 2, 1 ) * Right ( 1, 2 ) + Left ( 2, 2 ) * Right ( 2, 2 ) + Left ( 2, 3 ) * Right ( 3, 2 );
  Result ( 2, 3 ) = Left ( 2, 1 ) * Right ( 1, 3 ) + Left ( 2, 2 ) * Right ( 2, 3 ) + Left ( 2, 3 ) * Right ( 3, 3 );

  Result ( 3, 1 ) = Left ( 3, 1 ) * Right ( 1, 1 ) + Left ( 3, 2 ) * Right ( 2, 1 ) + Left ( 3, 3 ) * Right ( 3, 1 );
  Result ( 3, 2 ) = Left ( 3, 1 ) * Right ( 1, 2 ) + Left ( 3, 2 ) * Right ( 2, 2 ) + Left ( 3, 3 ) * Right ( 3, 2 );
  Result ( 3, 3 ) = Left ( 3, 1 ) * Right ( 1, 3 ) + Left ( 3, 2 ) * Right ( 2, 3 ) + Left ( 3, 3 ) * Right ( 3, 3 );

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixMatrixTransposeProduct ( const Matrix3x3& Left,
                                                         const Matrix3x3& Right,
                                                               Matrix3x3& Result ) {

  Result ( 1, 1 ) = Left ( 1, 1 ) * Right ( 1, 1 ) + Left ( 1, 2 ) * Right ( 1, 2 ) + Left ( 1, 3 ) * Right ( 1, 3 );
  Result ( 1, 2 ) = Left ( 1, 1 ) * Right ( 2, 1 ) + Left ( 1, 2 ) * Right ( 2, 2 ) + Left ( 1, 3 ) * Right ( 2, 3 );
  Result ( 1, 3 ) = Left ( 1, 1 ) * Right ( 3, 1 ) + Left ( 1, 2 ) * Right ( 3, 2 ) + Left ( 1, 3 ) * Right ( 3, 3 );

  Result ( 2, 1 ) = Left ( 2, 1 ) * Right ( 1, 1 ) + Left ( 2, 2 ) * Right ( 1, 2 ) + Left ( 2, 3 ) * Right ( 1, 3 );
  Result ( 2, 2 ) = Left ( 2, 1 ) * Right ( 2, 1 ) + Left ( 2, 2 ) * Right ( 2, 2 ) + Left ( 2, 3 ) * Right ( 2, 3 );
  Result ( 2, 3 ) = Left ( 2, 1 ) * Right ( 3, 1 ) + Left ( 2, 2 ) * Right ( 3, 2 ) + Left ( 2, 3 ) * Right ( 3, 3 );

  Result ( 3, 1 ) = Left ( 3, 1 ) * Right ( 1, 1 ) + Left ( 3, 2 ) * Right ( 1, 2 ) + Left ( 3, 3 ) * Right ( 1, 3 );
  Result ( 3, 2 ) = Left ( 3, 1 ) * Right ( 2, 1 ) + Left ( 3, 2 ) * Right ( 2, 2 ) + Left ( 3, 3 ) * Right ( 2, 3 );
  Result ( 3, 3 ) = Left ( 3, 1 ) * Right ( 3, 1 ) + Left ( 3, 2 ) * Right ( 3, 2 ) + Left ( 3, 3 ) * Right ( 3, 3 );

}


//------------------------------------------------------------//

void FiniteElementMethod::matrixTransposeMatrixProduct ( const Matrix3x3& Left,
                                                         const Matrix3x3& Right,
                                                               Matrix3x3& Result ) {

  Result ( 1, 1 ) = Left ( 1, 1 ) * Right ( 1, 1 ) + Left ( 2, 1 ) * Right ( 2, 1 ) + Left ( 3, 1 ) * Right ( 3, 1 );
  Result ( 1, 2 ) = Left ( 1, 1 ) * Right ( 1, 2 ) + Left ( 2, 1 ) * Right ( 2, 2 ) + Left ( 3, 1 ) * Right ( 3, 2 );
  Result ( 1, 3 ) = Left ( 1, 1 ) * Right ( 1, 3 ) + Left ( 2, 1 ) * Right ( 2, 3 ) + Left ( 3, 1 ) * Right ( 3, 3 );

  Result ( 2, 1 ) = Left ( 1, 2 ) * Right ( 1, 1 ) + Left ( 2, 2 ) * Right ( 2, 1 ) + Left ( 3, 2 ) * Right ( 3, 1 );
  Result ( 2, 2 ) = Left ( 1, 2 ) * Right ( 1, 2 ) + Left ( 2, 2 ) * Right ( 2, 2 ) + Left ( 3, 2 ) * Right ( 3, 2 );
  Result ( 2, 3 ) = Left ( 1, 2 ) * Right ( 1, 3 ) + Left ( 2, 2 ) * Right ( 2, 3 ) + Left ( 3, 2 ) * Right ( 3, 3 );

  Result ( 3, 1 ) = Left ( 1, 3 ) * Right ( 1, 1 ) + Left ( 2, 3 ) * Right ( 2, 1 ) + Left ( 3, 3 ) * Right ( 3, 1 );
  Result ( 3, 2 ) = Left ( 1, 3 ) * Right ( 1, 2 ) + Left ( 2, 3 ) * Right ( 2, 2 ) + Left ( 3, 3 ) * Right ( 3, 2 );
  Result ( 3, 3 ) = Left ( 1, 3 ) * Right ( 1, 3 ) + Left ( 2, 3 ) * Right ( 2, 3 ) + Left ( 3, 3 ) * Right ( 3, 3 );

}


//------------------------------------------------------------//

double FiniteElementMethod::determinant ( const Matrix3x3& M ) {

  double Det;

  Det = M ( 1, 1 ) * ( M ( 2, 2 ) * M ( 3, 3 ) - M ( 2, 3 ) * M ( 3, 2 )) +
        M ( 2, 1 ) * ( M ( 1, 3 ) * M ( 3, 2 ) - M ( 1, 2 ) * M ( 3, 3 )) +
        M ( 3, 1 ) * ( M ( 1, 2 ) * M ( 2, 3 ) - M ( 2, 2 ) * M ( 1, 3 ));

  return Det;
}

//============================================================//

void FiniteElementMethod::ElementVector::zero () {
  fill ( 0.0 );
}

//------------------------------------------------------------//
double FiniteElementMethod::ElementGeometryMatrix::getMidPoint() {

   int I, J;
   FiniteElement fe;
   ElementVector ve;

   fe.setGeometry( * this );
   fe.setQuadraturePoint( 0, 0, 0 );

   for ( I = 1, J = 0; I <= 8; ++ I, ++ J ) {
      ve(I) =  m_entries[ 2 ][ J ];
   }
   return fe.interpolate( ve );
}
//------------------------------------------------------------//


void FiniteElementMethod::ElementVector::operator=( const ElementVector& Vec ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    this->operator ()( I ) = Vec ( I );
  }

//    m_entries [ 0 ] = Vec.m_entries [ 0 ];
//    m_entries [ 1 ] = Vec.m_entries [ 1 ];
//    m_entries [ 2 ] = Vec.m_entries [ 2 ];
//    m_entries [ 3 ] = Vec.m_entries [ 3 ];
//    m_entries [ 4 ] = Vec.m_entries [ 4 ];
//    m_entries [ 5 ] = Vec.m_entries [ 5 ];
//    m_entries [ 6 ] = Vec.m_entries [ 6 ];
//    m_entries [ 7 ] = Vec.m_entries [ 7 ];
}

//------------------------------------------------------------//

void FiniteElementMethod::scale (       ElementMatrix& Result,
                                 const double          Factor ) {

  int i;

  for ( i = 1; i <= 8 ; ++i ) {
     Result ( 1, i ) *= Factor;
     Result ( 2, i ) *= Factor;
     Result ( 3, i ) *= Factor;
     Result ( 4, i ) *= Factor;
     Result ( 5, i ) *= Factor;
     Result ( 6, i ) *= Factor;
     Result ( 7, i ) *= Factor;
     Result ( 8, i ) *= Factor;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementVector::put ( const std::string& name,
                                               std::ostream&      o ) const {

  int I;

  for ( I = 0; I < 8; I++ ) {
    o << name << " (" << I + 1 << " ) = " << m_entries [ I ] << ";" << std::endl;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementVector::fill ( const double With_The_Value ) {

  int I;

  for ( I = 0; I < 8; I++ ) {
    m_entries [ I ] = With_The_Value;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementVector::Randomise () {

  int I;

  for ( I = 0; I < 8; I++ ) {
    m_entries [ I ] = drand48 ();
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const ElementVector& Left,
     const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Left ( I ) + Right ( I );
  }

  /// Try this with all loops below. Or would it be better to un-roll the loop totally?
  /// and use the implementation definition. result.values [ 0 ] = left.values [ 0 ] + right.values [ 0 ].
//    for ( I = 1; I <= 8; I += 4 ) {
//      Result ( I ) = Left ( I ) + Right ( I );
//      Result ( I + 1 ) = Left ( I + 1 ) + Right ( I + 1 );
//      Result ( I + 2 ) = Left ( I + 2 ) + Right ( I + 2 );
//      Result ( I + 3 ) = Left ( I + 3 ) + Right ( I + 3 );
//    }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const ElementVector& Left,
     const double         Right_Scale,
     const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Left ( I ) + Right_Scale * Right ( I );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const double         Left_Scale,
     const ElementVector& Left,
     const double         Right_Scale,
     const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Left_Scale * Left ( I ) + Right_Scale * Right ( I );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::subtract
   ( const ElementVector& Left,
     const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Left ( I ) - Right ( I );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::Increment
   ( const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Result ( I ) + Right ( I );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::Increment
   ( const double     Right_Scale,
     const ElementVector& Right,
           ElementVector& Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Result ( I ) + Right_Scale * Right ( I );
  }

}

//------------------------------------------------------------//

double FiniteElementMethod::length ( const ElementVector& Vec ) {

  double Result = 0.0;
  int         I;

  for ( I = 1; I <= 8; I++ ) {
    Result = Result + Vec ( I ) * Vec ( I );
  }

  return sqrt ( Result );
}

//------------------------------------------------------------//

double FiniteElementMethod::maxValue ( const ElementVector& Vec ) {

  double result = Vec ( 1 );
  int         i;

  for ( i = 2; i <= 8; ++i ) {
    result = NumericFunctions::Maximum ( result, Vec ( i ));
  }

  return result;
}

//------------------------------------------------------------//

FiniteElementMethod::ElementVector FiniteElementMethod::maximum ( const ElementVector& vec, 
                                                                  const double         scalar ) {

   ElementVector result;
   int i;

   for ( i = 1; i <= ElementVector::NumberOfEntries; ++i ) {
      result ( i ) = NumericFunctions::Maximum<double>( vec ( i ), scalar );
   }

   return result;
}

//------------------------------------------------------------//

FiniteElementMethod::ElementVector FiniteElementMethod::maximum ( const ElementVector& vec1,
                                                                  const ElementVector& vec2 ) {

   ElementVector result;
   int i;

   for ( i = 1; i <= ElementVector::NumberOfEntries; ++i ) {
      result ( i ) = NumericFunctions::Maximum<double>( vec1 ( i ), vec2 ( i ));
   }

   return result;
}

//------------------------------------------------------------//

FiniteElementMethod::ElementVector FiniteElementMethod::minimum ( const ElementVector& vec1,
                                                                  const ElementVector& vec2 ) {

   ElementVector result;
   int i;

   for ( i = 1; i <= ElementVector::NumberOfEntries; ++i ) {
      result ( i ) = NumericFunctions::Minimum<double>( vec1 ( i ), vec2 ( i ));
   }

   return result;
}

//------------------------------------------------------------//

void FiniteElementMethod::interpolate ( const ElementVector& start,
                                        const ElementVector& end,
                                        const double         lambda,
                                              ElementVector& result ) {

   int i;

   for ( i = 1; i <= ElementVector::NumberOfEntries; ++i ) {
      result ( i ) = ( 1.0 - lambda ) * start ( i ) + lambda * end ( i );
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::GradElementVector::zero () {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      m_entries [ i ][ 0 ] = 0.0;
      m_entries [ i ][ 1 ] = 0.0;
      m_entries [ i ][ 2 ] = 0.0;
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::GradElementVector::put ( const std::string& name,
                                                   std::ostream&      o ) const {

  int I;
  int J;

  for ( I = 0; I < 8; I++ ) {

    for ( J = 0; J < 3; J++ ) {
       o << name << " (" << I + 1 << ", " << J + 1 << " ) = " << m_entries [ I ][ J ] << ";" << std::endl;
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const GradElementVector& Left,
     const GradElementVector& Right,
           GradElementVector& Result ) {

  int i;

  for ( i = 1; i <= 8; ++i ) {
     Result ( i, 1 ) = Left ( i, 1 ) + Right ( i, 1 );
     Result ( i, 2 ) = Left ( i, 2 ) + Right ( i, 2 );
     Result ( i, 3 ) = Left ( i, 3 ) + Right ( i, 3 );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const GradElementVector& Left,
     const double             Right_Scale,
     const GradElementVector& Right,
           GradElementVector& Result ) {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      Result ( i, 1 ) = Left ( i, 1 ) + Right_Scale * Right ( i, 1 );
      Result ( i, 2 ) = Left ( i, 2 ) + Right_Scale * Right ( i, 2 );
      Result ( i, 3 ) = Left ( i, 3 ) + Right_Scale * Right ( i, 3 );
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const double             Left_Scale,
     const GradElementVector& Left,
     const double             Right_Scale,
     const GradElementVector& Right,
           GradElementVector& Result ) {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      Result ( i, 1 ) = Left_Scale * Left ( i, 1 ) + Right_Scale * Right ( i, 1 );
      Result ( i, 2 ) = Left_Scale * Left ( i, 2 ) + Right_Scale * Right ( i, 2 );
      Result ( i, 3 ) = Left_Scale * Left ( i, 3 ) + Right_Scale * Right ( i, 3 );
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::subtract
   ( const GradElementVector& Left,
     const GradElementVector& Right,
           GradElementVector& Result ) {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      Result ( i, 1 ) = Left ( i, 1 ) - Right ( i, 1 );
      Result ( i, 2 ) = Left ( i, 2 ) - Right ( i, 2 );
      Result ( i, 3 ) = Left ( i, 3 ) - Right ( i, 3 );
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixVectorProduct
   ( const GradElementVector& Left,
     const ThreeVector&       Right,
           ElementVector&     Result ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Left ( I, 1 ) * Right ( 1 ) + Left ( I, 2 ) * Right ( 2 ) + Left ( I, 3 ) * Right ( 3 );
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixTransposeVectorProduct
   ( const GradElementVector& Left,
     const ElementVector&     Right,
           ThreeVector&       Result  ) {

  int I;
  int J;
  double IP;


  for ( I = 1; I <= 3; I++ ) {
    IP = 0.0;

    for ( J = 1; J <= 8; J++ ) {
      IP = IP + Left ( J, I ) * Right ( J );
    }

    Result ( I ) = IP;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixMatrixProduct
   ( const GradElementVector& Left,
     const Matrix3x3&         Right,
           GradElementVector& Result  ) {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      Result ( i, 1 ) = Left ( i, 1 ) * Right ( 1, 1 ) + Left ( i, 2 ) * Right ( 2, 1 ) + Left ( i, 3 ) * Right ( 3, 1 );
      Result ( i, 2 ) = Left ( i, 1 ) * Right ( 1, 2 ) + Left ( i, 2 ) * Right ( 2, 2 ) + Left ( i, 3 ) * Right ( 3, 2 );
      Result ( i, 3 ) = Left ( i, 1 ) * Right ( 1, 3 ) + Left ( i, 2 ) * Right ( 2, 3 ) + Left ( i, 3 ) * Right ( 3, 3 );
   }

}

//------------------------------------------------------------//

void FiniteElementMethod::matrixMatrixTransposeProduct
   ( const GradElementVector& Left,
     const Matrix3x3&         Right,
           GradElementVector& Result  ) {

   int i;

   for ( i = 1; i <= 8; ++i ) {
      Result ( i, 1 ) = Left ( i, 1 ) * Right ( 1, 1 ) + Left ( i, 2 ) * Right ( 1, 2 ) + Left ( i, 3 ) * Right ( 1, 3 );
      Result ( i, 2 ) = Left ( i, 1 ) * Right ( 2, 1 ) + Left ( i, 2 ) * Right ( 2, 2 ) + Left ( i, 3 ) * Right ( 2, 3 );
      Result ( i, 3 ) = Left ( i, 1 ) * Right ( 3, 1 ) + Left ( i, 2 ) * Right ( 3, 2 ) + Left ( i, 3 ) * Right ( 3, 3 );
   }

}

//============================================================//

FiniteElementMethod::ElementMatrix::ElementMatrix () {
  m_entries [ 0 ] = &m_allEntries [  0 ];
  m_entries [ 1 ] = &m_allEntries [  8 ];
  m_entries [ 2 ] = &m_allEntries [ 16 ];
  m_entries [ 3 ] = &m_allEntries [ 24 ];
  m_entries [ 4 ] = &m_allEntries [ 32 ];
  m_entries [ 5 ] = &m_allEntries [ 40 ];
  m_entries [ 6 ] = &m_allEntries [ 48 ];
  m_entries [ 7 ] = &m_allEntries [ 56 ];
}

//------------------------------------------------------------//

FiniteElementMethod::ElementMatrix::~ElementMatrix () {
}

//------------------------------------------------------------//

void FiniteElementMethod::ElementMatrix::zero () {
  fill ( 0.0 );
}

//------------------------------------------------------------//

void FiniteElementMethod::ElementMatrix::put ( const std::string& name,
                                               std::ostream&      o ) const {

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
       o << name << " (" << I + 1 << ", " << J + 1 << " ) = " << m_entries [ I ][ J ] << ";" << std::endl;
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementMatrix::fill ( const double With_The_Value ) {

  int I;

  for ( I = 0; I < 64; I++ ) {
    m_allEntries [ I ] = With_The_Value;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementMatrix::Randomise () {

  int I;

  for ( I = 0; I < 64; I++ ) {
    m_allEntries [ I ] = drand48 ();
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::scale (       ElementVector& Result,
                                  const double          Factor ) {

  int I;

  for ( I = 1; I <= 8; I++ ) {
    Result ( I ) = Result ( I ) * Factor;
  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const ElementMatrix& Left,
     const ElementMatrix& Right,
           ElementMatrix& Result ) {

  // For this and the other ElementMatrix operations need to call the blas functions.

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left ( I, J ) + Right ( I, J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const ElementMatrix& Left,
     const double         Right_Scale,
     const ElementMatrix& Right,
           ElementMatrix& Result ) {

  int I;
  int J;


  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left ( I, J ) + Right_Scale * Right ( I, J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::add
   ( const double         Left_Scale,
     const ElementMatrix& Left,
     const double         Right_Scale,
     const ElementMatrix& Right,
           ElementMatrix& Result ) {

  int I;
  int J;


  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left_Scale * Left ( I, J ) + Right_Scale * Right ( I, J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::subtract
   ( const ElementMatrix& Left,
     const ElementMatrix& Right,
           ElementMatrix& Result ) {

  int I;
  int J;


  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left ( I, J ) - Right ( I, J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::OuterProduct
   ( const ElementVector& Left,
     const ElementVector& Right,
           ElementMatrix& Result ) {

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left ( I ) * Right ( J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::addOuterProduct
   ( const ElementVector& Left,
     const ElementVector& Right,
           ElementMatrix& Result ) {

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Result ( I, J ) + Left ( I ) * Right ( J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::addOuterProduct
   ( const double         scale,
     const ElementVector& Left,
     const ElementVector& Right,
           ElementMatrix& Result ) {

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Result ( I, J ) + scale * Left ( I ) * Right ( J );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::OuterProduct
   ( const GradElementVector& Left,
     const GradElementVector& Right,
           ElementMatrix&     Result ) {

  int I;
  int J;

  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      Result ( I, J ) = Left ( I, 1 ) * Right ( J, 1 ) + Left ( I, 2 ) * Right ( J, 2 ) + Left ( I, 3 ) * Right ( J, 3 );
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::addOuterProduct
   ( const double             scale,
     const GradElementVector& Left,
     const GradElementVector& Right,
           ElementMatrix&     Result ) {

  int         I;
  int         J;
  double IP;


  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      IP = Left ( I, 1 ) * Right ( J, 1 ) + Left ( I, 2 ) * Right ( J, 2 ) + Left ( I, 3 ) * Right ( J, 3 );
      Result ( I, J ) = Result ( I, J ) + scale * IP;
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::addOuterProduct
   ( const GradElementVector& Left,
     const GradElementVector& Right,
           ElementMatrix&     Result ) {

  int    I;
  int    J;
  double IP;


  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      IP = Left ( I, 1 ) * Right ( J, 1 ) + Left ( I, 2 ) * Right ( J, 2 ) + Left ( I, 3 ) * Right ( J, 3 );
      Result ( I, J ) = Result ( I, J ) + IP;
    }

  }

}

//============================================================//

FiniteElementMethod::ElementGeometryMatrix::ElementGeometryMatrix () {

  m_entries [ 0 ] = &m_allEntries [ 0 ];
  m_entries [ 1 ] = &m_allEntries [ 8 ];
  m_entries [ 2 ] = &m_allEntries [ 16 ];

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementGeometryMatrix::put ( const std::string& name,
                                                       std::ostream&      o ) const {

  int I;
  int J;

  for ( I = 0; I < 3; I++ ) {

    for ( J = 0; J < 8; J++ ) {
      o << name << " (" << I + 1 << ", " << J + 1 << " ) = " << m_entries [ I ][ J ] << ";" << std::endl;
    }

  }

}

//------------------------------------------------------------//

void FiniteElementMethod::ElementGeometryMatrix::zero () {

  int i;

  for ( i = 0; i < 8; ++i ) {
     m_entries [ 0 ][ i ] = 0.0;
     m_entries [ 1 ][ i ] = 0.0;
     m_entries [ 2 ][ i ] = 0.0;
  }

}

//------------------------------------------------------------//

#if 0
std::ostream& FiniteElementMethod::operator<< ( std::ostream& o, const ElementGeometryMatrix& Mat ) {

  int I;
  int J;

  for ( I = 1; I <= 3; I++ ) {

    for ( J = 1; J <= 8; J++ ) {
      o << Mat ( I, J ) << "  ";
    }

    o << std::endl;
  }

  return o;
}
#endif

//------------------------------------------------------------//

void FiniteElementMethod::matrixMatrixProduct ( const ElementGeometryMatrix& Geometry_Matrix,
                                                const GradElementVector&     Grad_Basis,
                                                      Matrix3x3&             Jacobian ) {

   int i;
   // int j;
   int k;

   double innerProduct1;
   double innerProduct2;
   double innerProduct3;

   for ( i = 1; i <= 3; ++i ) {

      // for ( J = 1; J <= 3; J++ ) {
      innerProduct1 = 0.0;
      innerProduct2 = 0.0;
      innerProduct3 = 0.0;

      for ( k = 1; k <= 8; ++k ) {
         innerProduct1 += Geometry_Matrix ( i, k ) * Grad_Basis ( k, 1 );
         innerProduct2 += Geometry_Matrix ( i, k ) * Grad_Basis ( k, 2 );
         innerProduct3 += Geometry_Matrix ( i, k ) * Grad_Basis ( k, 3 );
      }

      Jacobian ( i, 1 ) = innerProduct1;
      Jacobian ( i, 2 ) = innerProduct2;
      Jacobian ( i, 3 ) = innerProduct3;
      // }

   }

}

//============================================================//


bool FiniteElementMethod::BooleanVector::orValues () const {

  int I;

  for ( I = 0; I < 8; I++ ) {

    if ( m_entries [ I ]) { 
       // if any value is true then the result is true.
      return true;
    }

  }

  // All values must be false.
  return false;
}

//------------------------------------------------------------//


void FiniteElementMethod::BooleanVector::fill ( const bool With_The_Value ) {

  int I;

  for ( I = 0; I < 8; I++ ) {
    m_entries [ I ] = With_The_Value;
  }

}

//------------------------------------------------------------//


void FiniteElementMethod::BooleanVector::put () const {

  int I;

  for ( I = 0; I < 8; I++ ) {

    if ( m_entries [ I ]) {
      std::cout << "true  ";
    } else {
      std::cout << "false ";
    }

  }

  std::cout << std::endl;

}


//============================================================//
