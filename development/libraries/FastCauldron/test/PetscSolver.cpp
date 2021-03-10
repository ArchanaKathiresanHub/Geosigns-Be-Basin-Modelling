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
   EXPECT_EQ( std::string(cg.getPCtype()), std::string(PCBJACOBI) );

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

TEST(PetscSolver, negativePivotTest)
{
    KSP                solver;
    PC                 prec;
    Mat                A/*, M*/;
    Vec                X, B/*, D*/;
    MPI_Comm           comm;
    PetscScalar        v;
    KSPConvergedReason reason;
    PetscInt           i, j, its;
    PetscErrorCode     ierr;

    comm = MPI_COMM_SELF;

    /* 
        Taken from PETSc examples all the commented lines in this test are of instruction, hence retained
        *  Construct the Kershaw matrix
        *  and a suitable rhs / initial guess
     
     */
    ierr = MatCreateSeqAIJ(comm, 4, 4, 4, 0, &A); CHKERRV(ierr);
    ierr = VecCreateSeq(comm, 4, &B); CHKERRV(ierr);
    ierr = VecDuplicate(B, &X); CHKERRV(ierr);
    for (i = 0; i < 4; i++) {
        v = 3;
        ierr = MatSetValues(A, 1, &i, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);
        v = 1;
        ierr = VecSetValues(B, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);
        ierr = VecSetValues(X, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);
    }

    i = 0; v = 0;
    ierr = VecSetValues(X, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);

    for (i = 0; i < 3; i++) {
        v = -2; j = i + 1;
        ierr = MatSetValues(A, 1, &i, 1, &j, &v, INSERT_VALUES); CHKERRV(ierr);
        ierr = MatSetValues(A, 1, &j, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);
    }
    i = 0; j = 3; v = 2;

    ierr = MatSetValues(A, 1, &i, 1, &j, &v, INSERT_VALUES); CHKERRV(ierr);
    ierr = MatSetValues(A, 1, &j, 1, &i, &v, INSERT_VALUES); CHKERRV(ierr);
    ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
    ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
    ierr = VecAssemblyBegin(B); CHKERRV(ierr);
    ierr = VecAssemblyEnd(B); CHKERRV(ierr);


    PetscCG cg(1e-6, 100);
    /*
        *   Can change the PC type
            cg.setPCtype(PCILU);
    */
    EXPECT_EQ(std::string(cg.getPCtype()), std::string(PCBJACOBI));
    /*
     * A Conjugate Gradient method
     * with BJACOBI or ILU(0) preconditioning
     */
     //ierr = KSPCreate(comm, &solver); CHKERRV(ierr);
     //ierr = KSPSetOperators(solver, A, A); CHKERRV(ierr);

     //ierr = KSPSetType(solver, KSPCG); CHKERRV(ierr);
     //ierr = KSPSetInitialGuessNonzero(solver, PETSC_TRUE); CHKERRV(ierr);

     /*
      * ILU preconditioner;
      * this will break down unless you add the Shift line,
      * or use the -pc_factor_shift_positive_definite option */
      //ierr = KSPGetPC(solver, &prec); CHKERRV(ierr);
      //ierr = PCSetType(prec, PCILU); CHKERRV(ierr);
      /* ierr = PCFactorSetShiftType(prec,MAT_SHIFT_POSITIVE_DEFINITE);CHKERRV(ierr); */

      //ierr = KSPSetFromOptions(solver); CHKERRV(ierr);
      //ierr = KSPSetUp(solver); CHKERRV(ierr);

      /* Collect the factored Matrix in D
       * Now that the factorisation is done, show the pivots;
       * note that the last one is negative. This in itself is not an error,
       * but it will make the iterative method diverge.
       */
       //ierr = PCFactorGetMatrix(prec, &M); CHKERRV(ierr);
       //ierr = VecDuplicate(B, &D); CHKERRV(ierr);
       //ierr = MatGetDiagonal(M, D); CHKERRV(ierr);

       /*
        * Solve the system;
        * without the shift this will diverge with
        * an indefinite preconditioner
        */
    cg.solve(A, B, X, 0, &reason, 0);
    //ierr = KSPSolve(solver, B, X); CHKERRV(ierr);
    //ierr = KSPGetConvergedReason(solver, &reason); CHKERRV(ierr);
    EXPECT_EQ(reason, KSP_DIVERGED_INDEFINITE_PC);
    
    // After INDEFINITE_PC failure re-run with setPCFactorSetShiftType
    /*
     * A Conjugate Gradient method
     * with ILU(0) preconditioning
     */
    cg.setPCtype(PCILU);
    EXPECT_EQ(std::string(cg.getPCtype()), std::string(PCILU));
    cg.setPCFactorSetShiftType(MAT_SHIFT_POSITIVE_DEFINITE, PETSC_DECIDE);
	int iterations = 0;
	double residualNorm = 0.0;
    cg.solve(A, B, X,&iterations, &reason, &residualNorm);

    // The same matrix in now solvable with Relative tolerance condition... see PETSc manual for RTOL
    EXPECT_EQ(KSP_CONVERGED_RTOL, reason);
    EXPECT_LT(residualNorm,1e-7);
    ierr = VecDestroy(&X); CHKERRV(ierr);
    ierr = VecDestroy(&B); CHKERRV(ierr);
    //ierr = VecDestroy(&D); CHKERRV(ierr);
    ierr = MatDestroy(&A); CHKERRV(ierr);
    /*ierr = KSPDestroy(&solver); CHKERRV(ierr);*/

}


TEST(PetscSolver, CGSolveZeroPivotFailed)
{
    // \ The matrix used in this test & the following one is taken from the Pressure_modeling_no_Perma model
    // \  in the Basin-ModellingQA repository
    PetscCG cg(1e-6, 100);
    EXPECT_EQ(std::string(cg.getPCtype()), std::string(PCBJACOBI));

    Mat A;
    MatCreate(PETSC_COMM_WORLD, &A);

    int n = 8;
    int rows[] = { 0, 1, 2, 3 ,4,5,6,7 };
    int cols[] = { 0, 1, 2, 3 ,4,5,6,7 };
    double values[64] =
    { 2.72E-06,-6.80E-07,-6.80E-07,-1.36E-06,1.36E-06,-3.40E-07,-3.40E-07,-6.80E-07,
        -6.80E-07,2.72E-06,-1.36E-06,-6.80E-07,-3.40E-07,1.36E-06,-6.80E-07,-3.40E-07,
        -6.80E-07,-1.36E-06,2.72E-06,-6.80E-07,-3.40E-07,-6.80E-07,1.36E-06,-3.40E-07,
        -1.36E-06,-6.80E-07,-6.80E-07,2.72E-06,-6.80E-07,-3.40E-07,-3.40E-07,1.36E-06,
        1.36E-06,-3.40E-07,-3.40E-07,-6.80E-07,1.00E+25,-6.80E-07,-6.80E-07,-1.36E-06,
        -3.40E-07,1.36E-06,-6.80E-07,-3.40E-07,-6.80E-07,1.00E+25,-1.36E-06,-6.80E-07,
        -3.40E-07,-6.80E-07,1.36E-06,-3.40E-07,-6.80E-07,-1.36E-06,1.00E+25,-6.80E-07,
        -6.80E-07,-3.40E-07,-3.40E-07,1.36E-06,-1.36E-06,-6.80E-07,-6.80E-07,1.00E+25 };


    MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n, n);
    MatSetType(A, MATSEQAIJ);
    MatSetUp(A);
    MatSetValues(A, n, rows, n, cols, values, INSERT_VALUES);
    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

    Vec b, x;
    VecCreate(PETSC_COMM_WORLD, &b);
    VecSetSizes(b, PETSC_DECIDE, n);
    VecSetUp(b);
    VecSet(b, 1.0);

    VecCreate(PETSC_COMM_WORLD, &x);
    VecSetSizes(x, PETSC_DECIDE, n);
    VecSetUp(x);
    VecSet(x, 0.0);

    int iterations = 0;
    KSPConvergedReason reason;
    double residualNorm = 0.0;

    cg.solve(A, b, x, &iterations, &reason, &residualNorm);
	
    EXPECT_EQ(KSP_DIVERGED_PC_FAILED, reason);

    auto ierr = VecDestroy(&b); CHKERRV(ierr);
	ierr = VecDestroy(&x); CHKERRV(ierr);
	ierr = MatDestroy(&A); CHKERRV(ierr);
}

TEST(PetscSolver, CGSolveZeroPivotPassed)
{
    PetscCG cg(0.001, 100);
    cg.loadCmdLineOptionsAndSetZeroPivot();
    EXPECT_EQ(std::string(cg.getPCtype()), std::string(PCBJACOBI));

    Mat A;
    MatCreate(PETSC_COMM_WORLD, &A);

    int n = 8;
    int rows[] = { 0, 1, 2, 3 ,4,5,6,7 };
    int cols[] = { 0, 1, 2, 3 ,4,5,6,7 };
    double values[64] =
    { 2.72E-06,-6.80E-07,-6.80E-07,-1.36E-06,1.36E-06,-3.40E-07,-3.40E-07,-6.80E-07,
        -6.80E-07,2.72E-06,-1.36E-06,-6.80E-07,-3.40E-07,1.36E-06,-6.80E-07,-3.40E-07,
        -6.80E-07,-1.36E-06,2.72E-06,-6.80E-07,-3.40E-07,-6.80E-07,1.36E-06,-3.40E-07,
        -1.36E-06,-6.80E-07,-6.80E-07,2.72E-06,-6.80E-07,-3.40E-07,-3.40E-07,1.36E-06,
        1.36E-06,-3.40E-07,-3.40E-07,-6.80E-07,1.00E+25,-6.80E-07,-6.80E-07,-1.36E-06,
        -3.40E-07,1.36E-06,-6.80E-07,-3.40E-07,-6.80E-07,1.00E+25,-1.36E-06,-6.80E-07,
        -3.40E-07,-6.80E-07,1.36E-06,-3.40E-07,-6.80E-07,-1.36E-06,1.00E+25,-6.80E-07,
        -6.80E-07,-3.40E-07,-3.40E-07,1.36E-06,-1.36E-06,-6.80E-07,-6.80E-07,1.00E+25 };


    MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n, n);
    MatSetType(A, MATSEQAIJ);
    MatSetUp(A);
    MatSetValues(A, n, rows, n, cols, values, INSERT_VALUES);
    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);

    Vec b, x;
    VecCreate(PETSC_COMM_WORLD, &b);
    VecSetSizes(b, PETSC_DECIDE, n);
    VecSetUp(b);
    VecSet(b, 1.0);

    VecCreate(PETSC_COMM_WORLD, &x);
    VecSetSizes(x, PETSC_DECIDE, n);
    VecSetUp(x);
    VecSet(x, 0.0);

    int iterations = 0;
    KSPConvergedReason reason;
    double residualNorm = 0.0;

    cg.solve(A, b, x, &iterations, &reason, &residualNorm);
	double xs[8];
	VecGetValues(x, n, rows, xs);
    /*for (int i = 0; i < n; ++i)
        std::cout << xs[i] << std::endl;*/
	EXPECT_EQ(1, iterations);
    // \    CAREFULL residual norm will be high because the matrix has very high values compared to rhs=b & the check is ...
    // \    norm(r) <= rtol*norm(b) or rtol*norm(b - A*x_0) if KSPConvergedDefaultSetUIRNorm() is called
    // \    rtol = cg.getTolerance() ;	rtol	- the relative convergence tolerance (relative decrease in the residual norm)
	// EXPECT_DOUBLE_EQ(cg.getTolerance(), residualNorm);
    EXPECT_EQ(KSP_CONVERGED_RTOL, reason);

    auto ierr = VecDestroy(&b); CHKERRV(ierr);
    ierr = VecDestroy(&x); CHKERRV(ierr);
    ierr = MatDestroy(&A); CHKERRV(ierr);
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
   EXPECT_EQ( std::string(gmres.getPCtype()), std::string(PCBJACOBI) );

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

#ifndef _WIN32
TEST( PetscSolver, setPCType)
{
   PetscCG cg( 1e-6, 100);
   EXPECT_EQ( std::string(cg.getPCtype()), std::string(PCBJACOBI) );
   cg.setPCtype( PCHYPRE );
   EXPECT_EQ( cg.getPCtype(), PCHYPRE );
}
#else
TEST(PetscSolver, setPCType)
{
    PetscCG cg(1e-6, 100);
    EXPECT_EQ(std::string(cg.getPCtype()), std::string(PCBJACOBI));
    cg.setPCtype(PCMG);
    EXPECT_EQ(cg.getPCtype(), std::string(PCMG));
}
#endif
