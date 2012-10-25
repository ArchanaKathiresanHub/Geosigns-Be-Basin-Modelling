#ifndef __Numerics_LinearSolver_HH__
#define __Numerics_LinearSolver_HH__

#include "Vector.h"
#include "DenseMatrix.h"
#include "Preconditioner.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file LinearSolver.h
  /// \brief Provides the base linear solver class

  /// \brief The default maximum number of iterations.
  static const int DefaultMaximumNumberOfIterations = 1000;

  /// \brief The default linear solver tolerance.
  static const double DefaultSolverTolerance = 1.0e-5;


  /// \brief The linear solver base class.
  class LinearSolver {

  public :

    LinearSolver ();

    virtual ~LinearSolver () {}

    /// \brief Set the matrix to be used by the linear solver.
    void setMatrix ( const DenseMatrix& mat );

    /// \brief Set the preconditioner to be used by the linear solver.
    void setPreconditioner ( const Preconditioner& newPreconditioner );


    /// \brief Set the tolerance to be used by the linear solver.
    void setTolerance ( const double newTolerance );

    /// \brief Get the tolerance to be used by the linear solver.
    FloatingPoint getTolerance () const;


    /// \brief Set the maximum number of iterations that may be performed by linear solver.
    void setMaximumNumberOfIterations ( const int newMaxiumIterations );

    /// \brief Get the maximum number of iterations that may be performed by linear solver.
    int getMaximumNumberOfIterations () const;


    /// \brief Set whether or not residuals are to be output.
    void setPrintResidual ( const bool newPrintResidual );

    /// \brief Get current value of output option.
    bool getPrintResidual () const;


    /// \brief Solve system of linear equations.
    virtual void operator ()( const Vector& rhs,
                                    Vector& solution ) = 0;

    /// \brief Return the number of iterations taken in previous linear solve.
    ///
    /// If no solve has taken place, or the tolerance, maximum number of iterations, matrix 
    /// or preconditioner has changed the number of iterations taken will be reset to -1.
    int iterationsTaken () const;


  protected :

    /// \brief Indicates whether or not to print the residual.
    ///
    /// This is a user definable field.
    bool   printResidual;

    /// \brief The maximum number of iterations that may be performed.
    ///
    /// This is a user definable field.
    int    maximumNumberOfIterations;

    /// \brief The number of iterations that the last linear solve took.
    ///
    /// MUST be updated by the linear solver at end of solving process.
    int    numberOfIterations;

    /// \brief The tolerance to which the linear solver should converge.
    double solverTolerance;

    /// \brief The matrix to solve for.
    DenseMatrix    const* theMatrix;

    /// \brief The preconditioner used as part of the linear solve.
    Preconditioner const* thePreconditioner;

  };

  /** @} */

}

//------------------------------------------------------------//


#endif // __Numerics_LinearSolver_HH__
