//------------------------------------------------------------//

#include "LinearSolver.h"

//------------------------------------------------------------//

Numerics::LinearSolver::LinearSolver () {
  printResidual = false;
  numberOfIterations = -1;
  maximumNumberOfIterations = DefaultMaximumNumberOfIterations;
  solverTolerance = DefaultSolverTolerance;
}

//------------------------------------------------------------//

void Numerics::LinearSolver::setMatrix ( const DenseMatrix& newMatrix ) {
  theMatrix = &newMatrix;

  // The system has changed, so the the number of iterations taken must be reset.
  numberOfIterations = -1;
}

//------------------------------------------------------------//

void Numerics::LinearSolver::setPreconditioner ( const Preconditioner& newPreconditioner ) {
  thePreconditioner = &newPreconditioner;

  // The system has changed, so the the number of iterations taken must be reset.
  numberOfIterations = -1;
}

//------------------------------------------------------------//

void Numerics::LinearSolver::setTolerance ( const double newTolerance ) {

  // The tolerance has changed, so the the number of iterations taken must be reset.
  solverTolerance = newTolerance;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::LinearSolver::getTolerance () const {
  return solverTolerance;
}

//------------------------------------------------------------//

void Numerics::LinearSolver::setMaximumNumberOfIterations ( const int newMaxiumIterations ) {

  // The maximum number of iteratins has changed, so the the number of iterations taken must be reset.
  maximumNumberOfIterations = newMaxiumIterations;
}

//------------------------------------------------------------//

int Numerics::LinearSolver::getMaximumNumberOfIterations () const {
  return maximumNumberOfIterations;
}

//------------------------------------------------------------//

void Numerics::LinearSolver::setPrintResidual ( const bool newPrintResidual ) {
  printResidual = newPrintResidual;
}

//------------------------------------------------------------//

bool Numerics::LinearSolver::getPrintResidual () const {
  return printResidual;
}

//------------------------------------------------------------//

int Numerics::LinearSolver::iterationsTaken () const {
  return numberOfIterations;
}

//------------------------------------------------------------//
