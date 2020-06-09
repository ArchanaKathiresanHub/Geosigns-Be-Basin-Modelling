#include "gettime.h"

#include <stdio.h>
#include <sys/time.h>

namespace hpc
{

double getTime()
{
  struct timeval t;
  gettimeofday(&t, NULL);

  return t.tv_sec + 1.0e-6 * t.tv_usec;
}

}
