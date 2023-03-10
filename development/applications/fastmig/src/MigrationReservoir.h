//
// Copyright (C) 2010-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_RESERVOIR_H_
#define _MIGRATION_RESERVOIR_H_


#include "RequestDefs.h"
#include "migration.h"
#include "Reservoir.h"
#include "SurfaceProperty.h"
#include "FormationProperty.h"
#include "SurfaceGridMapContainer.h"

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
      class MigrationFormation;
      class Snapshot;
      class GridMap;
      class Grid;
   }
}

using namespace DataAccess;

/// Reservoir Class
namespace migration
{
   class OilToGasCracker;

   typedef bool (*ConditionTest)(Column * columnA, Column * columnB);

   /// This class implements the Reservoir-specific migration functionality.
   /// It also implements the inter-process communication functionality as this is reservoir-scoped.
   /// It is constructed on top of the DataAccess::Reservoir class.
   class MigrationReservoir : public Interface::Reservoir
   {
   public:
      /// This constructor is called by the object factory
      MigrationReservoir (Interface::ProjectHandle& projectHandle, Migrator * const migrator, database::Record * record);

      /// Destructor
      virtual ~MigrationReservoir (void);

      /// @defgroup ReservoirProperties
      /// @{
      /// @brief Retain properties calculated during the previous snapshot interval that are required during the current snapshot interval.
      void retainPreviousProperties (void);
      /// @brief Reset all computed property values
      bool clearProperties (void);
      /// @brief Compute new input-based property values.
      bool computeProperties (void);
      /// @brief Reset properties from a previous timestep
      bool clearPreviousProperties (void);
      AbstractDerivedProperties::FormationPropertyPtr getVolumeProperty (const MigrationFormation * formation,
                                                                         const std::string & propertyName,
                                                                         const Interface::Snapshot * snapshot) const;
      AbstractDerivedProperties::FormationPropertyPtr getFormationPropertyPtr (const std::string & propertyName,
                                                                               const Interface::Snapshot * snapshot) const;
      /// @brief Computes the depth offsets
      bool computeDepthOffsets(const Snapshot* presentDay) const;
      /// @brief Computes net to gross according to input (legacy only)
      bool computeNetToGross (void) const;
      /// @return Boolean whether a reservoir is active at a given snapshot
      bool isActive (const Interface::Snapshot * snapshot) const;
      /// @brief Save properties that were derived from input maps during the current snapshot interval
      bool saveComputedInputProperties (const bool saveSnapshot);
      /// @brief Save properties the migration module computed during the current snapshot interval
      bool saveComputedOutputProperties (const bool saveSnapshot);
      /// @}


      /// @defgroup ReservoirOptions
      /// @{
      /// @return Boolean whether diffusion takes place inside traps
      bool isDiffusionEnabled (void) const;
      /// @return Boolean whether biodegradation takes place inside traps
      bool isBiodegradationEnabled (void) const;
      /// @return Boolean whether OTGC takes place inside traps
      bool isOilToGasCrackingEnabled (void) const;
      /// @return Boolean whether vertical migration blocking into this reservoir is enabled
      bool isBlockingEnabled (void) const;
      /// @return The vertical migration blocking permeability threshold.
      double getBlockingPerm (void) const;
      /// @return The vertical migration blocking porosity threshold.
      double getBlockingPoro (void) const;
      /// @return The minimum capacity for the traps of this Reservoir
      double getMinTrapCapacity (void) const;
      /// @}



      /// @defgroup ReservoirCharge
      /// @{
      double getTotalToBeStoredCharges (bool onBoundaryOnly = false);
      double getTotalChargesToBeMigrated (void);
      double getTotalBiodegradedCharges (void);
      double getTotalDiffusionLeakedCharges (void);
      double getTotalToBeDistributedCharges (void);
      double getTotalStoredCharges (void);
      void addBlocked (ComponentId componentId, double mass);
      void addBlocked (const Composition & composition);
      void subtractBlocked (ComponentId componentId, double mass);
      void subtractBlocked (const Composition & composition);
      const Composition & getBlocked (void) const;
      double getTotalBlocked (void) const;
      void addToCrackingLoss (const Composition & composition);
      void addToCrackingGain (const Composition & composition);
      const Composition & getCrackingLoss (void) const;
      const Composition & getCrackingGain (void) const;
      double getTotalCrackingLoss (void) const;
      double getTotalCrackingGain (void) const;
#ifdef USEOTGC
      double getTotalImmobiles (void);
#endif
      bool computeFluxes (void);
      bool collectLeakedCharges (MigrationReservoir * leakingReservoir, Barrier * barrier);
      bool collectExpelledCharges (const MigrationFormation * formation, unsigned int direction, Barrier * barrier);
      void deleteExpelledChargeMaps (const MigrationFormation * formation);
      bool migrateChargesToBeMigrated (const MigrationFormation * srcFormation, const MigrationReservoir * srcReservoir);
      void reportLateralMigration (int sourceTrapId, Column * targetColumn, const Composition & composition);
      void reportVerticalMigration (Column * sourceColumn, Column * targetColumn, const Composition & composition);
      void reportTrapAbsorption (Trap * sourceTrap, Column * targetColumn, const Composition & composition);
      void reportWaste (Trap * sourceTrap, Column * targetColumn, const Composition & composition);
      void reportSpill (Trap * sourceTrap, Column * targetColumn, const Composition & composition);
      void reportBiodegradationLoss (Trap * sourceTrap, const Composition & composition);
      void reportCrackingLoss (int trapId, const Composition & composition);
      void reportCrackingGain (int trapId, const Composition & composition);
      void reportDiffusionLoss (Trap * sourceTrap, const Composition & composition);
      bool processMigration (MigrationRequest & mr);
      bool processAbsorption (MigrationRequest & mr);
      /// @}


      /// @defgroup ColumnHandling
      /// @{
      /// @brief Refine geometry, to take into account zero thicknesses
      bool refineGeometryZeroThicknessAreas (void);
      bool refineGeometrySetFaulStatus (void);
      bool resetProxiesBeforeRefine (void);
      double getMassStoredInColumns (void);
      void wasteNonReservoirColumns (const Snapshot * snapshot);

      /// @brief Whether a column has any undefined neighbours
      ///
      /// If a column is on boundary, then adjacentColumn = nullptr
      bool isOnBoundary (const Column* column) const;

      /// \brief Returns a non-sealing away from the immediate perimeter
      ///
      /// Looks at progressively further perimeters of neighbours starting
      /// at distances of 2 columns (immediate neighbours have already
      /// been checked by getAdjacentColumn()
      /// \param HC phase, starting column, trap (if it exists)
      /// \return the suitable column (if it exists)
      Column * avoidSealingColumn (const PhaseId phase, const Column * column, const Trap * trap = 0);
      Column * getAdjacentColumn (PhaseId phase, Column * column, Trap * trap = 0);
      LocalColumn * getLocalColumn (unsigned int i, unsigned int j) const;
      Column * getColumn (unsigned int i, unsigned int j) const;
      /// transfer the calculated seepage amounts from nodes to columns
      void putSeepsInColumns (const MigrationFormation * seepsFormation);
      /// save Seepage amounts at the top formation of the basin
      bool saveSeepageProperties (const Interface::Snapshot * end);
      /// @}

        /// @defgroup TrapHandling Trap Handling 
        /// @brief Takes care of the trap construction and trap merging process.
        /// 
        /// First trap construction takes place in 5 steps. To find more details about trap construction process go to computeTraps().
        /// 
        /// Two traps merge if the following two conditions are satisfied:
        ///     -# If any one of the traps is undersized. 
        ///     -# If the two traps spill into each other.
        /// 
        /// The trap merging takes place in mergeSpillingTraps() function.
        /// 
        /// @{

      /// @brief Move the traps computed during the previous snapshot interval to the list of previous traps.
      void retainPreviousTraps (void);
      /// @brief Remove the traps in the list of previous traps.
      void removePreviousTraps (void);
      /// @brief Remove the traps in the list of traps.
      void removeTraps (void);
      /// @brief Save trap-related scalar properties
      bool saveTrapProperties (const bool saveSnapshot);

      
        /// @brief Contructs the traps of this reservoir.
        /// 
        /// The trap construction takes places in the following 5 steps:
        /// -# Finding the trap tops i.e., finding the crest column and setting its 8 neighbor points. See computeTrapTops().
        /// -# Computing the total no of traps. See computeMaximumTrapCount().
        /// -# Computing the full extents of the identified traps. See computeTrapExtents().
        /// -# Initializing the attributes of the traps. See initializeTraps().
        /// -# Merging the undersized traps. Undersized traps are those whose capacity is smaller than a user-defined reservoir-wide threshold.
        bool computeTraps (void);

      void printInconsistentTrapVolumes (void);
      /// recompute the depth to volume functions if oil to gas cracking has been performed
      bool recomputeTrapDepthToVolumeFunctions (void);
      void processTrapProperties (TrapPropertiesRequest & tpRequest);
      bool crackChargesToBeMigrated (OilToGasCracker & otgc);

        /// @brief Perform the filling and spilling process.
        /// 
        /// This is where the HCs loss/exchange are accounted.
        /// Additionally, trap merging also happens in here.
        /// 
        /// In BPA2 engine, distribution happens first (i.e., calculate leakage, wasting and spillage),
        /// then biodegrade and then calculate diffusion losses. If something is biodegraded,
        /// or diffused, then a final re-distribution will be needed.
        /// 
        /// Also, go to collectAndSplitCharges(), distributeCharges(), mergeSpillingTraps() and biodegradeCharges() for more information.
        bool fillAndSpill (void);

      inline bool theDistributionCycle(bool &flashCharges);
      void incrementChargeDistributionCount (void);
      void reportChargeDistributionCount (void);
      void broadcastTrapDiffusionStartTimes (void);
      void broadcastTrapPenetrationDistances (void);
      void broadcastTrapChargeProperties (void);
      void broadcastTrapFillDepthProperties (void);
      void accumulateErrorInPVT (double error);
      double getErrorPVT (void);
      double getLossPVT (void);
      /// @}


      /// Migration Related

      void setSourceFormation (const MigrationFormation * formation);
      void setSourceReservoir (const MigrationReservoir * reservoir);
      const MigrationFormation * getSourceFormation (void);
      const MigrationReservoir * getSourceReservoir (void);
      inline void setStart (const DataAccess::Interface::Snapshot * start);
      inline void setEnd (const DataAccess::Interface::Snapshot * end);
      inline const Interface::Snapshot * getStart (void);
      inline const Interface::Snapshot * getEnd (void);
      /// save HCpathways along the reservoir seal
      bool saveComputedPathways (const Interface::Snapshot *) const;
      /// compute the migration paths of this reservoir
      bool computePathways (void);
      /// compute the migration target columns of all columns.
      bool computeTargetColumns (void);


      /// Parallelization

      void getValue (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse);
      void setValue (ColumnValueRequest & valueRequest);
      void getValue (ColumnValueArrayRequest & valueArrayRequest, ColumnValueArrayRequest & valueArrayResponse);
      void setValue (ColumnValueArrayRequest & valueArrayRequest);
      void clearProxyProperties (ColumnValueRequest & valueRequest);
      void manipulateColumn (ColumnColumnRequest & columnRequest);
      void manipulateColumnComposition (ColumnCompositionRequest & compositionRequest);
      void manipulateColumnCompositionPosition(ColumnCompositionPositionRequest & compositionPositionRequest);
      void getColumnComposition (ColumnCompositionRequest & compositionRequest, ColumnCompositionRequest & compositionResponse);
      void collectMigrationRequest (MigrationRequest & request);
      void processMigrationRequests ();
      MigrationRequest * findMigrationRequest (MigrationRequest & request);


      /// Grid Related

      /// Gets the grid
      const Interface::Grid * getGrid (void) const;
      /// Surface of a column
      double getSurface (unsigned int i, unsigned int j) const;


      /// Miscellaneous

      /// get index of the reservoir in a list of reservoirs
      int getIndex (void);
      double getUndefinedValue (void);
      bool saveGenexMaps (const std::string & speciesName, DataAccess::Interface::GridMap * aMap, const MigrationFormation * formation, const Snapshot * aSnapshot);
      const Migrator * getMigrator (void) const;


   private:

      /// Reservoir Properties

      /// compute the top and bottom depths of the reservoir
      bool computeDepths (void);
      /// compute the faults of the reservoir
      bool computeFaults (void);
      /// compute the overburdens of the reservoir
      bool computeOverburdens (void);
      /// compute the hydrostatic pressures as seabottom
      bool computeSeaBottomPressures (void);
      /// increase the overburdens of the reservoir in case it does not lie at the top of a formation
      bool adaptOverburdens (void);
      /// compute the porosities at the top of the reservoir
      bool computePorosities (void);
      /// compute the permeabilities:
      bool computePermeabilities (void);
      /// compute the temperatures at the top of the reservoir
      bool computeTemperatures (void);
      /// compute brine viscosity at the top of the reservoir
      bool computeViscosities (void);
      /// compute the pressures at the top of the reservoir
      bool computePressures (void);
      /// Sometimes we also need the hydrostatic pressure at the top of a trap:
      bool computeHydrostaticPressures (void);
      /// And the lithostatic pressures:
      bool computeLithostaticPressures (void);
      /// Load the necessary overburden properties used for diffusion:
      bool computeOverburdenGridMaps (void);
      /// save the property specified by valueSpec under the given name for the given phase (if applicable).
      bool saveComputedProperty (const std::string & name, ValueSpec valueSpec, PhaseId phase = NO_PHASE);
      bool computeFlux (PhaseId phase, unsigned int i, unsigned int j);
      inline bool hasName (const std::string & name);
      inline const std::string depthPropertyName () const;
      AbstractDerivedProperties::SurfacePropertyPtr getSeaBottomProperty (const std::string & propertyName, const Interface::Snapshot * snapshot) const;
      AbstractDerivedProperties::SurfacePropertyPtr getTopSurfaceProperty (const std::string & propertyName, const Interface::Snapshot * snapshot) const;
      AbstractDerivedProperties::SurfacePropertyPtr getBottomSurfaceProperty (const std::string & propertyName, const Interface::Snapshot * snapshot) const;
      AbstractDerivedProperties::FormationPropertyPtr getTopFormationProperty (const std::string & propertyName, const Interface::Snapshot * snapshot) const;
      AbstractDerivedProperties::FormationPropertyPtr getSeaBottomFormationProperty (const std::string & propertyName, const Interface::Snapshot * snapshot) const;
      const Interface::Formation* getSeaBottomFormation (const Interface::Snapshot * snapshot) const;
      const Interface::Formation* getTopFormation (const Interface::Snapshot * snapshot) const;
      const Interface::GridMap * getPropertyGridMap (const std::string & propertyName, const Interface::Snapshot * snapshot,
                                                     const Interface::Reservoir * reservoir, const Interface::Formation * formation,
                                                     const Interface::Surface * surface) const;


      /// Reservoir Charge

      bool distributionHasFinished (void);
      bool distributeCharges (const bool performAdvancedMigration, bool alwaysDistribute = false);
      /// <summary>
      /// This will put the loss in the reservoir of 
      /// </summary>
      /// <param name=""></param>
      void reportLeakages (void);
      bool computeDistributionParameters (void);
      double biodegradeCharges (void);

      // Pasteurization functions. They must be executed in the following order:
      bool computeHydrocarbonWaterContactDepth (void);
      bool computeHydrocarbonWaterTemperature (void);
      bool needToComputePasteurizationStatusFromScratch(void);
      bool pasteurizationStatus(void);
      bool setPasteurizationStatus(void);

      bool diffusionLeakCharges (void);
      bool addChargesToBeMigrated (ComponentId componentId, const DataAccess::Interface::GridMap * gridMap, double fraction, Barrier * barrier);
      bool addChargesToBeMigrated (const DataAccess::Interface::GridMap * gridMap, double fraction, Barrier * barrier);
      bool subtractChargesToBeMigrated (ComponentId componentId, const DataAccess::Interface::GridMap * gridMap, double fraction, Barrier * barrier);
      bool checkChargesToBeMigrated (ComponentId componentId);
      
        /// @addtogroup TrapHandling
        /// 
        /// @{
        /// 
        void collectAndSplitCharges(bool always = false);
        /// @}

      /// \brief Add the leakage before biodegradation/diffusion to the crest column once
      ///        re-distribution has finished. Care should be taken with merging traps.
      ///        Their initial leakage should be added to the crest column before deletion.
      void putInitialLeakageBack(void);


      /// Column Handling

      /// create the column grid of this reservoir.
      void createColumns (void);
      /// destroy the column grid of this reservoir.
      void destroyColumns (void);
      bool computeTargetColumn (PhaseId phase, unsigned int i, unsigned int j);
      Column * findNonSealingColumn (int kappa, const int n, const PhaseId phase, const Column * column, const Trap * trap);
      bool computeAdjacentColumn (PhaseId phase, unsigned int i, unsigned int j);
      ProxyColumn * getProxyColumn (unsigned int i, unsigned int j);
      double getNeighbourDistance (int neighbour);
      int getTotalNumberOfProxyColumns (void);
      int getNumberOfProxyColumns (void);


        /// @addtogroup TrapHandling
        /// 
        /// @{
        /// Trap

      /// Initialize the traps
      bool initializeTraps (void);

      void changeTrapPropertiesRequestId (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests, int oldId, int newId);
      /// collect properties of traps in tpRequests
      void collectTrapProperties (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests);
      /// renumber the traps in the MigrationIoTbl and fill the TrapIoTbl
      void populateMigrationTables (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests);
      void eliminateUndersizedTraps (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests);

        /// @brief Compute the columns for which the depth is at a local minimum and create a trap for them. 
        /// See computeTrapTop().
        bool computeTrapTops (void);

        /// @brief Computes the area covered by each trap.
        /// 
        ///  Loops over all the traps and for each trap the trap extension operation is carried out in 4 steps: 
        ///     -# Compute the trap area. See computeArea().
        ///     -# Convert the volume to depth. See computeDepthToVolumeFunction().
        ///     -# Compute the waste columns. See computeWasteColumns().
        ///     -# Find the new spill targets. See computeSpillTarget().
        bool computeTrapExtents(void);
      
        /// @brief If the column at the given indices is at a local depth minimum, create a trap for it.
        /// 
        /// If the column is:
        ///     -# Valid.
        ///     -# It is at a local depth minimum.
        ///     -# It is not wasting.
        ///     -# It is not sealing.
        ///     -# It is not already assigned to a trap.
        /// 
        /// Then its composition is reset and a new trap is constructed and a trap id is assigned.
        /// The column is set as interior column and its 8 neighbor is set as its perimeter.
        /// 
        /// @param HC phase, i-index of a column, j-index of a column
        void computeTrapTop(PhaseId phase, unsigned int i, unsigned int j);

      Trap * findTrap (int globalId);
      void addTrap (Trap * trap);

        /// @brief It merges the traps that are spilling to each other.
        /// 
        /// The trap merging takes place in three steps:
        ///     -# Determine the traps that need to be merged.
        ///     -# Transfer the interiors of the traps to be absorbed to the absorbing traps.
        ///     -# Re-compute traps extents.
        /// 
        /// When two traps are merged, their interiors are merged and ordered, their perimeters are merged and ordered, 
        /// and trap construction proceeds as described in computeTrapExtents().
      void mergeSpillingTraps (void);
#ifdef MERGEUNDERSIZEDTRAPSAPRIORI
      bool mergeUndersizedTraps (void);
#endif
        /// @brief Determine whether and identify traps that need to merge
        /// 
        /// @return Returns true if no traps needed to merge.
      bool determineTrapsToMerge (ConditionTest conditionTest);
      void absorbTraps (void);
      void removeUndersizedTraps (void);
      void completeTrapExtensions (void);

        /// @brief Loops over all the traps and counts the total no of traps 
        /// 
        /// If the parameter passed to this function is false, during the counting it excludes all the undersized traps.
        /// Otherwise, it returns the total number of traps in the reservoir.
        /// 
        /// @param A booleam argument. By Default it is set as true.
        /// @return An interger value containing the total number of traps.
        int computeMaximumTrapCount(bool countUndersized = true);
      
      int numberOfAllTraps (bool countUndersized = false);
      inline unsigned int getMaximumTrapCount (void);
      /// @}


      /// Miscellaneous

      inline bool lowResEqualsHighRes (void) const;
      bool allProcessorsFinished (bool finished);
      bool checkDistribution (void);


      /// Data Members

   private:

      // Pointer to the migrator object
      Migrator * const m_migrator;

      /// The Traps we work on
      TrapVector m_traps;

      // cached distances
      double m_neighbourDistances[8];

      /// the maximum number of traps at any processor
      unsigned int m_maximumTrapCount;

      /// the number of times distributeCharges was performed on a trap
      long m_chargeDistributionCount;

      /// The traps created during the previous interval
      TrapVector m_previousTraps;

      /// The columns of the reservoir
      ColumnArray * m_columnArray;

      /// start and end of the migration period.
      const DataAccess::Interface::Snapshot * m_start;
      const DataAccess::Interface::Snapshot * m_end;

      /// used in computation of the MigrationIoTbl
      const MigrationFormation * m_sourceFormation;
      const MigrationReservoir * m_sourceReservoir;

      Composition m_compositionBlocked;

      Composition m_crackingLoss;
      Composition m_crackingGain;

      double m_lossPVT;
      double m_errorPVT;

      double m_biodegraded;

      bool m_lowResEqualsHighRes;

      double m_undefinedValue;

      SurfaceGridMapContainer m_diffusionOverburdenGridMaps;
      SurfaceGridMapContainer m_sealPressureLeakageGridMaps;

      MigrationRequestVector m_migrationRequests;

      bool m_computeFluxesHasFinished;

      int m_index;
   };
}


/// Inline Functions

bool migration::MigrationReservoir::lowResEqualsHighRes (void) const
{
   return m_lowResEqualsHighRes;
}

bool migration::MigrationReservoir::hasName (const std::string & name)
{
   return getName () == name;
}

const std::string migration::MigrationReservoir::depthPropertyName () const
{
   if (m_lowResEqualsHighRes) return "Depth";
   else return "DepthHighRes";
}

unsigned int migration::MigrationReservoir::getMaximumTrapCount (void)
{
   return m_maximumTrapCount;
}

void migration::MigrationReservoir::setStart (const DataAccess::Interface::Snapshot * start)
{
   m_start = start;
}

void migration::MigrationReservoir::setEnd (const DataAccess::Interface::Snapshot * end)
{
   m_end = end;
}

const DataAccess::Interface::Snapshot * migration::MigrationReservoir::getStart (void)
{
   return m_start;
}

const DataAccess::Interface::Snapshot * migration::MigrationReservoir::getEnd (void)
{
   return m_end;
}

#endif // _MIGRATION_RESERVOIR_H_
