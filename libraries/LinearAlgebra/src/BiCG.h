#ifndef __Numerics_BiCG_HH__
#define __Numerics_BiCG_HH__

#include "Vector.h"
#include "DenseMatrix.h"
#include "Preconditioner.h"
#include "LinearSolver.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file BiCG.h
  /// \brief Implements the BiCG linear solver.

  /// \brief Implements the BiCG linear solver.
  class BiCG : public LinearSolver  {

  public:

    void operator ()( const Vector& rhs,
                            Vector& solution );


  };

  /** @} */

}

#endif // _Numerics_BiCG_HH__
