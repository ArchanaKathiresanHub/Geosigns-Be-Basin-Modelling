//------------------------------------------------------------//

#include "PThreadsApproximateCardinalPreconditioner.h"

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequence::solve ( const Vector& vec,
                                                       Vector& result ) {


  int i;

  for ( i = rowStart; i < rowEnd; i++ ) {
    result ( i ) = (*cardinalFunctions)[ i ].innerProduct ( vec );
  }

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequence::assemblePreconditioner ( const DenseMatrix& theMatrix ) {


  const int NumberOfPolynomialTerms = numberOfPolynomialTerms<Point::DIMENSION>( polynomialDegree );

  Vector rsSquared ( interpolationPoints->size ());

  int i;

  for ( i = rowStart; i < rowEnd; i++ ) {
    computeRsSquared ((*interpolationPoints)[ i ], *interpolationPoints, rsSquared );
    (*cardinalFunctions)[ i ].computeCardinalFunction ( theMatrix,
                                                        rsSquared, 
                                                        numberOfNeighbours,
                                                        NumberOfPolynomialTerms );
  }

}

//------------------------------------------------------------//

Numerics::CardinalFunctionSequenceThread::CardinalFunctionSequenceThread ( const CardinalFunctionSequence& data ) {

  cardinalFunctions = data;

  int status = pthread_create ( &threadID, 0, &preconditionerSolveThreadFunction, (void*)this );


  // Throw an exception if there is any problem
  if ( status != 0 ) {
//     throw Threading_Error ( " Tasking_Error : cannot create thread for Shared_Memory_Partition /n" );
  } // end if

}
//------------------------------------------------------------//

Numerics::CardinalFunctionSequenceThread::~CardinalFunctionSequenceThread () {
  selection.setData ( TERMINATE );
}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::startAssembly ( const DenseMatrix& fromTheMatrix ) {

  selection.setData ( ASSEMBLY_START );

  DenseMatrix* matrixPtr = const_cast<DenseMatrix*>( &fromTheMatrix );

  acceptStartAssembly.setData ( matrixPtr );

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::startSolve ( const Vector& vec,
                                                                  Vector& result ) {

  selection.setData ( PRECONDITIONER_START );

  Vector* vecCopy = const_cast<Vector*>( &vec );
  Vector* resCopy = const_cast<Vector*>( &result );

  acceptStart.setData ( vecCopy, resCopy );

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::done () {

  selection.setData ( PRECONDITIONER_DONE );
  acceptDone.setData ();

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::threadStartSolve ( Vector*& vec,
                                                                  Vector*& result ) {

  acceptStart.getData ( vec, result );

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::threadStartAssembly ( DenseMatrix*& theMatrix ) {

  acceptStartAssembly.getData ( theMatrix );

}

//------------------------------------------------------------//

void Numerics::CardinalFunctionSequenceThread::threadDone () {

  acceptDone.getData ();

}

//------------------------------------------------------------//

void* Numerics::CardinalFunctionSequenceThread::preconditionerSolveThreadFunction ( void* globalData ) {

  CardinalFunctionSequenceThread* rowSequence = static_cast<CardinalFunctionSequenceThread*> ( globalData );
  CardinalFunctionSequence&       cardinalFunctions = rowSequence->cardinalFunctions;
  CardinalFunctionSequenceThread::PreconditionerOperations        selection;

  bool terminate = false;

  DenseMatrix* theMatrix;
  Vector*      theVector;
  Vector*      theResult;


  while ( ! terminate ) {

    rowSequence->selection.getData ( selection );

    switch ( selection ) {

    case CardinalFunctionSequenceThread::ASSEMBLY_START :

      rowSequence->threadStartAssembly ( theMatrix );

      cardinalFunctions.assemblePreconditioner ( *theMatrix );

      break;

    case CardinalFunctionSequenceThread::PRECONDITIONER_START :

      rowSequence->threadStartSolve ( theVector, theResult );

      cardinalFunctions.solve ( *theVector, *theResult );

      break;

    case CardinalFunctionSequenceThread::PRECONDITIONER_DONE :

      rowSequence->threadDone ();
      // Nothing to do here, except provide a sychronisation point 
      // at the end of the assembly and solve operations.
      break;

    case CardinalFunctionSequenceThread::TERMINATE :
      terminate = true;
      break;
    }

  }

  return 0;
}

//------------------------------------------------------------//
