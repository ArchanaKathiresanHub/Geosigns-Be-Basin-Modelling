#ifndef __Numerics_Preconditioner_HH__
#define __Numerics_Preconditioner_HH__

#include "Vector.h"
#include "DenseMatrix.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */

  /// \file Preconditioner.h
  /// \brief Adds a preconditioner class to the Numerics namespace.

  /// \brief Base preconditioner class, used by all the iterative solvers.
  ///
  /// An object of this class may be allocated, any such use results in 
  /// the equivalent of having the identity operator as the preconditioner.
  class Preconditioner {

  public :

    Preconditioner  () {}
    virtual ~Preconditioner  () {}

    /// \brief Assemble the preconditioner from the user supplied matrix.
    virtual void assemblePreconditioner ( const DenseMatrix& fromTheMatrix );

    /// \brief Solve the preconditioner problem.
    virtual void solve ( const Vector& theVector, 
                               Vector& result ) const;

    /// \brief Print values of the vector to the output stream.
    virtual void print ( const std::string&  preconditionerName,
			       std::ostream& output ) const;

  protected :

    /// \brief A reference to the matrix that is part of the system of equations being solved.
    DenseMatrix const* theMatrix;

  }; // end class Preconditioner


  /** @} */

}

#endif // __Numerics_Preconditioner_HH__
