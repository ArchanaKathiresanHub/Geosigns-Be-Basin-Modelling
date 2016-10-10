//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DENSITY_CALCULATOR_H_
#define _DENSITY_CALCULATOR_H_

#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#include "DerivedPropertyManager.h"

using namespace DataAccess;

class DensityCalculator {

public:

   DensityCalculator ();

   ~DensityCalculator () {};

   
private:

   Interface::GridMap* m_depthBasementMap; 
   Interface::GridMap* m_depthWaterBottomMap; 

   DerivedProperties::SurfacePropertyPtr m_pressureBasement; 
   DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom;

  
   double m_sedimentDensity;
   double m_waterBottomDepthValue;
   double m_sedimentThickness;
   double m_topBasementDepthValue;
public:
   
   void loadData( GeoPhysics::ProjectHandle* projectHandle, const string & baseSurfaceName );
  
   void retrieveData();

   void restoreData();

   void computeNode( unsigned int i, unsigned int j );
 
   double getSedimentDensity( ) const;
   double getTopBasementDepthValue( ) const;
   double getWLS(  const double backstrippingMantleDensity, const double densityDiff ) const;
};

class PropertyManager : public DerivedProperties::DerivedPropertyManager {

public :
   
   PropertyManager ( GeoPhysics::ProjectHandle* projectHandle ); 
   
   ~PropertyManager() {};
   
};
#endif

