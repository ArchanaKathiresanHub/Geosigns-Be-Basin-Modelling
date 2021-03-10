//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON_PETSCSOLVER_H
#define FASTCAULDRON_PETSCSOVLER_H

#include <string>
#include "petscksp.h"

class PetscSolver
{
public:
   /// Copy constructor (deleted)
   PetscSolver( const PetscSolver & ) = delete;
   
   /// Assignment operator (deleted)
   PetscSolver & operator=(const PetscSolver & ) = delete;

   /// Destructor
   virtual ~PetscSolver();

   /// Sets PETSc KSP options from the options database
   void loadCmdLineOptionsAndSetZeroPivot();
   /// <summary>
   ///  Adds a particular type of quantity to the diagonal of the matrix during numerical factorization, thus the matrix has nonzero pivots
   /// </summary>
   /// <param name="ShiftType"></param> Numeric Shift for factorizations
   /// <param name="shiftAmount"></param> amount to shift
   void setPCFactorSetShiftType(MatFactorShiftType ShiftType, double shiftAmount);
   /// Prints to screen the settings of the KSP solver
   void viewSettings() const;
   /// <summary>
   ///  resets the solver context
   /// </summary>
   void reset() ;
   /// Gets the maximum number of iterations used by the KSP convergence tests
   int  getMaxIterations() const;

   /// Sets the maximum number of iterations used by the KSP convergence tests
   void setMaxIterations(int maxIterations);

   /// Gets the absolute convergence tolerance used by the KSP convergence tests
   double getTolerance() const;

   /// Sets the preconditioner
   void setPCtype( const PCType pcType );

   /// Sets the preconditioner
   PCType getPCtype() const;

   /// Finds a solution for the vector x in the matrix equation: A x = b
   void solve( const Mat & A,
               const Vec & b,
               Vec & x,
               int * iterations = 0,
               KSPConvergedReason * reason = 0,
               double * residualNorm = 0 );

   /// \brief Sets the prefix for changing solver or preconditioner on the command line.
   /// E.g. prefix = "temp_", then -temp_ksp_type gmres would set the solver to gmres
   void setSolverPrefix ( const std::string & solverPrefix );

   /// \brief Indicate that the initial solution vector is zero or not.
   /// \param [in] isNonZero False indicates that the initial solution is zero, true indicates otherwise.
   void setInitialGuessNonZero ( const bool isNonZero );

protected:
   /// Input constructor for CG and GMRES
   PetscSolver( const double tolerance = 0.0, const int maxIterations = 0 );

   /// PETSc Krylov solver
   KSP m_solver;

   /// Solver type
   KSPType m_type;

   /// Preconditioner type
   PCType m_pc;
   
private:
   /// Creates the hypre option string merging the default options with those provided via command line
   void createHypreOptionString( std::string & optionString ) const;
};

//! Class for solving linear systems using PETSc Conjugate Gradient method
class PetscCG : public PetscSolver
{
public:
   /// Input constructor with absolute convergence tolerance and maximum number of iterations
   PetscCG( const double tolerance = 0.0 , const int maxIterations = 0 );
};

//! Class for solving linear systems using PETSc GMRES method
class PetscGMRES : public PetscSolver
{
public:
   /// Input constructor with absolute convergence tolerance, restart and maximum number of iterations
   PetscGMRES( const double tolerance = 0.0, const int restart = 0, const int maxIterations = 0 );

   /// Sets number of iterations at which GMRES restarts.
   void setRestart( const int restart );

   /// Gets number of iterations at which GMRES restarts.
   int getRestart() const ;
};


#endif
