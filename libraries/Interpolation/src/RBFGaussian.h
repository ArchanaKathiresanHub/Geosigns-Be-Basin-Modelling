#ifndef __RBFGaussian_HH__
#define __RBFGaussian_HH__

#include <cmath>

#include "Numerics.h"

namespace Numerics {

  /** @addtogroup Interpolation
   *
   * @{
   */

  /// @addtogroup RadialBasisFunctions
  /// @{

  /// \brief The gaussian radial basis function \f$ \phi=exp ( -a^2 \cdot r^2 ) \f$.
  class RBFGaussian {

  public :

    /// \brief Compute the basis function.
    FloatingPoint operator ()( const FloatingPoint rSquared ) const;

    /// \brief Set the \f$a\f$ in the function \f$ \phi=exp ( -a^2 \cdot r^2 ) \f$
    void setParameter ( const FloatingPoint newParameter );

  protected :

    /// \brief Scaling coefficient of the separation distance squared.
    FloatingPoint parameter;

    /// \brief The square of the scaling coefficient.
    FloatingPoint parameterSquared;

  };

  /// @}
  /** @} */

}

inline Numerics::FloatingPoint Numerics::RBFGaussian::operator ()( const FloatingPoint rSquared ) const {
  return std::exp ( -parameterSquared * rSquared );
}


inline void Numerics::RBFGaussian::setParameter ( const FloatingPoint newParameter ) {

  parameter = newParameter;
  parameterSquared = newParameter * newParameter;

}


#endif // __RBFGaussian_HH__
