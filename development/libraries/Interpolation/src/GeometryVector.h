//------------------------------------------------------------//

#ifndef __Numerics__GeometryVector_HH__
#define __Numerics__GeometryVector_HH__

//------------------------------------------------------------//

#include "Numerics.h"
#include <stdexcept>

//------------------------------------------------------------//

namespace Numerics {

  /** @addtogroup Numerics
   *
   * @{
   */

  /// \file GeometryVector.h
  /// \brief A simple vector.

  /// \brief A vector for geometry operations.
  class GeometryVector {

  public :

    /// \brief The dimension of the system
    ///
    /// At some point this could be a template parameter enabling 
    /// points of different dimension.
    enum { DIMENSION = 3 };


    /// \name Accessors
    /// @{

    /// \brief Returns the value at the subscript.
    FloatingPoint  operator ()( const int coord ) const;

    /// \brief Returns a reference to the value at the subscript.
    FloatingPoint& operator ()( const int coord );

    /// @}

    /// \brief Return the low level array containing the values.
    const FloatingPoint* data () const;

  private :

    friend class Point;

    /// \brief Contains the values.
    FloatingPoint values [ DIMENSION ];

  };

  /** @} */

}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::GeometryVector::operator ()( const int coord ) const {
#ifndef NDEBUG
   if( coord < 0 || coord >= DIMENSION ) throw std::runtime_error("Out of bounds access");
#endif
  return values [ coord ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint& Numerics::GeometryVector::operator ()( const int coord ) {
#ifndef NDEBUG
   if( coord < 0 || coord >= DIMENSION ) throw std::runtime_error("Out of bounds access");
#endif
  return values [ coord ];
}

//------------------------------------------------------------//

inline const Numerics::FloatingPoint* Numerics::GeometryVector::data () const {
  return values;
}

//------------------------------------------------------------//

#endif // __Numerics__GeometryVector_HH__

//------------------------------------------------------------//
