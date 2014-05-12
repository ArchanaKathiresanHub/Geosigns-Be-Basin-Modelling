#ifndef FASTCAULDRON_VREGRID_H
#define FASTCAULDRON_VREGRID_H

#include <petsc.h>
#include "layer.h"

class VreGrid
{
public:
   /// Compute number of grid nodes that will contain Vre results given a list of
   /// layers and a map view of the domain.
   static int getNumberOfVreGridNodes( DM * mapViewOfDomain, const LayerList & layerList);

   /// Compute a unique node number on the basis of grid coordinates
   static int getNodeNumber( int offset, int K, int J, int I, int k, int j, int i)
   { return offset + k + K * ( j + J * i ); }
};

#endif
