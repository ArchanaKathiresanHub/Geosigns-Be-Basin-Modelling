#ifndef _FASTCAULDRON__GENERIC_FORTRAN_MATRIX__H_
#define _FASTCAULDRON__GENERIC_FORTRAN_MATRIX__H_

#include <stdlib.h>

/// A very simple stack based matrix used in testing.
template<const int GenericSize>
class GenericFortranMatrix {

public :

   GenericFortranMatrix ();

   double operator ()( const int i, const int j ) const;

   double& operator ()( const int i, const int j );


   void zero ();

   void randomise ();

   void transpose ();

   void identity ();

   void setDiag ( const double value );

   GenericFortranMatrix& operator*=( const double scale );

   GenericFortranMatrix& operator+=( const GenericFortranMatrix& mat );

   GenericFortranMatrix& operator=( const GenericFortranMatrix& mat );



   double* data ();

   int size () const;


private :

   double* columns [ GenericSize ];
   double  values  [ GenericSize * GenericSize ];

};

template<const int GenericSize>
GenericFortranMatrix<GenericSize>::GenericFortranMatrix () {

   int i;
   int position;

   for ( i = 0, position = 0; i < GenericSize; ++i, position += GenericSize ) {
      columns [ i ] = &values [ position ];
   }

}


template<const int GenericSize>
inline double GenericFortranMatrix<GenericSize>::operator ()( const int i, const int j ) const {
   return columns [ j - 1 ][ i - 1 ];
}

template<const int GenericSize>
inline double& GenericFortranMatrix<GenericSize>::operator ()( const int i, const int j ) {
   return columns [ j - 1 ][ i - 1 ];
}

template<const int GenericSize>
void GenericFortranMatrix<GenericSize>::zero () {

   int i;

   for ( i = 0; i < GenericSize * GenericSize; ++i ) {
      values [ i ] = 0.0;
   } 

}

template<const int GenericSize>
void GenericFortranMatrix<GenericSize>::randomise () {

   int i;

   for ( i = 0; i < GenericSize * GenericSize; ++i ) {
      values [ i ] = drand48 ();
   } 

}

template<const int GenericSize>
void GenericFortranMatrix<GenericSize>::transpose () {

   int i;
   int j;
   double temp;

   for ( i = 1; i <= GenericSize; ++i ) {

      for ( j = 1; j < i; ++j ) {
         temp = operator ()( i, j );
         operator ()( i, j ) = operator ()( j, i );
         operator ()( j, i ) = temp;
      }

   }

}

template<const int GenericSize>
void GenericFortranMatrix<GenericSize>::identity () {

   int i;

   zero ();

   for ( i = 1; i <= GenericSize; ++i ) {
      operator ()( i, i ) = 1.0;
   }


}

template<const int GenericSize>
void GenericFortranMatrix<GenericSize>::setDiag ( const double value ) {

   int i;

   for ( i = 1; i <= GenericSize; ++i ) {
      operator ()( i, i ) = value;
   } 

}


template<const int GenericSize>
GenericFortranMatrix<GenericSize>& GenericFortranMatrix<GenericSize>::operator*=( const double scale ) {

   int i;
   
   for ( i = 0; i < GenericSize * GenericSize; ++i ) {
      values [ i ] *= scale;
   } 

   return *this;
}


template<const int GenericSize>
GenericFortranMatrix<GenericSize>& GenericFortranMatrix<GenericSize>::operator+=( const GenericFortranMatrix& mat ) {

   int i;
   
   for ( i = 0; i < GenericSize * GenericSize; ++i ) {
      values [ i ] += mat.values [ i ];
   } 

   return *this;
}

template<const int GenericSize>
GenericFortranMatrix<GenericSize>& GenericFortranMatrix<GenericSize>::operator=( const GenericFortranMatrix& mat ) {

   int i;
   
   for ( i = 0; i < GenericSize * GenericSize; ++i ) {
      values [ i ] = mat.values [ i ];
   } 

   return *this;
}

template<const int GenericSize>
inline double* GenericFortranMatrix<GenericSize>::data () {
   return &values [ 0 ];
}

template<const int GenericSize>
inline int GenericFortranMatrix<GenericSize>::size () const {
   return GenericSize;
}


#endif // _FASTCAULDRON__GENERIC_FORTRAN_MATRIX__H_
