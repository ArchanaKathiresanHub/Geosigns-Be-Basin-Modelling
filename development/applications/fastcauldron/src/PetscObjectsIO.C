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

#include "PetscObjectsIO.h"
#include "petscsys.h"
#include "petscviewer.h"

namespace PetscObjectsIO
{
   static const std::string s_matlabExt = ".mat";
   static const std::string s_binaryExt = ".bin";

   int writeMatrixToFile( const Mat & matrix,
                          const std::string & outputFolder,
                          const std::string & outputFileName,
                          const bool binary )
   {
      struct stat buffer;
      assert( stat (outputFolder.c_str(), &buffer) == 0 );

      std::string fileName =  outputFolder + outputFileName;

      assert( matrix != 0 );

      PetscErrorCode status = 0;

      PetscViewer viewer;
      status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
      if( binary )
      {
         fileName += s_binaryExt;
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_WRITE, &viewer );
      }
      else
      {
         PetscMPIInt mpiSize = 0;
         MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
         PetscInt m, n;
         status = MatGetSize( matrix, &m, &n );
         if( (mpiSize > 1) and (m > 1024) )
         {
            char additionalOptions[] = "-mat_ascii_output_large";
            status = PetscOptionsInsertString( additionalOptions );
         }
         fileName += s_matlabExt;
         status = PetscViewerASCIIOpen( PETSC_COMM_WORLD, fileName.c_str(), &viewer );
         status = PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB);
      }
      status = PetscViewerSetFromOptions( viewer );
      status = MatView( matrix, viewer );
      status = PetscViewerDestroy( &viewer );

      return status;
   }
   

   int loadMatrixFromFile( const std::string & inputFolder,
                           const std::string & inputFileName,
                           Mat & matrix )
   {
      PetscErrorCode status = 0;

      const std::string fileName( inputFolder + inputFileName + s_binaryExt );

      struct stat buffer;
      const bool fileExists = (stat (fileName.c_str(), &buffer) == 0);

      if( fileExists )
      {
         PetscViewer viewer;
         status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_READ, &viewer );
         if( matrix == 0 )
         {
            PetscMPIInt mpiSize = 0;
            MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
            if( mpiSize > 1 ) std::cout << "WARNING: Loading matrix with default setup in parallel run";
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
      struct stat buffer;
      assert( stat (outputFolder.c_str(), &buffer) == 0 );

      std::string fileName =  outputFolder + outputFileName;

      assert( vector != 0 );

      PetscErrorCode status = 0;

      PetscViewer viewer;
      status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
      if( binary )
      {
         fileName += s_binaryExt;
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_WRITE, &viewer );
      }
      else
      {
         fileName += s_matlabExt;
         status = PetscViewerASCIIOpen( PETSC_COMM_WORLD, fileName.c_str(), &viewer );
         status = PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB);
      }
      status = PetscViewerSetFromOptions( viewer );
      status = VecView( vector, viewer );
      status = PetscViewerDestroy( &viewer );

      return status;
   }
   

   int loadVectorFromFile( const std::string & inputFolder,
                           const std::string & inputFileName,
                           Vec & vector )
   {
      PetscErrorCode status = 0;

      const std::string fileName( inputFolder + inputFileName + s_binaryExt );

      struct stat buffer;
      const bool fileExists = (stat (fileName.c_str(), &buffer) == 0);

      if( fileExists )
      {
         PetscViewer viewer;
         status = PetscViewerCreate( PETSC_COMM_WORLD, &viewer );
         status = PetscViewerBinaryOpen( PETSC_COMM_WORLD, fileName.c_str(), FILE_MODE_READ, &viewer );
         if( vector == 0 )
         {
            PetscMPIInt mpiSize = 0;
            MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
            if( mpiSize > 1 ) std::cout << "WARNING: Loading vector with default setup in parallel run";
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
