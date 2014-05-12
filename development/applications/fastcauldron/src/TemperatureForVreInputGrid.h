#ifndef FASTCAULDRON_TEMPERATUREFORVRINPUTGRID_H
#define FASTCAULDRON_TEMPERATUREFORVRINPUTGRID_H

#include <petscdmda.h>
#include "layer.h"

#include "VitriniteReflectance.h"
#include "VreGrid.h"

/// Provides an abstraction and a copy of the temperature grid for the computation of vitrinite reflectance
class TemperatureForVreInputGrid : private VreGrid, public GeoPhysics::VitriniteReflectance::InputGrid
{
public:
   /// Construct a temperature grid from stuff that is available in Cauldron's
   /// main data structure (AppCtx)
   explicit TemperatureForVreInputGrid( DM * mapViewOfDomain, const LayerList & allLayers,
         const Boolean2DArray & isValidNeedle, double time, bool withPreviousTemperature );

   virtual ~TemperatureForVreInputGrid() ;

   /// Pointer to array of temperature values
   virtual const double *getTemperature() const;

   /// Size of array of temperature values
   virtual int getSize() const;

   /// Pointer to array of indices into the temperature array which point to the
   /// active nodes. 
   virtual const int * getActiveNodes() const;

   /// Number of active nodes.
   virtual int getNumberOfActiveNodes() const;

   /// The time (in Ma) at which this snapshot is taken
   virtual double getTime() const;

   /// Swap the contents of this grid with an other one
   void swap( TemperatureForVreInputGrid & other);

private:
   /// Adds a copy of the layer to the the current grid
   void addLayers( const LayerList & layerList,   
         const Boolean2DArray & isValidNeedle,
         bool withPreviousTemperature
       );

   std::vector< double > m_temperature;
   std::vector< int>     m_activeNodes;
   double m_time;
};



#endif
