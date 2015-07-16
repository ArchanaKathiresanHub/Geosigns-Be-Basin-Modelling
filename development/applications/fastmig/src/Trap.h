#ifndef _MIGRATION_TRAP_H_
#define _MIGRATION_TRAP_H_

#include "migration.h"
#include "Composition.h"
#include "Biodegrade.h"
#include "Interface/DiffusionLeakageParameters.h"
#include "SurfaceGridMapContainer.h"
#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Interface/LithoType.h"
#include "Interface/Formation.h"
#include "CBMGenerics/src/capillarySealStrength.h"
#include "Distributor.h"
#include "DiffusionOverburdenProperties.h"
#include "translateProps.h"
#include "MassBalance.h"

using Interface::LithoType;
using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;


namespace migration
{

   class Interpolator;


   using namespace DataAccess;

   typedef vector<Column *> ColumnVector;
   typedef vector<Column *>::iterator ColumnIterator;
   typedef vector<Column *>::const_iterator ConstColumnIterator;
   typedef vector<Column *>::reverse_iterator ColumnReverseIterator;
   typedef vector<Column *>::const_reverse_iterator ConstColumnReverseIterator;
   
   typedef pair<int, int> IntPair;
   typedef vector<IntPair> IntPairVector;

   class TrapPropertiesRequest;
   class OilToGasCracker;

   class Trap
   {
      public:
   /// Constructor
   Trap (LocalColumn * column);

   /// Destructor
   virtual ~Trap (void);

   int getSize (void);

   void computeArea (void);

   void becomeObsolete (void);
   void beAbsorbed (void);

   void extendWith (Column * column, double minimumSpillDepth);
   void completeExtension (void);

   void migrateTo (Column * column);

#ifdef COMPUTECAPACITY
	void computeCapacity (void);
#endif

   /*!
   * \brief Initialize the filling depths for each phase (liquid and vapor).
   * \details The filling depth for each phase is initialy set to the top depth of the crest column.
   */
   void initialize (void);

   virtual bool isSpillPoint (Column * column);

   /*!
   * \brief Boolean which informed if the \param column belong to the interior of the trap or not.
   * \details All column "inside" a trap belong to the interior column vector.
   * For information, the interior column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the interior column vector is the crest column.
   * \return True if the \param column belong to the interior of the trap, false otherwise
   */
   bool isInInterior (Column * column) const;

   /*!
   * \brief Boolean which informed if the \param column belong to the perimeter of the trap or not.
   * \details All column "at the border" (perimeter) of the trap belong to the perimeter column vector.
   * For information, the perimeter column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the perimeter is the spill column. The sealing column(s) are at the end of the vector.
   * \return True if the \param column belong to the perimeter of the trap, false otherwise
   */
   bool isOnPerimeter (Column * column) const;

   /*!
   * \brief Close the perimeter of a trap around a \param column
   * \details After including a new \param column of the perimeter to the interior of a trap, the perimeter of the trap have now an opening.
   * That's why we need to close the perimeter of the trap around this new interior \column. 
   * In order to do this, each one of the 8 neighbours of the \column will be evaluated. If this column doesn't belong to the interior of a trap, this column will be included in the perimeter.
   * If this column is not valid (you are for instance on the border of the model), nothing happen.
   */
   void closePerimeter (Column * column);

   /*!
   * \brief Add the \param column the the interior column vector.
   * \details All column "at the border" (perimeter) of the trap belong to the perimeter column vector.
   * For information, the perimeter column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the perimeter is the spill column. The sealing column(s) are at the end of the vector.
   */
   void addToPerimeter (Column * column);
   void removeFromPerimeter (Column * column);

   /*!
   * \brief Add the \param column the the interior column vector.
   * \details All column "inside" a trap belong to the interior column vector.
   * For information, the interior column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the interior column vector is the crest column.
   */
   void addToInterior (Column * column);

   void addColumnsToBeAdded (void);
   void addToToBeAdded (int i, int j);

   bool contains (Column * column) const;

   /*!
   * \brief Print the columns belonging to the perimeter of the trap.
   * \details All columns "at the border" (perimeter) of the trap belong to the perimeter column vector.
   * For information, the perimeter column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the perimeter is the spill column. The sealing column(s) are at the end of the vector.
   */
   void printPerimeter (void);

   /*!
   * \brief Print the columns belonging to the interoir of the trap.
   * \details All columns "inside" a trap belong to the interior column vector.
   * For information, the interior column vector is sorted such that shallowest columns are at the beginning, and deepest at the end.
   * So, the first column of the interior column vector is the crest column.
   */
   void printInterior (void);

   void printInconsistentVolumes (void);

   /*!
   * \brief Give the reservoir to which belong the trap.
   * \return The reservoir to which belong the trap.
   */
   Reservoir * getReservoir (void);

   LocalColumn * getCrestColumn (void) const;

   /*!
   * \brief Give the Spilling column.
   * \details The spilling column is stored as the first element of the perimeter column vector.
   * \return The Spilling column of the trap.
   */
   Column * getSpillColumn (void) const;
   Column * getColumnToSpillTo (void);

   double getSpillDepth (void);

   void computeSpillTarget (void);
   Column * getSpillTarget (void);

   void computeWasteColumns (void);
   Column * getWasteColumn (PhaseId phase);
   double getWasteDepth (PhaseId phase);

	Column * getFinalSpillTarget (PhaseId phase);
   void printSpillTrajectory (PhaseId phase);

   /*!
   * \brief Give the global "I" position (m_iGlobal) of the crest column of the trap
   * \return The global "I" position (m_iGlobal) of the crest column of the trap
   */
   unsigned int getI (void);
   unsigned int getJ (void);

   void setSealPermeability (double permeability);
   void setFracturePressure (double pressure);

   /*!
   * \brief Get the top depth of the Trap, the top depth of the crest column
   * \return The crest depth of the trap
   */
   double getTopDepth (void) const;

   /*!
   * \brief Get the bottom depth of the Trap, the spilling point depth
   * \return The spilling depth of the trap
   */
   double getBottomDepth (void) const;

   /*!
   * \brief Get the temperature at the top of the Trap, at the top of the crest column
   * \return The temperature at the top of the crest column of the trap (in °C)
   */
   double getTemperature (void) const;
   double getPreviousTemperature (void) const;
   double getPermeability (void) const;
   double getSealPermeability (void) const;
   double getFracturePressure (void) const;
   double getPressure (void) const;
   double getPreviousPressure (void) const;
   double getHydrostaticPressure (void) const;
   double getLithostaticPressure (void) const;
   double getNetToGross (void) const;

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume() const;

   double getCapacity (void) const;

   void setSpilling (void);
   bool isSpilling (void) const;
   void resetSpilling (void);

   /*!
   * \brief Give the info if the trap is undersized or not.
   * \details If a trap is undersized, it means that its capacity is smaller than the minimum trap capacity.
   * If the capacity is smaller than the minimum trap capacity, this boolean will enable the merging of this trap with other ones.
   * \return True if the trap is undersized (Trap capacity < minimum trap capacity), else false.
   */
	bool isUndersized (void) const;

   /*!
   * \brief Get the minimum trap capacity" value in the [ReservoirIoTbl] of " .project3d".
   * \details The obtained value is the minimum trap capacity allowed. This parameter helps to define if a trap is undersized or not.
   * If a trap is undersized, it can be merged with others in further steps.
   * \return The minimum trap capacity (in m3)
   */
   double getMinimumTrapCapacity(void) const;

   double getWeight (void) const;
   double getWeight (PhaseId phase) const;
   double getWeight (ComponentId component) const;
   double getWeight (PhaseId phase, ComponentId component) const;

   double getWeightToBeDistributed (void) const;
   double getWeightToBeDistributed (PhaseId phase) const;

   /*!
   * \brief Give the volume occupied by a \param phase inside the trap
   * \details The volume is obtained by dividing the weight by the density
   * \return The the volume occupied by a \param phase inside the trap (in m3)
   */
   double getVolume (PhaseId phase) const;
   double getVolumeByColumns (PhaseId phase) const;

   double getVolumeBetweenDepths (double upperDepth, double lowerDepth) const;
   double getVolumeBetweenDepths2 (double upperDepth, double lowerDepth) const;

   void computeDepthToVolumeFunction (void);
   void computeVolumeToDepthFunction (void);
   void computeVolumeToDepthFunction2 (void);

   //void deleteDepthToVolumeFunction (void);
	 
   double getDepthForVolume (double volume);

   void setLocalId (int id);
   int getLocalId (void);

   void setGlobalId (int id);
   int getGlobalId (void);

   void setDrainageAreaId (int id);
   int getDrainageAreaId (void);

   void collectAndSplitCharges (bool always = false);

   //void checkDistributedCharges (PhaseId phase);

   /*!
   * \brief Compute the thickness above the hydrocarbon - water contact affected by biodegradation
   * \details This thickness is determined thanks to a coefficient (ex: 3m/10Myr) and the \param timeInterval
   * \return The thickness above the hydrocarbon - water contact affected by biodegradation (in meters)
   */
   double computethicknessAffectedByBiodegradationAboveOWC(const double timeInterval) const;

   /*!
   * \brief Compute the fraction of the volume of the \param phase in the trap which is impacted by biodegradation
   * \details This volume impacted by biodegradation is exclusively in a determined thickness above the hydrocarbon - water contact
   * This thickness is determined thanks to a coefficient (ex: 3m/10Myr) and the \param timeInterval
   * \return The fraction of volume of the \param phase in the trap impacted by biodegradation (from 0 to 1)
   */
   double computeFractionVolumeBiodegraded(const double timeInterval, const PhaseId phase);

   /*!
   * \brief Compute the temperature at the hydrocarbon - water contact (in °C)
   * \details This function makes an interpolation between the top and the bottom temperature of a column
   * in order to find the temperature around the hydrocarbon - water contact
   * \return The interpolated temperature at the hydrocarbon - water contact (in °C)
   */
   double computeHydrocarbonWaterContactTemperature();

   void Trap::computePhaseVolumeProportionInBiodegradadedZone(const double timeInterval, double& VolumeProportionGas, double& VolumeProportionOil);

   double biodegradeCharges (const double& timeInterval, const Biodegrade& biodegrade);

   /// If depths contains a vector of formations starting with the formation containing 
   /// this trap, return iterators pointing to the formations which constitute the 
   /// overburden of this trap:
   void iterateToFirstOverburdenFormation(const vector<FormationSurfaceGridMaps>& depths, vector<FormationSurfaceGridMaps>::
      const_iterator& begin, vector<FormationSurfaceGridMaps>::const_iterator& end) const;

   // Methods used for diffusion leakage calculation:
   bool computeDiffusionOverburden(const SurfaceGridMapContainer& fullOverburden,
      const Interface::Snapshot* snapshot, const double& maxSealThickness, int maxFormations);
   bool computeSealFluidDensity(const SurfaceGridMapContainer& fullOverburden, 
      const Snapshot* snapshot, bool& sealPresent, double& sealFluidDensity) const;
   bool computeDiffusionOverburdenImpl(const SurfaceGridMapContainer& fullOverburden,
      const Snapshot* snapshot, const double& maxSealThickness, int maxFormations,
      vector<DiffusionLeak::OverburdenProp>& diffusionOverburdenProps) const;

	 void diffusionLeakCharges(const double& intervalStartTime, const double & intervalEndTime,
	       const Interface::DiffusionLeakageParameters* 
	       parameters, const double& maxTimeStep, const double& maxFluxError);

	 /// Methods used for seal failure calculations:
   bool computeDistributionParameters(const Interface::FracturePressureFunctionParameters* 
      parameters, const SurfaceGridMapContainer& fullOverburden, const Interface::Snapshot* snapshot);
   bool computeSealPressureLeakParametersImpl(const Interface::FracturePressureFunctionParameters* 
      fracturePressureParameters, const SurfaceGridMapContainer& fullOverburden, const Snapshot* snapshot,
      bool& sealPresent, double& fracPressure, double& sealFluidDensity, vector<translateProps::
      CreateCapillaryLithoProp::output>& lithProps, vector<double>& lithFracs, CBMGenerics::capillarySealStrength::
      MixModel& mixModel, double& permeability) const;
   bool computeForFunctionOfLithostaticPressure(const SurfaceGridMapContainer& fullOverburden,
      const Formation* formation, const vector<double>& lithFracs, double& fracPressure) const;

	 bool distributeCharges (void);

	 void incrementChargeDistributionCount (void);

	 void broadcastDiffusionStartTimes (void);
	 void broadcastPenetrationDistances (void);

	 void broadcastFillDepthProperties (void);
	 void broadcastChargeProperties (void);

	 bool isSpillingBack (void);

	 void moveBackToToBeDistributed (PhaseId lighterPhase);
	 void moveBackToCrestColumn (void);
	 void moveBackToCrestColumn (PhaseId phase);

	 void moveBackToBeMigrated (void);
	 void moveBackToBeMigrated (PhaseId phase);

	 bool requiresDistribution (void);
	 bool requiresPVT (void);
	 bool computePVT (void);

	 void resetExtended (void);
	 void setExtended (void);
	 bool hasBeenExtended (void);

	 void resetToBeAbsorbed (void);
	 void setToBeAbsorbed (void);
	 bool isToBeAbsorbed (void);

    /*!
    * \brief Compute if a trap is full or not for a precise \param phase.
    * \details The trap is full if the filling depth for this precise \param phase is deeper or equal than the bottom depth of this trap (the spillling point depth)
    * Remark: a full mixed-fill trap will appear false for gas (on top), and true for oil (on the bottom)
    * \return True if the trap is full for a precise \param phase, false otherwise
    */
	 bool isFull (PhaseId phase);

	 void setDiffusionStartTime (double diffusionStartTime);
	 double getDiffusionStartTime ();

	 void setPenetrationDistances (const double * penetrationDistances);
	 const double * getPenetrationDistances ();

	 void setPenetrationDistance (ComponentId c, double penetrationDistance);
	 double getPenetrationDistance (ComponentId c);

	 void setFillDepth (PhaseId phase, double fillDepth);
	 double getFillDepth (PhaseId phase);

   /*!
   * \brief Compute the Hydrocarbon - Water contact depth
   * \return The absolute depth of the hydrocarbon - water contact (from the surface)
   */
   double computeHydrocarbonWaterContactDepth(void) const;

   void setMinimumSpillDepth (double minimumSpillDepth);
   double getMinimumSpillDepth (void);

   double getDensity (PhaseId phase) const;
   double getSurface (PhaseId phase);

   void negotiateDensity (PhaseId phase);

   double getSealPressureLeakages(void) const;
   double getSealPressureLeakages(PhaseId phase) const;

   double getDiffusionLeakages(void) const;         

   void reportLeakage ();

   void collectProperties (TrapPropertiesRequest & tpRequest);
   bool saveProperties (void);

      protected:
   Reservoir * m_reservoir;

   ColumnVector m_perimeter;
   ColumnVector m_interior;

   IntPairVector m_toBeAdded;

   Column * m_spillTarget;

   Column * m_wasteColumns[NUM_PHASES];

   double m_wasteDepths[NUM_PHASES];

   Composition m_toBeDistributed[NUM_PHASES];
   Composition m_distributed[NUM_PHASES];
   Composition m_diffusionLeaked[NUM_PHASES];
   Composition m_sealPressureLeaked[NUM_PHASES];

   double m_diffusionStartTime;
   double m_penetrationDistances[DiffusionComponentSize];

   double m_fillDepth[NUM_PHASES];
#ifdef COMPUTECAPACITY
   double m_capacity;
#endif

#ifdef THISISNOTRIGHT
   double m_minimumSpillDepth;
#endif
   double m_sealPermeability;
   double m_fracturePressure;

   migration::Interpolator * m_volumeToDepth2;
   const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;
   Distributor* m_distributor;

   int m_id;
   int m_globalId;
   int m_drainageAreaId;

   bool m_spilling;
   bool m_extended;
   bool m_toBeAbsorbed;
   bool m_computedPVT;

   DiffusionOverburdenProperties* m_diffusionOverburdenProps;

#ifdef DETAILED_MASS_BALANCE
         ofstream m_massBalanceFile;
	 MassBalance<ofstream>* m_massBalance;
#endif

#ifdef DETAILED_MASS_BALANCE
         ofstream m_volumeBalanceFile;
	 MassBalance<ofstream>* m_volumeBalance;
#endif

   };

   ostream & operator<< (ostream & stream, Trap & trap);
   ostream & operator<< (ostream & stream, Trap * trap);
}

#endif // _MIGRATION_TRAP_H_
