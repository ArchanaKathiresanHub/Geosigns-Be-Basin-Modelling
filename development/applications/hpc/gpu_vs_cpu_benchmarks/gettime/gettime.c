#include "gettime.h"

#ifdef _WIN32

#include <windows.h>
#include <time.h>

#else

#include <sys/time.h>

#endif


double getTime()
{

#ifdef _WIN32
  LARGE_INTEGER frequency = 0;
  LARGE_INTEGER count = 0;

  if (!QueryPerformanceCounter(&count) 
      || !QueryPerformanceFrequency(&frequency))
  {
    fprintf(stderr, "Warning: falling back to lower resolution timer\n");
    return (double) clock() / CLOCKS_PER_SEC;
  }

  return (double) count / frequency;

#else
  struct timeval t;
  gettimeofday(&t, NULL);

  return t.tv_sec + 1.0e-6 * t.tv_usec;
#endif
}


