#ifndef __Numerics_PThreadsApproximateCardinalPreconditioner_HH__
#define __Numerics_PThreadsApproximateCardinalPreconditioner_HH__

#include <vector>

#include "Vector.h"
#include "DenseMatrix.h"
#include "ApproximateCardinalFunction.h"
#include "ApproximateCardinalPreconditioner.h"
#include "RBFInterpolant.h"
#include "Rendezvous.h"

namespace Numerics {

  /** @addtogroup Interpolation
   *
   * @{
   */

  /// \file PThreadsApproximateCardinalPreconditioner.h
  /// \brief Adds a parallel approximate cardinal preconditioner class to the Numerics namespace.

  /// \brief Contains the limits of a slice of a cardinal function array.
  struct CardinalFunctionSequence {

    /// \brief Start position of the sequence.
    int rowStart;

    /// \brief End position of the sequence.
    int rowEnd;

    /// \brief The degree of the polynomial used in the interpolation.
    int polynomialDegree;

    /// \brief The number of neighbours used by the approximate cardinal functions.
    int numberOfNeighbours;

    /// \brief interpolationPoints will alias the interpolation points from the preconditioner.
    PointArray const* interpolationPoints;

    /// \brief cardinalFunctions will alias the cardinal functions array in the cardinal function preconditioner.
    ApproximateCardinalFunctionArray* cardinalFunctions;

    /// \brief Conputes the approximate cardinal functions for the specified interval.
    void assemblePreconditioner ( const DenseMatrix& theMatrix );

    /// \brief Solves the cardinal functions for the specified interval.
    void solve ( const Vector& vec,
                       Vector& result );

  };

  /// \brief The active process for computing and solving the preconditioner.
  class CardinalFunctionSequenceThread {

    /// \enum PreconditionerOperations
    enum PreconditionerOperations { ASSEMBLY_START, /*!< Indicates that the preconditioner assembly process should begin.  */
                                    PRECONDITIONER_START, /*!< Indicates that the preconditioner solving process should begin. */
                                    PRECONDITIONER_DONE /*!< Indicates that the process that started before has complete.   */,
                                    TERMINATE /*!< indicates that the process should be terminated. */ };

  public :

    CardinalFunctionSequenceThread ( const CardinalFunctionSequence& data );

    ~CardinalFunctionSequenceThread ();

    void startAssembly ( const DenseMatrix& fromTheMatrix );

    void startSolve ( const Vector& vec,
                            Vector& result );

    void done  ();

  private :

    ///
    void threadStartAssembly ( DenseMatrix*& fromTheMatrix );

    void threadStartSolve ( Vector*& vec,
                            Vector*& result );

    void threadDone ();

    CardinalFunctionSequence cardinalFunctions;

    POSIXThreads::Rendezvous1<PreconditionerOperations> selection;
    POSIXThreads::Rendezvous1<DenseMatrix*> acceptStartAssembly;
    POSIXThreads::Rendezvous2<Vector*, Vector*> acceptStart;
    POSIXThreads::Rendezvous acceptDone;
    pthread_t threadID;

    static void*  preconditionerSolveThreadFunction ( void* globalData );

  };


  /// \brief Implements a parallel preconditioner based on approximate cardinal functions.
  template <class RadialBasisFunction>
  class PThreadsApproximateCardinalPreconditioner : public ApproximateCardinalPreconditioner<RadialBasisFunction> {

  public :

    /// \brief Constructor.
    ///
    /// @param interp The interpolant ..
    /// @param newNumberOfNeighbours The number of points to be used in the approximate cardinal functions.
    /// @param nThreads The number of threads to be used in assembly and solving.
    PThreadsApproximateCardinalPreconditioner ( const RBFInterpolant<RadialBasisFunction>& interp,
                                                const int         newNumberOfNeighbours,
                                                const int         nThreads );

    ~PThreadsApproximateCardinalPreconditioner ();

    /// \brief Compute the approximate cardinal function preconditioner.
    void assemblePreconditioner ( const DenseMatrix& fromTheMatrix ); 

    /// \brief Solve preconditioner problem.
    ///
    /// Basically this is just a sparse matrix vector product.
    void solve ( const Vector& theVector, 
                       Vector& result ) const;

  private :

    typedef std::vector<CardinalFunctionSequenceThread*> CardinalFunctionSequenceThreadArray;

    CardinalFunctionSequenceThreadArray partitionedCardinalFunctions;

  };

  /** @} */

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::PThreadsApproximateCardinalPreconditioner<RadialBasisFunction>::PThreadsApproximateCardinalPreconditioner
    ( const RBFInterpolant<RadialBasisFunction>& interp,
      const int         newNumberOfNeighbours,
      const int         nThreads ) :

  ApproximateCardinalPreconditioner<RadialBasisFunction> ( interp, newNumberOfNeighbours ) {

  const int rowStep = ( interp.getInterpolationPoints ().size () + nThreads ) / nThreads;

  size_t i;
  CardinalFunctionSequence partitionData;

  partitionData.cardinalFunctions = &this->cardinalFunctions;
  partitionData.numberOfNeighbours = ApproximateCardinalPreconditioner<RadialBasisFunction>::numberOfNeighbours;
  partitionData.polynomialDegree = interp.getPolynomialDegree ();
  partitionData.interpolationPoints = &interp.getInterpolationPoints ();


  partitionedCardinalFunctions.resize ( nThreads );

  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    partitionData.rowStart = int ( i ) * rowStep;
    partitionData.rowEnd = integerMin ( partitionData.rowStart + rowStep, 
                                        ApproximateCardinalPreconditioner<RadialBasisFunction>::cardinalFunctions.size ());

    // Create a new thread class, assigning its partition data.
    partitionedCardinalFunctions [ i ] = new CardinalFunctionSequenceThread ( partitionData );
  }


}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::PThreadsApproximateCardinalPreconditioner<RadialBasisFunction>::~PThreadsApproximateCardinalPreconditioner () {

  size_t i;

  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    delete partitionedCardinalFunctions [ i ];
  }

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::PThreadsApproximateCardinalPreconditioner<RadialBasisFunction>::solve ( const Vector& theVector, 
                                                                                             Vector& result ) const {

  size_t i;

  // Start the preconditioner solve.
  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    partitionedCardinalFunctions [ i ]->startSolve ( theVector, result );
  }

  // Wait here until all processes are done.
  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    partitionedCardinalFunctions [ i ]->done ();
  }

  ApproximateCardinalPreconditioner<RadialBasisFunction>::solvePolynomialPart ( theVector, result );
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::PThreadsApproximateCardinalPreconditioner<RadialBasisFunction>::assemblePreconditioner ( const DenseMatrix& fromTheMatrix ) {

  size_t i;

  // Start computation of the preconditioner.
  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    partitionedCardinalFunctions [ i ]->startAssembly ( fromTheMatrix );
  }

  // Wait here until all processes are done.
  for ( i = 0; i < partitionedCardinalFunctions.size (); i++ ) {
    partitionedCardinalFunctions [ i ]->done ();
  }

  ApproximateCardinalPreconditioner<RadialBasisFunction>::assemblePreconditionerPolynomialPart ();
}

//------------------------------------------------------------//

#endif // __Numerics_PThreadsApproximateCardinalPreconditioner_HH__
