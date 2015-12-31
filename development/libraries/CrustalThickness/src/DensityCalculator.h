//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_DENSITYCALCULATOR_H_
#define _CRUSTALTHICKNESS_DENSITYCALCULATOR_H_

// DataAccess library
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"

// DerivedProperties library
#include "DerivedPropertyManager.h"

using namespace DataAccess;

typedef std::vector<double> snapshotsList;

/// @class DensityCalculator The density calcultor used during the CTC backstriping
class DensityCalculator {

   public:
   
      DensityCalculator ();
      ~DensityCalculator () {};

      // void loadData( Interface::ProjectHandle* projectHandle, const Interface::Property * depthProperty, const string & baseSurfaceName );
      /// @defgroup LoadData
      ///    Load data from projectHandle
      /// @{
      /// @brief Load snapshots from formations
      void loadSnapshots( Interface::ProjectHandle* projectHandle );

      /// @brief Load basement and water bottom depth maps at the defined snapshot
      /// @param depthProperty A fastcauldron depth property
      void loadDepthData   ( Interface::ProjectHandle* projectHandle, const Interface::Property * depthProperty, const double snapshotAge );
      /// @brief Load basement and water bottom pressure maps at the defined snapshot
      /// @param pressureProperty A fastcauldron pressure property
      void loadPressureData( Interface::ProjectHandle* projectHandle, const Interface::Property * pressureProperty, const double snapshotAge );

      /// @brief Load the lithostatic pressure at the defined snapshot
      /// @return The lithostatic pressure property
      const Interface::Property * loadPressureProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge );
      /// @brief Load the depth at the defined snapshot and intitalise the top and bottom surface of sediments according to the baseSurfaceName
      /// @param baseSurfaceName The name of the basement surface (bottom of sediments)
      /// @return The depth property
      const Interface::Property * loadDepthProperty( Interface::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName );
      /// @}

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve pressure and depth maps data
      void retrieveData();
      /// @brief Restore pressure and depth maps data
      void restoreData();
      /// @}

      /// @brief Set the mantle and densities
      bool setDensities( const double aMantleDensity, const double aWaterDensity );

      /// @brief Perfrom the backstriping at node (i,j)
      /// @details Compute the WLS map
      void computeNode( unsigned int i, unsigned int j );


      /// @defgroup Accessors
      /// @{
      Interface::GridMap* getDepthBasementMap      () const;
      Interface::GridMap* getDepthWaterBottomMap   () const;
      Interface::GridMap* getPressureBasementMap   () const;
      Interface::GridMap* getPressureWaterBottomMap() const;

      const Interface::Surface * getTopOfSedimentSurface() const;

      double getTopBasementDepthValue() const;
      double getWLS                  () const;
      double getBackstrip            () const;
      double getSedimentThickness    () const;
      double getSedimentDensity      () const;

      snapshotsList &getSnapshots();
      /// @}
      
   private:
   
      /// @defgroup Stratigraphy
      /// @{
      snapshotsList m_snapshots;                            ///< The list of snapshots (used in the CrustalThicnkessCalculator)
      const Interface::Surface * m_bottomOfSedimentSurface; ///< The basement surface
      const Interface::Surface * m_topOfSedimentSurface;    ///< The water bottom surface
      /// @}

      /// @defgroup Properties_legacy
      /// @{
      Interface::GridMap* m_depthBasementMap;       ///< The depth of the basement
      Interface::GridMap* m_depthWaterBottomMap;    ///< The depth of the water bottom
      Interface::GridMap* m_pressureBasementMap;    ///< The pressure of the basement
      Interface::GridMap* m_pressureWaterBottomMap; ///< The pressure of the water bottom
      /// @}
     
      /// @defgroup Properties_derived
      /// @{
      DerivedProperties::SurfacePropertyPtr m_pressureBasement;    ///< The pressure of the basement
      DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom; ///< The pressure of the water bottom
      /// @}
   
      /// @defgroup Variables
      /// @{
      double m_sedimentDensity;       ///< The density of the entire stack of sediments
      double m_waterBottomDepthValue; ///< The depth of the water bottom
      double m_sedimentThickness;     ///< The thickness of the entire stack of sediments
      double m_topBasementDepthValue; ///< The depth of the basement
      double m_WLS;                   ///< The water loaded subsidence
      double m_backstrip;             ///< The backstrip (https://en.wikipedia.org/wiki/Back-stripping)
      /// @}
   
      /// @defgroup ConfigFileData
      ///    Set from configuration file
      /// @{
      double  m_backstrippingMantleDensity; ///< The mantle density (is currently the same for lithospheric and asthenospheric mantle)
      double  m_waterDensity;               ///< The water density
      double m_densityTerm;                 ///< densityTerm = 1.0 / (mantleDensity - m_waterDensity)
      /// @}
};

//------------------------------------------------------------//

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

//------------------------------------------------------------//

class PropertyManager : public DerivedProperties::DerivedPropertyManager {

public:

   PropertyManager( GeoPhysics::ProjectHandle* projectHandle );
   ~PropertyManager() {};

};
#endif

