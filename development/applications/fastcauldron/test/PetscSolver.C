#include "PetscSolver.h"

#include <iostream>

#include <petsc.h>
#include <gtest/gtest.h>

struct Init
{
  Init() { PetscInitialize(0, 0, 0, 0); }
  ~Init() { PetscFinalize(); }
} initMe;

TEST( PetscSolver, CGSolve )
{
   PetscCG cg( 1e-6, 100);

   Mat A;
   MatCreate(PETSC_COMM_WORLD, &A);

   int n = 4;
   int rows[] = { 0, 1, 2, 3 };
   int cols[] = { 0, 1, 2, 3 };
   double values[16] = 
      { 1, 0, 0, 0
      , 0, 1, 0, 0
      , 0, 0, 1, 0
      , 0, 0, 0, 1 };


   MatSetSizes( A, PETSC_DECIDE, PETSC_DECIDE, n, n);
   MatSetType(A, MATMPIAIJ);
   MatSetUp(A);
   MatSetValues( A, n, rows, n, cols, values, INSERT_VALUES);
   MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
   MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

   Vec b, x;
   VecCreate(PETSC_COMM_WORLD, &b);
   VecSetSizes(b, PETSC_DECIDE, n);
   VecSetUp(b);
   VecSet(b, 5.0);

   VecCreate(PETSC_COMM_WORLD, &x);
   VecSetSizes(x, PETSC_DECIDE, n);
   VecSetUp(x);
   VecSet(x, 0.0);

   int iterations = 0;
   KSPConvergedReason reason;
   double residualNorm = 0.0;
   cg.solve( A, b, x, &iterations, &reason, &residualNorm);

   EXPECT_EQ( 1, iterations );
   EXPECT_EQ( KSP_CONVERGED_ATOL, reason);
   EXPECT_DOUBLE_EQ( 0.0, residualNorm );

   double xs[4];
   VecGetValues( x, n, rows, xs);

   for (int i = 0; i < n; ++i)
      EXPECT_DOUBLE_EQ( 5.0, xs[i] );
}

TEST( PetscSolver, CGMaxIterations)
{
   PetscCG cg( 1e-6, 100);

   EXPECT_EQ( 100, cg.getMaxIterations() );
   cg.setMaxIterations( 200 );
   EXPECT_EQ( 200, cg.getMaxIterations() );
}

TEST( PetscSolver, CGTolerance)
{
   PetscCG cg( 1e-6, 100);

   EXPECT_DOUBLE_EQ( 1.0e-6, cg.getTolerance() );
}

TEST( PetscSolver, GMRESSolve )
{
   PetscGMRES gmres( 1e-6, 10, 100);

   Mat A;
   MatCreate(PETSC_COMM_WORLD, &A);

   int n = 4;
   int rows[] = { 0, 1, 2, 3 };
   int cols[] = { 0, 1, 2, 3 };
   double values[16] = 
      { 1, 0, 0, 0
      , 0, 1, 0, 0
      , 0, 0, 1, 0
      , 0, 0, 0, 1 };


   MatSetSizes( A, PETSC_DECIDE, PETSC_DECIDE, n, n);
   MatSetType(A, MATMPIAIJ);
   MatSetUp(A);
   MatSetValues( A, n, rows, n, cols, values, INSERT_VALUES);
   MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
   MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

   Vec b, x;
   VecCreate(PETSC_COMM_WORLD, &b);
   VecSetSizes(b, PETSC_DECIDE, n);
   VecSetUp(b);
   VecSet(b, 5.0);

   VecCreate(PETSC_COMM_WORLD, &x);
   VecSetSizes(x, PETSC_DECIDE, n);
   VecSetUp(x);
   VecSet(x, 0.0);

   int iterations = 0;
   KSPConvergedReason reason;
   double residualNorm = 0.0;
   gmres.solve( A, b, x, &iterations, &reason, &residualNorm);

   EXPECT_EQ( 1, iterations );
   EXPECT_EQ( KSP_CONVERGED_ATOL, reason);
   EXPECT_DOUBLE_EQ( 0.0, residualNorm );

   double xs[4];
   VecGetValues( x, n, rows, xs);

   for (int i = 0; i < n; ++i)
      EXPECT_DOUBLE_EQ( 5.0, xs[i] );
}

TEST( PetscSolver, GMRESMaxIterations)
{
   PetscGMRES gmres( 1e-6, 10, 100);

   EXPECT_EQ( 100, gmres.getMaxIterations() );
   gmres.setMaxIterations( 200 );
   EXPECT_EQ( 200, gmres.getMaxIterations() );
}

TEST( PetscSolver, GMRESRestart)
{
   PetscGMRES gmres( 1e-6, 10, 100);

   EXPECT_EQ( 10, gmres.getRestart() );
   gmres.setRestart( 20 );
   EXPECT_EQ( 20, gmres.getRestart() );
}

TEST( PetscSolver, GMRESTolerance)
{
   PetscGMRES gmres( 1e-6, 100);

   EXPECT_DOUBLE_EQ( 1.0e-6, gmres.getTolerance() );
}

