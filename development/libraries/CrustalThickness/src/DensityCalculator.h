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

// utilitites library
#include "FormattingException.h"

using namespace DataAccess;

typedef std::vector<double> snapshotsList;

/// @class DensityCalculator The density calcultor used during the CTC backstriping
class DensityCalculator {

   typedef formattingexception::GeneralException DensityException;

   public:
   
      DensityCalculator ();
      ~DensityCalculator () {};

      /// @defgroup LoadData
      ///    Load data from projectHandle
      /// @{
      /// @brief Load snapshots from formations
      void loadSnapshots( Interface::ProjectHandle* projectHandle );

      /// @brief Load the water bottom and the basement surfaces at the defined snapshot by initializing class members (m_bottomOfSedimentSurface and m_topOfSedimentSurface)
      /// @param baseSurfaceName The name of the basement surface (bottom of sediments), if "" then find it in the stratigraphy, else find the surface according to the name
      void loadTopAndBottomOfSediments( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName );

      /// @brief Load basement and water bottom depth maps at the defined snapshot
      /// @param depthProperty A fastcauldron depth property
      void loadDepthData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* depthProperty, const double snapshotAge );
      /// @brief Load basement and water bottom pressure maps at the defined snapshot
      /// @param pressureProperty A fastcauldron pressure property
      void loadPressureData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* pressureProperty, const double snapshotAge );

      /// @brief Load the lithostatic pressure at the defined snapshot
      /// @return The lithostatic pressure property
      const DataModel::AbstractProperty* loadPressureProperty( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge );
      /// @brief Load the depth at the defined snapshot and intitalise the top and bottom surface of sediments according to the baseSurfaceName
      /// @return The depth property
      const DataModel::AbstractProperty* loadDepthProperty( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge);
      /// @}

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve pressure and depth maps data
      void retrieveData();
      /// @brief Restore pressure and depth maps data
      void restoreData();
      /// @}

      /// @brief Set the mantle and water densities
      bool setDensities( const double aMantleDensity, const double aWaterDensity );

      /// @brief Perfrom the backstriping at node (i,j)
      /// @details Compute the WLS map
      void computeNode( unsigned int i, unsigned int j );


      /// @defgroup Accessors
      /// @{
      
      snapshotsList &getSnapshots                          ()       { return m_snapshots;               };
      const Interface::Surface * getBottomOfSedimentSurface() const { return m_bottomOfSedimentSurface; };
      const Interface::Surface * getTopOfSedimentSurface   () const { return m_topOfSedimentSurface;    };

      DerivedProperties::SurfacePropertyPtr getDepthBasementMap      () const { return m_pressureBasement;    };
      DerivedProperties::SurfacePropertyPtr getDepthWaterBottomMap   () const { return m_pressureWaterBottom; };
      DerivedProperties::SurfacePropertyPtr getPressureBasementMap   () const { return m_depthBasement;       };
      DerivedProperties::SurfacePropertyPtr getPressureWaterBottomMap() const { return m_depthWaterBottom;    };

      double getTopBasementDepthValue() const { return m_topBasementDepthValue; };
      double getWLS                  () const { return m_WLS;                   };
      double getBackstrip            () const { return m_backstrip;             };
      double getSedimentThickness    () const { return m_sedimentThickness;     };
      double getSedimentDensity      () const { return m_sedimentDensity;       };

      /// @}
      
   private:
   
      /// @defgroup Stratigraphy
      /// @{
      snapshotsList m_snapshots;                            ///< The list of snapshots (used in the CrustalThicnkessCalculator)
      const Interface::Surface * m_bottomOfSedimentSurface; ///< The basement surface at the current snapshot
      const Interface::Surface * m_topOfSedimentSurface;    ///< The water bottom surface 
      /// @}
     
      /// @defgroup DerivedProperties
      /// @{
      DerivedProperties::SurfacePropertyPtr m_pressureBasement;    ///< The pressure of the basement at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom; ///< The pressure of the water bottom at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_depthBasement;       ///< The depth of the basement at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_depthWaterBottom;    ///< The depth of the water bottom at the current snapshot
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
      double  m_densityTerm;                ///< densityTerm = 1.0 / (mantleDensity - m_waterDensity)
      /// @}
};
#endif

