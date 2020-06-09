//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "VreOutputGrid.h"
#include "VreGrid.h"
#include "layer_iterators.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

VreOutputGrid::VreOutputGrid( DM * mapViewOfDomain, const LayerList & allLayers )
   : m_vitriniteReflectance( getNumberOfNodes( mapViewOfDomain, allLayers) )
{
   m_previousTime = 0.0;
}

double * VreOutputGrid::getVRe()
{
   return &m_vitriniteReflectance[0]; 
}

int VreOutputGrid::getSize() const
{
   return m_vitriniteReflectance.size(); 
}

void VreOutputGrid :: updateTimeStep( const LayerList & allLayers, const Boolean2DArray & isValidNeedle, const double currentTime)
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

      for (int i = xs; i < xs + xm; ++i)
      {
         for (int j = ys; j < ys + ym; ++j)
         {
            if (! isValidNeedle(i,j))
               continue;

            int prevNode = 0;

            for (int k = zs; k < zs + zm; ++k)
            {
               int node = getNodeNumber( gridOffset, zm, ym, xm, k-zs, j-ys, i-xs );
               assert( node < m_vitriniteReflectance.size() );
               assert( node >= 0 );
 
               if (k == zs) {
                  prevNode = node;
                  continue;
               }

               double thickness = currentLayer->getSolidThickness (i, j, k - 1, currentTime);
             
               if (thickness == Utilities::Numerical::IbsNoDataValue or thickness == 0.0) 
               {
                  m_vitriniteReflectance[ node ] = m_vitriniteReflectance[ prevNode ];
               } 
               else 
               {
                  bool Segment_May_Be_Eroding = currentLayer->getSolidThickness (i, j, k - 1).descending (currentTime);
                  
                  if (Segment_May_Be_Eroding) 
                  {
                     double prevThickness = currentLayer->getSolidThickness(i, j, k - 1, m_previousTime);

                     if (prevThickness != Utilities::Numerical::IbsNoDataValue ) 
                     {
                        double L = thickness / prevThickness;
                        
                        m_vitriniteReflectance[ node ] = L *  m_vitriniteReflectance[ node ] + (1.0 - L) * m_vitriniteReflectance[ prevNode ];
                     } 
                     else 
                     {
                        m_vitriniteReflectance[ node ] = m_vitriniteReflectance[ prevNode ];
                     }
                  } 
               }

               prevNode = node;
            }
         }
      }
         
      // Go the next layer
      gridOffset += layerGridSize;
      layers++;
   }
} 

void VreOutputGrid :: exportToModel( const LayerList & allLayers, const Boolean2DArray & isValidNeedle, const double currentTime)
{

   updateTimeStep (allLayers, isValidNeedle, currentTime);

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

      // Create a property vector for VR. In essence this vector doesn't need
      // to be global because no values from ghost nodes are needed, but if the
      // project contains related projects then there will be a petsc error
      // when creating the well file.
      if (!currentLayer->Vre) {
         DMCreateGlobalVector(currentLayer ->layerDA, &currentLayer ->Vre);
      }

      // Set the default value on the entire vector
      VecSet(currentLayer ->Vre, CauldronNoDataValue);

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

void VreOutputGrid :: setPreviousTime (const double time)
{
   m_previousTime = time;
}
