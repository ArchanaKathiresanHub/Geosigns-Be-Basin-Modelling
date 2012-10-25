#ifndef __Numerics_RichardsonIteration_HH__
#define __Numerics_RichardsonIteration_HH__

#include "Vector.h"
#include "DenseMatrix.h"
#include "Preconditioner.h"
#include "LinearSolver.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file RichardsonIteration.h
  /// \brief Implements the Richardson iteration linear solver.

  /// \brief Implements the Richardson iteration linear solver.
  class RichardsonIteration : public LinearSolver  {

  public:

    /// \brief Solve linear system using Richardson iteration scheme.
    void operator ()( const Vector& rhs,
                            Vector& solution );


  };

  /** @} */

}

#endif // _Numerics_RichardsonIteration_HH__
