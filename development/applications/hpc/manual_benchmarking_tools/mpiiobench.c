/**
  MPI-IO benchmark program that measures how long it takes to write or read a file

  Parameters
    - direction = read / write     -- whether to read or write the file
    - measure =   gross / nett     -- whether to include opening of the file (gross) or only measure the pure I/O bit
    - coordination = sync / async  -- whether to use collective or independent I/O
    - block size                   -- size of each write in bytes
    - total size                   -- total size of the file
    - file name                    -- Name of the file

  Outputs the time it takes to complete the read or write of the file


  Note: Compile this with C99 standard (i.e. use the -std=c99 parameter with gcc or intel)
**/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <mpi.h>

#define MIN( a, b) ( (a)<(b)? (a) : (b) )
#define MAX( a, b) ( (a)>(b)? (a) : (b) )

int fileExists(const char * file)
{
   struct stat  s;
   int x = stat( file, &s );

   return x ==0 || errno != ENOENT;
}

double getTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

int main( int argc, char ** argv)
{
   int x;

   MPI_Init( & argc, & argv);

   int rank, procs;
   MPI_Comm_size( MPI_COMM_WORLD, &procs);
   MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    // Parse command line
    //
   assert( argc >= 7 && "mpiiobench [write|read] [nett|gross] [sync|async] BLOCK_SIZE TOTAL_SIZE FILE");

   int writing = strcmp( argv[1], "write") == 0;
   int reading = strcmp( argv[1], "read") == 0;
   assert( reading || writing && "Only read or write mode is supported");

   int nett = strcmp( argv[2], "nett") == 0;
   int gross = strcmp( argv[2], "gross") == 0;
   assert( reading || writing && "Choose what time to measure");

   int sync = strcmp( argv[3], "sync") == 0;
   int async = strcmp( argv[3], "async") == 0;
   assert( sync || async && "Choose sync mode or async");


   char * endptr = 0;
   const long long blockSize= strtoll( argv[4], &endptr, 10);
   assert( endptr == NULL || *endptr == '\0' && "Cannot parse BLOCK_SIZE");
   assert( blockSize > 0 && "BLOCK_SIZE should be greater than 0");

   const long long totalSize = strtoll( argv[5], &endptr, 10);
   assert( endptr == NULL || *endptr == '\0' && "Cannot parse TOTAL_SIZE");
   assert( totalSize > 0 && "TOTAL_SIZE should be greater than 0");

   char * fileName = argv[6];

   assert( !writing || !fileExists(fileName) && "Output file already exists" );


   // Allocate block size and fill with silly data
   char * block = malloc( blockSize);
   assert( block && "Cannot allocate memory" );


   // Set MPI_INFO
   MPI_Info info;
   MPI_Info_create(&info);

   MPI_Info_set( info, "collective_buffering", "false");

   if (writing)
   {
      MPI_Info_set( info, "access_style", "write_once,sequential");

      for (size_t i = 0; i < blockSize; ++i)
         block[i] = i & 0xff;

      // Generate file
      double t0 = getTime();

      MPI_File file;
      MPI_File_open( MPI_COMM_WORLD, fileName,
               MPI_MODE_WRONLY | MPI_MODE_EXCL | MPI_MODE_CREATE | ( sync ? MPI_MODE_SEQUENTIAL : 0),
               info,
               & file );

      double s0 = getTime();

      if (sync)
      {
         for ( size_t size = 0; size < (size_t) totalSize; size += procs * blockSize )
         {
            size_t bs = MIN( MAX(0ll, totalSize - size - rank * blockSize), blockSize);
            assert( bs <= INT_MAX);
            MPI_File_write_ordered( file, block, bs, MPI_CHAR, MPI_STATUS_IGNORE);
        }
      }
      else
      {
         for ( size_t size = 0; size < (size_t) totalSize; size += procs * blockSize )
         {
            size_t bs = MIN( MAX(0ll, totalSize - size - rank * blockSize), blockSize);
            assert( bs <= INT_MAX);
            if (bs > 0)
               MPI_File_write_at( file, size + rank * blockSize, block, bs, MPI_CHAR, MPI_STATUS_IGNORE);
        }
      }


      MPI_File_close(&file);
      double s1 = getTime();
      double t1 = getTime();


      if (rank == 0)
         printf("%f\n", gross?(t1-t0):(s1-s0));
   }

   if (reading)
   {
      double t0 = getTime();
      MPI_File file;
      MPI_File_open( MPI_COMM_WORLD, fileName,
               MPI_MODE_RDONLY | (sync ? MPI_MODE_SEQUENTIAL : 0),
               info,
               & file );
      double s0 = getTime();

      if (sync)
      {
         for ( size_t size = 0; size < (size_t) totalSize; size += procs * blockSize )
         {
            size_t bs = MIN( MAX(0ll, totalSize - size - rank * blockSize), blockSize);
            assert( bs <= INT_MAX);
            MPI_File_read_ordered( file, block, bs, MPI_CHAR, MPI_STATUS_IGNORE);
         }
      }
      else
      {
         for ( size_t size = 0; size < (size_t) totalSize; size += procs * blockSize )
         {
            size_t bs = MIN( MAX(0ll, totalSize - size - rank * blockSize), blockSize);
            assert( bs <= INT_MAX);
            if (bs > 0)
               MPI_File_read_at( file, size + rank * blockSize, block, bs, MPI_CHAR, MPI_STATUS_IGNORE);
        }
      }
      double s1 = getTime();

      MPI_File_close(&file);
      double t1 = getTime();

      if (rank == 0)
         printf("%f\n", gross?(t1-t0):(s1-s0));
   }

   MPI_Info_free(&info);
   MPI_Finalize();
   return 0;
}
