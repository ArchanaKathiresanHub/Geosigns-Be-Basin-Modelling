#include "VreOutputGrid.h"
#include "VreGrid.h"
#include "layer_iterators.h"

   
VreOutputGrid::VreOutputGrid( DM * mapViewOfDomain, const LayerList & allLayers )
   : m_vitriniteReflectance( getNumberOfNodes( mapViewOfDomain, allLayers) )
{
}

double * VreOutputGrid::getVRe()
{
   return &m_vitriniteReflectance[0]; 
}

int VreOutputGrid::getSize() const
{
   return m_vitriniteReflectance.size(); 
}

void VreOutputGrid :: exportToModel( const LayerList & allLayers, const Boolean2DArray & isValidNeedle)
{
   // Traverse the layers from the basement to surface. This way values in the
   // m_vitriniteReflectance array will stay at the same spot when new layers are added.
   Basin_Modelling::Layer_Iterator layers;
   layers.Initialise_Iterator ( allLayers,
         Basin_Modelling::Ascending,
         Basin_Modelling::Sediments_Only,
         Basin_Modelling::Active_Layers_Only
      );

   // Maintain an offset in the vrOutputGrid to remember where the layer starts
   int gridOffset = 0;

   // Iterate over all the layers
   while( ! layers.Iteration_Is_Done () )
   {
      LayerProps_Ptr currentLayer = layers.Current_Layer ();
 
      // Get the dimensions of the current layer
      int xs, ys, zs, xm, ym, zm;
      DMDAGetCorners(currentLayer->layerDA,&xs,&ys,&zs,&xm,&ym,&zm);
      
      // determine its size
      const int layerGridSize = xm * ym * zm;

      // Create a property vector for VR. This vector doesn't need to be a
      // global vector since it doesn't need to know about ghost points
      if (!currentLayer->Vre) {
         DMCreateGlobalVector(currentLayer ->layerDA, &currentLayer ->Vre);
      }

      // Set the default value on the entire vector
      VecSet(currentLayer ->Vre, CAULDRONIBSNULLVALUE);

      // Get a proxy to the vr values in this layer.
      double *** vre;
      DMDAVecGetArray(currentLayer ->layerDA, currentLayer ->Vre, &vre);

      for (int i = xs; i < xs + xm; ++i)
      {
         for (int j = ys; j < ys + ym; ++j)
         {
            if (! isValidNeedle(i,j))
               continue;

            for (int k = zs; k < zs + zm; ++k)
            {
               int node = getNodeNumber( gridOffset, zm, ym, xm, k-zs, j-ys, i-xs );
               assert( node < m_vitriniteReflectance.size() );
               assert( node >= 0 );
               vre[k][j][i] = m_vitriniteReflectance[ node ];
            }
         }
      }
         
      // Clear any old reference to the previous layer.
      DMDAVecRestoreArray(currentLayer->layerDA, currentLayer->Vre, &vre);

      // Go the next layer
      gridOffset += layerGridSize;
      layers++;
   }
}
