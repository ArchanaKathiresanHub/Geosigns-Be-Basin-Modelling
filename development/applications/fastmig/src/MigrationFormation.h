//
// Copyright (C) 2010-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MIGRATION_FORMATION_H
#define MIGRATION_FORMATION_H

#include <boost/array.hpp>

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;
typedef CBMGenerics::ComponentManager::PhaseId Phase;

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

/// MigrationFormation Class
namespace migration
{
   class FormationNode;

   /// This class implements the MigrationFormation-specific migration functionality.
   /// It is constructed on top of the DataAccess::formation class.
   class MigrationFormation : public GeoPhysics::GeoPhysicsFormation
   {
   public:
      /// This constructor is called by the object factory
      MigrationFormation (Interface::ProjectHandle * projectHandle, Migrator * migrator, database::Record * record);

      /// Destructor
      virtual ~MigrationFormation (void);

      void createFormationNodes (void);

      inline static MigrationFormation * CastToFormation (const Interface::Formation * formation);

      bool computePropertyMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot, bool lowResEqualsHighRes,
                                bool isPressureRun, bool nonGeometricLoop, bool chemicalCompaction);

      bool computeCapillaryPressureMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot);

      double capillaryEntryPressureLiquidVapour (const double permeability, const double brinePressure, const double capC1, const double capC2) const;

      bool computeHCDensityMaps (const Interface::Snapshot * snapshot);

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
      void setFiniteElementDepths (int i, int j, int k, double * depths);

      LocalFormationNode * getLocalFormationNode (int i, int j, int k) const;
      FormationNode * getFormationNode (int i, int j, int k) const;
      double getFormationNodeDepth (int i, int j, int k);

      int getNodeDepth (void) const;
      int getGridMapDepth (void) const;

      bool detectReservoir (MigrationFormation * topFormation, const double minLiquidColumnHeight, const double minVapourColumnHeight, const bool pressureRun, const MigrationFormation * topActiveFormation);
      bool detectReservoirCrests ();
      bool getDetectedReservoir () const;
      void setDetectedReservoir (bool detectedReservoir);
      void addDetectedReservoir (const Interface::Snapshot * start);

      void saveReservoir (const Interface::Snapshot * curSnapshot);

      void identifyAsReservoir (const bool advancedMigration) const;

      double getPropertyValue (PropertyIndex propertyIndex, int i, int j, int k) const;

      void getTopBottomOverpressures (const int i, const int j, boost::array<double,2> & overPressures) const;

      inline double getDepth (int i, int j, int k) const;
      inline double getHorizontalPermeability (int i, int j, int k);
      inline double getVerticalPermeability (int i, int j, int k);
      inline double getPorosity (int i, int j, int k);
      inline double getCapillaryEntryPressureVapour (int i, int j, int k);
      inline double getCapillaryEntryPressureLiquid (int i, int j, int k);
      inline double getPressure (int i, int j, int k);
      inline double getTemperature (int i, int j, int k);
      inline double getLiquidDensity (int i, int j, int k);
      inline double getVapourDensity (int i, int j, int k);

      bool saveComputedSMFlowPaths (MigrationFormation * targetFormation, const Interface::Snapshot * end);
      bool saveComputedSMFlowPathsByGridOffsets (const DataAccess::Interface::Snapshot * end);

      bool computeFaults (const Interface::Snapshot * end);

      void loadExpulsionMaps (const Interface::Snapshot * begin, const Interface::Snapshot * end);
      void unloadExpulsionMaps ();
      void computeExpulsionMapsOnTheFly (const Interface::Snapshot * begin, const Interface::Snapshot * end);

      bool computeTargetFormationNodes (MigrationFormation * targetFormation);
      bool computeTargetFormationNodes (int depthIndex);
      bool computeTargetFormationNode (unsigned int i, unsigned int j, int depthIndex);

      bool retrievePropertyMaps (bool);
      bool restorePropertyMaps (bool);
      void deleteFormationProperties ();

      bool clearNodeProperties (void);
      bool clearNodeReservoirProperties (void);
      LocalFormationNode * validReservoirNode (const int i, const int j) const;
      LocalFormationNode * validSealNode (const int i, const int j, const MigrationFormation * topFormation, const MigrationFormation * topActiveFormation) const;
      bool computeNodeProperties ();

      const Interface::Grid * getGrid (void) const;
      double getDeltaI (void);
      double getDeltaJ (void);

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (int i, int j, int k);

      int getIndex (void);

      bool isOnBoundary (const FormationNode *) const;
      bool isShallowerThanNeighbour (const FormationNode *, const int, const int) const;

      SurfaceGridMap getTopSurfaceGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const GridMap * getTopGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const GridMap * getBaseGridMap (const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      MigrationFormation * getTopFormation () const;
      MigrationFormation * getBottomFormation () const;

      const GridMap* getFormationPrimaryPropertyGridMap (const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot) const;

      const GridMap* getSurfacePropertyGridMap (const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot,
                                                const Interface::Surface* surface) const;

      bool isActive (const DataAccess::Interface::Snapshot * snapshot) const;

      bool computeAnalogFlowDirections (MigrationFormation * targetFormation, const Interface::Snapshot * begin,
                                        const Interface::Snapshot * end);
      bool computeAnalogFlowDirections (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end);

      bool computeAdjacentNodes (MigrationFormation * targetFormation, const Interface::Snapshot * begin, const Interface::Snapshot * end);
      bool computeAdjacentNodes (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end);

      void migrateExpelledChargesToReservoir (unsigned int direction, MigrationReservoir * targetReservoir) const;
      void migrateLeakedChargesToReservoir (MigrationReservoir * targetReservoir) const;

      /// @brief Calculates seeps due to leakage from the top reservoir
      bool calculateLeakageSeeps (const Interface::Snapshot * end, const bool advancedMigration);
      /// @ brief Calculates seeps due to expulsion from all source rocks above the top reservoir
      bool calculateExpulsionSeeps (const Interface::Snapshot * end, const double expulsionFraction, const bool advancedMigration);

      /// @brief Returns whether the node is a 'crest' at a layer other than formation
      bool isValidNodeBelowFormation (FormationNode * formationNode, const DataAccess::Interface::Formation * formation) const;

      /// @brief Calculate expelled mass of HCs stuck in undetected crest
      void calculateStuckHCs (const double expulsionFraction, const unsigned int i, const unsigned int j, double& stuckHCMass) const;

      double getExpelledMass (int i, int j, int componentId) const;

      void manipulateFormationNodeComposition (FormationNodeCompositionRequest & compositionRequest);
      void getFormationNodeComposition (FormationNodeCompositionRequest & compositionRequest, FormationNodeCompositionRequest & compositionResponse);

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

      inline bool performAdvancedMigration (void) const;
      inline bool performHDynamicAndCapillary (void) const;
      inline bool performReservoirDetection (void) const;
      inline double getBlockingPermeability (void);
      inline double getBlockingPorosity (void);

      AbstractDerivedProperties::FormationPropertyPtr getFormationPropertyPtr (const string & propertyName, const Interface::Snapshot * snapshot) const;

      // Sets all top nodes of the given formation as ends of path due to the formation being a detected reservoir
      void setEndOfPath (void);

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

      // MigrationFormation property pointer using the derived properties library
      AbstractDerivedProperties::FormationPropertyPtr m_formationPropertyPtr[NUMBEROFPROPERTYINDICES];

      GridMap * m_expulsionGridMaps[ComponentId::NUMBER_OF_SPECIES];
      int m_index;

      // is a detected reservoir formation
      bool m_detectedReservoir;
   };

   Migrator * MigrationFormation::getMigrator (void) const
   {
      return m_migrator;
   }

   MigrationFormation * MigrationFormation::CastToFormation (const Interface::Formation * formation)
   {
      return dynamic_cast <MigrationFormation *> ((Interface::Formation *) (formation));
   }

   inline bool MigrationFormation::isPreprocessed () const
   {
      return m_isInitialised;
   }

   double MigrationFormation::getDepth (int i, int j, int k) const
   {
      return getPropertyValue (DEPTHPROPERTY, i, j, k);
   }

   double MigrationFormation::getPorosity (int i, int j, int k)
   {
      return getPropertyValue (POROSITYPROPERTY, i, j, k);
   }

   double MigrationFormation::getHorizontalPermeability (int i, int j, int k)
   {
      return getPropertyValue (HORIZONTALPERMEABILITYPROPERTY, i, j, k);
   }

   double MigrationFormation::getVerticalPermeability (int i, int j, int k)
   {
      return getPropertyValue (VERTICALPERMEABILITYPROPERTY, i, j, k);
   }

   double MigrationFormation::getCapillaryEntryPressureVapour (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYENTRYPRESSUREVAPOURPROPERTY, i, j, k);
   }

   double MigrationFormation::getCapillaryEntryPressureLiquid (int i, int j, int k)
   {
      return getPropertyValue (CAPILLARYENTRYPRESSURELIQUIDPROPERTY, i, j, k);
   }

   double MigrationFormation::getVapourDensity (int i, int j, int k)
   {
      return getPropertyValue (VAPOURDENSITYPROPERTY, i, j, k);
   }

   double MigrationFormation::getLiquidDensity (int i, int j, int k)
   {
      return getPropertyValue (LIQUIDDENSITYPROPERTY, i, j, k);
   }

   double MigrationFormation::getPressure (int i, int j, int k)
   {
      return getPropertyValue (PRESSUREPROPERTY, i, j, k);
   }

   double MigrationFormation::getTemperature (int i, int j, int k)
   {
      return getPropertyValue (TEMPERATUREPROPERTY, i, j, k);
   }

   bool MigrationFormation::performAdvancedMigration (void) const
   {
      return m_migrator->performAdvancedMigration ();
   }

   bool MigrationFormation::performHDynamicAndCapillary (void) const
   {
      return m_migrator->performHDynamicAndCapillary ();
   }

   bool MigrationFormation::performReservoirDetection (void) const
   {
      return m_migrator->performReservoirDetection ();
   }

   double MigrationFormation::getBlockingPermeability (void)
   {
      return m_migrator->getBlockingPermeability ();
   }

   double MigrationFormation::getBlockingPorosity (void)
   {
      return m_migrator->getBlockingPorosity ();
   }

}

#endif // MIGRATION_FORMATION_H
