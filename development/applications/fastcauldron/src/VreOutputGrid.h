#ifndef FASTCAULDRON_VROUTPUTGRID_H
#define FASTCAULDRON_VROUTPUTGRID_H

#include <petscdmda.h>
#include "layer.h"
#include "VitriniteReflectance.h"
#include "VreGrid.h"

class VreOutputGrid : private VreGrid, public VitriniteReflectance :: OutputGrid
{
public:
   // Allocate an output grid on the basis of the stuff that can be found in the AppCtx oject
   explicit VreOutputGrid( DM * mapViewOfDomain, const LayerList & allLayers );
   
   // Returns a pointer to the array of vitrinite reflectance values
   virtual double * getVRe();

   // Returns the size of the array of vitrinite reflectance values
   virtual int getSize() const;

   /// Update the VRe values
   void updateTimeStep( const LayerList & allLayers, const Boolean2DArray & isValidNeedle, const double currentTime );

   // Store the vitrinite reflectance values into the propertie vectors of the layers
   void exportToModel( const LayerList & allLayers, const Boolean2DArray & isValidNeedle, const double time); 

   void setPreviousTime( const double time );

private:
   std::vector< double > m_vitriniteReflectance;

   double m_previousTime;
};

#endif
