/**
  POSIX IO benchmark program that measures how long it takes to write or read a file

  Parameters
    - direction = read / write     -- whether to read or write the file
    - measure =   gross / nett     -- whether to include opening of the file (gross) or only measure the pure I/O bit
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

    // Parse command line
    //
   assert( argc >= 6 && "raw [write|read] [nett|gross] BLOCK_SIZE TOTAL_SIZE FILE");

   int writing = strcmp( argv[1], "write") == 0;
   int reading = strcmp( argv[1], "read") == 0;
   assert( reading || writing && "Only read or write mode is supported");

   int nett = strcmp( argv[2], "nett") == 0;
   int gross = strcmp( argv[2], "gross") == 0;
   assert( reading || writing && "Choose what time to measure");

   char * endptr = 0;
   const long long blockSize= strtoll( argv[3], &endptr, 10);
   assert( endptr == NULL || *endptr == '\0' && "Cannot parse BLOCK_SIZE");
   assert( blockSize > 0 && "BLOCK_SIZE should be greater than 0");

   const long long totalSize = strtoll( argv[4], &endptr, 10);
   assert( endptr == NULL || *endptr == '\0' && "Cannot parse TOTAL_SIZE");
   assert( totalSize > 0 && "TOTAL_SIZE should be greater than 0");

   const char * fileName = argv[5];

   assert( !writing || !fileExists(fileName) && "Output file already exists" );


   // Allocate block size and fill with silly data
   char * block = malloc( blockSize);
   assert( block && "Cannot allocate memory" );

   if (writing)
   {
      int x;

      for (size_t i = 0; i < blockSize; ++i)
         block[i] = i & 0xff;

      // Generate file
      double t0 = getTime();
      int fd = open( fileName, O_CREAT | O_WRONLY | O_EXCL, S_IRWXU );
      if (fd <= 0)
        perror("Opening file for writing");
      assert( fd > 0 && "Cannot open file");

      double s0 = getTime();

      for ( size_t size = 0; size < (size_t) totalSize; size += blockSize )
      {
         size_t n = blockSize;
         while (n > 0)
         {
            ssize_t x = write( fd, block, n);
            assert( x > 0 && "I/O error writing file");
            n -= x;
         }
      }

      x = close(fd);
      assert( x == 0 && "Error when closing file");
      double s1 = getTime();
      double t1 = getTime();


      printf("%f\n", gross?(t1-t0):(s1-s0));
   }

   if (reading)
   {
      double t0 = getTime();
      int fd = open( fileName, O_RDONLY );
      if (fd <= 0)
        perror("Opening file for reading");

      assert( fd > 0 && "Cannot open file");
      double s0 = getTime();

      for ( size_t size = 0; size < (size_t) totalSize; size += blockSize )
      {
         size_t n = blockSize;
         while (n > 0)
         {
            ssize_t x = read( fd, block, n);
            assert( x > 0 && "File is not of specified size");
            n -= x;
         }
      }
      double s1 = getTime();

      int x = close(fd);
      assert( x == 0 && "Error when closing file");
      double t1 = getTime();

      printf("%f\n", gross?(t1-t0):(s1-s0));
   }

   return 0;
}
