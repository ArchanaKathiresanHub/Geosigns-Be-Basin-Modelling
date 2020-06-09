//------------------------------------------------------------//

#include "Preconditioner.h"

//------------------------------------------------------------//

void Numerics::Preconditioner::assemblePreconditioner ( const DenseMatrix& fromTheMatrix ) {
  theMatrix = &fromTheMatrix;
}

//------------------------------------------------------------//

void Numerics::Preconditioner::solve ( const Vector& theVector, 
                                             Vector& result ) const {
  result = theVector;
}

//------------------------------------------------------------//

void Numerics::Preconditioner::print ( const std::string&  preconditionerName,
                                             std::ostream& output ) const {

  int i;

  for ( i = 0; i < theMatrix->numberOfRows (); i++ ) {
    output << preconditionerName << " ( " << i << " ) = 1.0; " << std::endl;
  }

}

//------------------------------------------------------------//
