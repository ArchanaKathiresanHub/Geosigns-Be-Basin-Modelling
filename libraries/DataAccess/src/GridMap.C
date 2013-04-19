#include <math.h>
#if defined(_WIN32) || defined (_WIN64)
#else
#include <values.h>
#endif
#include <assert.h>
#include <iomanip>

#include "Interface/GridMap.h"
#include "Interface/Grid.h"


#include "array.h"

using namespace DataAccess;
using namespace Interface;

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

unsigned int GridMap::firstI (void) const
{
   return getGrid()->firstI();
}

unsigned int GridMap::firstJ (void) const
{
   return getGrid()->firstJ();
}

unsigned int GridMap::firstK (void) const
{
   return 0;
}

unsigned int GridMap::lastI (void) const
{
   return getGrid ()->lastI();
}

unsigned int GridMap::lastJ (void) const
{
   return getGrid ()->lastJ();
}

unsigned int GridMap::lastK (void) const
{
   return getDepth () - 1;
}
