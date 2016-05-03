#include "PetscSolver.h"

PetscSolver :: PetscSolver(double newTolerance, int newMaxIterations)
   : m_solver()
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
                     newMaxIterations == 0 ? currentMaximumIterations : newMaxIterations
         );
}

void PetscSolver::loadCmdLineOptions()
{
   KSPSetFromOptions ( m_solver );
}

void PetscSolver::viewSettings() const
{
   KSPView( m_solver, PETSC_VIEWER_STDOUT_WORLD );
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


void PetscSolver :: setMaxIterations( int maxIterations)
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


void PetscSolver :: solve( const Mat & A, const Vec & b, Vec & x, int * iterations , KSPConvergedReason * reason , double * residualNorm )
{
   KSPSetOperators( m_solver, A, A );

   VecSet( x, 0.0 );
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

PetscCG :: PetscCG(double tolerance, int maxIterations)
   : PetscSolver(tolerance, maxIterations )
{
   KSPSetType( m_solver, KSPCG);
}

PetscGMRES :: PetscGMRES(double tolerance, int restart, int maxIterations)
   : PetscSolver(tolerance, maxIterations)
{
   KSPSetType( m_solver, KSPGMRES );

   if (restart != 0)
   {
      KSPGMRESSetRestart( m_solver, restart);
   }
}

void PetscGMRES :: setRestart( int restart )
{
   KSPGMRESSetRestart( m_solver, restart);
}

int PetscGMRES :: getRestart() const
{
   int restart = 0;
   KSPGMRESGetRestart( m_solver, &restart);
   return restart;
}

