//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "petscksp.h"
#include "petscmat.h"
#include "petscsys.h"
#include "petscvec.h"

#include "CompositeElementActivityPredicate.h"
#include "ComputationalDomain.h"
#include "ElementThicknessActivityPredicate.h"
#include "FastcauldronFactory.h"
#include "FastcauldronStartup.h"
#include "FilePath.h"
#include "HydraulicFracturingManager.h"
#include "ElementNonZeroPorosityActivityPredicate.h"
#include "PetscObjectAllocator.h"
#include "PetscObjectsIO.h"
#include "propinterface.h"
#include "SedimentElementActivityPredicate.h"

void printHelp();

bool getCommandLineParams( std::string & matrixFile,
                           std::string & rhsFile,
                           std::string & solutionFile,
                           bool & hasProject,
                           bool & isPressure );

double getTimeStepFromFileName( const std::string & fileName );

bool setupCauldron( int argc, char** argv,
                    const bool isPressure,
                    const double & timeStep,
                    Mat & A, Vec & b );

void solveLinearSystem( const Mat & A, const Vec & b, Vec & x );



int main(int argc, char** argv)
{
   int rc = 0;

   // Creating "fake" argv input with a run modality for fastcauldron
   int l_argc = argc + 1;
   char** l_argv = new char*[l_argc+1];
   l_argv[l_argc-1] = "-decompaction";
   for( unsigned int i = 0; i < argc; ++i )
   {
      l_argv[i] = argv[i];
      if(strcmp(l_argv[i],"-presMatrix")==0) l_argv[l_argc-1] = "-overpressure";
      if(strcmp(l_argv[i],"-tempMatrix")==0) l_argv[l_argc-1] = "-temperature";
   }
   l_argv[l_argc] = NULL;

   // Initializes the PETSc database and MPI
   rc = PetscInitialize(&l_argc, &l_argv, (char *)0, PETSC_NULL);

   // Check command line parameters
   std::string matrixFile, rhsFile, solutionFile;
   bool hasProject, isPressure;
   if( (argc < 3) or not getCommandLineParams( matrixFile, rhsFile, solutionFile, hasProject, isPressure ) )
   {
      printHelp();
      delete [] l_argv; l_argv = 0;
      return 1;
   }

   // Get saved time step (only if a project file is provided) to setup cauldron
   double timeStep = 0.0;
   if( hasProject )
   {
      timeStep = getTimeStepFromFileName( matrixFile );
      if( timeStep != getTimeStepFromFileName( rhsFile ) )
      {
         rc = PetscPrintf( PETSC_COMM_WORLD, "ERROR: matrix and rhs times do not match\n" );
         delete [] l_argv; l_argv = 0;
         return 1;
      }
   }

   // Matrix and RHS initialization
   Mat A = 0;
   Vec b = 0;
   Vec x = 0;
   Vec xIn = 0;
   if( !hasProject or !setupCauldron( l_argc, l_argv, isPressure, timeStep, A, b ) )
   {
      // If no project file has been provided or the Cauldron setup fails
      // the default PETSc settings will be used
      rc = MatCreate( PETSC_COMM_WORLD, &A );
      rc = VecCreate( PETSC_COMM_WORLD, &b );
   }

   // Load matrix, rhs
   rc = PetscObjectsIO::loadMatrixFromFile( std::string(), matrixFile.substr(0,matrixFile.find_last_of(".")), A );
   rc = PetscObjectsIO::loadVectorFromFile( std::string(), rhsFile.substr(0,rhsFile.find_last_of(".")), b );
   // Load solution (if provided)
   if( not solutionFile.empty() )
   {
      VecDuplicate( b, &xIn ); // allocates storage for xIn as in b
      rc = PetscObjectsIO::loadVectorFromFile( std::string(), solutionFile.substr(0,solutionFile.find_last_of(".")), xIn );
   }

   // Solve
   solveLinearSystem( A, b, x );

   // Write solution to file
   ibs::FilePath solPath( matrixFile );
   rc = PetscObjectsIO::writeVectorToFile( x, solPath.filePath(), std::string("solution_" + solPath.fileNameNoExtension()), false );

   // Compare solutions
   if( not solutionFile.empty() )
   {
      Vec err;
      rc = VecDuplicate( x, &err ); // allocates storage for err as in b
      rc = VecCopy( x, err );
      rc = VecAXPY( err, PetscScalar(-1), xIn); // err = x - xIn
      PetscReal norm2err;
      rc = VecNorm( err, NORM_2, &norm2err ); // sqrt(sum_i (err_i)^2)
      rc = PetscPrintf( PETSC_COMM_WORLD ,"\n" );
      rc = PetscPrintf( PETSC_COMM_WORLD ,"Absolute L2 norm error wrt provided solution: %g\n", norm2err );
      PetscReal norm2xIn;
      rc = VecNorm( xIn, NORM_2, &norm2xIn ); // sqrt(sum_i (xIn_i)^2)
      if( norm2xIn > 0. ) rc = PetscPrintf( PETSC_COMM_WORLD ,"Relative L2 norm error wrt provided solution: %g\n", norm2err/norm2xIn );
      if( err != 0 ) rc = VecDestroy( &err );
   }

   // Free memory and finalize
   if( A != 0 ) rc = MatDestroy( &A );
   if( b != 0 ) rc = VecDestroy( &b );
   if( x != 0 ) rc = VecDestroy( &x );
   if( xIn != 0 ) rc = VecDestroy( &xIn );
   delete [] l_argv; l_argv = 0;
   return PetscFinalize();
}


void printHelp()
{
   std::stringstream helpBuffer;

   helpBuffer << std::endl << std::endl;
   helpBuffer << "Usage: mpirun -np <procs> linearSolver [-help] [-options]" << std::endl;
   helpBuffer << "The command line options include:" << std::endl << std::endl;
   helpBuffer << "    -help                   Shows this help message" << std::endl;
   helpBuffer << "    -matrix <filename>      Matrix file" << std::endl;
   helpBuffer << "    -rhs <filename>         Rhs file" << std::endl;
   helpBuffer << "    -solution <filename>    Solution file (optional, for comparison)" << std::endl;
   helpBuffer << "    -project <filename>     Project file (optional, to reproduce load balance)" << std::endl;
   helpBuffer << "                            If specified, the type of matrix has to be provided as well (-presMatrix or -tempMatrix)" << std::endl;
   helpBuffer << "    -presMatrix             Provided input matrix comes from the pressure solver (only with -project)" << std::endl;
   helpBuffer << "    -tempMatrix             Provided input matrix comes from the temperature solver (only with -project)" << std::endl;
   helpBuffer << "    -ksp_view               Displays linear solver settings" << std::endl;
   helpBuffer << "    -ksp_converged_reason   Displays the reason a KSP solve converged or diverged" << std::endl;
   helpBuffer << "    -ksp_monitor            Displays the residual norm at each iteration of the linear solver" << std::endl;
   helpBuffer << "    -ksp_rtol               Sets the relative convergence tolerance (Cauldron default is 1e-6)" << std::endl;
   helpBuffer << "    -ksp_max_it             Sets the maximum number of iterations to use" << std::endl;
   helpBuffer << "    -log_summary            Prints performance data at program conclusion." << std::endl;
   helpBuffer << std::endl;
   helpBuffer << "Any other PETSc command line option can be provided to change the linear solver settings" << std::endl;
   helpBuffer << std::endl << std::endl;

   PetscPrintf( PETSC_COMM_WORLD, helpBuffer.str().c_str() );
}



bool getCommandLineParams( std::string & matrixFile,
                           std::string & rhsFile,
                           std::string & solutionFile,
                           bool & hasProject,
                           bool & isPressure )
{
   int rc = 0;
   hasProject = false;
   isPressure = false;

   char fileName[PETSC_MAX_PATH_LEN];
   PetscBool flg = PETSC_FALSE;

   rc = PetscOptionsGetString( NULL, "-matrix", fileName, sizeof(fileName), &flg );
   if( !flg ) return false;
   matrixFile = std::string( fileName );
   ibs::FilePath fPath = ibs::FilePath( matrixFile );
   if( !fPath.exists() )
   {
      std::stringstream msg;
      msg << "ERROR: " << matrixFile << " does not exist" << std::endl;
      PetscPrintf( PETSC_COMM_WORLD, msg.str().c_str() );
      return false;
   }
   
   rc = PetscOptionsGetString( NULL, "-rhs", fileName, sizeof(fileName), &flg );
   if( !flg ) return false;
   rhsFile = std::string( fileName );
   fPath = ibs::FilePath( rhsFile );
   if( !fPath.exists() )
   {
      std::stringstream msg;
      msg << "ERROR: " << rhsFile << " does not exist" << std::endl;
      PetscPrintf( PETSC_COMM_WORLD, msg.str().c_str() );
      return false;
   }
   
   rc = PetscOptionsGetString( NULL, "-project", fileName, sizeof(fileName), &flg );
   hasProject = (flg == PETSC_TRUE);
   fPath = ibs::FilePath( std::string( fileName ) );
   if( hasProject and !fPath.exists() )
   {
      std::stringstream msg;
      msg << "ERROR: " << std::string( fileName ) << " does not exist" << std::endl;
      PetscPrintf( PETSC_COMM_WORLD, msg.str().c_str() );
      return false;
   }
   
   rc = PetscOptionsGetString( NULL, "-solution", fileName, sizeof(fileName), &flg );
   solutionFile = (flg == PETSC_TRUE) ? std::string( fileName ) : std::string();
   fPath = ibs::FilePath( solutionFile );
   if( (!solutionFile.empty()) and !fPath.exists() )
   {
      std::stringstream msg;
      msg << "ERROR: " << solutionFile << " does not exist" << std::endl;
      PetscPrintf( PETSC_COMM_WORLD, msg.str().c_str() );
      return false;
   }

   if( hasProject )
   {
      rc = PetscOptionsHasName( NULL, "-presMatrix", &flg );
      isPressure = (flg == PETSC_TRUE);
      if( !flg ) rc = PetscOptionsHasName( NULL, "-tempMatrix", &flg );
      if( !flg )
      {
         rc = PetscPrintf( PETSC_COMM_WORLD, "ERROR: specify matrix type (-presMatrix or -tempMatrix)\n" );
         return false;
      }
      rc = PetscPrintf( PETSC_COMM_WORLD, "Project file provided\n\n" );
   }
   else { rc = PetscPrintf( PETSC_COMM_WORLD, "Project file not provided\n\n" ); }

   return (rc == 0);
}


double getTimeStepFromFileName( const std::string & fileName )
{
   std::string timeStr;
   timeStr = fileName.substr( fileName.find_last_of("t_") + 1 );
   timeStr = timeStr.substr( 0, timeStr.find_last_of(".") );
   double time = std::atof( timeStr.c_str() );
   return time;
}


bool setupCauldron( int argc, char** argv,
                    const bool isPressure,
                    const double & timeStep,
                    Mat & A, Vec & b )
{
   // Redirecting cout to hide cauldron setup messages
   std::streambuf *coutold = std::cout.rdbuf();
   std::stringstream sscout;
   std::cout.rdbuf(sscout.rdbuf());

   AppCtx * appctx = new AppCtx(argc, argv);

   HydraulicFracturingManager::getInstance().setAppCtx(appctx);

   bool canRunSaltModelling = false;
   const bool prepareOk = FastcauldronStartup::prepare( canRunSaltModelling );
   const bool startupOk = FastcauldronStartup::startup( argc, argv, canRunSaltModelling, false, false );
   if ( startupOk && prepareOk )
   {
      const LayerList & layersList = FastcauldronSimulator::getInstance().getCauldron()->layers;

      // Create computational domain
      ComputationalDomain * domain = NULL;
      if( isPressure )
      {
         // Construct the computational domain for the pressure simulation.
         // This requires only the sediment layers in the domain.
         domain = new ComputationalDomain( *layersList[0],
                                           *layersList[layersList.size() - 3],
                                           CompositeElementActivityPredicate().compose( ElementActivityPredicatePtr(new ElementThicknessActivityPredicate) )
                                                                              .compose( ElementActivityPredicatePtr(new SedimentElementActivityPredicate) )
                                                                              .compose( ElementActivityPredicatePtr(new ElementNonZeroPorosityActivityPredicate) ) );
      }
      else
      {
         // Construct the computational domain for the temperature simulation.
         // This requires all layers in the domain.
         domain = new ComputationalDomain( *layersList[0],
                                           *layersList[layersList.size() - 1],
                                           CompositeElementActivityPredicate().compose( ElementActivityPredicatePtr(new ElementThicknessActivityPredicate) ) );
      }

      AppCtx* cauldron = const_cast<AppCtx*>( FastcauldronSimulator::getInstance().getCauldron() );
      bool status = true;
      status &= cauldron->findActiveElements( timeStep );

      Basin_Modelling::Layer_Iterator layers;
      LayerProps* currentLayer;
      layers.Initialise_Iterator( cauldron->layers,
                                  Basin_Modelling::Descending,
                                  Basin_Modelling::Basement_And_Sediments, 
                                  Basin_Modelling::Active_Layers_Only );

      while( ! layers.Iteration_Is_Done ()) {
         currentLayer = layers.Current_Layer();
         status &= currentLayer->allocateNewVecs( cauldron, timeStep );
         layers++;
      }

      FastcauldronSimulator::getInstance().switchLithologies( timeStep );

      status &= cauldron->calcNodeVes( timeStep );
      assert( status );

      status &= cauldron->calcNodeMaxVes( timeStep );
      assert( status );

      status &= cauldron->calcPorosities(  timeStep  );
      assert( status );

      if( cauldron->IsCalculationCoupled and cauldron->DoHighResDecompaction and not cauldron->isGeometricLoop()) {
         status &= cauldron->setNodeDepths( timeStep );
      } else {
         status &= cauldron->calcNodeDepths( timeStep );
      }
      assert( status );
  
      status &= cauldron->Calculate_Pressure( timeStep );
      assert( status );

      Basin_Modelling::Layer_Iterator basinLayers( cauldron->layers,
                                                   Basin_Modelling::Descending,
                                                   Basin_Modelling::Basement_And_Sediments,
                                                   Basin_Modelling::Active_And_Inactive_Layers );

      while( not basinLayers.Iteration_Is_Done() ) {
         currentLayer = basinLayers.Current_Layer();
         currentLayer->setLayerElementActivity( timeStep );
         basinLayers++;
      }

      domain->resetAge( timeStep );

      // Allocate matrix and rhs
      A = PetscObjectAllocator::allocateMatrix( *domain );
      b = PetscObjectAllocator::allocateVector( *domain );

      delete domain; domain = 0;
   }

   // Restoring cout
   std::cout.rdbuf( coutold );

   const bool saveResults = false;
   FastcauldronStartup::finalise( saveResults );

   if ( not prepareOk ) PetscPrintf( PETSC_COMM_WORLD, "FastcauldronPrepare failed, default uniform distribution will be applied\n" );
   if ( not startupOk ) PetscPrintf( PETSC_COMM_WORLD, "FastcauldronStartup failed, default uniform distribution will be applied\n" );

   return startupOk;
}


void solveLinearSystem( const Mat & A, const Vec & b, Vec & x )
{
   int rc = 0;
   rc = VecDuplicate( b, &x ); // allocates storage for x as in b

   KSP solver;
   rc = KSPCreate( PETSC_COMM_WORLD, &solver );
   rc = KSPSetType(solver, KSPCG);
   rc = KSPSetFromOptions( solver ); // might be overriding the previous setting
   rc = KSPSetOperators( solver, A, A );
   rc = VecSet( x, 0.0 );

   PetscLogDouble Start_Time;
   rc = PetscTime( &Start_Time );
   rc = KSPSolve( solver, b, x );
   assert( rc == 0 );
   PetscLogDouble End_Time;
   rc = PetscTime( &End_Time );
   rc = PetscPrintf( PETSC_COMM_WORLD ,"Solving time: %g s\n", End_Time - Start_Time );

   rc = KSPDestroy( &solver );
}
