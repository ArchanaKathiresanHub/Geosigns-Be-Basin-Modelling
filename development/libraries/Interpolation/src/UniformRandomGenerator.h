//------------------------------------------------------------//

#ifndef __UniformRandomGenerator_HH__
#define __UniformRandomGenerator_HH__

//------------------------------------------------------------//

#include <stdlib.h>

//------------------------------------------------------------//

#include "Numerics.h"

//------------------------------------------------------------//

namespace Numerics {


  /** @addtogroup Numerics
   *
   * @{
   */

  /// \file UniformRandomGenerator.h
  /// \brief Provides a simple uniform pseudo-random number generator.

  /// \brief Implementation of a a simple uniform pseudo-random number generator.
  ///
  /// Returns result of the UNIX drand48 function.
  class UniformRandomGenerator {

  public :

    /// \brief Compute next random number in sequence.
    ///
    /// Result lies in half-open domain [0,1).
    FloatingPoint operator ()();

  };

  /** @} */

}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::UniformRandomGenerator::operator ()() {
  return drand48 ();
}

//------------------------------------------------------------//

#endif // __UniformRandomGenerator_HH__

//------------------------------------------------------------//
