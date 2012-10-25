//------------------------------------------------------------//

#include "PThreadsDenseRowMatrix.h"

//------------------------------------------------------------//

Numerics::DenseRowSequenceThread::DenseRowSequenceThread ( const DenseRowSequence& data ) {

  matrixPartition = data;

  int status = pthread_create ( &threadID, 0, &denseRowMatrixVectorProductFunction, (void*)this );

  // Throw an exception if there is any problem
  if ( status != 0 ) {
//     throw Threading_Error ( " Tasking_Error : cannot create thread for Shared_Memory_Partition /n" );
  } // end if

}

//------------------------------------------------------------//

Numerics::DenseRowSequenceThread::~DenseRowSequenceThread () {
  selection.setData ( TERMINATE );
}

//------------------------------------------------------------//

void Numerics::DenseRowSequence::construct () {
  
  int i;

  for ( i = rowStart; i < rowEnd; i++ ) {
    (*rowSequence)[ i ] = new Vector ( dimension );
  }

}

//------------------------------------------------------------//

void Numerics::DenseRowSequence::matrixVectorProduct ( const Vector& vec,
                                                             Vector& result  ) {

  
  int i;

  for ( i = rowStart; i < rowEnd; i++ ) {
    result ( i ) = innerProduct ( vec, *(*rowSequence)[ i ]);
  }

}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::construct () {
  acceptConstruct.setData ();
}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::startMVP ( const Vector& vec,
                                                        Vector& result ) {

  selection.setData ( MVP_START );

  Vector* vecCopy = const_cast<Vector*>( &vec );
  Vector* resCopy = const_cast<Vector*>( &result );

  acceptStart.setData ( vecCopy, resCopy );

}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::done () {

  selection.setData ( MVP_DONE );
  acceptDone.setData ();

}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::threadConstruct () {
  acceptConstruct.getData ();
}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::threadStartMVP ( Vector*& vec,
                                                        Vector*& result ) {

  acceptStart.getData ( vec, result );

}

//------------------------------------------------------------//

void Numerics::DenseRowSequenceThread::threadDone () {

  acceptDone.getData ();

}

//------------------------------------------------------------//

void* Numerics::DenseRowSequenceThread::denseRowMatrixVectorProductFunction ( void* globalData ) {

  DenseRowSequenceThread*  rowSequence = static_cast<DenseRowSequenceThread*> ( globalData );
  DenseRowSequence&        matrixPartition = rowSequence->matrixPartition;
  DenseRowSequenceThread::DenseRowMatrixOperations selection;

  bool terminate = false;

  Vector* theVector;
  Vector* theResult;

  rowSequence->threadConstruct ();
  matrixPartition.construct ();

  while ( ! terminate ) {

    rowSequence->selection.getData ( selection );

    switch ( selection ) {

    case DenseRowSequenceThread::MVP_START :

      // Get rhs-vector and result vector.
      rowSequence->threadStartMVP ( theVector, theResult );

      // perform matrix vector product operation on matrix partition.
      matrixPartition.matrixVectorProduct ( *theVector, *theResult );

      break;

    case DenseRowSequenceThread::MVP_DONE :

      // Nothing to do here, its just so that main process
      // knows that the mvp operation has finished.
      rowSequence->threadDone ();
      break;

    case DenseRowSequenceThread::TERMINATE :

      // Terminate process.
      terminate = true;
      break;
    }

  }

  return 0;
}

//------------------------------------------------------------//


Numerics::PThreadsDenseRowMatrix::PThreadsDenseRowMatrix ( const int rowCount,
                                                           const int colCount,
                                                           const int nThreads ) {

  const int rowStep = ( rowCount + nThreads ) / nThreads;

  int i;

  DenseRowMatrix::allRows.resize ( rowCount );

  rowPartition.resize ( nThreads );
  DenseRowSequence partitionData;

  // set matrix dimensions;
  rows = rowCount;
  columns = colCount;

  partitionData.dimension = rowCount;
  partitionData.rowSequence = &allRows;

  for ( i = 0; i < nThreads; i++ ) {
    partitionData.rowStart = i * rowStep;
    partitionData.rowEnd   = integerMin ( partitionData.rowStart + rowStep, rows );

    rowPartition [ i ] = new DenseRowSequenceThread ( partitionData );
  }

  for ( i = 0; i < nThreads; i++ ) {
    rowPartition [ i ]->construct ();
  }

  for ( i = 0; i < nThreads; i++ ) {
    rowPartition [ i ]->done ();
  }

}

//------------------------------------------------------------//

Numerics::PThreadsDenseRowMatrix::~PThreadsDenseRowMatrix () {

  int i;

  for ( i = 0; i < rowPartition.size (); i++ ) {
    delete rowPartition [ i ];
  }

}

//------------------------------------------------------------//

void Numerics::PThreadsDenseRowMatrix::matrixVectorProduct ( const Vector& vec,
                                                                   Vector& result ) const {


  int i;

  // Tell all threads to start to perform the matrix vector product.
  for ( i = 0; i < rowPartition.size (); i++ ) {
    rowPartition [ i ]->startMVP ( vec, result );
  }

  // Wait here until all threads have finished.
  for ( i = 0; i < rowPartition.size (); i++ ) {
    rowPartition [ i ]->done ();
  }

}

//------------------------------------------------------------//
