#ifndef __Numerics_HH__
#define __Numerics_HH__

/*! \defgroup Numerics Numerics
 *
 */

#include <vector>

namespace Numerics {

  /** @addtogroup Numerics
   *
   * @{
   */

  /// \namespace Numerics
  /// \brief Namespace of classes, functions, ... that are involved in numerical calculations.

  /// \file Numerics.h
  /// \brief Some basic types and simeple functions used throughout the library.

  /// \typedef FloatingPoint
  /// \brief The floating point type used throughout the code.
  // To use single precision floating point numbers all the code MUST be
  // capable of handling them. Ther may be some problem in the HDF file output.
  // So DO NOT use single precision floating point numbers for now.
  #ifdef FLOATINGPOINTISSINGLE
  typedef float FloatingPoint;
  #else
  typedef double FloatingPoint;
  #endif

  /// \typedef FloatingPointArray
  /// \brief The floating point array type used throughout the code.
  typedef std::vector<FloatingPoint> FloatingPointArray;

  /// \typedef IntegerArray
  /// \brief The integer array type used throughout the code.
  typedef std::vector<int> IntegerArray;

  /// \brief Returns the maximum of two integers.
  int integerMax ( const int i1, const int i2 );

  /// \brief Returns the minimum of two integers.
  int integerMin ( const int i1, const int i2 );

  /// \brief Returns the maximum of two floating point numbers.
  FloatingPoint floatingPointMax ( const FloatingPoint x1, const FloatingPoint x2 );

  /// \brief Returns the minimum of two floating point numbers.
  FloatingPoint floatingPointMin ( const FloatingPoint x1, const FloatingPoint x2 );



  /** @} */

}

//------------------------------------------------------------//

inline int Numerics::integerMax ( const int i1, const int i2 ) {

  if ( i1 > i2 ) {
    return i1;
  } else {
    return i2;
  }

}

//------------------------------------------------------------//

inline int Numerics::integerMin ( const int i1, const int i2 ) {

  if ( i1 < i2 ) {
    return i1;
  } else {
    return i2;
  }

}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::floatingPointMax ( const FloatingPoint x1, const FloatingPoint x2 ) {

  if ( x1 > x2 ) {
    return x1;
  } else {
    return x2;
  }

}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::floatingPointMin ( const FloatingPoint x1, const FloatingPoint x2 ) {

  if ( x1 < x2 ) {
    return x1;
  } else {
    return x2;
  }

}

//------------------------------------------------------------//

#endif // __Numerics_HH__
