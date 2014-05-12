#include "TemperatureForVreInputGrid.h"
#include "VreGrid.h"
#include "layer_iterators.h"

TemperatureForVreInputGrid::TemperatureForVreInputGrid(
      DM * mapViewOfDomain, 
      const LayerList & allLayers, 
      const Boolean2DArray & isValidNeedle, 
      double time,
      bool withPreviousTemperature 
   )
   : m_temperature( getNumberOfVreGridNodes( mapViewOfDomain, allLayers) )
   , m_activeNodes()
   , m_time(time)
{
  addLayers( allLayers, isValidNeedle, withPreviousTemperature );
}

TemperatureForVreInputGrid::~TemperatureForVreInputGrid() 
{
}

const double *TemperatureForVreInputGrid::getTemperature() const
{
   return &m_temperature[0]; 
}

int TemperatureForVreInputGrid::getSize() const
{
   return m_temperature.size(); 
} 

const int * TemperatureForVreInputGrid::getActiveNodes() const
{
   return &m_activeNodes[0]; 
}

int TemperatureForVreInputGrid::getNumberOfActiveNodes() const
{
   return m_activeNodes.size(); 
}

double TemperatureForVreInputGrid::getTime() const
{
   return m_time ; 
}

void TemperatureForVreInputGrid::swap( TemperatureForVreInputGrid & other)
{
   using std::swap;
   this->m_temperature.swap( other.m_temperature );
   this->m_activeNodes.swap( other.m_activeNodes );
   swap( this->m_time, other.m_time );
}

void TemperatureForVreInputGrid :: addLayers( const LayerList & layerList, const Boolean2DArray & isValidNeedle, bool withPreviousTemperature)
{
   // Traverse the layers from the basement to surface. This way values in the
   // m_temperature array will stay at the same spot when new layers are added.

   Basin_Modelling::Layer_Iterator layers;
   layers.Initialise_Iterator( layerList, 
         Basin_Modelling::Ascending, 
         Basin_Modelling::Sediments_Only,
         Basin_Modelling::Active_Layers_Only
         );

   int gridOffset = 0;
   while ( ! layers.Iteration_Is_Done () ) 
   {
      const LayerProps & layer = *layers.Current_Layer ();
      
      // Get the dimensions
      int xs, ys, zs, xm, ym, zm;
      DMDAGetCorners(layer.layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

      const int gridSize = xm*ym*zm;

      // Get pointers to temperature and depth properties
      double *** temperature;
      if (withPreviousTemperature)
      {
         DMDAVecGetArray(layer.layerDA, 
                         layer.Previous_Properties ( Basin_Modelling::Temperature ),
                          &temperature);
      }
      else
      {
         DMDAVecGetArray(layer.layerDA, 
                         layer.Current_Properties ( Basin_Modelling::Temperature ),
                          &temperature);
      }
          
      double***depth;
      DMDAVecGetArray(layer. layerDA, 
                       layer. Current_Properties ( Basin_Modelling::Depth ), 
                       &depth);

      const double currentTime = getTime();
      const int activeNodeOffset =  m_activeNodes.size() ;
      m_activeNodes.resize( activeNodeOffset + gridSize );
      int activeNode = activeNodeOffset;
      assert( m_activeNodes.size() <= m_temperature.size() );

      // Traverse the grid in the layer
      for (int i = xs; i < xs+xm; i++)
      {
         for (int j = ys; j < ys+ym; j++) 
         {
            if (! isValidNeedle(i,j) )
               continue; // then the node is inactive, and should therefore be ignored

            for (int k = zs; k < zs+zm; k++) 
            {
               int node = getNodeNumber( gridOffset, zm, ym, xm, k-zs, j-ys, i-xs);

               assert( activeNode < m_activeNodes.size() );
               assert( activeNode >= 0);
               assert( node < m_temperature.size() );
               assert( node >= 0 );

               double seaBottomDepth = FastcauldronSimulator::getInstance().getSeaBottomDepth( i, j, currentTime );

               // If, during deposition, the node is at the surface then do not start the vre calculation.
               if ( ( currentTime >= layer.depoage  ||
                      k > zs && layer.getSolidThickness ( i, j, k - 1, currentTime) <= 0.0
                    ) && NumericFunctions::isEqual( depth[k][j][i], seaBottomDepth, 1.0e-06)
                  )
               {
                  /* then the node is inactive */ 
               }
               else
               {
                  m_activeNodes[activeNode++] = node;
               }
               m_temperature[node] = temperature[k][j][i];
            }  // for k
         } // for j
      } // for i

      m_activeNodes.resize( activeNode );

      // The pointers to the depth and temperatures properties are no longer necessary
      if (withPreviousTemperature)
      {
         DMDAVecRestoreArray(layer.layerDA, 
                         layer.Previous_Properties ( Basin_Modelling::Temperature ),
                          &temperature);
      }
      else
      {
         DMDAVecRestoreArray(layer.layerDA, 
                         layer.Current_Properties ( Basin_Modelling::Temperature ),
                          &temperature);
      }

      DMDAVecRestoreArray(layer.layerDA, 
                           layer. Current_Properties ( Basin_Modelling::Depth ), 
                           &depth);

      // go to next layer
      layers++;
      gridOffset += gridSize;
   }
}
   


