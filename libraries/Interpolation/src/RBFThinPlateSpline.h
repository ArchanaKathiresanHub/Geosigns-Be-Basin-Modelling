#ifndef __RBFThinPlateSpline_HH__
#define __RBFThinPlateSpline_HH__

#include <cmath>
#include "Numerics.h"

namespace Numerics {

  /** @addtogroup Interpolation
   *
   * @{
   */

  /// @addtogroup RadialBasisFunctions
  /// @{

  /// \brief The ThinPlateSpline radial basis function \f$ \phi=r^2 log r \f$.
  class RBFThinPlateSpline {

  public :

    /// \brief Compute the basis function \f$ \phi=r^2 log r \f$.
    FloatingPoint operator ()( const FloatingPoint rSquared ) const;

  };

  /// @}
  /** @} */

}

inline Numerics::FloatingPoint Numerics::RBFThinPlateSpline::operator ()( const FloatingPoint rSquared ) const {

  if ( rSquared == 0.0 ) {
    return 0.0;
  } else {
    // use fact that log (a^b) = b * log a
    return 0.5 * rSquared * std::log ( rSquared );
  }

}


#endif // __RBFThinPlateSpline_HH__
