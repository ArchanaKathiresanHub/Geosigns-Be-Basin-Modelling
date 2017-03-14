//------------------------------------------------------------//

#ifndef __GenericTwoDArray_HH__
#define __GenericTwoDArray_HH__

//------------------------------------------------------------//

#include "Numerics.h"
#include <stdexcept>

//------------------------------------------------------------//

/** @addtogroup Common
 *
 * @{
 */

/// \file GenericTwoDArray.h
/// \brief Generic two dimensional array.

/// \brief Templated two dimensional array
template <typename DataType>
class GenericTwoDArray {

  /// \typedef DataTypeAccess
  /// \brief Define a pointer type to the template type parameter.
  typedef DataType* DataTypeAccess;

public :

  /// \brief Default constructor
  GenericTwoDArray ();


  /// \brief Size of each of the dimensions of the array. The default start value is 1.
  ///
  /// Create a two dimensional array with dimensions
  /// [1, dimension1]x[1, dimension2].
  /// For each dimension the end dimension must be less than or equal to 1.
  GenericTwoDArray ( const int dimension1,
                     const int dimension2 );

  /// \brief Start and end values of each of the dimensions of the array.
  ///
  /// Create a two dimensional array with dimensions
  /// [startDimension1, endDimension1]x[startDimension2, endDimension2].
  /// For each dimension the start must be less than or equal than the end value.
  GenericTwoDArray ( const int startDimension1,
                     const int endDimension1,
                     const int startDimension2,
                     const int endDimension2 );

  /// Copy constructor
  GenericTwoDArray ( const GenericTwoDArray& array );

  /// Destructor
  virtual ~GenericTwoDArray ();


  void setSize ( const int startDimension1,
                 const int endDimension1,
                 const int startDimension2,
                 const int endDimension2 );

  void setSize ( const int dimension1,
                 const int dimension2 );

  /// \brief The lower bound of the array
  ///
  ///
  /// \param dimension The dimension of the array for which the lower bound is sort
  ///
  /// If dimension in not in [ 1, 2 ] then ConstraintError will be thrown
  int first  ( const int dimension ) const;

  /// \brief The upper bound the array
  ///
  ///
  /// \param dimension The dimension of the array for which the upper bound is sort
  ///
  /// If dimension in not in [ 1, 2 ] then ConstraintError will be thrown
  int last   ( const int dimension ) const;

  /// \brief The number of elements stored in the array
  ///
  ///
  /// \param dimension The dimension of the array for which the length is sort
  ///
  /// Will return last ( dimension ) - first ( dimension ) + 1.
  /// If dimension in not in [ 1, 2 ] then ConstraintError will be thrown
  int length ( const int dimension ) const;

  /// The total number of elements stored in the array
  int numberOfEntries () const;


  /// Accessor
  const DataType& operator ()( const int position1,
                               const int position2 ) const;

  /// Accessor
  DataType& operator ()( const int position1,
                         const int position2 );

  /// Assignment operator
  GenericTwoDArray& operator= ( const GenericTwoDArray& array );

  /// Fills the array with constant value
  void fill ( const DataType& withTheValue );

  /// The low level one dimensional array
  DataType const* data () const;

  /// The low level one dimensional array
  DataType* data ();


  //------------------------------------------------------------//


protected :

  /// \brief Performs low level allocation of values.
  void initialise ( const int dimension1,
                    const int dimension2 );

  /// Low level array.
  DataType*   entries1D;

  /// Low level array.
  DataType**  entries2D;

  /// Start positions of the array.
  int startPosition [ 2 ];

  /// End positions of the array.
  int endPosition [ 2 ];

  /// Total number of values store in the array.
  int totalNumberOfEntries;

};

/** @} */

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>::GenericTwoDArray () {
  entries1D = (DataType*)(nullptr);
  entries2D = (DataType**)(nullptr);
  totalNumberOfEntries = 0;
}

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>::GenericTwoDArray ( const int dimension1,
                                               const int dimension2 ) {

  initialise ( dimension1, dimension2 );

  startPosition [ 0 ] = 1;
  startPosition [ 1 ] = 1;
  endPosition   [ 0 ] = dimension1;
  endPosition   [ 1 ] = dimension2;

  totalNumberOfEntries = dimension1 * dimension2;
}

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>::GenericTwoDArray ( const int startDimension1,
                                               const int endDimension1,
                                               const int startDimension2,
                                               const int endDimension2 ) {

  int dimension1 = endDimension1 - startDimension1 + 1;
  int dimension2 = endDimension2 - startDimension2 + 1;

  initialise ( dimension1, dimension2 );

  startPosition [ 0 ] = startDimension1;
  startPosition [ 1 ] = startDimension2;
  endPosition   [ 0 ] = endDimension1;
  endPosition   [ 1 ] = endDimension2;

  totalNumberOfEntries = dimension1 * dimension2;
}

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>::GenericTwoDArray ( const GenericTwoDArray& array ) {

  int I;
  int dimension1 = array.endPosition [ 0 ] - array.startPosition [ 0 ] + 1;
  int dimension2 = array.endPosition [ 1 ] - array.startPosition [ 1 ] + 1;

  initialise ( dimension1, dimension2 );

  startPosition [ 0 ] = array.startPosition [ 0 ];
  startPosition [ 1 ] = array.startPosition [ 1 ];
  endPosition   [ 0 ] = array.endPosition [ 0 ];
  endPosition   [ 1 ] = array.endPosition [ 1 ];
  totalNumberOfEntries = array.totalNumberOfEntries;

  // Copy all the data stored in the array
  for ( I = 0; I < totalNumberOfEntries; I++ ) {
    entries1D [ I ] = array.entries1D [ I ];
  }

}

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>::~GenericTwoDArray () {

  if ( entries1D != (DataType*)(nullptr)) {
    delete [] entries1D;
  }

  if ( entries2D != (DataType**)(nullptr)) {
    delete [] entries2D;
  }

}

//------------------------------------------------------------//

template<typename DataType>
void GenericTwoDArray<DataType>::initialise ( const int dimension1,
                                              const int dimension2 ) {

  int entryCount;
  int I;

  entryCount = dimension1 * dimension2;

  entries1D = new DataType [ entryCount ];
  entries2D = new DataTypeAccess [ dimension1 ];

  for ( I = 0; I < dimension1; I++ ) {
    entries2D [ I ] = &entries1D [ I * dimension2 ];
  }

}

//------------------------------------------------------------//

template<typename DataType>
void GenericTwoDArray<DataType>::setSize ( const int startDimension1,
                                           const int endDimension1,
                                           const int startDimension2,
                                           const int endDimension2 ) {

  int dimension1 = endDimension1 - startDimension1 + 1;
  int dimension2 = endDimension2 - startDimension2 + 1;

  initialise ( dimension1, dimension2 );

  startPosition [ 0 ] = startDimension1;
  startPosition [ 1 ] = startDimension2;
  endPosition   [ 0 ] = endDimension1;
  endPosition   [ 1 ] = endDimension2;

  totalNumberOfEntries = dimension1 * dimension2;
}

//------------------------------------------------------------//

template<typename DataType>
void GenericTwoDArray<DataType>::setSize ( const int dimension1,
                                           const int dimension2 ) {

  initialise ( dimension1, dimension2 );

  startPosition [ 0 ] = 1;
  startPosition [ 1 ] = 1;
  endPosition   [ 0 ] = dimension1;
  endPosition   [ 1 ] = dimension2;

  totalNumberOfEntries = dimension1 * dimension2;
}


//------------------------------------------------------------//

template<typename DataType>
int GenericTwoDArray<DataType>::first ( const int dimension ) const {

#ifdef __Constraint_Check__
   if( dimension <= 0 || dimension > 2 ) throw std::runtime_error("Out of bounds access");
#endif

  return startPosition [ dimension - 1 ];
}

//------------------------------------------------------------//

template<typename DataType>
int GenericTwoDArray<DataType>::last ( const int dimension ) const {

#ifdef __Constraint_Check__
   if( dimension <= 0 || dimension > 2 ) throw std::runtime_error("Out of bounds access");
#endif

  return endPosition [ dimension - 1 ];
}

//------------------------------------------------------------//

template<typename DataType>
int GenericTwoDArray<DataType>::length ( const int dimension ) const {

#ifdef __Constraint_Check__
   if( dimension <= 0 || dimension > 2 ) throw std::runtime_error("Out of bounds access");
#endif

  return endPosition [ dimension - 1 ] - startPosition [ dimension - 1 ] + 1;
}

//------------------------------------------------------------//

template<typename DataType>
int GenericTwoDArray<DataType>::numberOfEntries () const {
  return totalNumberOfEntries;
}

//------------------------------------------------------------//

template<typename DataType>
const DataType& GenericTwoDArray<DataType>::operator ()( const int position1,
                                                         const int position2 ) const {

#ifdef __Constraint_Check__
if (( position1 < startPosition [ 0 ] ) || ( position1 > endPosition [ 0 ] ) ||
   ( position2 < startPosition [ 1 ] ) || ( position2 > endPosition [ 1 ] )) {
   throw std::runtime_error("Out of bounds access");
}
#endif
      
  return entries2D [ position1 - startPosition [ 0 ]][ position2 - startPosition [ 1 ]];
}

//------------------------------------------------------------//

template<typename DataType>
DataType& GenericTwoDArray<DataType>::operator ()( const int position1,
                                                   const int position2 ) {

#ifdef __Constraint_Check__
if (( position1 < startPosition [ 0 ] ) || ( position1 > endPosition [ 0 ] ) ||
   ( position2 < startPosition [ 1 ] ) || ( position2 > endPosition [ 1 ] )) {
   throw std::runtime_error("Out of bounds access");
}
#endif
      
  return entries2D [ position1 - startPosition [ 0 ]][ position2 - startPosition [ 1 ]];
}

//------------------------------------------------------------//

template<typename DataType>
GenericTwoDArray<DataType>& GenericTwoDArray<DataType>::operator= ( const GenericTwoDArray& array ) {

  if ( this -> length ( 1 ) != array.length ( 1 ) || this -> length ( 2 ) != array.length ( 2 )) {
//     throw Exceptions::ConstraintError ( " array dimensions not equal ",
//                                         __LINE__, __FUNCTION__, __FILE__ );
  }

  int I;

  for ( I = 0; I < totalNumberOfEntries; I++ ) {
    entries1D [ I ] = array.entries1D [ I ];
  }

  return *this;
}

//------------------------------------------------------------//

template<typename DataType>
void GenericTwoDArray<DataType>::fill ( const DataType& withTheValue ) {

  int I;

  for ( I = 0; I < totalNumberOfEntries; I++ ) {
    entries1D [ I ] = withTheValue;
  }

}

//------------------------------------------------------------//

template<typename DataType>
DataType const* GenericTwoDArray<DataType>::data () const {
  return entries1D;
}

//------------------------------------------------------------//

template<typename DataType>
DataType* GenericTwoDArray<DataType>::data () {
  return entries1D;
}

//------------------------------------------------------------//

#endif // __GenericTwoDArray_HH__
