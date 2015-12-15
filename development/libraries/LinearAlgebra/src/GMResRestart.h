#ifndef __Numerics_GMResRestart_HH__
#define __Numerics_GMResRestart_HH__

#include "Vector.h"
#include "DenseMatrix.h"
#include "Preconditioner.h"
#include "LinearSolver.h"
#include "FortranMatrix.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file GMResRestart.h
  /// \brief Implements the GMRes(n), GMRes with restart, linear solver.


  /// \brief Implements the generalised minimum residual iterative solver with restart.
  class GMResRestart : public LinearSolver  {

  public:

    /// \var DefaultRestartValue
    /// \brief The default restart value.
    static const int DefaultRestartValue = 100;

    GMResRestart ();

    /// Solve using GMRes(s).
    void operator ()( const Vector& rhs,
                            Vector& solution );

    /// \brief Enables setting of the restart value.
    ///
    /// \warning This value must be greater than 0.
    void setRestartValue ( const int newRestartValue );

  protected :

    void fill ( VectorArray& vecs, 
                Vector*      value ) const;

    void update
       ( Vector&        solution,
         int            iterationCount, 
         FortranMatrix& hessenbergMatrix,
         Vector&        s,
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
 
    int restartValue;

  };


  /** @} */

}

#endif // __Numerics_GMResRestart_HH__
