#ifndef __FASTCAULDRON_SIMULATOR_HH__
#define __FASTCAULDRON_SIMULATOR_HH__

#include <map>
#include <vector>

#include "timefilter.h"
#include "globaldefs.h"

#include "PropertyOutputConstraints.h"

// TableIO
#include "database.h"

// Data access
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

#include "IBSinterpolator.h"

#include "GeoPhysicsProjectHandle.h"

#include "ElementGrid.h"
#include "NodalGrid.h"
#include "MapElement.h"
#include "ghost_array.h"

// #include "MultiComponentFlowHandler.h"

// Forward declarations
class AppCtx;
class LayerProps;
class Subdomain;
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


/// \brief An array of subdomains.
typedef std::vector<Subdomain*> SubdomainArray;



class FastcauldronSimulator : public GeoPhysics::ProjectHandle {

public :


   /// Constructor.
   ///
   /// Only to be created by using the "CreateFrom" function.
   FastcauldronSimulator (database::Database * database, const std::string & name, const std::string & accessMode);

   ~FastcauldronSimulator ();

   /// Return the reference to the project data.
   static FastcauldronSimulator& getInstance ();

   /// Return a pointer to the AppCtx class.
   // This is a temporary function, until more functionality is moved from AppCtx to here.
   const AppCtx* getCauldron () const;

   /// Open the project file and return a pointed to the fastcauldron simulator.
   ///
   /// Fix when singlton is complete.
   static FastcauldronSimulator* CreateFrom ( AppCtx* cauldron );

   /// Finish any activity and deallocate the singleton object.
   ///
   /// \b Must be called at end of calculation.
   static void finalise ( const bool saveResults );

   /// \brief If required, the number of 
   void setFormationElementHeightScalingFactors ();

   /// Set the calculation mode.
   ///
   /// Now the the calculation mode is known, the activity grid and sizes are set.
   // Perhaps should re-name this to some-thing else, since it now does much more than just set calculation mode.
   bool setCalculationMode ( const CalculationMode mode,
                             const bool            printElementValidityMap = false );

   /// Get the calculation mode of the current run.
   CalculationMode getCalculationMode () const;

   void initialiseFastcauldronLayers ();

   // Here temporarily until element-array has been moved from propinterface.
   void initialiseElementGrid ( const bool printElementValidityMap = false );

   /// Add properties that were not added.
   ///
   /// Those that were not loaded by 'Interface::ProjectHandle::loadProperties', e.g. chemical-compaction.
   /// Some of these may not be output but nay be required for calculation of derived properties. 
   /// This function should be called after the project file has been read-in, since some properties depend
   /// on this, e.g. FaultElements.
   void correctAllPropertyLists ();

   void setOutputPropertyOption ( const PropertyList                    property,
                                  const Interface::PropertyOutputOption option );


   void updateSnapshotFileCreationFlags ();

   void setConstrainedOverpressureIntervals ();

   Interface::PropertyValue * createMapPropertyValue ( const string &                    propertyValueName,
                                                            const Interface::Snapshot *  snapshot,
                                                            const Interface::Reservoir * reservoir,
                                                            const Interface::Formation * formation,
                                                            const Interface::Surface *   surface );

   Interface::PropertyValue * createVolumePropertyValue ( const string &                    propertyValueName,
                                                               const Interface::Snapshot *  snapshot,
                                                               const Interface::Reservoir * reservoir,
                                                               const Interface::Formation * formation,
                                                                     unsigned int                depth );

   void deleteSnapshotProperties ();

   void deleteSnapshotPropertyValueMaps ();

   bool saveCreatedVolumePropertyValues ();




   ///
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
   static int DACreate2D ( DA& theDA );

   /// Create a 3d distributed array based on the dimensions used by the DAL and the number of z-nodes passed by the user.
   ///
   /// The distributed array will not be partitioned in the z-direction.
   // Remove when no-longer needed.
   static int DACreate3D ( const int numberOfZNodes,
                           DA& theDA );

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


   /// \brief Read the command-line parameters.
   void readCommandLineParameters ( const int argc, char **argv );


   /// \brief Set the lateral-stress-factor interpolator.
   void addLateralStressInterpolator ( ibs::Interpolator* interp );

   /// \brief Evaluate the lateral-stress-factor interpolator.
   double lateralStressFactor ( const double time ) const;


   /// \brief Set the relative-permeability funtion type.
   void setRelativePermeabilityType ( const RelativePermeabilityType relPerm );

   /// \brief Get the relative-permeability funtion type.
   RelativePermeabilityType getRelativePermeabilityType () const;

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

private :


   // Change name here.
   class PropertyPartitioningPredicate {
   public :

      PropertyPartitioningPredicate ( const Interface::MutablePropertyValueList& list );

      bool operator ()( const Interface::PropertyValue* property ) const;

   private :

      const Interface::MutablePropertyValueList& m_list;

   };

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

   bool savePropertiesOnSegmentNodes1D ();


   static FastcauldronSimulator* m_fastcauldronSimulator;

   AppCtx* m_cauldron;

   void correctTimeFilterDefaults1D ();

   void correctTimeFilterDefaults3D ();


   void correctAllPropertyLists1D ();

   void correctAllPropertyLists3D ();

   /// \brief Read command line parameters associated with the relative-permeabilities.
   void readRelPermCommandLineParameters ();


   CalculationMode m_calculationMode;
   Interface::MutablePropertyValueList m_propertyValuesForOutput;
   Interface::MutableSnapshotList m_minorSnapshots;
   PropertyOutputConstraints m_propertyConstraints;
   ElementGrid               m_elementGrid;
   NodalGrid                 m_nodalGrid;
   MapElementArray           m_mapElements;

   // /// \brief Array of subdomains that are to be used in the multi-component flow solver.
   // SubdomainArray            m_mcfSubdomains;

   ibs::Interpolator*        m_lateralStressInterpolator;


   MultiComponentFlowHandler* m_mcfHandler;
   RelativePermeabilityType   m_relativePermeabilityType;
   double                     m_minimumHcSaturation;
   double                     m_minimumWaterSaturation;
   double                     m_waterCurveExponent;
   double                     m_hcLiquidCurveExponent;
   double                     m_hcVapourCurveExponent;
   double                     m_fctCorrectionScalingWeight;
   bool                       m_printCommandLine;

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

inline RelativePermeabilityType FastcauldronSimulator::getRelativePermeabilityType () const {
   return m_relativePermeabilityType;
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


#endif // __FASTCAULDRON_SIMULATOR_HH__
