#ifndef _DENSITY_CALCULATOR_H_
#define _DENSITY_CALCULATOR_H_

#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

using namespace DataAccess;

class DensityCalculator {

public:

   DensityCalculator ();

   ~DensityCalculator () {};

   
private:

   Interface::GridMap* m_depthBasementMap; 
   Interface::GridMap* m_depthWaterBottomMap; 
   Interface::GridMap* m_pressureBasementMap; 
   Interface::GridMap* m_pressureWaterBottomMap;
  
   double m_sedimentDensity;
   double m_waterBottomDepthValue;
   double m_sedimentThickness;
   double m_topBasementDepthValue;
public:
   
   void loadData( Interface::ProjectHandle* projectHandle, const string & baseSurfaceName );
  
   void retrieveData();

   void restoreData();

   void computeNode( unsigned int i, unsigned int j );
 
   double getSedimentDensity( ) const;
   double getTopBasementDepthValue( ) const;
   double getWLS(  const double backstrippingMantleDensity, const double densityDiff ) const;
};

#endif

