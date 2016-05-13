#ifndef FASTCAULDRON_PETSCSOLVER_H
#define FASTCAULDRON_PETSCSOVLER_H

#include <string>
#include <petscksp.h>

class PetscSolver
{
public:
   virtual ~PetscSolver();

   void loadCmdLineOptions();

   // Prints to screen the settings of the KSP solver
   void viewSettings() const;
   
   int  getMaxIterations() const;
   void setMaxIterations(int maxIterations);

   double getTolerance() const;

   // Finds a solution for the vector x in the matrix equation: A x = b
   void solve( const Mat & A, const Vec & b, Vec & x, int * iterations = 0, KSPConvergedReason * reason = 0, double * residualNorm = 0 );

   /// \brief Sets the prefix for changing solver or preconditioner on the command line.
   ///
   /// E.g. prefix = "temp_", then -temp_ksp_type gmres would set the solver to gmres
   void setSolverPrefix ( const std::string& solverPrefix );
   
protected:
   PetscSolver(double tolerance = 0.0, int maxIterations = 0);
   KSP m_solver;

private:
   PetscSolver( const PetscSolver & ); // prohibity copying
   PetscSolver & operator=(const PetscSolver & ); // prohibit assignment
};

class PetscCG : public PetscSolver
{
public:
   PetscCG(double tolerance = 0.0 , int maxIterations = 0);
};

class PetscGMRES : public PetscSolver
{
public:
   PetscGMRES(double tolerance = 0.0, int restart = 0, int maxIterations = 0);

   void setRestart( int restart);
   int getRestart() const ;
};


#endif
