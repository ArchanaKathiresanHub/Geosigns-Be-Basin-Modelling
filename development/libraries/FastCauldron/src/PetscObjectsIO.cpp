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
#include <iostream>
#include <sys/stat.h>

#include "FilePath.h"
#include "PetscObjectsIO.h"
#include "petscsys.h"
#include "petscviewer.h"

namespace PetscObjectsIO
{
   static const std::string s_matlabExt = ".m";
   static const std::string s_binaryExt = ".bin";

   int writeMatrixToFile( const Mat & matrix,
                          const std::string & outputFolder,
                          const std::string & outputFileName,
                          const bool binary )
   {
      ibs::FilePath fPath( outputFolder );
      assert( fPath.exists() );

      fPath << outputFileName;
      std::string fileName;

      assert( matrix != 0 );

      PetscErrorCode status = 0;

      PetscViewer viewer;
      status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
      if( binary )
      {
         fileName = fPath.path() + s_binaryExt;
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_WRITE, &viewer );
      }
      else
      {
         PetscMPIInt mpiSize = 0;
         MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
         PetscInt m, n;    // Number of rows and columns
         status = MatGetSize( matrix, &m, &n );
         if( (mpiSize > 1) and (m > 1024) )
         {
            // PETSc by default prevents writing to ASCII file matrices with more than 1024 rows,
            // in this case an additional command line option has to be added to override it
            char additionalOptions[] = "-mat_ascii_output_large";
            status = PetscOptionsInsertString(PETSC_IGNORE, additionalOptions );
         }
         fileName = fPath.path() + s_matlabExt;
         status = PetscViewerASCIIOpen( PETSC_COMM_WORLD, fileName.c_str(), &viewer );
         status = PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_MATLAB);
      }
      status = PetscViewerSetFromOptions( viewer );
      status = PetscObjectSetName( (PetscObject) matrix, "A" );
      status = MatView( matrix, viewer ); // Write matrix to file
      if(!binary)
          status = PetscViewerPopFormat(viewer);
      status = PetscViewerDestroy( &viewer );

      return status;
   }
   

   int loadMatrixFromFile( const std::string & inputFilePath,
                           Mat & matrix )
   {
      PetscErrorCode status = 0;

      ibs::FilePath fPath( inputFilePath );

      if( fPath.exists() )
      {
         PetscViewer viewer;
         status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fPath.path().c_str(), FILE_MODE_READ, &viewer );
         if( matrix == 0 )
         {
            // If the matrix (to be filled in by reading the file) provided in input has not been initialized
            // it will be initialized with default setup. If the current run is multiprocessor we might be
            // loading the matrix in a different way with respect to how it was saved, for example in terms
            // of local sizes on each processor
            PetscMPIInt mpiSize = 0;
            MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
            if( mpiSize > 1 ) std::cout << "Basin_Warning: Loading matrix with default setup in parallel run";
            status = MatCreate( PETSC_COMM_WORLD, &matrix );
            status = MatSetFromOptions( matrix );
         }
         status = MatLoad( matrix, viewer );
         assert( status == 0 );
         status = PetscViewerDestroy( &viewer );
      }
      else
      {
         status = -1;
      }
      
      return status;
   }


   int writeVectorToFile( const Vec & vector,
                          const std::string & outputFolder,
                          const std::string & outputFileName,
                          const bool binary )
   {
      ibs::FilePath fPath( outputFolder );
      assert( fPath.exists() );

      fPath << outputFileName;
      std::string fileName;

      assert( vector != 0 );

      PetscErrorCode status = 0;

      PetscViewer viewer;
      status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
      if( binary )
      {
         fileName = fPath.path() + s_binaryExt;
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_WRITE, &viewer );
      }
      else
      {
         fileName = fPath.path() + s_matlabExt;
         status = PetscViewerASCIIOpen( PETSC_COMM_WORLD, fileName.c_str(), &viewer );
         status = PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_MATLAB); 
      }
      status = PetscViewerSetFromOptions( viewer );
      status = PetscObjectSetName( (PetscObject) vector, "b" );
      status = VecView( vector, viewer ); // Write vector to file
      if (!binary)
          status = PetscViewerPopFormat(viewer);
      status = PetscViewerDestroy( &viewer );

      return status;
   }
   

   int loadVectorFromFile( const std::string & inputFilePath,
                           Vec & vector )
   {
      PetscErrorCode status = 0;

      ibs::FilePath fPath( inputFilePath );

      if( fPath.exists() )
      {
         PetscViewer viewer;
         status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fPath.path().c_str(), FILE_MODE_READ, &viewer );
         if( vector == 0 )
         {
            // If the vector (to be filled in by reading the file) provided in input has not been initialized
            // it will be initialized with default setup. If the current run is multiprocessor we might be
            // loading the vector in a different way with respect to how it was saved, for example in terms
            // of local sizes on each processor
            PetscMPIInt mpiSize = 0;
            MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
            if( mpiSize > 1 ) std::cout << "Basin_Warning: Loading vector with default setup in parallel run";
            status = VecCreate( PETSC_COMM_WORLD, &vector );
            status = VecSetFromOptions( vector );
         }
         status = VecLoad( vector, viewer );
         assert( status == 0 );
         status = PetscViewerDestroy( &viewer );
      }
      else
      {
         status = -1;
      }
      
      return status;
   }

} // Closing PetscObjectsIO
