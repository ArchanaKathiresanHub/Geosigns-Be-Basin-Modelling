#ifndef __RBFCubic_HH__
#define __RBFCubic_HH__

#include <cmath>
#include "Numerics.h"

namespace Numerics {

  /** @addtogroup Interpolation
   *
   * @{
   */

  /** 
   * \defgroup RadialBasisFunctions Basis Functions
   **/


  /// @addtogroup RadialBasisFunctions
  /// @{

  /// \brief The Cubic radial basis function \f$ \phi=r^3 \f$.
  class RBFCubic {

  public :

    /// \brief Compute the basis function \f$ \phi=r^3 \f$.
    FloatingPoint operator ()( const FloatingPoint rSquared ) const;

  };

  /// @}
  /** @} */

}

inline Numerics::FloatingPoint Numerics::RBFCubic::operator ()( const FloatingPoint rSquared ) const {

  // r^3 = r^2 * sqrt ( r^2 )
  return rSquared * std::sqrt ( rSquared );
}


#endif // __RBFCubic_HH__
