#ifndef _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_H_
#define _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_H_

#include "FiniteElementTypes.h"

namespace FiniteElementMethod {

   /// Functor class for evaluating basis functions and grad-basis functions on the reference element.
   ///
   /// The reference is the bi-unit cube: [-1,1]^3.
   class BasisFunction {

   public :

      /// Evaluate the basis function at the evaluation point (x,y,z).
      void operator ()( const double         xi,
                        const double         eta,
                        const double         zeta,
                              ElementVector& basis ) const;

      /// Evaluate the grad-basis function at the evaluation point (x,y,z).
      void operator ()( const double             xi,
                        const double             eta,
                        const double             zeta,
                              GradElementVector& gradBasis ) const;

      /// Evaluate both the basis and grad-basis function at the evaluation point (x,y,z).
      void operator ()( const double             xi,
                        const double             eta,
                        const double             zeta,
                              ElementVector&     basis,
                              GradElementVector& gradBasis ) const;

   };

}

#endif // _FINITE_ELEMENT_METHOD__BASIS_FUNCTION_H_
