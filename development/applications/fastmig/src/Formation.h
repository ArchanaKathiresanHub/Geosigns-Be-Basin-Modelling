//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_FORMATION_H_
#define _MIGRATION_FORMATION_H_

namespace database
{
   class Record;
   class Table;
}

#include "GeoPhysicsFormation.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class Property;
      class Snapshot;
   }
}
namespace Genex6
{
   class LinearGridInterpolator;
}

using namespace DataAccess;

#include "Migrator.h"
#include "FormationNode.h"
#include "SurfaceGridMap.h"
#include "FormationSurfaceGridMaps.h"
#include "RequestDefs.h"
#include "FormationProperty.h"

/// Formation Class
namespace migration
{
   class FormationNode;

   /// This class implements the Formation-specific migration functionality.
   /// It is constructed on top of the DataAccess::formation class.
   class Formation : public GeoPhysics::Formation
   {
   public:
      /// This constructor is called by the object factory
      Formation (Interface::ProjectHandle * projectHandle, Migrator * migrator, database::Record * record);

      /// Destructor
      virtual ~Formation (void);

      void createFormationNodes (void);

      inline static Formation * CastToFormation (const Interface::Formation * formation);

      bool computePropertyMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot, bool lowResEqualsHighRes,
                                bool isPressureRun, bool nonGeometricLoop, bool chemicalCompaction);

      bool computeCapillaryPressureMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot);

      double capillaryEntryPressureOilGas (const double permeability, const double brinePressure, const double capC1, const double capC2) const;

      bool computeHCDensityMaps ();

      bool removeComputedPropertyMaps (void);

      inline Migrator * getMigrator (void) const;

      SurfaceGridMap getBaseSurfaceGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      FormationSurfaceGridMaps getFormationSurfaceGridMaps (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      bool hasVolumePropertyGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot * snapshot) const;
      bool hasVolumePropertyGridMap (const DataAccess::Interface::Property* prop, double snapshotAge) const;
      bool hasVolumePropertyGridMap (const string & propertyName, const DataAccess::Interface::Snapshot * snapshot) const;
      bool hasVolumePropertyGridMap (const string & propertyName, double snapshotAge) const;

      const GridMap * getVolumePropertyGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot * snapshot) const;
      const GridMap * getVolumePropertyGridMap (const DataAccess::Interface::Property* prop, double snapshotAge) const;
      const GridMap * getVolumePropertyGridMap (const string & propertyName, const DataAccess::Interface::Snapshot * snapshot) const;
      const GridMap * getVolumePropertyGridMap (const string & propertyName, double snapshotAge) const;

      bool computeFiniteElement (int i, int j, int k);
      void setFiniteElementQuadraturePoint (int i, int j, int k, double iOffset, double jOffset, double kOffset);
      double getFiniteElementValue (int i, int j, int k, double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex);
      double getFiniteElementValue (int i, int j, int k, PropertyIndex propertyIndex);
      FiniteElementMethod::FiniteElement & getFiniteElement (int i, int j, int k);

      LocalFormationNode * getLocalFormationNode (int i, int j, int k) const;
      FormationNode * getFormationNode (int i, int j, int k);
      double getFormationNodeDepth (int i, int j, int k);

      int getNodeDepth (void) const;
      int getGridMapDepth (void) const;

      bool detectReservoir (Formation * topFormation, const double minOilColumnHeight, const double minGasColumnHeight, const bool pressureRun);
      bool detectReservoirCrests();
      bool getDetectedReservoir() const;
      void addDetectedReservoir ();

      void saveReservoir (const Interface::Snapshot * curSnapshot);

      void identifyAsReservoir (void) const;

      double getPropertyValue (PropertyIndex propertyIndex, int i, int j, int k) const;

      double getMinOilColumnHeight (void) const;
      double getMinGasColumnHeight (void) const;

      inline double getDepth (int i, int j, int k) const;
      inline double getHorizontalPermeability (int i, int j, int k);
      inline double getVerticalPermeability (int i, int j, int k);
      inline double getPorosity (int i, int j, int k);
      inline double getCapillaryPressureGas100 (int i, int j, int k);
      inline double getCapillaryPressureGas0 (int i, int j, int k);
      inline double getCapillaryPressureOil100 (int i, int j, int k);
      inline double getCapillaryPressureOil0 (int i, int j, int k);
      inline double getPressure (int i, int j, int k);
      inline double getTemperature (int i, int j, int k);
      inline double getOilDensity (int i, int j, int k);
      inline double getGasDensity (int i, int j, int k);

      inline const GridMap * getGridMap (PropertyIndex propertyIndex) const;
      inline const GridMap * & gridMap (PropertyIndex propertyIndex);

      inline const GridMap * getDepthGridMap (void) const;
      inline const GridMap * getPorosityGridMap (void) const;
      inline const GridMap * getHorizontalPermeabilityGridMap (void) const;
      inline const GridMap * getVerticalPermeabilityGridMap (void) const;
      inline const GridMap * getTemperatureGridMap (void) const;
      inline const GridMap * getPressureGridMap (void) const;
      inline const GridMap * getCapPressureOil100GridMap (void) const;
      inline const GridMap * getCapPressureGas100GridMap (void) const;
      inline const GridMap * getCapPressureGas0GridMap (void) const;
      inline const GridMap * getCapPressureOil0GridMap (void) const;

      bool saveComputedSMFlowPaths (Formation * targetFormation, const Interface::Snapshot * end);
      bool saveComputedSMFlowPathsByGridOffsets (const DataAccess::Interface::Snapshot * end);

      bool computeFaults (const Interface::Snapshot * end);

      void loadExpulsionMaps (const Interface::Snapshot * begin, const Interface::Snapshot * end);
      void unloadExpulsionMaps ();

      bool computeTargetFormationNodes (Formation * targetFormation);
      bool computeTargetFormationNodes (int depthIndex);
      bool computeTargetFormationNode (unsigned int i, unsigned int j, int depthIndex);
      void prescribeTargetFormationNodes (void);

      bool retrievePropertyMaps ();
      bool restorePropertyMaps (void);

      bool retrieveCapillaryPressureMaps ();
      bool restoreCapillaryPressureMaps (void);

      bool clearNodeProperties (void);
      bool clearNodeReservoirProperties (void);
      bool computeNodeProperties ();

      const Interface::Grid * getGrid (void) const;
      double getDeltaI (void);
      double getDeltaJ (void);

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (int i, int j, int k);

      int getIndex (void);

      SurfaceGridMap getTopSurfaceGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const GridMap * getTopGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const GridMap * getBaseGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      Formation * getTopFormation () const;
      Formation * getBottomFormation () const;

      const GridMap* getFormationPrimaryPropertyGridMap (const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot) const;

      const GridMap* getSurfacePropertyGridMap (const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot,
                                                const Interface::Surface* surface) const;

      bool isActive (const DataAccess::Interface::Snapshot * snapshot) const;

      bool computeAnalogFlowDirections (Formation * targetFormation, const Interface::Snapshot * begin,
                                        const Interface::Snapshot * end);
      bool computeAnalogFlowDirections (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end);

      bool computeAdjacentNodes (Formation * targetFormation, const Interface::Snapshot * begin, const Interface::Snapshot * end);
      bool computeAdjacentNodes (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end);

      void migrateExpelledChargesToReservoir (unsigned int direction, Reservoir * targetReservoir) const;
      void migrateLeakedChargesToReservoir (Reservoir * targetReservoir) const;

      void getValue (FormationNodeValueRequest & request, FormationNodeValueRequest & response);
      void setValue (FormationNodeValueRequest & request);

      void getThreeVector (FormationNodeThreeVectorRequest & request, FormationNodeThreeVectorRequest & response);
      void getThreeVectorValue (FormationNodeThreeVectorValueRequest & request, FormationNodeThreeVectorValueRequest & response);

      bool isPreprocessed () const;

      bool calculateGenexTimeInterval (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool printDebug = false);
      bool preprocessSourceRock (const double startTime, const bool printDebug = false);

      const Interface::GridMap * getOneComponentMap (int componentId);

      Interface::GridMap* getGenexData () const { return m_genexData; }

      const Interface::GridMap * getPropertyGridMap (const string & propertyName,
                                                     const Interface::Snapshot * snapshot) const;

      inline bool performVerticalMigration (void) const;
      inline bool performHDynamicAndCapillary (void) const;
      inline bool performReservoirDetection (void) const;
      inline double getBlockingPermeability (void);
      inline double getBlockingPorosity (void);


   private:

      // Pointer to the migrator object
      Migrator * const m_migrator;

      // indicates whether the source rock is initialized and preprocessed
      bool m_isInitialised;

      // Last calculated genex time interval
      double m_startGenexTime;
      double m_endGenexTime;

      /// The formation nodes of the formation
      FormationNodeArray * m_formationNodeArray;

      // Map of all genex data
      Interface::GridMap* m_genexData;

      DerivedProperties::FormationPropertyPtr getFormationPropertyPtr (const string & propertyName, const Interface::Snapshot * snapshot) const;

      bool computeInterpolator (const string & propertyName, const Interface::Snapshot *intervalStart, const Interface::Snapshot *intervalEnd,
                                Genex6::LinearGridInterpolator& interpolator);
      bool extractGenexDataInterval (const Interface::Snapshot *intervalStart, const Interface::Snapshot *intervalEnd,
                                     Genex6::LinearGridInterpolator& thickness,
                                     Genex6::LinearGridInterpolator& ves,
                                     Genex6::LinearGridInterpolator& temperature,
                                     Genex6::LinearGridInterpolator& hydrostaticPressure,
                                     Genex6::LinearGridInterpolator& lithostaticPressure,
                                     Genex6::LinearGridInterpolator& porePressure,
                                     Genex6::LinearGridInterpolator& porosity,
                                     Genex6::LinearGridInterpolator& permeability,
                                     Genex6::LinearGridInterpolator& vre);

      // loaded or computed grid maps
      const Interface::GridMap * m_gridMaps[NUMBEROFPROPERTYINDICES];

      GridMap * m_expulsionGridMaps[NUM_COMPONENTS];
      int m_index;

      // is a detected reservoir formation
      bool m_detectedReservoir;
      bool  m_detectedReservoirRecord;

   };

   Migrator * Formation::getMigrator (void) const
   {
      return m_migrator;
   }

   Formation * Formation::CastToFormation (const Interface::Formation * formation)
   {
      return dynamic_cast <Formation *> ((Interface::Formation *) (formation));
   }

   inline bool Formation::isPreprocessed () const
   {
      return m_isInitialised;
   }

   double Formation::getDepth (int i, int j, int k) const
   {
      return getPropertyValue (DEPTHPROPERTY, i, j, k);
   }

   double Formation::getPorosity (int i, int j, int k)
   {
      return getPropertyValue (POROSITYPROPERTY, i, j, k);
   }

   double Formation::getHorizontalPermeability (int i, int j, int k)
   {
      return getPropertyValue (HORIZONTALPERMEABILITYPROPERTY, i, j, k);
   }

   double Formation::getVerticalPermeability (int i, int j, int k)
   {
      return getPropertyValue (VERTICALPERMEABILITYPROPERTY, i, j, k);
   }

   double Formation::getCapillaryPressureGas100 (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYPRESSUREGAS100PROPERTY, i, j, k);
   }

   double Formation::getCapillaryPressureGas0 (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYPRESSUREGAS0PROPERTY, i, j, k);
   }

   double Formation::getCapillaryPressureOil100 (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYPRESSUREOIL100PROPERTY, i, j, k);
   }

   double Formation::getCapillaryPressureOil0 (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYPRESSUREOIL0PROPERTY, i, j, k);
   }

   double Formation::getGasDensity (int i, int j, int k)
   {
      return getPropertyValue (GASDENSITYPROPERTY, i, j, k);
   }

   double Formation::getOilDensity (int i, int j, int k)
   {
      return getPropertyValue (OILDENSITYPROPERTY, i, j, k);
   }

   double Formation::getPressure (int i, int j, int k)
   {
      return getPropertyValue (PRESSUREPROPERTY, i, j, k);
   }

   double Formation::getTemperature (int i, int j, int k)
   {
      return getPropertyValue (TEMPERATUREPROPERTY, i, j, k);
   }

   const GridMap * & Formation::gridMap (PropertyIndex propertyIndex)
   {
      return m_gridMaps[propertyIndex];
   }

   const GridMap * Formation::getGridMap (PropertyIndex propertyIndex) const
   {
      return m_gridMaps[propertyIndex];
   }

   const GridMap * Formation::getDepthGridMap (void) const
   {
      return getGridMap (DEPTHPROPERTY);
   }

   const GridMap * Formation::getPorosityGridMap (void) const
   {
      return getGridMap (POROSITYPROPERTY);
   }

   const GridMap * Formation::getHorizontalPermeabilityGridMap (void) const
   {
      return getGridMap (HORIZONTALPERMEABILITYPROPERTY);
   }

   const GridMap * Formation::getVerticalPermeabilityGridMap (void) const
   {
      return getGridMap (VERTICALPERMEABILITYPROPERTY);
   }

   const GridMap * Formation::getTemperatureGridMap (void) const
   {
      return getGridMap (TEMPERATUREPROPERTY);
   }

   const GridMap * Formation::getPressureGridMap (void) const
   {
      return getGridMap (PRESSUREPROPERTY);
   }

   const GridMap * Formation::getCapPressureGas100GridMap (void) const
   {
      return getGridMap (CAPILLARYPRESSUREGAS100PROPERTY);
   }

   const GridMap * Formation::getCapPressureGas0GridMap (void) const
   {
      return getGridMap (CAPILLARYPRESSUREGAS0PROPERTY);
   }

   const GridMap * Formation::getCapPressureOil100GridMap (void) const
   {
      return getGridMap (CAPILLARYPRESSUREOIL100PROPERTY);
   }

   const GridMap * Formation::getCapPressureOil0GridMap (void) const
   {
      return getGridMap (CAPILLARYPRESSUREOIL0PROPERTY);
   }

   bool Formation::performVerticalMigration (void) const
   {
      return m_migrator->performVerticalMigration ();
   }

   bool Formation::performHDynamicAndCapillary (void) const
   {
      return m_migrator->performHDynamicAndCapillary ();
   }

   bool Formation::performReservoirDetection (void) const
   {
      return m_migrator->performReservoirDetection ();
   }

   double Formation::getBlockingPermeability (void)
   {
      return m_migrator->getBlockingPermeability ();
   }

   double Formation::getBlockingPorosity (void)
   {
      return m_migrator->getBlockingPorosity ();
   }

}

#endif // _MIGRATION_FORMATION_H_

