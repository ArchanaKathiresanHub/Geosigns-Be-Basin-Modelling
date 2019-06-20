//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
      Column (unsigned int i, unsigned int j, MigrationReservoir * reservoir);

      /// Destructor
      virtual ~Column (void);

      virtual void clearProperties (void);
      virtual void clearPreviousProperties (void);

      inline unsigned int getI (void) const;
      inline unsigned int getJ (void) const;
      inline MigrationReservoir * getReservoir (void);
      inline const MigrationReservoir * reservoir (void) const;

      bool isValid (void) const;

      virtual Column * getFinalTargetColumn (PhaseId phase);

      virtual int getDrainageAreaId (void) = 0;

      virtual int getGlobalTrapId (void) = 0;
      virtual void setGlobalTrapId (int trapId) = 0;

      virtual void setTopDepth (double depth) = 0;
      virtual double getTopDepth (void) const = 0;
      virtual void setBottomDepth (double depth) = 0;
      virtual double getBottomDepth (void) const = 0;
      virtual double getOWCTemperature (const double hydrocarbonWatercontactDepth) const = 0;
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

      virtual Composition & getComposition (void) = 0;
      virtual void setChargesToBeMigrated (PhaseId phase, Composition & composition) = 0;

      virtual void addCompositionToBeMigrated (const Composition & composition) = 0;

      virtual void addMigrated (PhaseId phase, double weight) = 0;
      virtual void addFlux (double weight) = 0;

      virtual double getSurface (void) const;
      virtual double getVolumeBetweenDepths (double upperDepth, double lowerDepth) const;

      virtual int compareDepths (Column * column, bool useTieBreaker = true);
      inline bool isDeeperThan (Column * column, bool useTieBreaker = true);
      inline bool isShallowerThan (Column * column, bool useTieBreaker = true);

      virtual int getPasteurizationStatus () const = 0;
      virtual void setPasteurizationStatus (int status) = 0;

      virtual Column * getAdjacentColumn (PhaseId phase) = 0;

      virtual Column * getTargetColumn (PhaseId phase) = 0;
      virtual Column * getSpillTarget (const PhaseId phase) = 0;

      virtual bool computeTargetColumn (PhaseId phase) = 0;

      virtual Column * getFinalSpillTarget (PhaseId phase);

      virtual Column * getTrapSpillColumn (void) = 0;

	  // Buffer the compositions of target, spilling, wasting column and undersized traps in local buffers. 
	  // These local buffers will be sorted by global positions in the 2D grid and then the compositions added sequentially. 
	  // this is to guarantee determinism in parallel runs, where the addition used to be done in the order of the first arriving MPI process 
	  // (truncation error accumulated and gave different phase distribution under certain conditions)
	  virtual void addTargetCompositionToBuffer(PhaseId phase, int position, Composition & composition) = 0;
	  virtual void addWasteCompositionToBuffer(PhaseId phase, int position, Composition & composition) = 0;
      virtual void addSpillCompositionToBuffer(PhaseId phase, int position, Composition & composition) = 0;
	  virtual void addMergingCompositionToBuffer(int position, Composition & composition) = 0;

      virtual void addToYourTrap (unsigned int i, unsigned int j) = 0;

      virtual bool isTrue (unsigned int bitSpec);
      virtual void setTrue (unsigned int bitSpec);
      virtual void setFalse (unsigned int bitSpec);
      virtual void setBit (unsigned int bitSpec, bool state);

   protected:
      MigrationReservoir * m_reservoir;

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
      ProxyColumn (unsigned int i, unsigned int j, MigrationReservoir * reservoir);

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
      virtual double getOWCTemperature (const double hydrocarbonWatercontactDepth) const;

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
      virtual Composition & getComposition (void);
      virtual void setChargeDensity (PhaseId phase, double chargeDensity);

      virtual void setChargesToBeMigrated (PhaseId phase, Composition & composition);

      virtual void addCompositionToBeMigrated (const Composition & composition);

      virtual void addMigrated (PhaseId phase, double weight);
      virtual void addFlux (double weight);

      virtual int getPasteurizationStatus () const;
      virtual void setPasteurizationStatus (int status);

      virtual Column * getAdjacentColumn (PhaseId phase);

      virtual Column * getTargetColumn (PhaseId phase);
      virtual Column * getSpillTarget (const PhaseId phase);

      virtual bool computeTargetColumn (PhaseId phase);

      virtual Column * getTrapSpillColumn (void);

      void registerWithLocal (void);
      void deregisterWithLocal (void);

      virtual void addTargetCompositionToBuffer(PhaseId phase, int position, Composition & composition);
      virtual void addWasteCompositionToBuffer(PhaseId phase, int position, Composition & composition);
      virtual void addSpillCompositionToBuffer(PhaseId phase, int position, Composition & composition);
      virtual void addMergingCompositionToBuffer(int position, Composition & composition);

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
      mutable int    m_pasteurizationStatus;

      FaultStatus m_faultStatus;

      Column * m_adjacentColumn[NumPhases];
      Column * m_targetColumn[NumPhases];

      mutable BitField m_cachedValues;
      Composition * m_composition;
   };

   class LocalColumn : public Column
   {
   public:
      /// Constructor
      LocalColumn (unsigned int i, unsigned int j, MigrationReservoir * reservoir);

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
      virtual double getOWCTemperature (const double hydrocarbonWatercontactDepth) const;
      virtual double getPreviousTopDepth (void);
      virtual void setTopDepth (double newTopDepth);

      virtual void setNetToGross (double fraction);
      virtual double getNetToGross (void) const;

      virtual double getOverburden (void) const;
      virtual void setOverburden (double topSurfaceDepth);

      virtual double getSeaBottomPressure (void) const;
      virtual void setSeaBottomPressure (double seaBottomPressure);

      virtual double getFlowDirection (PhaseId phase);
      virtual double getFlowDirectionI (PhaseId phase);
      virtual double getFlowDirectionJ (PhaseId phase);

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

      /*!
      * \brief Get the Temperature at the top of the column
      * \return The Temperature at the top of the column (in °C)
      */
      virtual double getTemperature (void) const;
      virtual double getPreviousTemperature (void) const;
      virtual void setTemperature (double newTemperature);

      /*!
      * \brief Get the pasteurization status of a column
      * \details -1 = the column has been part of a trap before but is not pasteurized
      *           0 = the column has not yet been part of a trap and is set as neutral column (i.e. the column was outside of any trap at all previous snapshots)
      *           1 = the column has been part of a trap before and is pasteurized
      * \return The pasteurization status of a column
      */
      inline int getPasteurizationStatus () const { return m_pasteurizationStatus; };

      /*!
      * \brief Set the pasteurization status of a column
      * \details -1 = the column has been part of a trap before but is not pasteurized
      *           0 = the column has not yet been part of a trap and is set as neutral column (i.e. the column was outside of any trap at all previous snapshots)
      *           1 = the column has been part of a trap before and is pasteurized
      */
      inline void setPasteurizationStatus (int status) { m_pasteurizationStatus = status; };

      virtual double getPressure (void) const;
      virtual double getPreviousPressure (void) const;
      virtual void setPressure (double newPressure);

      virtual double getViscosity (void) const;
      virtual void setViscosity (double viscosity);

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

      void setTargetColumn (PhaseId phase, Column * column);
      virtual Column * getTargetColumn (PhaseId phase);

      virtual bool computeTargetColumn (PhaseId phase);

      virtual Column * getSpillTarget (const PhaseId phase);

      virtual Column * getTrapSpillColumn (void);

      virtual void setValue (ColumnValueRequest & request);
      virtual void setValue (ColumnValueArrayRequest & request);

      virtual void getValue (ColumnValueRequest & request, ColumnValueRequest & response);
      virtual void getValue (ColumnValueArrayRequest & request, ColumnValueArrayRequest & response);

      double getValue (ValueSpec valueSpec, PhaseId phase = NO_PHASE);

      bool isMinimum (const PhaseId phase);

      virtual void setWasting (PhaseId phase);
      virtual void setSealing (PhaseId phase);

      virtual bool isWasting (PhaseId phase);
      virtual bool isSealing (PhaseId phase);

      void resetFillDepths (void);
      void resetChargeDensities (void);

      inline double getCompositionWeight (void);

      /// \brief Returns the mass of HCs in a specified phase within a column
      ///
      /// \param the phase for which the mass will be returned
      /// \return the mass of the specified phase
      double getPhaseWeight (PhaseId phase);
      inline double getWeightToBeMigrated (void);
#ifdef USEOTGC
      inline double getImmobilesWeight (void);
#endif

      inline void resetCompositionToBeMigrated (void);
      virtual void addCompositionToBeMigrated (const Composition & composition);

#ifdef USEOTGC
      void crackChargesToBeMigrated (OilToGasCracker & otgc, double startTime, double endTime);
#endif

      void migrateChargesToBeMigrated (void);

      inline void addComponentToBeMigrated (ComponentId componentId, double value);
      inline void subtractComponentToBeMigrated (ComponentId componentId, double value);
      inline double getComponentToBeMigrated (ComponentId componentId);

      inline bool containsComposition (void);
      inline void resetComposition (void);

      virtual void resetCompositionState ();
      void addComposition (const Composition & composition);
      void addLeakComposition (Composition & composition);

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
	  void manipulateCompositionPosition(ValueSpec valueSpec, int phase, int position, Composition & composition);
      void getComposition (ValueSpec valueSpec, int phase, Composition & composition);

      inline double getComponent (ComponentId componentId);
      inline Composition & getComposition (void);
      inline int getCompositionState (void);

      void addProxy (int rank);
      void deleteProxy (int rank);
      void resetProxy (int rank);
      void resetProxies (void);

	  virtual void addTargetCompositionToBuffer(PhaseId phase, int position, Composition & composition);
	  virtual void addWasteCompositionToBuffer(PhaseId phase, int position, Composition & composition);
	  virtual void addSpillCompositionToBuffer(PhaseId phase, int position, Composition & composition);
	  virtual void addMergingCompositionToBuffer(int position, Composition & composition);

	  // the local buffers are sorted and added to m_composition
	  void addTargetBuffer();
	  void addWasteBuffer();
	  void addSpillBuffer();
	  void addMergedBuffer();

   private:

      /// net/gross fractions of the reservoir
      double m_netToGross;

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
      double m_viscosity;

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
      int m_pasteurizationStatus;

      Composition * m_compositionToBeMigrated;

#ifdef USEOTGC
      Immobiles m_immobiles;
#endif

      Composition * m_composition;
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

	  std::vector<std::pair<int, Composition>>  m_vaporTargetBuffer;
	  std::vector<std::pair<int, Composition>>  m_liquidTargetBuffer;
	  std::vector<std::pair<int, Composition>>  m_vaporWasteBuffer;
	  std::vector<std::pair<int, Composition>>  m_liquidWasteBuffer;
	  std::vector<std::pair<int, Composition>>  m_vaporSpillBuffer;
	  std::vector<std::pair<int, Composition>>  m_liquidSpillBuffer;
	  std::vector<std::pair<int, Composition>>  m_mergingBuffer;
   };

   class ColumnArray
   {
   public:
      ColumnArray (MigrationReservoir * reservoir, int numIGlobal, int numJGlobal, int firstILocal, int firstJLocal, int lastILocal, int lastJLocal);

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
      MigrationReservoir * m_reservoir;

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

   // Sorter functor
   class bufferCompositionSorter
   {
   public:
	   bool operator() (const std::pair<int, Composition>& lhs, const std::pair<int, Composition>& rhs)
	   {
		   return lhs.first < rhs.first;
	   }
   };
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

migration::MigrationReservoir *migration::Column::getReservoir(void)
{
   return m_reservoir;
}

const migration::MigrationReservoir *migration::Column::reservoir(void) const
{
   return m_reservoir;
}

void migration::LocalColumn::resetCompositionToBeMigrated (void)
{
   if (m_compositionToBeMigrated) m_compositionToBeMigrated->reset ();
}

void migration::LocalColumn::addComponentToBeMigrated (ComponentId componentId, double value)
{
   if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
   m_compositionToBeMigrated->add (componentId, value);
}

void migration::LocalColumn::subtractComponentToBeMigrated (ComponentId componentId, double value)
{
   if (m_compositionToBeMigrated) m_compositionToBeMigrated->subtract (componentId, value);
}

double migration::LocalColumn::getComponentToBeMigrated (ComponentId componentId)
{
   if (!m_compositionToBeMigrated) return 0.0;
   return m_compositionToBeMigrated->getWeight (componentId);
}

double migration::LocalColumn::getWeightToBeMigrated (void)
{
   if (!m_compositionToBeMigrated) return 0.0;
   return m_compositionToBeMigrated->getWeight ();
}

bool migration::LocalColumn::containsComposition (void)
{
   if (!m_composition) return false;
   return !m_composition->isEmpty ();
}

void migration::LocalColumn::resetComposition (void)
{
   if (m_composition) m_composition->reset ();
}

double migration::LocalColumn::getCompositionWeight (void)
{
   if (!m_composition) return 0.0;
   return m_composition->getWeight ();
}

#ifdef USEOTGC
double migration::LocalColumn::getImmobilesWeight (void)
{
   return m_immobiles.getWeight ();
}
#endif

double migration::LocalColumn::getComponent (ComponentId componentId)
{
   if (!m_composition) return 0.0;
   return m_composition->getWeight (componentId);
}

migration::Composition & migration::LocalColumn::getComposition (void)
{
   if (!m_composition) m_composition = new Composition;
   return (*m_composition);
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
