//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_INTERFACEINPUT_H_
#define _CRUSTALTHICKNESS_INTERFACEINPUT_H_

// std library
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// boost library
#include <boost/shared_ptr.hpp>

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/Surface.h"

// DataModel library
#include "AbstractProperty.h"

// CrustalThickness library
#include "LinearFunction.h"
#include "ConfigFileParameterCtc.h"

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

/// @class InterfaceInput The CTC input interface
/// @throw InputException This class throw many exceptions in order to avoid calculator failures because of bad inputs
class InterfaceInput : public Interface::CrustalThicknessData
{

   typedef formattingexception::GeneralException InputException;

public:

   InterfaceInput (Interface::ProjectHandle * projectHandle, database::Record * record);
   virtual ~InterfaceInput ();
   
   /// @defgroup LoadData_cfg
   ///    Load data from configuration file
   /// @{
   /// @throw Fatal error exceptions to main after error loging
   void loadInputDataAndConfigurationFile( const string & inFile );
   /// @throw InputException if the user input data can't be loaded
   void loadInputData ();
   /// @}

   /// @defgroup LoadData_strati
   ///    Load data from the stratigraphy via GeoPhysics or DataMining projectHandle 
   /// @{
   void loadDerivedPropertyManager();

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
   double getFlexuralAge()                 const { return m_t_felxural;                                }
   double getInitialSubsidence()           const { return m_constants.getInitialSubsidenceMax();       }
   double getBackstrippingMantleDensity()  const { return m_constants.getBackstrippingMantleDensity(); }
   double getWaterDensity()                const { return m_constants.getWaterDensity();               }

   const CrustalThickness::ConfigFileParameterCtc& getConstants() const { return m_constants;           }
   const string& getBaseRiftSurfaceName()                         const { return m_baseRiftSurfaceName; }

   const GridMap& getT0Map     () const;
   const GridMap& getTRMap     () const;
   const GridMap& getHCuMap    () const;
   const GridMap& getHBuMap    () const;
   const GridMap& getHLMuMap   () const;
   const GridMap& getDeltaSLMap() const;

   DerivedProperties::SurfacePropertyPtr getPressureBasement           () const { return m_pressureBasement;           }
   DerivedProperties::SurfacePropertyPtr getPressureWaterBottom        () const { return m_pressureWaterBottom;        }
   DerivedProperties::SurfacePropertyPtr getPressureMantle             () const { return m_pressureMantle;             }
   DerivedProperties::SurfacePropertyPtr getPressureMantleAtPresentDay () const { return m_pressureMantleAtPresentDay; }
   DerivedProperties::SurfacePropertyPtr getDepthBasement              () const { return m_depthBasement;              }
   DerivedProperties::SurfacePropertyPtr getDepthWaterBottom           () const { return m_depthWaterBottom;           }

   const Interface::Surface* getTopOfSedimentSurface() const { return m_topOfSedimentSurface;    }
   const Interface::Surface* getBotOfSedimentSurface() const { return m_bottomOfSedimentSurface; }
   /// @}

   /// @defgroup Mutators
   /// @{
   void setSmoothingRadius( const unsigned int radius ) { m_smoothRadius = radius; };
   /// @}

   /// @defgroup GridUtilities
   ///    Defined from m_T0Map
   /// @{
   unsigned firstI() const { return m_T0Map->firstI(); };
   unsigned firstJ() const { return m_T0Map->firstJ(); };
   unsigned lastI()  const { return m_T0Map->lastI();  };
   unsigned lastJ()  const { return m_T0Map->lastJ();  };
   /// @}

protected:

   /// @defgroup User_interface_data
   /// @{
   unsigned int m_smoothRadius; ///< Smoothing radius                                                      [Cells]
   double m_t_felxural;         ///< Timing of flexural basin, after this age there is no more CTC outputs [Ma]

   const GridMap * m_T0Map;      ///< Beginning of rifting                       [Ma]
   const GridMap * m_TRMap;      ///< End of rifting                             [Ma]
   const GridMap * m_HCuMap;     ///< Initial continental crust thickness        [m]
   const GridMap * m_HLMuMap;    ///< Initial lithospheric mantle thickness      [m]
   const GridMap * m_HBuMap;     ///< Maximum oceanic (basaltic) crust thickness [m]
   const GridMap * m_DeltaSLMap; ///< Sea level adjustment                       [m]
   /// @}

   /// @defgroup Stratigraphy
   /// @{
   const Interface::Surface * m_bottomOfSedimentSurface; ///< The basement surface at the current snapshot
   const Interface::Surface * m_topOfSedimentSurface;    ///< The water bottom surface
   const Interface::Surface * m_topOfMantle;             ///< The top mantle (bottom crust) surface
   const Interface::Surface * m_botOfMantle;             ///< The bottom mantle surface 
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
