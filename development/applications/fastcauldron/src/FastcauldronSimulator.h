//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __FASTCAULDRON_SIMULATOR_HH__
#define __FASTCAULDRON_SIMULATOR_HH__

#include <map>
#include <vector>

#include "timefilter.h"
#include "ConstantsFastcauldron.h"

#include "PropertyOutputConstraints.h"

// TableIO
#include "database.h"

// Data access
#include "Interface.h"
#include "ProjectHandle.h"

#include "IBSinterpolator.h"

#include "GeoPhysicsProjectHandle.h"

#include "ElementGrid.h"
#include "NodalGrid.h"
#include "MapElement.h"
#include "ghost_array.h"

// #include "MultiComponentFlowHandler.h"

// Forward declarations
class AppCtx;
class FastcauldronFactory;
class LayerProps;
class MultiComponentFlowHandler;

using namespace DataAccess;

// namespace database {
   bool SnapshotRecordLess ( database::Record* left,
                             database::Record* right );
// }


/// A two-d array of map-elements.
///
/// The size of the array is determined by the element-grid.
typedef PETSc_Local_2D_Array<MapElement> MapElementArray;


class FastcauldronSimulator : public GeoPhysics::ProjectHandle {

public :


   /// Constructor.
   ///
   /// Only to be created by using the "CreateFrom" function.
   FastcauldronSimulator (database::ProjectFileHandlerPtr& pfh,
                          const std::string & name,
                          const DataAccess::Interface::ObjectFactory* factory);

   ~FastcauldronSimulator ();

   /// Return the reference to the project data.
   static FastcauldronSimulator& getInstance ();

   /// Return a pointer to the AppCtx class.
   // This is a temporary function, until more functionality is moved from AppCtx to here.
   const AppCtx* getCauldron () const;

   /// Open the project file and return a pointed to the fastcauldron simulator.
   ///
   /// Fix when singlton is complete.
   static FastcauldronSimulator* CreateFrom (AppCtx* cauldron,
                                             const FastcauldronFactory* objectFactory,
                                             const std::vector<std::string>& outputTableNames );

   /// Finish any activity and deallocate the singleton object.
   ///
   /// \b Must be called at end of calculation.
   static void finalise ( const bool saveResults );

   /// Merge output files if nessecary
   ///
   /// \b Must be called at end of calculation.
   bool mergeOutputFiles ( );
   bool mergeSharedOutputFiles ( );

   /// \brief If required, the number of
   void setFormationElementHeightScalingFactors ();

   /// Set the calculation mode.
   ///
   /// Now the the calculation mode is known, the activity grid and sizes are set.
   // Perhaps should re-name this to some-thing else, since it now does much more than just set calculation mode.
   bool setCalculationMode ( const CalculationMode mode, const bool saveAsInputGrid = false, const bool createResultsFile = true );

   /// Get the calculation mode of the current run.
   CalculationMode getCalculationMode () const;

   /// Add the current simulation detailes record (temporary)
   database::Record * addCurrentSimulationDetails();
   /// Remove the temporary added simulation detailes record
   void removeCurrentSimulationDetails( database::Record * );

   /// \brief Determine if the last fastcauldron PT run was coupled or not.
   bool getLastPTWasCoupled () const;

   void clear1DTimeIoTbl ();

   void initialiseFastcauldronLayers ();

   /// Determines which element pillars are active. A pillar is a stack of elements through all layers.
   /// \param hasActiveElements This is an output parameter by which the
   //         function returns whether there are any active elements at all.
   void initialiseElementGrid ( bool& hasActiveElements );

   /// Prints a 2D map of active elements in a text file with ASCII art.
   void printElementValidityMap( const std::string & fileName ) const;

   /// Add properties that were not added.
   ///
   /// Those that were not loaded by 'Interface::ProjectHandle::loadProperties', e.g. chemical-compaction.
   /// Some of these may not be output but nay be required for calculation of derived properties.
   /// This function should be called after the project file has been read-in, since some properties depend
   /// on this, e.g. FaultElements.
   void correctAllPropertyLists ();

   void setOutputPropertyOption ( const PropertyIdentifier                    property,
                                  const Interface::PropertyOutputOption option );

   Interface::PropertyOutputOption getOutputPropertyOption ( const std::string& propertyName );

   void updateSnapshotFileCreationFlags ();

   void setConstrainedOverpressureIntervals ();

   Interface::PropertyValue * createMapPropertyValue ( const string &                    propertyValueName,
                                                            const Interface::Snapshot *  snapshot,
                                                            const Interface::Reservoir * reservoir,
                                                            const Interface::Formation * formation,
                                                            const Interface::Surface *   surface );

   Interface::PropertyValue * createVolumePropertyValue ( const string &               propertyValueName,
                                                          const Interface::Snapshot *  snapshot,
                                                          const Interface::Reservoir * reservoir,
                                                          const Interface::Formation * formation,
                                                          unsigned int                 depth,
                                                          const std::string &          fileName = "");

   void deleteSnapshotProperties ();

   void deleteSnapshotPropertyValueMaps ();


   ///
   void saveSourceRockProperties ( const Interface::Snapshot* snapshot,
                                   const PropListVec&         genexProperties,
                                   const PropListVec&         shaleGasProperties );

   void saveMapProperties ( const PropListVec&                    requiredProperties,
                            const Interface::Snapshot*            snapshot,
                            const Interface::PropertyOutputOption maximumOutputOption );

   void saveVolumeProperties ( const PropListVec&                    requiredProperties,
                               const Interface::Snapshot*            snapshot,
                               const Interface::PropertyOutputOption maximumOutputOption );

   void saveProperties ( const PropListVec&                    mapProperties,
                         const PropListVec&                    volumeProperties,
                         const Interface::Snapshot*            snapshot,
                         const Interface::PropertyOutputOption maximumOutputOption );

   const Interface::Snapshot* findOrCreateSnapshot ( const double time, const int type );

   const Interface::Snapshot* findOrCreateSnapshot ( const double time );

   void deleteMinorSnapshots ();

   void deleteMinorSnapshotsFromSnapshotTable ();

   void printSnapshotProperties () const;

   /// \brief Update any flags in the source rocks for the GenEx simulation.
   ///
   /// This includes adding node history.
   void updateSourceRocksForGenex ();

   /// \brief Save GenEx history.
   void saveGenexHistory ();

   /// \brief Update any flags in the source rocks for the Darcy simulation.
   ///
   /// This includes disabling of Adsorption.
   void updateSourceRocksForDarcy ();

   database::Record* findTimeIoRecord ( database::Table*   timeIoTbl,
                                        const std::string& propertyName,
                                        const double       time,
                                        const std::string& surfaceName,
                                        const std::string& formationName ) const;

   LayerProps* findLayer ( const std::string& layerName ) const;

   /// Returns whether or not the node (in the plane) is defined or not.
   // Would like to move some functionality, including this, from AppCtx to this class.
   bool nodeIsDefined ( const int i, const int j ) const;

   /// Create a 2d distributed array based on the dimensions used by the DAL.
   // Remove when no-longer needed.
   static int DACreate2D ( DM& theDA );

   /// Create a 3d distributed array based on the dimensions used by the DAL and the number of z-nodes passed by the user.
   ///
   /// The distributed array will not be partitioned in the z-direction.
   // Remove when no-longer needed.
   static int DACreate3D ( const int numberOfZNodes,
                           DM& theDA );

   /// Return constant reference to the element-grid.
   const ElementGrid& getElementGrid () const;

   /// Return constant reference to the nodal-grid.
   const NodalGrid& getNodalGrid () const;

   /// Return constant reference to the map-element-array.
   const MapElementArray& getMapElementArray () const;

   /// Return a constnt reference to a map-element from the array.
   const MapElement& getMapElement ( const int i,
                                     const int j ) const;


   /// \brief Return a reference to the multi-component flow handler.
   MultiComponentFlowHandler& getMcfHandler ();

   const Interface::OutputProperty* findOutputProperty ( const Interface::Property* property ) const;

   /// \brief Delete major snapshots files from TMPDIR
   void deleteTemporaryDirSnapshots();

   /// \brief Read the command-line parameters that can be processed during early stage
   void readCommandLineParametersEarlyStage( const int argc, char **argv );

   /// \brief Read the command-line parameters that can only be processed in a later stage
   void readCommandLineParametersLateStage( const int argc, char **argv );

   /// \brief Set the lateral-stress-factor interpolator.
   void addLateralStressInterpolator ( ibs::Interpolator* interp );

   /// \brief Evaluate the lateral-stress-factor interpolator.
   double lateralStressFactor ( const double time ) const;

   double getMinimumHcSaturation () const;

   double getMaximumHcSaturation () const;

   double getWaterCurveExponent () const;

   double getHcLiquidCurveExponent () const;

   double getHcVapourCurveExponent () const;


   /// \brief The
   double getFctCorrectionScalingWeight () const;

   /// \name Consistency and error checks.
   /// @{

   /// \brief Checks to see if any mobile layer has a negative thickness defined.
   ///
   /// All mobile layers must have non-negative thicknesses.
   bool checkMobileLayerThicknesses () const;

   /// @}

   /// \brief Indicate whether or not the capillary-entry pressure should be calculated or use the default.
   ///
   /// By default the constant 1.0e6 Pa is the capillary entry pressure.
   bool useCalculatedCapillaryPressure () const;

   /// \brief Return the flag indicating the calculation of derived properties
   bool doDerivedPropertiesCalc () const;

   /// \brief Clean the recordless properies list
   void removeRecordlessDerivedPropertyValues();

   /// \brief Connect one property to output property from FilterTimeIoTbl
   void connectOutputProperty ( const Interface::Property* aProperty );

private :


   // Change name here.
   class PropertyPartitioningPredicate {
   public :

      PropertyPartitioningPredicate ( const Interface::MutablePropertyValueList& list );

      bool operator ()( const Interface::PropertyValue* property ) const;

   private :

      const Interface::MutablePropertyValueList& m_list;

   };

   /// \brief Correct permeability entries in filter time-io table.
   ///
   /// These are:
   /// - PermeabilityVec
   /// - PermeabilityHVec
   /// - HorizontalPermeability
   void correctPermeabilityTimeFilter ();

   /// Some properties that are required may not be selected, or the output selection may be too restrictive.
   void correctTimeFilterDefaults ();

   /// Adds an output-property pointer to the property.
   void connectOutputProperties ();

   /// Set the fluid density to constant density if the calculation mode is one of:
   ///
   ///   o hydrostatic decompaction;
   ///   o hydrostatic high res decompaction;
   ///   o hydrostatic temperature;
   ///
   void setToConstantDensity ();

   /// \brief Set the viscosity model of the fluid types.
   void setBrineViscosities ( const Interface::ViscosityModel model );

   /// \brief Prints the command if requested.
   void printCommandLine ( const int argc, char **argv );

   static FastcauldronSimulator* m_fastcauldronSimulator;

   AppCtx* m_cauldron;

   void correctTimeFilterDefaults1D ();

   void correctTimeFilterDefaults3D ();


   /// \brief Read command line parameters associated with the relative-permeabilities.
   void readRelPermCommandLineParameters ();

   /// \brief Read positions for outputting data along pseudo well from command line.
   ///
   /// Data is added to related projects list.
   void readCommandLineWells ();


   CalculationMode m_calculationMode;
   Interface::MutablePropertyValueList m_propertyValuesForOutput;
   Interface::MutableSnapshotList m_minorSnapshots;
   PropertyOutputConstraints m_propertyConstraints;
   ElementGrid               m_elementGrid;
   NodalGrid                 m_nodalGrid;
   MapElementArray           m_mapElements;

   ibs::Interpolator*        m_lateralStressInterpolator;


   MultiComponentFlowHandler* m_mcfHandler;
   double                     m_minimumHcSaturation;
   double                     m_minimumWaterSaturation;
   double                     m_waterCurveExponent;
   double                     m_hcLiquidCurveExponent;
   double                     m_hcVapourCurveExponent;
   double                     m_fctCorrectionScalingWeight;
   bool                       m_printCommandLine;
   std::string                m_commandLine;
   bool                       m_computeCapillaryPressure;
   bool                       m_derivedPropertiesCalc;

};

//------------------------------------------------------------//

inline FastcauldronSimulator& FastcauldronSimulator::getInstance () {
   return *m_fastcauldronSimulator;
}

//------------------------------------------------------------//

inline const ElementGrid& FastcauldronSimulator::getElementGrid () const {
   return m_elementGrid;
}

//------------------------------------------------------------//

inline const NodalGrid& FastcauldronSimulator::getNodalGrid () const {
   return m_nodalGrid;
}

//------------------------------------------------------------//

inline const MapElementArray& FastcauldronSimulator::getMapElementArray () const {
   return m_mapElements;
}

//------------------------------------------------------------//

inline const MapElement& FastcauldronSimulator::getMapElement ( const int i, const int j ) const {
   return m_mapElements ( i, j );
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::lateralStressFactor ( const double time ) const {

   if ( m_lateralStressInterpolator == 0 ) {
      return 0.0;
   } else {
      return m_lateralStressInterpolator->compute ( time, ibs::Interpolator::constant );
   }

}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getMinimumHcSaturation () const {
   return m_minimumHcSaturation;
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getMaximumHcSaturation () const {
   return 1.0 - m_minimumWaterSaturation;
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getWaterCurveExponent () const {
   return m_waterCurveExponent;
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getHcLiquidCurveExponent () const {
   return m_hcLiquidCurveExponent;
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getHcVapourCurveExponent () const {
   return m_hcVapourCurveExponent;
}

//------------------------------------------------------------//

inline double FastcauldronSimulator::getFctCorrectionScalingWeight () const {
   return m_fctCorrectionScalingWeight;
}

//------------------------------------------------------------//


inline bool FastcauldronSimulator::doDerivedPropertiesCalc () const {
   return m_derivedPropertiesCalc;
}
//------------------------------------------------------------//


inline bool FastcauldronSimulator::useCalculatedCapillaryPressure () const {
   return m_computeCapillaryPressure;
}


#endif // __FASTCAULDRON_SIMULATOR_HH__
