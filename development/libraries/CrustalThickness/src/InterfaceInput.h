//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_INTERFACEINPUT_H
#define CRUSTALTHICKNESS_INTERFACEINPUT_H

// std library
#include <map>
#include <memory>

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/CrustalThicknessRiftingHistoryData.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"

// DataModel library
#include "AbstractProperty.h"

// CrustalThickness library
#include "ConfigFileParameterCtc.h"
#include "LinearFunction.h"
#include "RiftingEvent.h"

// Geophysics library
#include "GeoPhysicsProjectHandle.h"

// Datamining library
#include "DataMiningProjectHandle.h"

// Derived properties library
#include "SurfaceProperty.h"
#include "DerivedPropertyManager.h"

// TableIO library
#include "database.h"

// utilitites
#include "FormattingException.h"

using namespace std;
using namespace DataAccess;
using Interface::GridMap;
using Interface::Formation;
using Interface::Surface;
using Interface::CrustalThicknessData;
using Interface::CrustalThicknessRiftingHistoryData;

/// @class InterfaceInput The CTC input interface
/// @throw InputException This class throw many exceptions in order to avoid calculator failures because of bad inputs
class InterfaceInput
{

   typedef formattingexception::GeneralException InputException;

public:

   /// @param[in] crustalThicknessData The data from the CTCIoTbl
   /// @param[in] crustalThicknessRiftingHistoryData The list of data (one per line) from the CTCRiftingHistoryIoTbl
   InterfaceInput( const std::shared_ptr< const CrustalThicknessData >                             crustalThicknessData,
                   const std::vector< std::shared_ptr<const CrustalThicknessRiftingHistoryData> >& crustalThicknessRiftingHistoryData );
   ~InterfaceInput ();
   
   /// @brief Load data from project file and configuration file
   /// @param[in] inFile The CTC configuration file name
   /// @throw std::invalid_argument exceptions if the inputs are not valid
   void loadInputData( const string & inFile );

   /// @defgroup LoadDataFromStratigraphy
   ///    Load data from the stratigraphy via GeoPhysics or DataMining projectHandle 
   /// @{
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
   const DataModel::AbstractProperty* loadPressureProperty( );
   /// @brief Load the depth at the defined snapshot and intitalise the top and bottom surface of sediments according to the baseSurfaceName
   /// @return The depth property
   const DataModel::AbstractProperty* loadDepthProperty( );

   /// @brief Load a property at the defined snapshot for every point specified in the depth map
   /// @param[in] handle The datamining project handle
   /// @param[in] depthMap The depth map whcih defines the location of the points where we want to load the property
   /// @param[in] property The property to load
   /// @param[in] snapshot The snapshot at which we want to load the property
   /// @return A grid map with the property values at the defined snapshot and depth
   GridMap* loadPropertyDataFromDepthMap( DataAccess::Mining::ProjectHandle* handle,
                                          const GridMap* depthMap,
                                          const Interface::Property* property,
                                          const Interface::Snapshot* snapshot );
   /// @}

   /// @defgroup Accessors
   /// @{
   unsigned int getSmoothRadius()          const { return m_smoothRadius;                              }
   double getFlexuralAge()                 const { return m_flexuralAge;                               }
   double getFirstRiftAge()                const { return m_firstRiftAge;                              }
   double getInitialSubsidence()           const { return m_constants.getInitialSubsidenceMax();       }
   double getBackstrippingMantleDensity()  const { return m_constants.getBackstrippingMantleDensity(); }
   double getWaterDensity()                const { return m_constants.getWaterDensity();               }

   const CrustalThickness::ConfigFileParameterCtc& getConstants() const { return m_constants;           }
   const string& getBaseRiftSurfaceName()                         const { return m_baseRiftSurfaceName; }

   const GridMap& getHCuMap    () const;
   const GridMap& getHLMuMap   () const;

   double getContinentalCrustRatio() const { return m_continentalCrustRatio; };
   double getOceanicCrustRatio()     const { return m_oceanicCrustRatio;     };

   DerivedProperties::SurfacePropertyPtr getPressureBasement           () const { return m_pressureBasement;           }
   DerivedProperties::SurfacePropertyPtr getPressureWaterBottom        () const { return m_pressureWaterBottom;        }
   DerivedProperties::SurfacePropertyPtr getPressureMantle             () const { return m_pressureMantle;             }
   DerivedProperties::SurfacePropertyPtr getPressureMantleAtPresentDay () const { return m_pressureMantleAtPresentDay; }
   DerivedProperties::SurfacePropertyPtr getDepthBasement              () const { return m_depthBasement;              }
   DerivedProperties::SurfacePropertyPtr getDepthWaterBottom           () const { return m_depthWaterBottom;           }

   const Interface::Surface* getTopOfSedimentSurface() const { return m_topOfSedimentSurface;    }
   const Interface::Surface* getBotOfSedimentSurface() const { return m_bottomOfSedimentSurface; }
   /// @}

   /// @defgroup RiftingEventAPI
   ///    API to use for accessing the rifting events data
   /// @{
   double getRiftingStartAge   ( const double age ) const;
   double getRiftingEndAge     ( const double age ) const;
   double getRiftId            ( const double age ) const;
   const GridMap& getHBuMap    ( const double age ) const;
   const GridMap& getDeltaSLMap( const double age ) const;
   /// @}

   /// @defgroup Mutators
   /// @{
   void setSmoothingRadius( const unsigned int radius ) { m_smoothRadius = radius; }
   /// @}

   /// @defgroup GridUtilities
   ///    Defined from m_HCuMap
   /// @{
   unsigned int firstI() const { return m_HCuMap->firstI(); }
   unsigned int firstJ() const { return m_HCuMap->firstJ(); }
   unsigned int lastI()  const { return m_HCuMap->lastI();  }
   unsigned int lastJ()  const { return m_HCuMap->lastJ();  }
   /// @}

   std::vector<double> copySnapshots() const { return m_snapshots; }

protected:

   /// @defgroup Loaders
   /// @{
      /// @brief Loads the snapshots from the stratigraphy via the crustal thickness history data
      void loadSnapshots();
      /// @throw InputException If the project handle is a null pointer or if the derived property manager cannot be retrieved
      void loadDerivedPropertyManager();

      /// @defgroup CTCRiftingHistoryIoTblLoader
      /// @{
      /// @brief Loads user input data from the project file CTCRiftingHistoryIoTbl
      void loadCTCRiftingHistoryIoTblData();
      /// @brief Load the rifting events for all the loaded snapshots
      /// @details After this their tectonic flag and maximum oceanic crustal thicknesses are set
      /// @throw std::runtime_error if one of theere are more event than snapshots or vice versa
      void loadRiftingEvents();
         /// @defgroup RiftAnalysis
         ///    A rift is defined by a succession of active rifting events (A) and passive margin (P) events
         ///    up to the next active rifting event, see example below:
         ///    Rifting event  1;2;3;4;5;6;7;8;9;10;11;12
         ///    Tectonic flag  A;A;A;P;P;A;A;A;A;P ;P ;A;....
         ///    Rifting        <------->;<----------->;<--...
         ///    Rifting ID         1    ;      2      ; 3 ...
         ///    Start age      ^        ;^            ;^  ...
         ///    End age                ^;            ^;......
         /// @{
         /// @brief Set the rifting start and end ages according to the suite of rifting events
         /// throw std::invalid_argument if the start is anterior or equal to the end for any rifting event
         void analyseRiftingHistory();
         /// @brief Set the rifting start ages according to the suite of rifting events
         /// @details Also set the rift ID and the last computation age (flexural age)
         void analyseRiftingHistoryStartAge();
         /// @brief Set the rifting end ages according to the suite of rifting events
         void analyseRiftingHistoryEndAge();
         /// @}
      /// @}

      /// @defgroup CTCIoTblLoader
      /// @{
      /// @brief Loads user input data from the project file CTCIoTbl
      /// @throw std::invalid_argument if one of the user input is missing or unvalid
      void loadCTCIoTblData();
      /// @}

   /// @}


   /// @defgroup DataAccess
   /// @{
   const std::shared_ptr<const CrustalThicknessData>                             m_crustalThicknessData;               ///< The data accessors for the CTCIoTbl
   const std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>>& m_crustalThicknessRiftingHistoryData; ///< The data accessors for the CTCRiftingHistoryIoTbl (one per snapshot from 0Ma)
   /// @}

   /// @defgroup UserInterface_Data
   /// @{
   unsigned int m_smoothRadius;  ///< Smoothing radius                                                                                      [Cells]
   double m_flexuralAge;         ///< Timing of flexural basin, after this age there is no more CTC outputs                                 [Ma]
   double m_firstRiftAge;        ///< Timing of the first fully defined rift, before this age there is no McKenzie Crust Thicknesses ouptus [Ma]

   std::map<const double, std::shared_ptr<CrustalThickness::RiftingEvent>> m_riftingEvents; /// All the rifting events mapped to their stratigraphic age

   GridMap const * m_HCuMap;     ///< Initial continental crust thickness        [m]
   GridMap const * m_HLMuMap;    ///< Initial lithospheric mantle thickness      [m]
   /// @}

   /// @defgroup Debug_ProjectFile_Data
   /// @{
   double m_continentalCrustRatio; ///< Ratio which defines the spliting of the continental crust in its upper and lower part (r=Upper/Lower)
   double m_oceanicCrustRatio;     ///< Ratio which defines the spliting of the oceanic crust in its upper and lower part (r=Upper/Lower)
   /// @}

   /// @defgroup Stratigraphy
   /// @{
   const Surface * m_bottomOfSedimentSurface; ///< The basement surface at the current snapshot
   const Surface * m_topOfSedimentSurface;    ///< The water bottom surface
   const Surface * m_topOfMantle;             ///< The top mantle (bottom crust) surface
   const Surface * m_botOfMantle;             ///< The bottom mantle surface

   std::vector<double> m_snapshots; ///< The list of stratigraphic snapshots in reverse order surrounded by 0 snapshot (i.e. [0,150,110,50,...,0])
   /// @}

   /// @defgroup DerivedProperties
   /// @{
   DerivedProperties::DerivedPropertyManager* m_derivedManager;        ///< The derived property manager (we have to use a pointer for forward declaration due to crossdependent libraries)
   DerivedProperties::SurfacePropertyPtr m_pressureBasement;           ///< The pressure of the basement at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom;        ///< The pressure of the water bottom at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureMantle;             ///< The pressure of the bottom of mantle at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureMantleAtPresentDay; ///< The pressure of the bootom of mantle at the present day
   DerivedProperties::SurfacePropertyPtr m_depthBasement;              ///< The depth of the basement at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_depthWaterBottom;           ///< The depth of the water bottom at the current snapshot
   /// @}

   CrustalThickness::ConfigFileParameterCtc m_constants; ///< Constants loaded from the configuration file

   string m_baseRiftSurfaceName;  ///< Name of a base of syn-rift

};

#endif
