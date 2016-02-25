//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PETSC_MATRIX_IO_H
#define PETSC_MATRIX_IO_H

#include <string>

#include "petscmat.h"

namespace PetscObjectsIO
{
   /// \brief Output PETSc matrix to file
   /// 
   /// \param [in] matrix         The PETSc matrix.
   /// \param [in] outputFolder   The output folder full path.
   /// \param [in] outputFileName The output file name (extension will be added depending on output format).
   /// \param [in] binary         TRUE: binary output, FALSE: ASCII matlab format.
   /// \return Return code        0 success, else failure.
   int writeMatrixToFile( const Mat & matrix,
                          const std::string & outputFolder,
                          const std::string & outputFileName,
                          const bool binary );

   /// \brief Load PETSc matrix from file. It works only with binary files
   /// 
   /// \param [in] inputFolder   The full path to input folder.
   /// \param [in] inputFileName The file name.
   /// \param [out] matrix       The PETSc matrix (it might be already initialized as well, otherwise it uses default options).
   /// \return Return code       0 success, -1 file does not exist, else PETSc loading error.
   int loadMatrixFromFile( const std::string & inputFolder,
                           const std::string & inputFileName,
                           Mat & matrix );
   
   /// \brief Output PETSc vector to file
   /// 
   /// \param [in] vector         The PETSc vector.
   /// \param [in] outputFolder   The output folder full path.
   /// \param [in] outputFileName The output file name (extension will be added depending on output format).
   /// \param [in] binary         TRUE: binary output, FALSE: ASCII matlab format.
   /// \return Return code        0 success, else failure.
   int writeVectorToFile( const Vec & vector,
                          const std::string & outputFolder,
                          const std::string & outputFileName,
                          const bool binary );

   /// \brief Load PETSc vector from file. It works only with binary files
   /// 
   /// \param [in] inputFolder   The full path to input folder.
   /// \param [in] inputFileName The file name.
   /// \param [out] vector       The PETSc vector (it might be already initialized as well, otherwise it uses default options).
   /// \return Return code       0 success, -1 file does not exist, else PETSc loading error.
   int loadVectorFromFile( const std::string & inputFolder,
                           const std::string & inputFileName,
                           Vec & vector );
}

#endif 
