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

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/Surface.h"

// CrustalThickness library
#include "LinearFunction.h"

// utilitites
#include "FormattingException.h"

using namespace std;
using namespace DataAccess;
using Interface::GridMap;

// Forward declaration because of crossdependency between libraries Geophysics and CrustlThickness
namespace database
{
   class Record;
   class Table;
}
namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
      class Grid;
   }
   namespace Mining
   {
      class ProjectHandle;
   }
}

namespace DerivedProperties
{
   class SurfaceProperty;
   class DerivedPropertyManager;
   typedef boost::shared_ptr<const SurfaceProperty> SurfacePropertyPtr;
}

namespace GeoPhysics {
   class ProjectHandle;
}

namespace DataModel{
   class AbstractProperty;
}

namespace CrustalThicknessInterface {
   const string TableBasicConstants          = "Table:[BasicConstants]";
   const string TableLithoAndCrustProperties = "Table:[LithoAndCrustProperties]";
   const string TableTemperatureData         = "Table:[TemperatureData]";
   const string TableSolidus                 = "Table:[Solidus]";
   const string TableMagmaLayer              = "Table:[MagmaLayer]";
   const string TableUserDefinedData         = "Table:[UserDefinedData]";
   const string TableMantle                  = "Table:[Mantle]";
   const string TableStandardCrust           = "Table:[StandardCrust]";
   const string TableLowCondCrust            = "Table:[LowCondCrust]";
   const string TableBasalt                  = "Table:[Basalt]";
   const string EndOfTable                   = "[EndOfTable]";

   const string t_0                          = "t_0";
   const string t_r                          = "t_r";
   const string initialCrustThickness        = "initialCrustThickness";
   const string maxBasalticCrustThickness    = "maxBasalticCrustThickness";
   const string initialLithosphericThickness = "initialLithosphericThickness";
   const string seaLevelAdjustment           = "seaLevelAdjustment";
   const string coeffThermExpansion          = "coeffThermExpansion";
   const string initialSubsidenceMax         = "initialSubsidenceMax";
   const string pi                           = "pi";
   const string E0                           = "E0";
   const string tau                          = "tau";
   const string modelTotalLithoThickness     = "modelTotalLithoThickness";
   const string backstrippingMantleDensity   = "backstrippingMantleDensity";
   const string lithoMantleDensity           = "lithoMantleDensity";
   const string baseLithosphericTemperature  = "baseLithosphericTemperature";
   const string referenceCrustThickness      = "referenceCrustThickness";
   const string referenceCrustDensity        = "referenceCrustDensity";
   const string waterDensity                 = "waterDensity";
   const string A                            = "A";
   const string B                            = "B";
   const string C                            = "C";
   const string D                            = "D";
   const string E                            = "E";
   const string F                            = "F";
   const string T                            = "T";
   const string Heat                         = "Heat";
   const string Rho                          = "Rho";
   const string decayConstant                = "decayConstant";
   const string lithosphereThicknessMin      = "HLmin";
   const string maxNumberOfMantleElements    = "NLMEmax";

   /// @brief Parse the CTC command line
   /// @param theString The command to be parsed
   /// @param theDelimiter The command option delimiter
   /// @param theTokens The command's options parsed from the command line
   void parseLine(const string &theString, const string &theDelimiter, vector<string> &theTokens);
   /// @brief Get the MPI rank
   /// @return The MPI rank
   int GetRank ();
   /// @brief Get the number of processes
   /// @return The number of processes
   int GetNumOfProcs ();
}

/// @class InterfaceInput The CTC input interface
class InterfaceInput : public Interface::CrustalThicknessData
{

   typedef formattingexception::GeneralException InputException;

public:

   InterfaceInput (Interface::ProjectHandle * projectHandle, database::Record * record);
   virtual ~InterfaceInput ();
   
   /// @defgroup LoadData_cfg
   ///    Load data from configuration file
   /// @{
   void loadInputDataAndConfigurationFile( const string & inFile );
   void loadInputData                    ();
   void loadConfigurationFile            ( const string & inFile );
   void LoadBasicConstants               ( ifstream &ConfigurationFile );
   void LoadLithoAndCrustProperties      ( ifstream &ConfigurationFile );
   void LoadTemperatureData              ( ifstream &ConfigurationFile );
   void LoadSolidus                      ( ifstream &ConfigurationFile );
   void LoadMagmaLayer                   ( ifstream &ConfigurationFile );
   void LoadUserDefinedData              ( ifstream &ConfigurationFile );
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
   /// @param handle The datamining project handle
   /// @param depthMap The depth map whcih defines the location of the points where we want to load the property
   /// @property The property to load
   /// @snapshot The snapshot at which we want to load the property
   GridMap* loadPropertyDataFromDepthMap( DataAccess::Mining::ProjectHandle* handle,
                                          const GridMap* depthMap,
                                          const Interface::Property* property,
                                          const Interface::Snapshot* snapshot );
   /// @}

   /// @defgroup Accessors
   /// @{
   int    getSmoothRadius               () const;
   double getMidAge                     () const;
   double getDensityDifference          () const;
   double getInitialCrustThickness      () const;
   double getInitialLithosphereThickness() const;
   double getBackstrippingMantleDensity () const;
   double getWaterDensity               () const;
   double getEstimatedCrustDensity      () const;
   double getTFOnset                    () const;
   double getTFOnsetLin                 () const;
   double getTFOnsetMig                 () const;
   double getPTa                        () const;
   double getMagmaticDensity            () const;
   double getWLSexhume                  () const;
   double getWLScrit                    () const;
   double getWLSonset                   () const;
   double getWLSexhumeSerp              () const;

   double getDeltaSLValue               (unsigned int i, unsigned int j) const;
   const string& getBaseRiftSurfaceName () const;
   double getInitialSubsidence          () const { return m_initialSubsidenceMax; }

   const GridMap* getT0Map     () const;
   const GridMap* getTRMap     () const;
   const GridMap* getHCuMap    () const;
   const GridMap* getHLMuMap   () const;
   const GridMap* getDeltaSLMap() const;

   DerivedProperties::SurfacePropertyPtr getPressureBasement           () const { return m_pressureBasement;           };
   DerivedProperties::SurfacePropertyPtr getPressureWaterBottom        () const { return m_pressureWaterBottom;        };
   DerivedProperties::SurfacePropertyPtr getPressureMantle             () const { return m_pressureMantle;             };
   DerivedProperties::SurfacePropertyPtr getPressureMantleAtPresentDay () const { return m_pressureMantleAtPresentDay; };
   DerivedProperties::SurfacePropertyPtr getDepthBasement              () const { return m_depthBasement;              };
   DerivedProperties::SurfacePropertyPtr getDepthWaterBottom           () const { return m_depthWaterBottom;           };

   const Interface::Surface* getTopOfSedimentSurface() const { return m_topOfSedimentSurface;    };
   const Interface::Surface* getBotOfSedimentSurface() const { return m_bottomOfSedimentSurface; };

   /// @}

   /// @brief Calculate coefficients for the linear function to invert from WLS to TF (thinning factor) for the (i,j) node
   bool defineLinearFunction( LinearFunction & theFunction, unsigned int i, unsigned int j );

   /// @defgroup DataUtilities
   /// @{
   /// @brief Retrieve all CTC maps data
   void retrieveData();
   /// @brief Restore all CTC maps data
   void restoreData();
   /// @}

   /// @defgroup GridUtilities
   ///    Defined from m_T0Map
   /// @{
   unsigned firstI() const;
   unsigned firstJ() const;
   unsigned lastI() const;
   unsigned lastJ() const;
   /// @}

private:

   /// @defgroup User_interface_data
   /// @{
   int    m_smoothRadius;                  ///< Smoothing radius                           [Cells]
   double m_t_0;                           ///< Beginning of rifting                       [Ma]
   double m_t_r;                           ///< End of rifting                             [Ma]
   double m_initialCrustThickness;         ///< Initial continental crust thickness        [m]
   double m_initialLithosphericThickness;  ///< Initial lithospheric mantle thickness      [m]
   double m_maxBasalticCrustThickness;     ///< Maximum oceanic (basaltic) crust thickness [m]
   double m_seaLevelAdjustment;            ///< Sea level adjustment                       [m]

   const GridMap * m_T0Map;        ///< Beginning of rifting                       [Ma]
   const GridMap * m_TRMap;        ///< End of rifting                             [Ma]
   const GridMap * m_HCuMap;       ///< Initial continental crust thickness        [m]
   const GridMap * m_HLMuMap;      ///< Initial lithospheric mantle thickness      [m]
   const GridMap * m_HBuMap;       ///< Maximum oceanic (basaltic) crust thickness [m]
   const GridMap * m_DeltaSLMap;   ///< Sea level adjustment                       [m]
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
   DerivedProperties::DerivedPropertyManager* m_derivedManager;          ///< The derived property manager (we have to use a pointer for forward declaration due to crossdependent libraries)
   DerivedProperties::SurfacePropertyPtr m_pressureBasement;             ///< The pressure of the basement at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom;          ///< The pressure of the water bottom at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureMantle;               ///< The pressure of the bottom of mantle at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_pressureMantleAtPresentDay;   ///< The pressure of the bootom of mantle at the present day
   DerivedProperties::SurfacePropertyPtr m_depthBasement;                ///< The depth of the basement at the current snapshot
   DerivedProperties::SurfacePropertyPtr m_depthWaterBottom;             ///< The depth of the water bottom at the current snapshot
   /// @}

   /// @defgroup Basic_constants
   /// @{
   double m_coeffThermExpansion;   ///< Thermal expension coefficeint
   double m_initialSubsidenceMax;  ///< Maximum initial subsidence
   double m_pi;                    ///< Pi
   double m_E0;
   double m_tau;
   /// @}

   /// @defgroup Lithosphere_and_crust_properties
   /// @{
   double m_modelTotalLithoThickness;    ///< Total lithospher thickness (crust + lithospheric mantle)
   /// @todo Why do we use two mantle and crust density?
   double m_backstrippingMantleDensity;  ///< The backstriped lithospheric mantle density
   double m_lithoMantleDensity;          ///< The lithospheric mantle density
   double m_baseLithosphericTemperature; ///< The bottom lithospheric mantle temperature
   double m_referenceCrustThickness;     ///< The reference continental crust thickness
   double m_referenceCrustDensity;       ///< The reference continental crust density
   double m_waterDensity;                ///< The water density
   /// @}

   /// @defgroup Asthenosphere_potential_temperature_data
   /// @{
   double m_A;
   double m_B;
   /// @}

   /// @defgroup Solidus_data
   ///    onsetof adiabatic melting
   /// @{
   double m_C;
   double m_D;
   /// @}

   /// @defgroup Magma-layer_density_data
   /// @{
   double m_E;
   double m_F;
   double m_decayConstant;
   /// @}

   /// @defgroup Variables
   /// @{
   double m_modelCrustDensity; ///< Estimated continental crust density
   double m_TF_onset;          ///< Crustal thinning factor at melt onset
   double m_TF_onset_lin;      ///< Liner approximation of thinning factor at melt onset
   double m_TF_onset_mig;      ///< Crustal thinning factor at threshold 'm_maxBasalticCrustThickness'
   double m_PTa;               ///< Asthenospheric mantle potential temperature
   double m_magmaticDensity;   ///< Asthenospheric mantle density
   double m_WLS_onset;         ///< Water loaded subsidence at melt onset
   /// @todo Ask Natalya what are these WLS for
   double m_WLS_crit;          ///< Water loaded subsidence at critical point (TF=1, basalt)
   double m_WLS_exhume;        ///< Water loaded subsidence at exhumation point (TF=1, no basalt)
   double m_WLS_exhume_serp;   ///< Water loaded subsidence at exhime point with serpentinite (TF=1, serpentinite)
   /// @}

   string m_baseRiftSurfaceName;  ///< Name of a base of syn-rift 

   /// @brief Clean all class members
   void clean();
};

//------------------------------------------------------------//

inline const GridMap* InterfaceInput::getT0Map() const {
   return m_T0Map;
}

inline const GridMap* InterfaceInput::getTRMap() const {
   return m_TRMap;
}

inline const GridMap* InterfaceInput::getHCuMap() const {
   return m_HCuMap;
}

inline const GridMap* InterfaceInput::getHLMuMap() const {
   return m_HLMuMap;
}

inline const GridMap* InterfaceInput::getDeltaSLMap() const {
   return m_DeltaSLMap;
}

inline int InterfaceInput::getSmoothRadius() const {
   return m_smoothRadius;
}

inline double InterfaceInput::getDeltaSLValue( unsigned int i, unsigned int j ) const {
   
   if( m_DeltaSLMap->getValue(i, j) != m_DeltaSLMap->getUndefinedValue() ) {
      return m_DeltaSLMap->getValue(i, j);
   } else {
      return Interface::DefaultUndefinedMapValue;
   }
}

inline double InterfaceInput::getMidAge() const {
   return (m_t_r + m_t_0) / 2;
}

inline double InterfaceInput::getInitialCrustThickness() const {
   
   return m_initialCrustThickness;
}

inline double InterfaceInput::getInitialLithosphereThickness() const {
   
   return m_initialLithosphericThickness;
}

inline double InterfaceInput::getBackstrippingMantleDensity() const {
   
   return m_backstrippingMantleDensity;
}

inline double InterfaceInput::getWaterDensity() const {
   
   return m_waterDensity;
}

inline double InterfaceInput::getEstimatedCrustDensity() const {
   
   return m_modelCrustDensity;
}

inline double InterfaceInput::getTFOnset() const {
   
   return m_TF_onset;
}

inline double InterfaceInput::getTFOnsetLin() const {
   
   return m_TF_onset_lin;
}

inline double InterfaceInput::getTFOnsetMig() const {
   
   return m_TF_onset_mig;
}

inline double InterfaceInput::getPTa() const {
   
   return m_PTa;
}

inline double InterfaceInput::getMagmaticDensity() const {
   
   return m_magmaticDensity;
}

inline double InterfaceInput::getWLSexhume() const {
   
   return m_WLS_exhume;
}

inline double InterfaceInput::getWLScrit() const {
   
   return m_WLS_crit;
}

inline double InterfaceInput::getWLSonset() const {
   
   return m_WLS_onset;
}

inline double InterfaceInput::getWLSexhumeSerp() const {
   
   return m_WLS_exhume_serp;
}

inline const string& InterfaceInput::getBaseRiftSurfaceName() const {
   return m_baseRiftSurfaceName;
}

inline unsigned InterfaceInput::firstI() const {
   return m_T0Map->firstI();
}

inline unsigned InterfaceInput::firstJ() const {
   return m_T0Map->firstJ();
}
inline unsigned InterfaceInput::lastI() const {
   return m_T0Map->lastI();
}

inline unsigned InterfaceInput::lastJ() const {
   return m_T0Map->lastJ();
}

#endif

