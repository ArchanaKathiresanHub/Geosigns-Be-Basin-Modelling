#ifndef __Numerics_GMRes_HH__
#define __Numerics_GMRes_HH__

#include "Vector.h"
#include "DenseMatrix.h"
#include "Preconditioner.h"
#include "LinearSolver.h"

namespace Numerics {
  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file GMRes.h
  /// \brief Implements the GMRes linear solver.


  /// \brief Implements the generalised minimum residual iterative solver.
  class GMRes : public LinearSolver  {

  public:

    /// \brief Solve using GMRes.
    void operator ()( const Vector& rhs,
                            Vector& solution );

  protected :

    void fill ( VectorArray& vecs, 
                Vector*      value ) const;

    void update
       ( Vector&        solution,
         int            iterationCount,
         VectorArray&   hessenbergMatrix,
         Vector&        S,
         VectorArray&   vectorSequence,
         Vector&        workSpace );

    void generatePlaneRotation
       ( FloatingPoint  dx, 
         FloatingPoint  dy, 
         FloatingPoint& cs, 
         FloatingPoint& sn );

    void applyPlaneRotation
       ( FloatingPoint& dx, 
         FloatingPoint& dy, 
         FloatingPoint  cs,  
         FloatingPoint  sn );
 

  };


  /** @} */

}
//------------------------------------------------------------//


#endif // __Numerics_GMRes_HH__
