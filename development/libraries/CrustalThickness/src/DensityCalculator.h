#ifndef _DENSITY_CALCULATOR_H_
#define _DENSITY_CALCULATOR_H_

#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

using namespace DataAccess;

typedef std::vector<double> snapshotsList;

class DensityCalculator {

public:

   DensityCalculator ();

   ~DensityCalculator () {};

   
private:

   snapshotsList m_snapshots;
   const Interface::Surface * m_bottomOfSedimentSurface;
   const Interface::Surface * m_topOfSedimentSurface;

   Interface::GridMap* m_depthBasementMap; 
   Interface::GridMap* m_depthWaterBottomMap; 
   Interface::GridMap* m_pressureBasementMap; 
   Interface::GridMap* m_pressureWaterBottomMap;
  
   double m_waterBottomDepthValue;
   double m_sedimentThickness;
   double m_topBasementDepthValue;
   double m_WLS;
   double m_backstrip;
   double m_sedimentDensity;

   /// set from configuration file
   double  m_backstrippingMantleDensity;
   double  m_waterDensity;
   /// densityTerm = 1.0 / (mantleDensity - m_waterDensity)
   double m_densityTerm;
public:
   
   // void loadData( Interface::ProjectHandle* projectHandle, const Interface::Property * depthProperty, const string & baseSurfaceName );
   void loadSnapshots( Interface::ProjectHandle* projectHandle );

   void loadDepthData( Interface::ProjectHandle* projectHandle, const Interface::Property * depthProperty, const double snapshotAge );
   void loadPressureData( Interface::ProjectHandle* projectHandle, const Interface::Property * pressureProperty, const double snapshotAge );
   const Interface::Property * loadPressureProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge );
   const Interface::Property * loadDepthProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName );
  
   void retrieveData();
   void restoreData();

   bool setDensities( const double aMantleDensity, const double aWaterDensity );
   void computeNode( unsigned int i, unsigned int j );
   double getTopBasementDepthValue( ) const;
   double getWLS() const;
   double getBackstrip() const;
   double getSedimentThickness() const;
   double getSedimentDensity() const;
   snapshotsList &getSnapshots() ;  

   Interface::GridMap* getDepthBasementMap() const;
   Interface::GridMap* getDepthWaterBottomMap() const;
   Interface::GridMap* getPressureBasementMap() const;
   Interface::GridMap* getPressureWaterBottomMap() const;

   const Interface::Surface * getTopOfSedimentSurface( ) const;
};

inline const Interface::Surface * DensityCalculator::getTopOfSedimentSurface( ) const {

   return m_topOfSedimentSurface;
}

inline Interface::GridMap* DensityCalculator::getDepthBasementMap( ) const {

   return m_depthBasementMap;
}

inline Interface::GridMap* DensityCalculator::getDepthWaterBottomMap( ) const {

   return m_depthWaterBottomMap;
}

inline Interface::GridMap* DensityCalculator::getPressureBasementMap( ) const {

   return m_pressureBasementMap;
}

inline Interface::GridMap* DensityCalculator::getPressureWaterBottomMap( ) const {

   return m_pressureWaterBottomMap;
}

inline double DensityCalculator::getWLS( ) const {

   return m_WLS;
}

inline double DensityCalculator::getBackstrip( ) const {

   return m_backstrip;
}

inline double DensityCalculator::getSedimentThickness() const {

   return m_sedimentThickness;
}

inline double DensityCalculator::getSedimentDensity() const {

   return m_sedimentDensity;
}

inline snapshotsList &DensityCalculator::getSnapshots() {

   return m_snapshots;
}

#endif

