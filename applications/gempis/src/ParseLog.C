
#include <stdio.h>
#include <stdlib.h>


int main( int argc, char ** argv )
{
  /*
  if ( argc < 3 )
  {
    printf( "Usage: %s <file> <user>\n", argv[0] );
    exit( 1 );
  }
  */
  int state = 0;
  int gotline = 0;
  
  long sts, ets;
  char sresource[128], eresource[128];
  char shost[128], ehost[128];
  char suserid[128], euserid[128];
  int sjobid, ejobid;
  char sdate[128], edate[128]; 
  char stime[128], etime[128]; 
  int snumcpus, eresult; 

  FILE * fdstart, *fdend;
  fdstart = fopen( "start.log", "r" );
  fdend = fopen( "end.log", "r" );
  if ( fdstart == NULL || fdend == NULL )
    {
      printf( "error opening file start.log or end.log\n" );
      exit( 1 );
    }

  // will only have data for same resource
  while ( fscanf( fdend, "%ld %s %s %s %d %s %s %d", 
                  &ets, eresource, ehost, euserid, &ejobid, edate, etime, &eresult ) == 8 )
    {
      // reset file pointer, and find the beginning
      fseek( fdstart, 0L, SEEK_SET );
      while ( fscanf( fdstart, "%ld %s %s %s %d %s %s %d", 
                      &sts, sresource, shost, suserid, &sjobid, sdate, stime, &snumcpus ) == 8 )
        {
          // start was later than end, skip
          if ( sts > ets )
            {
              continue;
            }
          // is there no end result
          if ( ejobid != sjobid )
            {
              continue;
            }

          // we have compute time difference
          long diff = ( ets - sts ) * snumcpus;

          // upper limit is a run of 28 days
          int maxdiff = snumcpus * 28 * 86400;
          if ( diff < maxdiff )
            {
              // output format example
              // 00009209 2007-06-06 nlohu0 0 21350
              printf( "%08d %s %s %d %ld\n", sjobid, sdate, suserid, eresult, diff );
            }

          // whether or not it printed, we're done
          break;
        }
    }

  fclose( fdend );
  fclose( fdstart );
}
