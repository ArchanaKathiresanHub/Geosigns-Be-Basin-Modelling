//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PetscSolver.h"

#include "petscsys.h"
#include "petsctime.h"

#include <cassert>
#include <iostream>
#include <utility>
#include <string>

// Default Hypre boomerAMG preconditioner options
// They will be overwritten if already provided via command line
const int hypreNumOpt = 3;
const std::pair< std::string, std::string > hypreOptions[hypreNumOpt] = {
   std::make_pair( "pc_hypre_boomeramg_coarsen_type", "HMIS" ),
   std::make_pair( "pc_hypre_boomeramg_interp_type", "ext+i" ),
   std::make_pair( "pc_hypre_boomeramg_agg_nl", "2" )
};

PetscSolver :: PetscSolver( const double newTolerance, const int newMaxIterations )
   : m_solver(nullptr),
     m_type( KSPCG ),
     m_pc( PCBJACOBI )
{
   // Create the solver object
   KSPCreate ( PETSC_COMM_WORLD, & m_solver);

   double relativeTolerance = 0.0;
   double absoluteTolerance = 0.0;
   double divergenceTolerance =  0.0 ;
   int    currentMaximumIterations = 0;

   KSPGetTolerances ( m_solver,
                     &relativeTolerance,
                     &absoluteTolerance,
                     &divergenceTolerance,
                     &currentMaximumIterations );

   KSPSetTolerances ( m_solver,
                     newTolerance == 0.0 ? relativeTolerance : newTolerance,
                     absoluteTolerance,
                     divergenceTolerance,
                     newMaxIterations == 0 ? currentMaximumIterations : newMaxIterations );

   // By default use a zero initial solution for iterative solvers.
   setInitialGuessNonZero ( false );
}

void PetscSolver::setSolverPrefix ( const std::string & solverPrefix )
{
   PC preconditioner;

   KSPGetPC ( m_solver, &preconditioner );

   if ( preconditioner != nullptr ) {
      PCSetOptionsPrefix ( preconditioner, solverPrefix.c_str ());
   }

   KSPSetOptionsPrefix ( m_solver, solverPrefix.c_str ());
}

void PetscSolver::setInitialGuessNonZero ( const bool isNonZero ) {

   PetscBool petscIsNonZero = ( isNonZero ? PETSC_TRUE : PETSC_FALSE );

   KSPSetInitialGuessNonzero ( m_solver, petscIsNonZero );
}

void PetscSolver::loadCmdLineOptionsAndSetZeroPivot()
{
    KSPSetFromOptions(m_solver);

    // \Set the ZERO for NULL pivot detection in preconditioner;
    PC pc; PetscReal zero(1e-30);
    KSPGetPC(m_solver, &pc);

    if (pc != nullptr) {
        PCFactorSetZeroPivot(pc, zero);
        /* The above should work; else apply the next two lines
            setPCFactorSetShiftType(MAT_SHIFT_NONZERO,PETSC_DECIDE);
            or;
            -pc_factor_shift_type NONZERO -pc_factor_shift_amount <dampingfactor>
            from the command line
        */
    }
}

void PetscSolver::setPCFactorSetShiftType(MatFactorShiftType ShiftType, double shiftAmount)
{
	PC pc; KSPGetPC(m_solver, &pc);
	if (pc != nullptr) {
		PCFactorSetShiftType(pc, ShiftType);
		if (shiftAmount != PETSC_DECIDE)
			PCFactorSetShiftAmount(pc, shiftAmount);
		else
			PCFactorSetShiftAmount(pc, PETSC_DECIDE);
	}
}



void PetscSolver::viewSettings() const
{
   KSPView( m_solver, PETSC_VIEWER_STDOUT_WORLD );
}

void PetscSolver::reset()
{
    KSPReset(m_solver);
}

int PetscSolver :: getMaxIterations() const
{
   double relativeTolerance = 0.0;
   double absoluteTolerance = 0.0;
   double divergenceTolerance =  0.0 ;
   int    currentMaximumIterations = 0;

   KSPGetTolerances ( m_solver,
                     &relativeTolerance,
                     &absoluteTolerance,
                     &divergenceTolerance,
                     &currentMaximumIterations );

   return currentMaximumIterations;
}

double PetscSolver :: getTolerance() const
{
   double relativeTolerance = 0.0;
   double absoluteTolerance = 0.0;
   double divergenceTolerance =  0.0 ;
   int    currentMaximumIterations = 0;

   KSPGetTolerances ( m_solver,
                     &relativeTolerance,
                     &absoluteTolerance,
                     &divergenceTolerance,
                     &currentMaximumIterations );

   return relativeTolerance;
}


void PetscSolver :: setPCtype ( const PCType pcType )
{
   PC preconditioner;
   KSPGetPC ( m_solver, &preconditioner );
   if ( nullptr != preconditioner && m_pc != pcType )
   {
      std::string hypreOption;
      if( PCHYPRE == m_pc )
      {
         // Clear an option name-value pair in the options database, overriding whatever is already present
         for( unsigned int i = 0; i < hypreNumOpt; ++i )
         {
            hypreOption = "-" + hypreOptions[i].first;
            PetscOptionsClearValue(PETSC_IGNORE, hypreOption.c_str() );
         }
      }
      m_pc = pcType;
      if( PCHYPRE == m_pc )
      {
         createHypreOptionString( hypreOption );
         PetscOptionsInsertString(PETSC_IGNORE, hypreOption.c_str() );
      }

      // Destroy current solver
      KSPDestroy( & m_solver );

      // Recreate the solver with new PC options
      KSPCreate( PETSC_COMM_WORLD, & m_solver );
      KSPSetType( m_solver, m_type );

      // Set new preconditioner type
      KSPGetPC ( m_solver, &preconditioner );
      if ( nullptr != preconditioner )
      {
         PCSetType( preconditioner, pcType );
      }
   }
}


PCType PetscSolver :: getPCtype() const
{
   return m_pc;
}


void PetscSolver :: createHypreOptionString( std::string & optionString ) const
{
   char optionValue[PETSC_MAX_PATH_LEN];
   PetscBool flg = PETSC_FALSE;

   optionString.clear();

   // Loop over all hardcoded hypre options
   for( unsigned int i = 0; i < hypreNumOpt; ++i )
   {
      memset ( optionValue, 0, PETSC_MAX_PATH_LEN );
      const std::string optionName = "-" + hypreOptions[i].first + " ";
      optionString.append( optionName );
      // If the options has already been defined at command line it has higher priority
      PetscOptionsGetString(PETSC_IGNORE, PETSC_IGNORE, optionName.c_str(), optionValue, sizeof(optionValue), &flg);
      if( flg and optionValue[0] != 0 )
      {
         optionString.append( optionValue );
      }
      else
      {
         optionString.append( hypreOptions[i].second );
      }
      optionString.append( " " );
   }
}


void PetscSolver :: setMaxIterations( const int maxIterations )
{
   double relativeTolerance = 0.0;
   double absoluteTolerance = 0.0;
   double divergenceTolerance =  0.0 ;
   int    currentMaximumIterations = 0;

   KSPGetTolerances ( m_solver,
                     &relativeTolerance,
                     &absoluteTolerance,
                     &divergenceTolerance,
                     &currentMaximumIterations );

   KSPSetTolerances ( m_solver,
                     relativeTolerance,
                     absoluteTolerance,
                     divergenceTolerance,
                     maxIterations );
}


void PetscSolver :: solve( const Mat & A,
                           const Vec & b,
                           Vec & x,
                           int * iterations,
                           KSPConvergedReason * reason,
                           double * residualNorm )
{
   KSPSetOperators( m_solver, A, A );
   KSPSolve( m_solver, b, x );

   if (iterations)
      KSPGetIterationNumber( m_solver, iterations);

   if (reason)
      KSPGetConvergedReason( m_solver, reason);

   if (residualNorm)
      KSPGetResidualNorm( m_solver, residualNorm );
}


PetscSolver :: ~PetscSolver()
{
   KSPDestroy( & m_solver );
}

PetscCG :: PetscCG( const double tolerance, const int maxIterations )
   : PetscSolver(tolerance, maxIterations)
{
   m_type = KSPCG;
   KSPSetType( m_solver, m_type );
}

PetscGMRES :: PetscGMRES( const double tolerance, const int restart, const int maxIterations )
   : PetscSolver(tolerance, maxIterations)
{
   m_type = KSPGMRES;
   KSPSetType( m_solver, m_type );

   if (restart != 0)
   {
      KSPGMRESSetRestart( m_solver, restart );
   }
}

void PetscGMRES :: setRestart( const int restart )
{
   KSPGMRESSetRestart( m_solver, restart);
}

int PetscGMRES :: getRestart() const
{
   int restart = 0;
   KSPGMRESGetRestart( m_solver, &restart);
   return restart;
}
