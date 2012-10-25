#ifndef _CONSTRAINED_ARRAY__H_
#define _CONSTRAINED_ARRAY__H_


/// \brief A simple class providing a mechanism for constructing a small stack based array.
///
/// I.e. There is no explicit allocation performed in this class.
template<typename Type,
         const int FirstIndex,
         const int LastIndex>
class ConstrainedArray {

public :

   /// The number of iterms in the array.
   static const int Size = LastIndex - FirstIndex + 1;


   /// Simple constructor.
   ConstrainedArray ();


   /// \brief Get a const reference to the item at the position i.
   const Type& operator ()( const int i ) const;

   /// \brief Get a reference to the item at the position i.
   Type& operator ()( const int i );


   /// \brief The position of the first item in the array.
   int first () const;

   /// \brief The position of the last item in the array.
   int last () const;

   /// \brief The number of items in the the array.
   int size () const;

   /// \brief Fill array with a single value 
   void fill ( const Type& value );

private :

   /// The actual array of items.
   Type m_items [ Size ];

};

//------------------------------------------------------------//

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
ConstrainedArray<Type, FirstIndex, LastIndex>::ConstrainedArray () {
}

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
inline const Type& ConstrainedArray<Type, FirstIndex, LastIndex>::operator ()( const int i ) const {
   return m_items [ i - first ()];
}

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
inline Type& ConstrainedArray<Type, FirstIndex, LastIndex>::operator ()( const int i ) {
   return m_items [ i - first ()];
}

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
inline int ConstrainedArray<Type, FirstIndex, LastIndex>::first () const {
   return FirstIndex;
}

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
inline int ConstrainedArray<Type, FirstIndex, LastIndex>::last () const {
   return LastIndex;
}

template<typename Type,
         const int FirstIndex,
         const int LastIndex>
inline int ConstrainedArray<Type, FirstIndex, LastIndex>::size () const {
   return Size;
}


template<typename Type,
         const int FirstIndex,
         const int LastIndex>
void ConstrainedArray<Type, FirstIndex, LastIndex>::fill ( const Type& value ) {

   int i;

   for ( i = 0; i < Size; ++i ) {
      m_items [ i ] = value;
   }

}


#endif // _CONSTRAINED_ARRAY__H_
