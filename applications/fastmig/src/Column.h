#ifndef _MIGRATION_COLUMN_H_
#define _MIGRATION_COLUMN_H_

#include "RequestDefs.h"

#include "BitField.h"
#include "Composition.h"
#ifdef USEOTGC
#include "Immobiles.h"
#endif


#include "migration.h"

#include <vector>

namespace migration
{
#ifdef USEOTGC
   class OilToGasCracker;
#endif

   /// Class containing grid point specific information for a reservoir.
   /// Mainly used as a data container.
   class Column
   {
      public:
	 /// Constructor
	 Column (unsigned int i, unsigned int j, Reservoir * reservoir);

	 /// Destructor
	 virtual ~Column (void);

	 virtual void clearProperties (void);
	 virtual void clearPreviousProperties (void);

	 inline unsigned int getI (void) const;
	 inline unsigned int getJ (void) const;
	 inline Reservoir * getReservoir (void);
	 inline const Reservoir * reservoir (void) const;

	 bool isValid (void) const;

	 virtual Column * getFinalTargetColumn (PhaseId phase);

	 virtual int getDrainageAreaId (void) = 0;

	 virtual int getGlobalTrapId (void) = 0;
	 virtual void setGlobalTrapId (int trapId) = 0;

	 virtual void setTopDepth (double depth) = 0;
	 virtual double getTopDepth (void) const = 0;
	 virtual void setBottomDepth (double depth) = 0;
	 virtual double getBottomDepth (void) const = 0;
	 virtual double getThickness (void);
	 virtual double getCapacity (double spillDepth = 1e8);

	 virtual double getFillHeight (PhaseId phase);

	 virtual double getNetToGross (void) const = 0;
	 virtual double getPorosity (void) const = 0;

#ifdef USEOTGC
	 virtual double getImmobilesVolume (void) const = 0;
#endif

	 virtual FaultStatus getFaultStatus (void) = 0;

         virtual bool isSpilling (void) = 0;
         virtual bool isUndersized (void) = 0;

         virtual bool isSealing (PhaseId phase) = 0;
         virtual bool isWasting (PhaseId phase) = 0;

         bool isSealing (void);
         bool isWasting (void);

	 virtual double getDiffusionStartTime () = 0;
	 virtual void setDiffusionStartTime (double diffusionStartTime) = 0;

	 virtual void setPenetrationDistances (const double * penetrationDistances) = 0;
	 virtual const double * getPenetrationDistances () = 0;

	 virtual double getFillDepth (PhaseId phase) = 0;
	 virtual void setFillDepth (PhaseId phase, double fillDepth) = 0;

	 virtual void setChargeDensity (PhaseId phase, double chargeDensity) = 0;

	 virtual void setChargesToBeMigrated (PhaseId phase, Composition & composition) = 0;

	 virtual void addMigrated (PhaseId phase, double weight) = 0;
	 virtual void addFlux (double weight) = 0;

	 virtual double getSurface (void) const;
	 virtual double getVolumeBetweenDepths (double upperDepth, double lowerDepth) const;

	 virtual int compareDepths (Column * column, bool useTieBreaker = true);
	 inline bool isDeeperThan (Column * column, bool useTieBreaker = true);
	 inline bool isShallowerThan (Column * column, bool useTieBreaker = true);


	 virtual Column * getAdjacentColumn (PhaseId phase) = 0;

	 virtual Column * getTargetColumn (PhaseId phase) = 0;
	 virtual Column * getSpillTarget (void) = 0;

	 virtual bool computeTargetColumn (PhaseId phase) = 0;

	 virtual Column * getFinalSpillTarget (PhaseId phase);

	 virtual Column * getTrapSpillColumn (void) = 0;

	 virtual void addComposition (Composition & composition) = 0;
	 virtual void addLeakComposition (Composition & composition) = 0;
	 virtual void addWasteComposition (Composition & composition) = 0;
	 virtual void addSpillComposition (Composition & composition) = 0;

	 virtual void addToYourTrap (unsigned int i, unsigned int j) = 0;

	 virtual bool isTrue (unsigned int bitSpec);
	 virtual void setTrue (unsigned int bitSpec);
	 virtual void setFalse (unsigned int bitSpec);
	 virtual void setBit (unsigned int bitSpec, bool state);

      protected:
	 Reservoir * m_reservoir;

         int m_iGlobal;
	 int m_jGlobal;

	 BitField m_bitField;
   };

   bool IsValid (const Column * column);

   /// Proxy for a column that sits on another processor.
   /// Takes care of all the data retrieval and caching.
   class ProxyColumn : public Column
   {
      public:
	 /// Constructor
	 ProxyColumn (unsigned int i, unsigned int j, Reservoir * reservoir);

	 /// Destructor
	 virtual ~ProxyColumn (void);

	 virtual void clearProperties (void);
	 virtual void clearPreviousProperties (void);

	 virtual int getDrainageAreaId (void);

	 virtual int getGlobalTrapId (void);
	 virtual void setGlobalTrapId (int trapId);

	 virtual void setTopDepth (double depth);
	 virtual double getTopDepth (void) const;
	 virtual void setBottomDepth (double depth);
	 virtual double getBottomDepth (void) const;
	 virtual double getNetToGross (void) const;
	 virtual double getPorosity (void) const;

#ifdef USEOTGC
	 virtual double getImmobilesVolume (void) const;
#endif

	 virtual FaultStatus getFaultStatus (void);

         virtual bool isSpilling (void);
         virtual bool isUndersized (void);

         virtual bool isSealing (PhaseId phase);
         virtual bool isWasting (PhaseId phase);

	 virtual double getDiffusionStartTime ();
	 virtual void setDiffusionStartTime (double diffusionStartTime);

	 virtual void setPenetrationDistances (const double * penetrationDistances);
	 virtual const double * getPenetrationDistances ();

	 virtual double getFillDepth (PhaseId phase);
	 virtual void setFillDepth (PhaseId phase, double fillDepth);

	 virtual void setChargeDensity (PhaseId phase, double chargeDensity);

	 virtual void setChargesToBeMigrated (PhaseId phase, Composition & composition);

	 virtual void addMigrated (PhaseId phase, double weight);
	 virtual void addFlux (double weight);

	 virtual Column * getAdjacentColumn (PhaseId phase);

	 virtual Column * getTargetColumn (PhaseId phase);
	 virtual Column * getSpillTarget (void);

	 virtual bool computeTargetColumn (PhaseId phase);

	 virtual Column * getTrapSpillColumn (void);

	 void registerWithLocal (void);
	 void deregisterWithLocal (void);

	 virtual void addComposition (Composition & composition);
	 virtual void addLeakComposition (Composition & composition);
	 virtual void addWasteComposition (Composition & composition);
	 virtual void addSpillComposition (Composition & composition);

	 virtual void addToYourTrap (unsigned int i, unsigned int j);

	 virtual void clearCache (void);

      private:
	 virtual bool isCached (CacheBit bit) const;
	 virtual void setCached (CacheBit bit) const;
	 virtual void resetCached (CacheBit bit) const;

	 mutable double m_topDepth;
	 mutable double m_bottomDepth;
	 mutable double m_netToGross;
	 mutable double m_porosity;

	 FaultStatus m_faultStatus;

	 Column * m_adjacentColumn[NumPhases];
	 Column * m_targetColumn[NumPhases];

	 mutable BitField m_cachedValues;
   };

   class LocalColumn : public Column
   {
      public:
	 /// Constructor
	 LocalColumn (unsigned int i, unsigned int j, Reservoir * reservoir);

	 /// Destructor
	 virtual ~LocalColumn (void);


	 void retainPreviousProperties (void);
	 virtual void clearProperties (void);
	 virtual void clearPreviousProperties (void);

	 virtual int getDrainageAreaId (PhaseId phase);

	 virtual int getDrainageAreaId (void);

	 virtual int getGlobalTrapId (void);
	 virtual int getPreviousGlobalTrapId (void);
	 virtual void setGlobalTrapId (int trapId);

	 virtual double getTopDepth (void) const;
	 virtual double getPreviousTopDepth (void);
	 virtual void setTopDepth (double newTopDepth);

	 virtual void setNetToGross (double fraction);
	 virtual double getNetToGross (void) const;

	 virtual void setTopDepthOffset (double fraction);
	 virtual double getTopDepthOffset (void) const;

	 virtual void setBottomDepthOffset (double fraction);
	 virtual double getBottomDepthOffset (void);

	 virtual double getOverburden (void) const;
	 virtual void setOverburden (double topSurfaceDepth);

	 virtual double getSeaBottomPressure (void) const;
	 virtual void setSeaBottomPressure (double seaBottomPressure);

	 virtual double getFlowDirection (PhaseId phase);

	 virtual double getBottomDepth (void) const;
	 virtual double getPreviousBottomDepth (void);
	 virtual void setBottomDepth (double newBottomDepth);

	 virtual double getPorosity (void) const;
	 virtual double getPreviousPorosity (void);
	 virtual void setPorosity (double newPorosity);
	 virtual double getPorosityPercentage (void);

#ifdef USEOTGC
	 virtual double getImmobilesVolume (void) const;
	 virtual double getImmobilesDensity (void) const;
#endif

         virtual double getPermeability (void) const;
         virtual void setPermeability (double newPermeability);

	 virtual FaultStatus getFaultStatus (void);
	 virtual void setFaultStatus (FaultStatus newFaultStatus);

	 unsigned int getColumnStatus (void);

	 virtual double getTemperature (void) const;
	 virtual double getPreviousTemperature (void) const;
	 virtual void setTemperature (double newTemperature);

	 virtual double getPressure (void) const;
	 virtual double getPreviousPressure (void) const;
	 virtual void setPressure (double newPressure);

         virtual double getHydrostaticPressure (void) const;
         virtual void setHydrostaticPressure (double newHydrostaticpressure);

         virtual double getLithostaticPressure (void) const;
         virtual void setLithostaticPressure (double newLithostaticPressure);

#ifdef USEOTGC
	 void setImmobiles (const Immobiles & immobiles);
	 const Immobiles & getImmobiles (void);
#endif

	 void setTrap (Trap * trap);
	 Trap * getTrap (void);

	 virtual void setAdjacentColumn (PhaseId phase, Column * column);
	 virtual Column * getAdjacentColumn (PhaseId phase);
	 bool adjacentColumnSet (PhaseId phase);

	 /// whethere this column lies on the boundary of the model
	 bool isOnBoundary (void);

	 void setTargetColumn (PhaseId phase, Column * column);
	 virtual Column * getTargetColumn (PhaseId phase);

	 virtual bool computeTargetColumn (PhaseId phase);

	 virtual Column * getSpillTarget (void);

	 virtual Column * getTrapSpillColumn (void);

	 virtual void setValue (ColumnValueRequest & request);
	 virtual void setValue (ColumnValueArrayRequest & request);

	 virtual void getValue (ColumnValueRequest & request, ColumnValueRequest & response);
	 virtual void getValue (ColumnValueArrayRequest & request, ColumnValueArrayRequest & response);

	 double getValue (ValueSpec valueSpec, PhaseId phase = NO_PHASE);

	 bool isMinimum (void);

	 virtual void setWasting (PhaseId phase);
	 virtual void setSealing (PhaseId phase);

	 virtual bool isWasting (PhaseId phase);
	 virtual bool isSealing (PhaseId phase);

	 void resetFillDepths (void);
	 void resetChargeDensities (void);

	 inline double getCompositionWeight (void);
	 inline double getWeightToBeMigrated (void);
#ifdef USEOTGC
	 inline double getImmobilesWeight (void);
#endif

	 inline bool containsCompositionToBeMigrated (void);
	 inline void resetCompositionToBeMigrated (void);
	 virtual void addCompositionToBeMigrated (Composition & composition);

#ifdef USEOTGC
	 void crackChargesToBeMigrated (OilToGasCracker & otgc, double startTime, double endTime);
#endif

	 void migrateChargesToBeMigrated (void);

	 inline bool containsComponentToBeMigrated (ComponentId componentId);
	 inline void addComponentToBeMigrated (ComponentId componentId, double value);
	 inline void subtractComponentToBeMigrated (ComponentId componentId, double value);
	 inline void setComponentToBeMigrated (ComponentId componentId, double value);
	 inline double getComponentToBeMigrated (ComponentId componentId);

	 inline bool containsComposition (void);
	 inline void resetComposition (void);

         virtual void resetCompositionState ();
	 virtual void addComposition (Composition & composition);
	 virtual void addLeakComposition (Composition & composition);
	 virtual void addWasteComposition (Composition & composition);
	 virtual void addSpillComposition (Composition & composition);

	 void flashChargesToBeMigrated (Composition * compositionsOut);
	 void computePVT (Composition * compositionsOut);

	 void manipulateColumn (ValueSpec valueSpec, unsigned int i, unsigned int j);

	 virtual void addToYourTrap (unsigned int i, unsigned int j);

         virtual bool isSpilling (void);
         virtual bool isUndersized (void);

	 Column * getSpillBackTarget (PhaseId phase);

         virtual bool isSpillingBack (void);
         virtual bool isSpillingBack (PhaseId phase);

	 virtual double getDiffusionStartTime ();
	 virtual void setDiffusionStartTime (double diffusionStartTime);

	 virtual void setPenetrationDistances (const double * penetrationDistances);
	 virtual const double * getPenetrationDistances ();

	 double getPenetrationDistance (ComponentId c);
	 void setPenetrationDistance (ComponentId c, double penetrationDistance);

	 virtual void setFillDepth (PhaseId phase, double fillDepth);
	 virtual double getFillDepth (PhaseId phase);


	 virtual void setChargeDensity (PhaseId phase, double chargeDensity);
	 virtual double getChargeDensity (PhaseId phase);

	 virtual void setChargesToBeMigrated (PhaseId phase, Composition & composition);

	 virtual void resetMigrated (PhaseId phase);
	 virtual void addMigrated (PhaseId phase, double weight);
	 virtual double getMigrated (PhaseId phase);

	 virtual void addFlux (double weight);
	 virtual double getFlux ();

	 virtual double getFlow ();

	 virtual double getLateralChargeDensity (PhaseId phase);
	 virtual double getChargeQuantity (PhaseId phase);

	 void manipulateComposition (ValueSpec valueSpec, int phase, Composition & composition);

	 inline bool containsComponent (ComponentId componentId);
	 inline void addComponent (ComponentId componentId, double value);
	 inline void subtractComponent (ComponentId componentId, double value);
	 inline void setComponent (ComponentId componentId, double value);
	 inline double getComponent (ComponentId componentId);
	 inline Composition & getComposition (void);
         inline int getCompositionState (void);

	 void addProxy (int rank);
	 void deleteProxy (int rank);
	 void resetProxy (int rank);
	 void resetProxies (void);

      private:
	 /// net/gross fractions of the reservoir
	 double m_netToGross;
	 /// reservoir top offset from the top of the formation, fraction of the present day thickness
	 double m_topDepthOffset;
	 /// reservoir bottom offset from the bottom of the formation, fraction of the present day thickness
	 double m_bottomDepthOffset;

	 double m_topDepth;
	 double m_bottomDepth;
	 double m_overburden;
	 double m_seaBottomPressure;
	 double m_porosity;
#ifdef USEOTGC
	 double m_immobilesVolume;
#endif
         double m_permeability;
	 double m_temperature;
	 double m_pressure;
         double m_hydrostaticPressure;
         double m_lithostaticPressure;

	 FaultStatus m_faultStatus;

	 double m_topDepthPrevious;
	 double m_bottomDepthPrevious;
	 double m_porosityPrevious;
	 double m_temperaturePrevious;
	 double m_pressurePrevious;
	 int m_globalTrapIdPrevious;

	 Column * m_adjacentColumn[NumPhases];
	 Column * m_targetColumn[NumPhases];

	 Trap * m_trap;

	 Composition m_compositionToBeMigrated;

#ifdef USEOTGC
	 Immobiles m_immobiles;
#endif

	 Composition m_composition;
         int m_compositionState;

	 double m_diffusionStartTime;
	 double m_penetrationDistances[DiffusionComponentSize];

	 double m_fillDepth[NUM_PHASES];
	 double m_chargeDensity[NUM_PHASES];

	 double m_migrated[NUM_PHASES];
	 double m_flux;

	 int m_globalTrapId;
	 int m_drainageAreaId;

	 vector<int> m_proxies;
   };

   class ColumnArray
   {
      public:
	 ColumnArray (Reservoir * reservoir, int numIGlobal, int numJGlobal, int firstILocal, int firstJLocal, int lastILocal, int lastJLocal);

	 virtual ~ColumnArray (void);

	 Column * getColumn (unsigned int i, unsigned int j);

	 LocalColumn * getLocalColumn (unsigned int i, unsigned int j);
	 ProxyColumn * getProxyColumn (unsigned int i, unsigned int j);

	 inline unsigned int numIGlobal (void);
	 inline unsigned int numJGlobal (void);
	 inline unsigned int firstILocal (void);
	 inline unsigned int firstJLocal (void);
	 inline unsigned int lastILocal (void);
	 inline unsigned int lastJLocal (void);

	 inline int getNumberOfProxyColumns (void);

	 void retainPreviousProperties (void);
	 bool clearProperties (void);
	 bool clearPreviousProperties (void);

      private:
	 Reservoir * m_reservoir;

	 unsigned int m_numIGlobal;
	 unsigned int m_numJGlobal;
	 unsigned int m_firstILocal;
	 unsigned int m_lastILocal;
	 unsigned int m_firstJLocal;
	 unsigned int m_lastJLocal;

	 Column *** m_columns;

	 int m_numberOfProxyColumns;
   };


   ostream & operator<< (ostream & stream, Column & column);
   ostream & operator<< (ostream & stream, Column * column);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int migration::Column::getI (void) const
{
   return m_iGlobal;
}

unsigned int migration::Column::getJ (void) const
{
   return m_jGlobal;
}

bool migration::Column::isDeeperThan (Column * column, bool useTieBreaker)
{
   return compareDepths (column, useTieBreaker) == 1;
}

bool migration::Column::isShallowerThan (Column * column, bool useTieBreaker)
{
   return compareDepths (column, useTieBreaker) == -1;
}

migration::Reservoir * migration::Column::getReservoir (void)
{
   return m_reservoir;
}

const migration::Reservoir * migration::Column::reservoir (void) const
{
   return m_reservoir;
}

bool migration::LocalColumn::containsCompositionToBeMigrated (void)
{
   return !m_compositionToBeMigrated.isEmpty ();
}

void migration::LocalColumn::resetCompositionToBeMigrated (void)
{
   m_compositionToBeMigrated.reset ();
}

bool migration::LocalColumn::containsComponentToBeMigrated (ComponentId componentId)
{
   return !m_compositionToBeMigrated.isEmpty (componentId);
}

void migration::LocalColumn::addComponentToBeMigrated (ComponentId componentId, double value)
{
   m_compositionToBeMigrated.add (componentId, value);
}

void migration::LocalColumn::subtractComponentToBeMigrated (ComponentId componentId, double value)
{
   m_compositionToBeMigrated.subtract (componentId, value);
}

void migration::LocalColumn::setComponentToBeMigrated (ComponentId componentId, double value)
{
   m_compositionToBeMigrated.set (componentId, value);
}

double migration::LocalColumn::getComponentToBeMigrated (ComponentId componentId)
{
   return m_compositionToBeMigrated.getWeight (componentId);
}

double migration::LocalColumn::getWeightToBeMigrated (void)
{
   return m_compositionToBeMigrated.getWeight ();
}

bool migration::LocalColumn::containsComposition (void)
{
   return !m_composition.isEmpty ();
}

void migration::LocalColumn::resetComposition (void)
{
   m_composition.reset ();
}

double migration::LocalColumn::getCompositionWeight (void)
{
   return m_composition.getWeight ();
}

#ifdef USEOTGC
double migration::LocalColumn::getImmobilesWeight (void)
{
   return m_immobiles.getWeight ();
}
#endif

bool migration::LocalColumn::containsComponent (ComponentId componentId)
{
   return !m_composition.isEmpty (componentId);
}

void migration::LocalColumn::addComponent (ComponentId componentId, double value)
{
   m_composition.add (componentId, value);
}

void migration::LocalColumn::subtractComponent (ComponentId componentId, double value)
{
   m_composition.subtract (componentId, value);
}

void migration::LocalColumn::setComponent (ComponentId componentId, double value)
{
   m_composition.set (componentId, value);
}

double migration::LocalColumn::getComponent (ComponentId componentId)
{
   return m_composition.getWeight (componentId);
}

migration::Composition & migration::LocalColumn::getComposition (void)
{
   return m_composition;
}

int migration::LocalColumn::getCompositionState (void)
{
   return m_compositionState;
}

unsigned int migration::ColumnArray::numIGlobal (void)
{
   return m_numIGlobal;
}

unsigned int migration::ColumnArray::numJGlobal (void)
{
   return m_numJGlobal;
}

unsigned int migration::ColumnArray::firstILocal (void)
{
   return m_firstILocal;
}

unsigned int migration::ColumnArray::firstJLocal (void)
{
   return m_firstJLocal;
}

unsigned int migration::ColumnArray::lastILocal (void)
{
   return m_lastILocal;
}

unsigned int migration::ColumnArray::lastJLocal (void)
{
   return m_lastJLocal;
}

int migration::ColumnArray::getNumberOfProxyColumns (void)
{
   return m_numberOfProxyColumns;
}

#endif // _MIGRATION_COLUMN_H_
