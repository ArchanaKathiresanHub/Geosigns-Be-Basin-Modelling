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
   void initialize (void);
   virtual bool isSpillPoint (Column * column);
   bool isInInterior (Column * column) const;
   bool isOnPerimeter (Column * column) const;
   void closePerimeter (Column * column);
   void addToPerimeter (Column * column);
   void removeFromPerimeter (Column * column);
   void addToInterior (Column * column);

   void addColumnsToBeAdded (void);
   void addToToBeAdded (int i, int j);

   bool contains (Column * column) const;

   void printPerimeter (void);
   void printInterior (void);

   void printInconsistentVolumes (void);

   Reservoir * getReservoir (void);

   LocalColumn * getCrestColumn (void) const;
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

	bool isUndersized (void) const;
	double getTrapCapacity (void) const;

   double getWeight (void) const;
   double getWeight (PhaseId phase) const;
   double getWeight (ComponentId component) const;
   double getWeight (PhaseId phase, ComponentId component) const;

   double getWeightToBeDistributed (void) const;
   double getWeightToBeDistributed (PhaseId phase) const;

   double getVolume (PhaseId phase) const;
   double getVolumeByColumns (PhaseId phase) const;

   double getVolumeBetweenDepths (double upperDepth, double lowerDepth) const;
   double getVolumeBetweenDepths2 (double upperDepth, double lowerDepth) const;

   void computeDepthToVolumeFunction (void);
   void computeVolumeToDepthFunction (void);
   void computeVolumeToDepthFunction2 (void);

   void deleteDepthToVolumeFunction (void);
	 
   double getDepthForVolume (double volume);

   void setLocalId (int id);
   int getLocalId (void);

   void setGlobalId (int id);
   int getGlobalId (void);

   void setDrainageAreaId (int id);
   int getDrainageAreaId (void);

   void collectAndSplitCharges (bool always = false);

   void decomposeCharges (void);

   void checkDistributedCharges (PhaseId phase);

   /*!
   * \brief Compute the fraction of the volume in the trap which is impacted by biodegradation
   * This volume impacted by biodegradation is exclusively in a determined thickness above the hydrocarbon - water contact
   * This thickness is determined thanks to a coefficient (ex: 3m/10Myr) and the \param timeInterval
   * \return The fraction of volume in the trap impacted by biodegradation (from 0 to 1)
   */
   double computeFractionVolumeBiodegraded(const double& timeInterval);

   /*!
   * \brief Compute the temperature at the hydrocarbon - water contact (in °C)
   * \details This function makes an interpolation between the top and the bottom temperature of a column
   * in order to find the temperature around the hydrocarbon - water contact
   * \return The interpolated temperature at the hydrocarbon - water contact (in °C)
   */
   double computeHydrocarbonWaterContactTemperature();

   double biodegradeCharges (const double& timeInterval, const Biodegrade& biodegrade);
   double biodegradeCharges (const double& timeInterval, const Biodegrade& biodegrade, PhaseId phase);

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

/* 	 void spill (PhaseId phase); */
/*          void leak (PhaseId phase); */
/* 	 void waste (PhaseId phase); */

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
