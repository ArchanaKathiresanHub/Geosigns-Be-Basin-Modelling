#ifndef FASTCAULDRON__SIMPLE_BOUNDED_ARRAY_H
#define FASTCAULDRON__SIMPLE_BOUNDED_ARRAY_H

/// \brief A simple array class that whose size is bounded by the template parameter Size.
///
/// An instantiation of this class can be used to also instantiate the PetscBlockVector class.
template <const unsigned int Size>
class SimpleBoundedArray {

public :

   /// \brief The number of values stored in the array.
   static const unsigned int Dimension = Size;

   static const int BlockSize = Size;

   /// \brief Return the value at position i.
   double operator []( const unsigned int i ) const;

   /// \brief Return the value at position i.
   double& operator []( const unsigned int i );


private :

   double m_values [ Size ];

}; 


template <const unsigned int Size>
inline double SimpleBoundedArray<Size>::operator []( const unsigned int i ) const {

// #ifdef DEBUG
//    if ( i >= Size ) {
//       // Error
//    }
// #endif

   return m_values [ i ];
}

template <const unsigned int Size>
inline double& SimpleBoundedArray<Size>::operator []( const unsigned int i ) {

// #ifdef DEBUG
//    if ( i >= Size ) {
//       // Error
//    }
// #endif

   return m_values [ i ];
}


#endif // FASTCAULDRON__SIMPLE_BOUNDED_ARRAY_H
