#include "PetscObjectsIO.h"

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include "gtest/gtest-death-test.h"
#include "petsc.h"
#include "petscmat.h"
#include "FilePath.h"


struct Init
{
  Init() { PetscInitialize(0, 0, 0, 0); }
  ~Init() { PetscFinalize(); }
} initMe;

TEST(PetscObjectsIO, PETScVersionNumber)
{
    char           version[128];
    PetscInt       major, minor, subminor;
    PetscGetVersion(version, sizeof(version));
    PetscGetVersionNumber(&major, &minor, &subminor, NULL);
    EXPECT_EQ(major ,3);
    EXPECT_EQ(minor , 13);
    EXPECT_EQ(subminor , 1);
}

TEST( PetscObjectsIO, MissingFolderOnRead )
{
   Mat A = 0;
   ibs::FilePath fpath( std::string("MissingFolder") );
   fpath << std::string("MissingFile");
   EXPECT_EQ( PetscObjectsIO::loadMatrixFromFile( fpath.path(), A ), -1 );
   MatDestroy( &A );
   
   Vec b = 0;
   EXPECT_EQ( PetscObjectsIO::loadVectorFromFile( fpath.path(), b ), -1 );
   VecDestroy( &b );
}


TEST( PetscObjectsIO, MissingFileOnRead )
{
   Mat A = 0;
   ibs::FilePath fpath( std::string("") );
   fpath << std::string("MissingFile");
   EXPECT_EQ( PetscObjectsIO::loadMatrixFromFile( fpath.path(), A ), -1 );
   MatDestroy( &A );
   
   Vec b = 0;
   EXPECT_EQ( PetscObjectsIO::loadVectorFromFile( fpath.path(), b ), -1 );
   VecDestroy( &b );
}


TEST( PetscObjectsIO, MissingFolderOnWrite )
{
   (void)(::testing::GTEST_FLAG(death_test_style) = "threadsafe");

   Mat A = 0;
   ibs::FilePath fpath( std::string("MissingFolder") );
   ASSERT_DEATH( PetscObjectsIO::writeMatrixToFile( A, fpath.filePath(), fpath.fileName(), true ), "" );
   MatDestroy( &A );
   
   Vec b = 0;
   ASSERT_DEATH( PetscObjectsIO::writeVectorToFile( b, fpath.filePath(), fpath.fileName(), true ), "" );
   VecDestroy( &b );
}


TEST( PetscObjectsIO, UninitialisedObjectOnWrite )
{
   (void)(::testing::GTEST_FLAG(death_test_style) = "threadsafe");

   Mat A = 0;
   ibs::FilePath fpath( std::string("") );
   ASSERT_DEATH( PetscObjectsIO::writeMatrixToFile( A, fpath.filePath(), fpath.fileName(), true ), "" );
   MatDestroy( &A );
   
   Vec b = 0;
   ASSERT_DEATH( PetscObjectsIO::writeVectorToFile( b, fpath.filePath(), fpath.fileName(), true ), "" );
   VecDestroy( &b );
}


TEST( PetscObjectsIO, ReadAndWriteMatrix )
{
   Mat A2write = 0;
   MatCreate(PETSC_COMM_WORLD, &A2write);

   int n = 4;
   int rows[] = { 0, 1, 2, 3 };
   int cols[] = { 0, 1, 2, 3 };
   double values[16] = 
      { 1.0, 0.0, 0.0, 0.0,
        0.0, 2.0, 0.0, 0.0,
        0.0, 0.0, 3.0, 0.0,
        0.0, 0.0, 0.0, 4.0 };

   MatSetSizes( A2write, PETSC_DECIDE, PETSC_DECIDE, n, n );
   MatSetType( A2write, MATSEQAIJ );
   MatSetUp( A2write );
   MatSetValues( A2write, n, rows, n, cols, values, INSERT_VALUES );
   MatAssemblyBegin( A2write, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd( A2write, MAT_FINAL_ASSEMBLY );
      
   const ibs::Path path( std::string("") );
   const std::string fileName = "testMatrix";

   // Delete old files
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".m";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin.info";
      if( fpath.exists() ) fpath.remove();
   }
   
   ibs::FilePath fpath( path );
   fpath << fileName;

   // Write Matrix to file in matlab format
   EXPECT_EQ( PetscObjectsIO::writeMatrixToFile( A2write, fpath.filePath(), fpath.fileName(), false ), 0 );
   
   // Remove matlab file
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".m";
      if( fpath.exists() ) fpath.remove();
   }
   
   // Write Matrix to file in binary format
   EXPECT_EQ( PetscObjectsIO::writeMatrixToFile( A2write, fpath.filePath(), fpath.fileName(), true ), 0 );

   // Read
   Mat A2read = 0;
   EXPECT_EQ( PetscObjectsIO::loadMatrixFromFile( fpath.path() + ".bin", A2read ), 0 );

   // Compare
   Mat err = 0;
   MatDuplicate( A2read, MAT_COPY_VALUES, &err );
   MatAXPY( err, PetscScalar(-1), A2write, SAME_NONZERO_PATTERN);
   PetscReal norm;
   MatNorm( err, NORM_FROBENIUS, &norm );
   EXPECT_EQ( static_cast<double>(norm), 0.0 );
   
   // Remove binary files
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin.info";
      if( fpath.exists() ) fpath.remove();
   }
   
   MatDestroy( &A2read );
   MatDestroy( &A2write );
   MatDestroy( &err );
}


TEST( PetscObjectsIO, ReadAndWriteVector )
{
   Vec b2write = 0;
   VecCreate( PETSC_COMM_WORLD, &b2write );
   VecSetSizes( b2write, PETSC_DECIDE, 4 );
   VecSetUp( b2write );
   VecSet( b2write, 5.0 );
      
   const ibs::Path path( std::string("") );
   const std::string fileName = "testVector";

   // Delete old files
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".m";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin.info";
      if( fpath.exists() ) fpath.remove();
   }
   
   ibs::FilePath fpath( path );
   fpath << fileName;

   // Write Matrix to file in matlab format
   EXPECT_EQ( PetscObjectsIO::writeVectorToFile( b2write, fpath.filePath(), fpath.fileName(), false ), 0 );
   
   // Remove matlab file
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".m";
      if( fpath.exists() ) fpath.remove();
   }
   
   // Write Matrix to file in binary format
   EXPECT_EQ( PetscObjectsIO::writeVectorToFile( b2write, fpath.filePath(), fpath.fileName(), true ), 0 );

   // Read
   Vec b2read = 0;
   EXPECT_EQ( PetscObjectsIO::loadVectorFromFile( fpath.path() + ".bin", b2read ), 0 );

   // Compare
   Vec err = 0;
   VecDuplicate( b2read, &err );
   VecCopy( b2read, err );
   VecAXPY( err, PetscScalar(-1), b2write );
   PetscReal norm;
   VecNorm( err, NORM_2, &norm );
   EXPECT_EQ( static_cast<double>(norm), 0.0 );
   
   // Remove binary files
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin";
      if( fpath.exists() ) fpath.remove();
   }
   {
      ibs::FilePath fpath( path );
      fpath << fileName + ".bin.info";
      if( fpath.exists() ) fpath.remove();
   }

   VecDestroy( &b2read );
   VecDestroy( &b2write );
   VecDestroy( &err );
}
