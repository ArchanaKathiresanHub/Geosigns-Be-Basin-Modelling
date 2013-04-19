#ifndef __Numerics_PThreadsDenseRowMatrix_HH__
#define __Numerics_PThreadsDenseRowMatrix_HH__

#include <vector>

// Common types
#include "Rendezvous.h"

// Linear algebra
#include "DenseRowMatrix.h"
#include "Vector.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file PThreadsDenseRowMatrix.h
  /// \brief Provides base dense-matrix class.


  /// \brief Contains the row partition data.
  struct DenseRowSequence {

    void construct ();

    /// \brief Compute the part of the matrix vector product 
    /// associated with the partition data.
    void matrixVectorProduct ( const Vector& vec,
                                     Vector& result );


    /// \brief The dimension of the system.
    ///
    /// The system must be square, so there is no need to store both number of rows and columns.
    int dimension;

    /// \brief The start of the row partition.
    int rowStart;

    /// \brief The end of the row partition.
    int rowEnd;

    /// \brief The vectors of the row partition.
    VectorArray* rowSequence;

  };


  /// \brief Controls the parallel matrix operations.
  class DenseRowSequenceThread {

    enum DenseRowMatrixOperations { MATRIX_CONSTRUCT, MVP_START, MVP_DONE, TERMINATE };

  public :

    DenseRowSequenceThread ( const DenseRowSequence& data );

    ~DenseRowSequenceThread ();

    /// \brief Construct the part of the matrix for this partition.
    void construct ();

    /// \brief Start the matrix vector product of the partition.
    void startMVP ( const Vector& vec,
                          Vector& result );

    /// \brief \b MUST be called after calling either of construct of startMVP functions.
    ///
    /// Not calling this function directly after calling either construct or startMVP \b will result in deadlock.
    void done  ();

  private :

    /// \brief Exiting this indicates to the thread that construction can proceed.
    void threadConstruct ();

    /// \brief Get the rhs and result vectors from the calling process.
    ///
    /// On exit, the computation of the matrix vector product for the partition can proceed.
    void threadStartMVP ( Vector*& vec,
                          Vector*& result );

    /// \brief Tell the calling process that the operation has successfully completed.
    void threadDone ();

    /// \brief The data for the partition.
    DenseRowSequence matrixPartition;

    ///\brief For indicating to the thread which operation should be performed.
    POSIXThreads::Rendezvous1<DenseRowMatrixOperations> selection;

    /// \brief Allows passing of the rhs and result vectors.
    POSIXThreads::Rendezvous2<Vector*, Vector*> acceptStart;

    /// \brief For sychronising the construction functions.
    POSIXThreads::Rendezvous acceptConstruct;

    /// \brief For sychronising the completion function.
    POSIXThreads::Rendezvous acceptDone;

    ///\brief The process-id of the thread.
    pthread_t threadID;

    /// \brief The function that the thread is to execute.
    static void* denseRowMatrixVectorProductFunction ( void* globalData );



  };


  /// \brief Parallel matrix with separately allocated rows.
  ///
  /// Entries are accessed in the ranges in the half open set [0,rows)x[0,columns).
  class PThreadsDenseRowMatrix : public DenseRowMatrix {

  protected :

    /// \typedef DenseRowSequenceThreadArray
    /// \brief An array of the process class.
    typedef std::vector <DenseRowSequenceThread*> DenseRowSequenceThreadArray;

  public :

    PThreadsDenseRowMatrix ( const int rowCount,
                             const int colCount,
                             const int nThreads );


    ~PThreadsDenseRowMatrix ();

    /// \brief Matrix vector product is performed in parallel.
    void matrixVectorProduct ( const Vector& vec,
                                     Vector& result ) const;


  protected :

    /// \brief An array of the matrix partitions.
    DenseRowSequenceThreadArray rowPartition;

  };

  /** @} */

}


//------------------------------------------------------------//

#endif // __Numerics_PThreadsDenseRowMatrix_HH__
