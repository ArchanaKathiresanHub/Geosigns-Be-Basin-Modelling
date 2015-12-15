#include "RewriteFileName.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))


size_t rewriteFileName( const char * pattern, const char *  fileName, int mpiRank, int mpiSize, char *buffer, size_t bufferSize )
{
   assert( pattern != NULL );
   assert( fileName != NULL);
   assert( mpiRank >= 0 );
   assert( mpiSize > 0);
   assert( buffer != NULL || bufferSize == 0);

   // first count length of the output string
   const ptrdiff_t patternLength = strlen( pattern );
   const ptrdiff_t fileNameLength = strlen( fileName );

   const char * const patternBegin = pattern;
   const char * const patternEnd = pattern + patternLength;
   char * const bufferBegin = buffer;
   char * const bufferEnd = buffer + bufferSize;
   do 
   {
      const char * nextPatternToken = strchr(pattern, '{');

      if (nextPatternToken == NULL)
         nextPatternToken = patternEnd;
      
      {
         assert( nextPatternToken >= pattern );
         ptrdiff_t length = nextPatternToken - pattern;
         memcpy( buffer, pattern, MAX(0l, MIN( bufferEnd - buffer, length )) );
         buffer += length;
         pattern += length;
      }
      
      if (nextPatternToken != patternEnd)
      {
         if ( strncmp( pattern , "{MPI_RANK}", 10 ) == 0)
         {
            buffer += snprintf( buffer, MAX(0, bufferEnd - buffer), "%d", mpiRank);
            pattern+=10;
         }
         else if ( strncmp( pattern, "{MPI_SIZE}", 10 ) == 0)
         {
            buffer += snprintf( buffer, MAX(0, bufferEnd - buffer), "%d", mpiSize);
            pattern+=10;
         }
         else if ( strncmp( pattern, "{NAME}", 6) == 0)
         {
            memcpy( buffer, fileName, MAX(0l, MIN( bufferEnd - buffer, fileNameLength)));
            buffer += fileNameLength;
            pattern+=6;
         }
         else
         {
            if (buffer < bufferEnd)
               *buffer = '{';
           
            buffer++;
            pattern++;
         }
      }
   } while ( pattern < patternEnd );

   if (buffer < bufferEnd)
      *buffer = '\0';
   buffer++;

   return buffer - bufferBegin;
}


