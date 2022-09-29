#include <mpi.h>
#include <hdf5.h>
#include <stdio.h>

#include "../src/RewriteFileName.h"

#include "../src/h5merge.h"
#include "../src/fileHandler.h"
#include "../src/fileHandlerReuse.h"
#include "../src/fileHandlerAppend.h"

#include <gtest/gtest.h>

#include <string>
#include <cstdio>
#include <algorithm>
#include <random>

// Tesing of the merging of "one file per process" output files into a one hdf file
//
// Each process creates a local file, process 0 creates a global file. All processes call "readDataset" method.
// Output of "readDataset": The merging file is written (not closed yet)
//
//  1: Dataset is written correctly
//  2: Dataset and attribute are written correctly
//  3: LocalFileId = NULL
//  4: DataSet name is empty (doesn't exist)
//  5: Reallocation of buffers
//  6: Collective check of an error
//  7: Dataset and attribute are written correctly. Reuse of the file structure.
//  7: Dataset and attribute are written correctly. Update existing file.

namespace
{
  const std::string RewritePattern = "{NAME}-{MPI_SIZE}-{MPI_RANK}";
  const std::string StdDataSetName = "/DummyData";
  const std::string AttributeName  = "Attribute";
  const std::string AttributeData  = "Dummy attribute data";
}

class MPIHelper
{
public:
   MPIHelper()
   {
      MPI_Init(NULL, NULL);
      H5Eset_auto( H5E_DEFAULT, NULL, NULL);
   }

   ~MPIHelper()
   {
      MPI_Finalize();
   }

   MPIHelper(const MPIHelper& mpiHelper) = delete;
   MPIHelper(MPIHelper&& mpiHelper) = delete;

   MPIHelper& operator= (const MPIHelper& mpiHelper) = delete;
   MPIHelper& operator= (MPIHelper&& mpiHelper) = delete;

   static MPIHelper & instance()
   {
      static MPIHelper object;
      return object;
   }

   static int rank()
   {
      instance();

      int rank;
      MPI_Comm_rank( MPI_COMM_WORLD, &rank);
      return rank;
   }

   static int size()
   {
      instance();

      int size;
      MPI_Comm_size( MPI_COMM_WORLD, &size);
      return size;
   }

   static void barrier()
   {
      instance();
      MPI_Barrier(MPI_COMM_WORLD);
   }

};


// The convenience class makes it a bit easier to write tests.
class File
{
public:
   enum Access { Create, CreateWithAttr, Open, OpenWithAttr };
   enum ErrorCode { CannotCreateFile = -3,  CannotReadFile=-4, CannotOpenFile = -5};

   // Construct a file name with a normal name.
   File( const std::string & name, bool extendedName )
      : m_file( name )
  {

      int mpiRank = MPIHelper::rank();
      int mpiSize = MPIHelper::size();

      if (extendedName)
      {
         std::vector<char> buffer( rewriteFileName( RewritePattern.c_str(), name.c_str(), mpiRank, mpiSize, 0, 0 ) );
         rewriteFileName( RewritePattern.c_str(), name.c_str(), mpiRank, mpiSize, &buffer[0], buffer.size() );
         m_file = std::string( &buffer[0], buffer.size() );
      }
      m_h5file = H5P_DEFAULT;
   }

   File(const File& file) = delete;
   File(File&& file) = delete;

   File& operator= (const File& file) = delete;
   File& operator= (File&& file) = delete;

   int open( Access access, int data, bool saveOnDisk )
   {
      const int rank = MPIHelper::rank();

      herr_t status = 0;
      hsize_t dims[2]  = { 1, 1 };


      hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
      EXPECT_GE( fapl, 0);

      if( !saveOnDisk ) {
         H5Pset_fapl_core( fapl, 128, false);
      }

      switch ( access )
      {
         case CreateWithAttr:
         case Create:
            m_h5file = H5Fcreate(m_file.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, fapl);
            m_exists = (m_h5file >= 0 );
            if (!m_exists)
            {
               if ( !saveOnDisk )
                  H5Pclose(fapl);

               return CannotCreateFile;
            }
            break;

         case OpenWithAttr:
         case Open:
            m_h5file = H5Fopen(m_file.c_str(), H5F_ACC_RDONLY, fapl);
            if ( m_h5file < 0)
            {
               if ( !saveOnDisk )
                  return CannotOpenFile;
            }
            break;

         default:
            ADD_FAILURE() << "Fell through switch-case statement";
      }

       m_saveOnDisk = saveOnDisk;

      hid_t space = H5Screate_simple( 2, dims, dims );
      EXPECT_GE(space, 0);

      if (data < 0)
      {
         // Open global file
         data = -1;

         hid_t dataset = H5Dopen( m_h5file, StdDataSetName.c_str(), H5P_DEFAULT);
         EXPECT_GE(dataset, 0);

         status = H5Dread( dataset, H5T_NATIVE_FLOAT, space, space, H5P_DEFAULT, &data );
         EXPECT_GE(status, 0);

         if( access == OpenWithAttr ) {
            // initialize character buffer with nonsense data
            char attrData [64];
            memset( attrData, 'X', sizeof(attrData) );

            hid_t attrId = H5Aopen_idx(dataset, 0 );
            EXPECT_GE(attrId, 0);

            // get the name of the attribute
            status = H5Aget_name( attrId, 64, attrData );
            EXPECT_EQ(status, AttributeName.size()/sizeof(char));
            attrData[status] = '\0';

            EXPECT_STREQ (  AttributeName.c_str(), attrData );

            // Read the attribute data (start with a buffer full of zeros)
            memset( attrData, 0, sizeof(attrData) );
            status = H5Aread( attrId, H5T_C_S1, attrData );
            EXPECT_GE(status, 0);

            EXPECT_STREQ (  AttributeData.c_str(), attrData );

            status = H5Aclose( attrId );
            EXPECT_GE(status, 0);

         }
         status = H5Dclose(dataset);
         EXPECT_GE(status, 0);

      }
      else if ( data > 0 )
      {
         // Create local files
         EXPECT_GE(data, 0);
         hid_t dataset = H5Dcreate( m_h5file, StdDataSetName.c_str(), H5T_NATIVE_FLOAT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
         EXPECT_GE(dataset, 0);

         status = H5Dwrite( dataset, H5T_NATIVE_FLOAT, space, space, H5P_DEFAULT, &data );
         EXPECT_GE(status, 0);

         if( access == CreateWithAttr ) {
            hsize_t spatialAttrDims = AttributeData.size()/sizeof(char);

            size_t  attrSpace = H5Screate_simple ( 1, &spatialAttrDims, NULL );

            hid_t attrData = H5Acreate( dataset, AttributeName.c_str(), H5T_C_S1, attrSpace, H5P_DEFAULT, H5P_DEFAULT );
            EXPECT_GE(attrData, 0);

            status = H5Awrite ( attrData, H5T_C_S1, AttributeData.c_str() );
            EXPECT_GE(status, 0);

            status = H5Aclose( attrData );
            EXPECT_GE(status, 0);

            status = H5Sclose(attrSpace);
            EXPECT_GE(status, 0);
         }
         status = H5Dclose( dataset);
         EXPECT_GE(status, 0);
      }
      else
      {
        // Create global file
      }

      status = H5Sclose(space);
      EXPECT_GE(status, 0);
      status = H5Pclose(fapl);
      EXPECT_GE(status, 0);

      return data;
   }


   void close() {

      if( m_h5file != H5P_DEFAULT ) {

         herr_t status = H5Fclose( m_h5file );
         EXPECT_GE(status, 0);

         m_h5file = H5P_DEFAULT;
      }
   }

   ~File()
   {
      close();

      if ( m_exists )
      {
         int status = -1;
         if (m_saveOnDisk) {
            status = std::remove( m_file.c_str() );

            EXPECT_EQ(0, status);
         }
      }
   }

   const std::string & name() const
   { return m_file ; }

   hid_t fileId()
   { return m_h5file ; }


   static std::string tempName()
   {
      int rank =  MPIHelper::rank();
      char * name = 0;
      std::vector<char> buffer(8);
      if ( rank == 0)
      {
         std::generate( buffer.begin(), buffer.end(), randomChar);
      }
      MPI_Bcast( &buffer[0], buffer.size(), MPI_CHAR, 0, MPI_COMM_WORLD);

      return std::string( buffer.begin(), buffer.end());
   }

private:
   static char randomChar()
   {
     static char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklnmnopqrstuvwxyz0123456789";
     std::random_device rd;
     std::shuffle(std::begin(characters), std::end(characters) - 1, std::mt19937(rd()));
     return characters[ 0 ];
   }

   hid_t m_h5file;
   std::string m_file;
   bool m_exists = false;
   bool m_saveOnDisk = false;
};

TEST( h5mergeTest, MergeExistingFiles1 )
{
   if (MPIHelper::size() > 1)
   {
      std::string name   = File::tempName();
      File a( name, true );

      FileHandler reader( MPI_COMM_WORLD, name, "." );

      int status = a.open( File::Create, 5, false );
      ASSERT_FALSE( File::CannotCreateFile == status ) << "Local file can't be created." << std::endl;

      EXPECT_EQ( 5, status );

      File b( name, false );
      if (MPIHelper::rank() == 0)
      {
         status = b.open( File::Create, 0, true );
         ASSERT_FALSE( File::CannotCreateFile == status ) << "Global file can't be created." << std::endl;

         EXPECT_EQ( 0, status );
      }
      reader.setGlobalFileId ( b.fileId () );
      reader.setLocalFileId ( a.fileId () );

      EXPECT_EQ( 0, readDataset( a.fileId(), StdDataSetName.c_str(), &reader ));

      if (MPIHelper::rank() == 0)
      {
         b.close();
         EXPECT_EQ( 5, b.open( File::Open, -1, true ));
      }
   }

}

TEST( h5mergeTest, MergeExistingFiles2 )
{
   if (MPIHelper::size() > 1)
   {

      std::string name  = File::tempName();
      File a( name, true );

      FileHandler reader(  MPI_COMM_WORLD, name, "." );

      int status = a.open( File::CreateWithAttr, 5, false );
      ASSERT_FALSE( File::CannotCreateFile == status ) << "Local file can't be created." << std::endl;

      EXPECT_EQ( 5, status );

      File b( name, false );
      if (MPIHelper::rank() == 0)
      {
         status = b.open( File::CreateWithAttr, 0, true );
         ASSERT_FALSE( File::CannotCreateFile == status ) << "Global file can't be created." << std::endl;

         EXPECT_EQ( 0, status );
      }
      reader.setGlobalFileId ( b.fileId () );
      reader.setLocalFileId ( a.fileId () );

      EXPECT_EQ( 0, readDataset( a.fileId(), StdDataSetName.c_str(), &reader ));

      if (MPIHelper::rank() == 0)
      {
         b.close();
         EXPECT_EQ( 5, b.open( File::OpenWithAttr, -1, true ));
      }
   }

}

TEST( h5mergeTest, MergeExistingFiles3 )
{
   if (MPIHelper::size() > 1)
   {
      std::string name  = File::tempName();
      FileHandler reader(  MPI_COMM_WORLD, name, "." );

      EXPECT_EQ( -1, readDataset( NULL, StdDataSetName.c_str(), &reader ));
   }

}

TEST( h5mergeTest, MergeExistingFiles4 )
{
   if (MPIHelper::size() > 1)
   {
      std::string name  = File::tempName();
      FileHandler reader(  MPI_COMM_WORLD, name, "." );

      File a( name, true );

      int status = a.open( File::Create, 5, false );
      ASSERT_FALSE( File::CannotCreateFile == status ) << "Local file can't be created." << std::endl;

      EXPECT_EQ( 5, status );

      File b( name, false );
      if (MPIHelper::rank() == 0)
      {
         status = b.open( File::Create, 0, true );
         ASSERT_FALSE( File::CannotCreateFile == status ) << "Global file can't be created." << std::endl;

         EXPECT_EQ( 0, status );
      }
      reader.setGlobalFileId ( b.fileId () );
      reader.setLocalFileId  ( a.fileId () );

      EXPECT_EQ( -1, readDataset( a.fileId(), "", &reader ));
   }

}

TEST( h5mergeTest, MergeExistingFiles5 )
{
   FileHandler reader( MPI_COMM_WORLD, "", "." );
   reader.setSpatialDimension ( 3 );

   EXPECT_EQ( 0, reader.reallocateBuffers ( 5 ));
   EXPECT_EQ( 0, reader.reallocateBuffers ( 0 ));
   EXPECT_EQ( 0, reader.reallocateBuffers ( -1 ));
}

TEST( h5mergeTest, MergeExistingFiles6 )
{
   FileHandler reader(  MPI_COMM_WORLD, "", "." );

   if (MPIHelper::size() > 1)
   {
      hid_t value = ( MPIHelper::rank() == 0 ? -1 : 0 );
      EXPECT_EQ( -1, reader.checkError ( value ));

      EXPECT_EQ( 0, reader.checkError ( 0 ));

      EXPECT_EQ( -1, reader.checkError ( -1 ));
   }
}

TEST( h5mergeTest, ReuseExistingFiles7 )
{
   if (MPIHelper::size() > 1)
   {
      std::string name  = File::tempName();
      FileHandlerReuse reader(  MPI_COMM_WORLD, name, "." );

      File a( name, true );

      int status = a.open( File::CreateWithAttr, 5, true );
      ASSERT_FALSE( File::CannotCreateFile == status ) << "Local file can't be created." << std::endl;

      EXPECT_EQ( 5, status );

      reader.setLocalFileId(  a.fileId ());
      reader.setGlobalFileId ( a.fileId () );

      EXPECT_EQ( 0, readDataset( a.fileId(), StdDataSetName.c_str(), &reader ));

      if (MPIHelper::rank() == 0)
      {
         a.close();
         EXPECT_EQ( 5, a.open( File::OpenWithAttr, -1, true ));
      }
   }

}

TEST( h5mergeTest, AppendExistingFiles8 )
{
   if (MPIHelper::size() > 1)
   {
      std::string name  = File::tempName();
      FileHandlerAppend reader(  MPI_COMM_WORLD, name, "." );

      File a( name, true );

      int status = a.open( File::CreateWithAttr, 5, true );
      ASSERT_FALSE( File::CannotCreateFile == status ) << "Local file can't be created." << std::endl;

      EXPECT_EQ( 5, status );

      reader.setLocalFileId(  a.fileId ());
      reader.setGlobalFileId ( a.fileId () );

      EXPECT_EQ( 0, readDataset( a.fileId(), StdDataSetName.c_str(), &reader ));

      if (MPIHelper::rank() == 0)
      {
         a.close();
         EXPECT_EQ( 5, a.open( File::OpenWithAttr, -1, true ));
      }
   }

}
