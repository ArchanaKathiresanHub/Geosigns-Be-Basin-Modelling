#ifndef FASTCAULDRON_VREGRID_H
#define FASTCAULDRON_VREGRID_H

#include <petsc.h>
#include "layer.h"

class VreGrid
{
public:
   /// Compute number of grid nodes that will contain Vre results given a list of
   /// layers and a map view of the domain.
   static int getNumberOfNodes( DM * mapViewOfDomain, const LayerList & layerList);

   /// Compute a locally unique node number on the basis of grid coordinates
   /// \param offset The offset in the local Vre grid for the current layer
   /// \param maxK The number of local nodes in the Z direction in the current layer
   /// \param maxJ The number of local nodes in the Y direction in the current layer
   /// \param maxI The number of local nodes in the X direction in the current layer
   /// \param k The local grid coordinate in the Z-direction in the current layer
   /// \param j The local grid coordinate in the Y-direction in the current layer
   /// \param i The local grid coordinate in the X-direction in the current layer
   /// \return A number that identifies a node in the Vre grid.
   static int getNodeNumber( int offset, int maxK, int maxJ, int maxI, int k, int j, int i)
   { 
      (void) maxI; // ignore the maxI parameter, because it isn't actually used.
      return offset + k + maxK * ( j + maxJ * i ); 
   }
};

#endif
