#include "VreGrid.h"
#include "layer.h"
#include "layer_iterators.h"


int VreGrid::getNumberOfNodes( DM * mapDA, const LayerList & layerList )
{
   int xs, ys, zs, xm, ym, zm;
   DMDAGetCorners( *mapDA, PETSC_NULL, PETSC_NULL, PETSC_NULL ,&xm, &ym, PETSC_NULL);

   Basin_Modelling::Layer_Iterator layers;
   layers.Initialise_Iterator ( layerList, 
         Basin_Modelling::Descending, 
         Basin_Modelling::Sediments_Only,
         Basin_Modelling::Active_And_Inactive_Layers
      );
 
   int nodeCount = 0;
   while ( ! layers.Iteration_Is_Done () ) 
   {
      nodeCount += xm * ym  * ( layers.Current_Layer()->getMaximumNumberOfElements() + 1 );
      layers++;
   }

   return nodeCount;
}


