#include "../src/HDF5VirtualFileDriver.h"
#include "../src/RewriteFileName.h"
#include <mpi.h>
#include <hdf5.h>
#include <stdio.h>

#include <gtest/gtest.h>

#include <string>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

// This is not really a unit test but more an integration test: Testing the
// integration of the virtual file driver within HDF5. 
// The driver enables the following
// - Open an existing file for Read&Write
//     1) FILE-SIZE-RANK exists, but FILE doesn't
//     2) FILE-SIZE-RANK exists, and also FILE
//     3) FILE-SIZE-RANK doesn't exist, but FILE does
//     4) FILE-SIZE-RANK doesn't exist, and also FILE doesn't
//     5) FILE-SIZE-0 exists, FILE-SIZE-1 doesn't, FILE does
//     6) FILE-SIZE-0 doesn't, FILE-SIZE-1 does, FILE doesn't

// - Create a file and open it for Read&Write
//     1) FILE-SIZE-RANK exists, but FILE doesn't
//     2) FILE-SIZE-RANK exists, and also FILE
//     3) FILE-SIZE-RANK doesn't exist, but FILE does
//     4) FILE-SIZE-RANK doesn't exist, and also FILE doesn't
//     5) FILE-SIZE-0 exists, FILE-SIZE-1 doesn't, FILE does
//     6) FILE-SIZE-0 doesn't, FILE-SIZE-1 does, FILE doesn't

// - Truncate a file and open it for Read&Write
//     1) FILE-SIZE-RANK exists, but FILE doesn't
//     2) FILE-SIZE-RANK exists, and also FILE
//     3) FILE-SIZE-RANK doesn't exist, but FILE does
//     4) FILE-SIZE-RANK doesn't exist, and also FILE doesn't
//


namespace
{
  static const char * RewritePattern = "{NAME}-{MPI_SIZE}-{MPI_RANK}";
  static const char * StdDataSetName = "/DummyData";
  using OneFilePerProcess::rewriteFileName;
}

struct MPI
{
   MPI()
   {
      MPI_Init(NULL, NULL);
      H5Eset_auto( H5E_DEFAULT, NULL, NULL);
   }

   ~MPI()
   {
      MPI_Finalize();
   }

   static MPI & instance()
   {
      static MPI object;
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
   enum Access { None, Create, Truncate, Open };
   enum ErrorCode { CannotCreateFile = -1, CannotTruncateFile=-2, CannotWriteToFile=-3, CannotReadFile=-4, CannotOpenFile = -5};

   // Construct a file name with either a normal name or a name as the OFPP driver would generate it.
   File( const std::string & name, bool extendedName)
      : m_file( name )
      , m_exists( false )
      , m_useOfpp( false )
      , m_extendedName( extendedName)
   {

      int mpiRank = MPI::rank();
      int mpiSize = MPI::size();

      if (extendedName)
      {
         std::vector<char> buffer( rewriteFileName( RewritePattern, name.c_str(), mpiRank, mpiSize, 0, 0 ) );
         rewriteFileName( RewritePattern, name.c_str(), mpiRank, mpiSize, &buffer[0], buffer.size() );
         m_file = std::string( &buffer[0], buffer.size() );
      }
   }

   // Open the file (by means as defined by 'access'). If useOFPP is defined, the OFPP driver is used. 
   // (Note: it is silly to create an instance of this class with an extended
   // file name and to use the OFPP driver again.) If 'data' is negative it will try to read from 
   // the file. If data is positive it will write that number to the file. 
   // The return value is the data that is has written to or read from the file. It will
   // return one of the ErrorCodes (all negative values) in case the operation fails.
   int open( Access access, bool useOFPP, int data)
   {
      const int rank = MPI::rank();

      herr_t status = 0;
      hsize_t dims[]  = { 1 };
      
      hid_t fapl = H5P_DEFAULT;
      if (useOFPP)
      {
         fapl = H5Pcreate(H5P_FILE_ACCESS);
         EXPECT_GE( fapl, 0);
         H5Pset_fapl_ofpp( fapl, MPI_COMM_WORLD, RewritePattern, false);
      }

      hid_t file = 0;
      switch ( access )
      {
         case None:
            break;

         case Create:
            file = H5Fcreate(m_file.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, fapl);
            m_exists = (file >= 0);
            if (!m_exists)
            {
               if (useOFPP)
                  H5Pclose(fapl);

               return CannotCreateFile;
            }
            break;

         case Truncate:
            file = H5Fcreate(m_file.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
            m_exists = (file >= 0);
            if (!m_exists)
            {
               if (useOFPP)
                  H5Pclose(fapl);

               return CannotTruncateFile;
            }
            break;

         case Open:
            file = H5Fopen(m_file.c_str(), H5F_ACC_RDWR, fapl);
            if (file < 0)
            {
               if (useOFPP)
                  H5Pclose(fapl);

               return CannotOpenFile;
            }
            break;

         default:
            ADD_FAILURE() << "Fell through switch-case statement";
      }

      m_useOfpp = useOFPP;

      hid_t space = H5Screate_simple( 1, dims, dims );
      EXPECT_GE(space, 0);

      if (data < 0)
      {
         data = -1;
         hid_t dataset = H5Dopen( file, StdDataSetName, H5P_DEFAULT);
         EXPECT_GE(dataset, 0);
         status = H5Dread( dataset, H5T_NATIVE_INT, space, space, H5P_DEFAULT, &data);
         EXPECT_GE(status, 0);
         status = H5Dclose(dataset);
         EXPECT_GE(status, 0);
      }
      else
      {
         EXPECT_GT(data, 0);
         hid_t dataset = H5Dcreate( file, StdDataSetName, H5T_NATIVE_INT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
         EXPECT_GE(dataset, 0);
         status = H5Dwrite( dataset, H5T_NATIVE_INT, space, space, H5P_DEFAULT, & data);
         EXPECT_GE(status, 0);
         status = H5Dclose( dataset);
         EXPECT_GE(status, 0);
      }

      status = H5Sclose(space);
      EXPECT_GE(status, 0);
      status = H5Fclose(file);
      EXPECT_GE(status, 0);
      status = H5Pclose(fapl);
      EXPECT_GE(status, 0);

      if (useOFPP)
         H5Pclose( fapl );

      return data;
   }

   ~File()
   {
      if (m_exists)
      {
         int status = -1;
         if (m_useOfpp)
           status = std::remove( File( m_file, true).name().c_str() );
         else
           status = std::remove( m_file.c_str());

         EXPECT_EQ(0, status);
      }
   }

   const std::string & name() const
   { return m_file ; }

   static std::string tempName() 
   {
      int rank =  MPI::rank();
      char * name = 0;
      std::vector<char> buffer(8);
      if ( rank == 0)
      {
         std::generate( buffer.begin(), buffer.end(), randomChar);
      } 
      MPI_Bcast( &buffer[0], buffer.size(), MPI_CHAR, 0, MPI_COMM_WORLD);
      
      return std::string( buffer.begin(), buffer.end());
   }

   void forgetFile()
   {
      m_exists = false;
   }

private:
   File & operator=(File & );
   File( const File & );

   static char randomChar()
   {
      static const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklnmnopqrstuvwxyz0123456789";
      static bool seeded = false;
      if (!seeded)
      {
         srand( time(0) );
         seeded = true;
      }
      size_t index = (sizeof(characters) - 1) * ( rand() / (double) RAND_MAX );
      return characters[ index ];
   }

   std::string m_file;
   bool m_exists;
   bool m_useOfpp;
   bool m_extendedName;
};



TEST( HDF5VirtualFileDriverTest, OpenExistingFile1 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 1);
   MPI::barrier();

   File c( name, false );
   EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, false, -1));
   EXPECT_EQ( 1, c.open( File::Open, true, -1) );
}

TEST( HDF5VirtualFileDriverTest, OpenExistingFile2 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 2);
   if (MPI::rank() == 0)
   {
      b.open( File::Create, false, 3);
   }
   MPI::barrier();

   File c( name, false );
   EXPECT_EQ( 3, c.open( File::Open, false, -1));
   EXPECT_EQ( 2, c.open( File::Open, true, -1) );
}

TEST( HDF5VirtualFileDriverTest, OpenExistingFile3 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   if (MPI::rank() == 0)
      b.open( File::Create, false, 4);

   MPI::barrier();

   File c( name, false );
   EXPECT_EQ( 4, c.open( File::Open, false, -1));
   EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, true, -1) );
}

TEST( HDF5VirtualFileDriverTest, OpenExistingFile4 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   File c( name, false );
   EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, false, -1));
   EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, true, -1) );
}


TEST( HDF5VirtualFileDriverTest, OpenExistingFile5 )
{
   if (MPI::size() > 1)
   {
      std::string name  = File::tempName();
      File a( name, true);
      File b( name, false);

      if (MPI::rank() == 0)
      {
         a.open( File::Create, false, 5);
         b.open( File::Create, false, 6);
      }

      MPI::barrier();

      File c( name, false );

    
      EXPECT_EQ( 6, c.open( File::Open, false, -1));
      EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, true, -1) );
   }
}

TEST( HDF5VirtualFileDriverTest, OpenExistingFile6 )
{
   if (MPI::size() > 1)
   {
      std::string name  = File::tempName();
      File a( name, true);
      File b( name, false);

      if (MPI::rank() == 1)
      {
         a.open( File::Create, false, 5);
      }

      File c( name, false );

      EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, false, -1) );
      EXPECT_EQ( File::CannotOpenFile, c.open( File::Open, true, -1) );
   }
}

TEST( HDF5VirtualFileDriverTest, CreateFile1 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 1);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( 10, File( name, false).open( File::Create, false, 10));

   EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, true, 11) );
}

TEST( HDF5VirtualFileDriverTest, CreateFile2 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 2);
   if (MPI::rank() == 0)
      b.open( File::Create, false, 3);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, false, 12));

   EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, true, 13) );
}

TEST( HDF5VirtualFileDriverTest, CreateFile3 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   if (MPI::rank() == 0)
      b.open( File::Create, false, 4);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, false, 14));

   EXPECT_EQ( 15, File(name, false).open( File::Create, true, 15) );
}

TEST( HDF5VirtualFileDriverTest, CreateFile4 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   if (MPI::rank() == 0)
      EXPECT_EQ( 16, File(name, false).open( File::Create, false, 16));

   EXPECT_EQ( 17, File(name, false).open( File::Create, true, 17) );
}

TEST( HDF5VirtualFileDriverTest, CreateFile5 )
{
   if (MPI::size() > 1)
   {
      std::string name  = File::tempName();
      File a( name, true);
      File b( name, false);

      if (MPI::rank() == 0)
      {
         a.open( File::Create, false, 5);
         b.open( File::Create, false, 6);
      }
      MPI::barrier();

      if (MPI::rank() == 0)
         EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, false, 18));

      EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, true, 19) );
   }
}

TEST( HDF5VirtualFileDriverTest, CreateFile6 )
{
   if (MPI::size() > 1 )
   {
      std::string name  = File::tempName();
      File a( name, true);
      File b( name, false);

      if (MPI::rank() == 1)
      {
         a.open( File::Create, false, 5);
      }
      MPI::barrier();

      if (MPI::rank() == 0)
         EXPECT_EQ( 20, File(name, false).open( File::Create, false, 20));
      EXPECT_EQ( File::CannotCreateFile, File(name, false).open( File::Create, true, 21) );
   }
}


TEST( HDF5VirtualFileDriverTest, TruncateFile1 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 1);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( 18, File( name, false).open( File::Truncate, false, 18));

   EXPECT_EQ( 19, File(name, false).open( File::Truncate, true, 19) );

   a.forgetFile();
}

TEST( HDF5VirtualFileDriverTest, TruncateFile2 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   a.open( File::Create, false, 2);
   if ( MPI::rank() == 0)
      b.open( File::Create, false, 3);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( 20, File(name, false).open( File::Truncate, false, 20));
   EXPECT_EQ( 21, File(name, false).open( File::Truncate, true, 21) );

   a.forgetFile();
   b.forgetFile();
}

TEST( HDF5VirtualFileDriverTest, TruncateFile3 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   if ( MPI::rank() == 0)
      b.open( File::Create, false, 4);
   MPI::barrier();

   if (MPI::rank() == 0)
      EXPECT_EQ( 22, File(name, false).open( File::Truncate, false, 22));
   EXPECT_EQ( 23, File(name, false).open( File::Truncate, true, 23) );

   b.forgetFile();
}

TEST( HDF5VirtualFileDriverTest, TruncateFile4 )
{
   std::string name  = File::tempName();
   File a( name, true);
   File b( name, false);

   if (MPI::rank() == 0)
      EXPECT_EQ( 24, File(name, false).open( File::Truncate, false, 24));
   EXPECT_EQ( 25, File(name, false).open( File::Truncate, true, 25) );
}

