// This program 'Shuffle' permutes lines in a file. It assumes lines are
// separated by the '\n' newline character.   
//
// Notes:
//  1) The same can be done as a one-liner in Perl
//  2) This program is writte in  C99, so be sure to compile it with -std=c99
//     when you compile it with gcc.

#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>


size_t getSize(const char * fileName)
{
   struct stat buf;
   stat( fileName, &buf);
   return buf.st_size;
}

int main(int argc, char ** argv)
{
   assert( argc >= 2 && "Usage: shuffle FILENAME [SEED]");

   const char * fileName=argv[1];
   const size_t fileSize = getSize(fileName);

   unsigned int seed = time(NULL);
   if (argc >= 3)
      seed = atoi(argv[2]);
   srand(seed);

   /* Open the file */
   int fd = open( fileName, O_RDONLY);
   const char * file = mmap( 0, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
   
   /* Find location of newlines / the line starts */
   size_t * lineStarts = malloc( fileSize * sizeof(size_t) );
   size_t * lineLengths = malloc( fileSize * sizeof(size_t) );

   size_t numberOfLines = 0;
   lineStarts[0] = 0;
   for ( size_t i = 0; i < fileSize; ++i)
   {
      if (file[i] == '\n')
      {
         lineLengths[ numberOfLines ] = i - lineStarts[numberOfLines];
         numberOfLines++;
         lineStarts[numberOfLines] = i+1;
      }
   }
   lineLengths[numberOfLines]=fileSize - lineStarts[numberOfLines];
   if (lineLengths[numberOfLines] > 0)
      numberOfLines++;

   /* Permute the line starts */
   for (size_t i = numberOfLines; i > 0; --i)
   {
      // pick a random line
      size_t j = i * (rand() / (RAND_MAX + 1.0));

      // print the random line
      assert( lineLengths[j] < INT_MAX);
      printf("%.*s\n", (int) lineLengths[j], &file[ lineStarts[j] ]);

      // move it to the end of the array
      size_t k = lineStarts[j];
      lineStarts[j] = lineStarts[i-1];
      lineStarts[i-1] = k;

      size_t l = lineLengths[j];
      lineLengths[j] = lineLengths[i-1];
      lineLengths[i-1] = l;
   }

   /* Clean up */
   free(lineLengths);
   free(lineStarts);
   munmap( (void *) file, fileSize );

   return 0;
}
