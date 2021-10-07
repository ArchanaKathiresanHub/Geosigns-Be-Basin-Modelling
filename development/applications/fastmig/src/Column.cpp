//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Column.h"
#ifdef USEOTGC
#include "OilToGasCracker.h"
#include "Immobiles.h"
#endif
#include "MigrationReservoir.h"
#include "Trap.h"
#include "RequestHandling.h"

#include "rankings.h"

#include "array.h"

// std library
#include <assert.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

// utilities library
#include "LogHandler.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

using namespace AbstractDerivedProperties;
using std::ostringstream;
using namespace std;
using namespace ibs;

namespace migration
{

   Column::Column (unsigned int i, unsigned int j, MigrationReservoir * reservoir)
   {
      m_iGlobal = i;
      m_jGlobal = j;
      m_reservoir = reservoir;
   }

   Column::~Column (void)
   {
   }

   bool Column::isValid (void) const
   {
      return (getTopDepth () != m_reservoir->getUndefinedValue ());
   }

   bool IsValid (const Column * column)
   {
      return (column != 0 && column->isValid ());
   }

   double Column::getSurface (void) const
   {
      return reservoir ()->getSurface (getI (), getJ ());
   }

   double Column::getVolumeBetweenDepths (double upperDepth, double lowerDepth) const
   {
      double volume = 0;

      if (getBottomDepth () <= getTopDepth ())
         return volume;

      if (getTopDepth () < lowerDepth)
      {
         double topOfFill = Max (upperDepth, getTopDepth ());
         double bottomOfFill = Min (lowerDepth, getBottomDepth ());

         if (bottomOfFill > topOfFill)
         {
            volume = getSurface () * getNetToGross () * getPorosity () * (bottomOfFill - topOfFill);
         }

         // Subtract the previously generated immobiles stuff from the volume
         // This should really take into account the depths at which the immobiles were generated
#ifdef USEOTGC
         if (bottomOfFill > topOfFill)
            volume -= getImmobilesVolume () * (bottomOfFill - topOfFill) / (getBottomDepth () - getTopDepth ());
#endif
         if (volume < 0) volume = 0;
      }

      return volume;
   }

   /// compareDepths returns -1 if this is shallower, 0 if equally deep and 1 if this is deeper
   /// Also used to break the tie between columns with equal top depth
   /// Does not take into account whether columns are sealing or wasting
   int Column::compareDepths (Column * column, bool useTieBreaker)
   {
      // Use top or bottom depths depending on sealing status
      double depth = isSealing()? getBottomDepth() : getTopDepth();
      double columnDepth = column->isSealing()? column->getBottomDepth() : column->getTopDepth();

      if (depth < columnDepth) return -1;
      if (depth > columnDepth) return 1;

      if (useTieBreaker)
      {
         if (getI () + getJ () > column->getI () + column->getJ ()) return -1;
         if (getI () + getJ () < column->getI () + column->getJ ()) return 1;

         if (getI () > column->getI ()) return -1;
         if (getI () < column->getI ()) return 1;
         if (getJ () > column->getJ ()) return -1;
         if (getJ () < column->getJ ()) return 1;

         assert (this == column);
      }

      return 0;
   }

   bool Column::isWasting (void)
   {
      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         if (isWasting ((PhaseId)phase))
            return true;
      }
      return false;
   }

   bool Column::isSealing (void)
   {
      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         if (isSealing ((PhaseId)phase))
            return true;
      }
      return false;
   }


   double Column::getCapacity (double spillDepth)
   {
      if (IsValid (this))
      {
         return getVolumeBetweenDepths (Min (getTopDepth (), spillDepth), Min (getBottomDepth (), spillDepth));
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double Column::getThickness (void)
   {
      if (IsValid (this))
      {
         return getBottomDepth () - getTopDepth ();
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   Column * Column::getFinalSpillTarget (PhaseId phase)
   {
      Column * spillTarget = getSpillTarget (phase);

      if (!spillTarget or spillTarget->isSealing ()) return 0;
      else return spillTarget->getFinalTargetColumn (phase);
   }

   Column * Column::getFinalTargetColumn (PhaseId phase)
   {
      Column * column = this;
      Column * targetColumn = column->getTargetColumn (phase);

      while (column != targetColumn)
      {
         column = targetColumn;
         targetColumn = column->getTargetColumn (phase);
      }

      return targetColumn;
   }

   void Column::clearProperties (void)
   {
      m_bitField.clearAll ();
   }

   void Column::clearPreviousProperties (void)
   {
   }

   bool Column::isTrue (unsigned int bitSpec)
   {
      return m_bitField.valueIsTrue ((unsigned int)bitSpec);
   }

   void Column::setTrue (unsigned int bitSpec)
   {
      m_bitField.setValue ((unsigned int)bitSpec, true);
   }

   void Column::setFalse (unsigned int bitSpec)
   {
      m_bitField.setValue ((unsigned int)bitSpec, false);
   }

   void Column::setBit (unsigned int bitSpec, bool state)
   {
      m_bitField.setValue ((unsigned int)bitSpec, state);
   }

   LocalColumn::LocalColumn (unsigned int i, unsigned int j, MigrationReservoir * reservoir) : Column (i, j, reservoir)
   {
      m_bottomDepthOffset = 0;
      m_netToGross = 1; /// default net_to_gross is set to one if not defined for a reservoir. And for ARD its always 1

      m_composition = 0;
      m_compositionToBeMigrated = 0;
      /// Local column penetration distance is initially zero
      std::fill (m_penetrationDistances, m_penetrationDistances + DiffusionComponentSize, (double)0);
      m_diffusionStartTime = -1;
      clearProperties ();
      m_pasteurizationStatus = 0;
          // Composition buffers
          m_vaporTargetBuffer.clear();
          m_liquidTargetBuffer.clear();
          m_vaporSpillBuffer.clear();
          m_liquidSpillBuffer.clear();
          m_vaporWasteBuffer.clear();
          m_liquidWasteBuffer.clear();
          m_mergingBuffer.clear();
   }

   LocalColumn::~LocalColumn (void)
   {
      if (m_composition)
      {
         delete m_composition;
         m_composition = 0;
      }
      if (m_compositionToBeMigrated)
      {
         delete m_compositionToBeMigrated;
         m_compositionToBeMigrated = 0;
      }
          // Composition buffers
          m_vaporTargetBuffer.clear();
          m_liquidTargetBuffer.clear();
          m_vaporSpillBuffer.clear();
          m_liquidSpillBuffer.clear();
          m_vaporWasteBuffer.clear();
          m_liquidWasteBuffer.clear();
          m_mergingBuffer.clear();
   }

   void LocalColumn::retainPreviousProperties (void)
   {
      m_topDepthPrevious = m_topDepth;
      m_bottomDepthPrevious = m_bottomDepth;
      m_porosityPrevious = m_porosity;
      m_pressurePrevious = m_pressure;
      m_temperaturePrevious = m_temperature;
      m_globalTrapIdPrevious = m_globalTrapId;
   }

   void LocalColumn::clearProperties (void)
   {
      m_topDepth = m_reservoir->getUndefinedValue ();
      m_bottomDepth = m_reservoir->getUndefinedValue ();
      m_porosity = m_reservoir->getUndefinedValue ();
      m_permeability = m_reservoir->getUndefinedValue ();
      m_temperature = m_reservoir->getUndefinedValue ();
      m_viscosity = m_reservoir->getUndefinedValue ();
      m_pressure = m_reservoir->getUndefinedValue ();
      m_hydrostaticPressure = m_reservoir->getUndefinedValue ();
      m_lithostaticPressure = m_reservoir->getUndefinedValue ();

      m_trap = 0;
      m_flux = 0;
      m_globalTrapId = NoTrapId;
      m_drainageAreaId = NoTrapId;
      m_faultStatus = NOFAULT;

      if (m_composition)
      {
         delete m_composition;
         m_composition = 0;
      }

      resetFillDepths ();
      resetChargeDensities ();
      resetComposition ();

      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         m_adjacentColumn[phase] = 0;
         m_targetColumn[phase] = 0;
         m_migrated[phase] = 0;
      }

      Column::clearProperties ();
   }

   void LocalColumn::clearPreviousProperties (void)
   {
      m_globalTrapIdPrevious = NoTrapId;
      Column::clearPreviousProperties ();
   }

   int LocalColumn::getDrainageAreaId (PhaseId phase)
   {
      if (IsValid (this))
      {
         if (!isWasting (phase) && !isSealing (phase))
         {
            return getFinalTargetColumn (phase)->getDrainageAreaId ();
         }
         else
         {
            return NoTrapId;
         }
      }
      else
      {
         return (int)m_reservoir->getUndefinedValue ();
      }
   }

   int LocalColumn::getGlobalTrapId (void)
   {
      return (IsValid (this) ? m_globalTrapId : (int)m_reservoir->getUndefinedValue ());
   }

   void LocalColumn::setGlobalTrapId (int trapId)
   {
      m_globalTrapId = trapId;
   }

   int LocalColumn::getPreviousGlobalTrapId (void)
   {
      return (IsValid (this) ? m_globalTrapIdPrevious : (int)m_reservoir->getUndefinedValue ());
   }

   int LocalColumn::getDrainageAreaId (void)
   {
      int id = (m_trap ? m_trap->getDrainageAreaId () : NoTrapId);
      return id;
   }

   bool LocalColumn::isSpilling (void)
   {
      // only to be performed by crest columns
      assert (m_trap);
      return (m_trap->isSpilling ());
   }

   bool LocalColumn::isUndersized (void)
   {
      // only to be performed by crest columns
      assert (m_trap);
      return (m_trap->isUndersized ());
   }

   bool LocalColumn::isSpillingBack (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if (isSpillingBack ((PhaseId)phase))
         {
            return true;
         }
      }
      return false;
   }

   bool LocalColumn::isSpillingBack (PhaseId phase)
   {
      Column * spillTarget = getFinalSpillTarget (phase);
      if (isSpilling () && spillTarget && spillTarget->isSpilling ())
      {
         // may be spilling back in another phase!!
         for (int spillBackPhase = FIRST_PHASE; spillBackPhase < NUM_PHASES; ++spillBackPhase)
         {
            if (spillTarget->getFinalSpillTarget ((PhaseId)spillBackPhase) == this)
               return true;
         }
      }
      return false;
   }

   Column * LocalColumn::getSpillBackTarget (PhaseId phase)
   {
      Column * spillTarget = getFinalSpillTarget (phase);
      if (spillTarget)
      {
         // may be spilling back in another phase!!
         for (int spillBackPhase = FIRST_PHASE; spillBackPhase < NUM_PHASES; ++spillBackPhase)
         {
            if (spillTarget->getFinalSpillTarget ((PhaseId)spillBackPhase) == this)
               return spillTarget;
         }
      }
      return 0;
   }

   void LocalColumn::setDiffusionStartTime (double diffusionStartTime)
   {
      if (m_diffusionStartTime < 0)
      {
         m_diffusionStartTime = diffusionStartTime;
      }
   }

   double LocalColumn::getDiffusionStartTime ()
   {
      return m_diffusionStartTime;
   }

   void LocalColumn::setPenetrationDistances (const double * penetrationDistances)
   {
      std::copy (penetrationDistances, penetrationDistances + DiffusionComponentSize, m_penetrationDistances);
   }

   const double * LocalColumn::getPenetrationDistances ()
   {
      return m_penetrationDistances;
   }

   double LocalColumn::getPenetrationDistance (ComponentId c)
   {
      return m_penetrationDistances[c];
   }

   void LocalColumn::setFillDepth (PhaseId phase, double fillDepth)
   {
      if (phase > FIRST_PHASE)
      {
         fillDepth = Max (fillDepth, getFillDepth (PhaseId (phase - 1)));
      }

      fillDepth = Max (fillDepth, getTopDepth ());
      fillDepth = Min (fillDepth, getBottomDepth ());
      m_fillDepth[phase] = fillDepth;

   }

   double LocalColumn::getFillDepth (PhaseId phase)
   {
      if (IsValid (this))
      {
         return (m_fillDepth[phase]);
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double Column::getFillHeight (PhaseId phase)
   {
      if (phase == FIRST_PHASE)
      {
         return getFillDepth (phase) - getTopDepth ();
      }
      else
      {
         return getFillDepth (phase) - getFillDepth (PhaseId (phase - 1));
      }
   }

   void LocalColumn::setChargeDensity (PhaseId phase, double chargeDensity)
   {
      m_chargeDensity[phase] = chargeDensity;
   }

#ifdef USEOTGC
   void LocalColumn::setImmobiles (const Immobiles & immobiles)
   {
      m_immobiles = immobiles;
   }

   const Immobiles & LocalColumn::getImmobiles (void)
   {
      return m_immobiles;
   }
#endif

   void LocalColumn::setChargesToBeMigrated (PhaseId phase, Composition & composition)
   {
      double weightStored = getVolumeBetweenDepths (getFillDepth (phase) - getFillHeight (phase), getFillDepth (phase)) * composition.getDensity ();
      double weightFraction = 0;

      if (!composition.isEmpty ())
      {
         weightFraction = weightStored / composition.getWeight ();
      }

      if (weightFraction > 1)
      {
         if (weightFraction > 1.001 && weightStored >= 0.1)
         {
            LogHandler (LogHandler::WARNING_SEVERITY) << "WARNING: weight fraction of column " << this
               << " is too large (" << weightStored << "/" << composition.getWeight ()
               << " = " << weightFraction << "), truncating to 1";
         }
         weightFraction = 1;
      }

      if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
      for (unsigned int component = 0; component < NumComponents; ++component)
      {
         m_compositionToBeMigrated->add ((ComponentId)component, weightFraction * composition.getWeight ((ComponentId)component));
      }

      setChargeDensity (phase, composition.getDensity ());

   }

   double LocalColumn::getChargeDensity (PhaseId phase)
   {
      if (IsValid (this))
      {
         return m_chargeDensity[phase];
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   void LocalColumn::resetMigrated (PhaseId phase)
   {
      m_migrated[phase] = 0;
   }

   void LocalColumn::addMigrated (PhaseId phase, double weight)
   {
      m_migrated[phase] += weight;
   }

   double LocalColumn::getMigrated (PhaseId phase)
   {
      if (IsValid (this))
      {
         return m_migrated[phase];
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   void LocalColumn::addFlux (double weight)
   {
      m_flux += weight;
   }

   double LocalColumn::getFlux (void)
   {
      if (IsValid (this))
      {
         return m_flux / getSurface ();
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getFlow (void)
   {
      if (IsValid (this))
      {
         if (m_flux > 1)
            return log (m_flux) / log (10.0);
         else
            return 0.0;
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getLateralChargeDensity (PhaseId phase)
   {
      if (IsValid (this))
      {
         return getChargeDensity (phase) * getFillHeight (phase) * getPorosity ();
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getChargeQuantity (PhaseId phase)
   {
      if (IsValid (this))
      {
         return getLateralChargeDensity (phase) * getSurface ();
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getTopDepth (void) const
   {
      return m_topDepth;
   }

   double LocalColumn::getOWCTemperature (const double hydrocarbonWaterContactDepth) const
   {
      // Obtain a gridMap of temperature thanks to the DerivedProperties
      FormationPropertyPtr gridMapTemperature = m_reservoir->getFormationPropertyPtr ("Temperature", m_reservoir->getEnd ());

      if (gridMapTemperature == 0) // No gridMap, then we use the temperature of the crest column for biodegradation
      {
         LogHandler (LogHandler::WARNING_SEVERITY) << " The grid map for the reservoir  " << m_reservoir->getName () << " can't be retrieved. Using the temperature at the column's top ";
         return getTemperature ();
      }

      gridMapTemperature->retrieveData ();
      double depth = gridMapTemperature->lengthK ();
      assert (depth > 1);

      // Obtain the depths at the top and bottom of the crest column
      double const topDepth = getTopDepth ();
      double const bottomDepth = getBottomDepth ();

      // Transform the depth of the hydrocarbon - water contact in a node position of the crest column
      double const percentageHeightHydrocarbonWaterContact = (bottomDepth - hydrocarbonWaterContactDepth) / (bottomDepth - topDepth);
      double const nodeHydrocarbonWaterContact = (depth - 1) * percentageHeightHydrocarbonWaterContact;

      double index = nodeHydrocarbonWaterContact;
      index = Max ((double)0, nodeHydrocarbonWaterContact);
      index = Min ((double)depth - 1, nodeHydrocarbonWaterContact);

      double owcTemperature = gridMapTemperature->interpolate (getI (), getJ (), index);

      // we are in a local column, we should always get a valid owcTemperature
      assert (owcTemperature != gridMapTemperature->getUndefinedValue ());

      gridMapTemperature->restoreData ();

      return owcTemperature;
   }

   double LocalColumn::getPreviousTopDepth (void)
   {
      return m_topDepthPrevious;
   }

   void LocalColumn::setTopDepth (double newTopDepth)
   {
      m_topDepth = newTopDepth;
      resetFillDepths ();
      resetProxies ();
   }

   void LocalColumn::setNetToGross (double fraction)
   {
      fraction = Max (0., fraction);
      fraction = Min (1., fraction);

      m_netToGross = fraction;
   }

   double LocalColumn::getNetToGross (void) const
   {
      return m_netToGross;
   }

   void LocalColumn::setBottomDepthOffset (double fraction)
   {
      fraction = Max (0., fraction);
      fraction = Min (1., fraction);

      m_bottomDepthOffset = fraction;
   }

   double LocalColumn::getBottomDepthOffset (void)
   {
      return m_bottomDepthOffset;
   }

   void LocalColumn::setOverburden (double overburden)
   {
      m_overburden = overburden;
   }

   double LocalColumn::getOverburden (void) const
   {
      if (IsValid (this))
      {
         return m_overburden;
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   void LocalColumn::setSeaBottomPressure (double seaBottomPressure)
   {
      m_seaBottomPressure = seaBottomPressure;
   }

   double LocalColumn::getSeaBottomPressure (void) const
   {
      if (IsValid (this))
      {
         return m_seaBottomPressure;
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getFlowDirection (PhaseId phase)
   {
      if (IsValid (this))
      {
         if (getAdjacentColumn (OIL) == this)
            return 0;

         for (int n = 0; n < NumNeighbours; ++n)
         {
            Column * neighbourColumn = m_reservoir->getColumn (getI () + NeighbourOffsets2D[n][I], getJ () + NeighbourOffsets2D[n][J]);
            if (getAdjacentColumn (OIL) == neighbourColumn)
               return n + 1;
         }
      }
      return m_reservoir->getUndefinedValue ();
   }

   double LocalColumn::getFlowDirectionI (PhaseId phase)
   {
      if (!IsValid (this)) return m_reservoir->getUndefinedValue ();

      if (isWasting (phase)) return 0.0;

      if (getAdjacentColumn (phase) == 0) return 0;

      return double (int (getAdjacentColumn (phase)->getI ()) - int (getI ()));
   }

   double LocalColumn::getFlowDirectionJ (PhaseId phase)
   {
      if (!IsValid (this)) return m_reservoir->getUndefinedValue ();

      if (isWasting (phase)) return 0.0;

      if (getAdjacentColumn (phase) == 0) return 0;

      return double (int (getAdjacentColumn (phase)->getJ ()) - int (getJ ()));
   }

   double LocalColumn::getBottomDepth (void) const
   {
      return m_bottomDepth;
   }

   double LocalColumn::getPreviousBottomDepth (void)
   {
      return m_bottomDepthPrevious;
   }

   void LocalColumn::setBottomDepth (double newBottomDepth)
   {
      m_bottomDepth = newBottomDepth;
      resetFillDepths ();
      resetProxies ();
   }

   void LocalColumn::setPorosity (double newPorosity)
   {
      m_porosity = newPorosity;
   }

   double LocalColumn::getPorosity (void) const
   {
      return m_porosity;
   }

#ifdef USEOTGC
   double LocalColumn::getImmobilesVolume (void) const
   {
      return m_immobiles.getVolume ();
   }

   double LocalColumn::getImmobilesDensity (void) const
   {
      return m_immobiles.getWeight () / getSurface ();
   }
#endif

   double LocalColumn::getPorosityPercentage (void)
   {
      if (IsValid (this))
      {
         return m_porosity * Fraction2Percentage;
      }
      else
      {
         return m_reservoir->getUndefinedValue ();
      }
   }

   double LocalColumn::getPreviousPorosity (void)
   {
      return m_porosityPrevious;
   }

   void LocalColumn::setPermeability (double newPermeability)
   {
      m_permeability = newPermeability;
   }

   double LocalColumn::getPermeability (void) const
   {
      return m_permeability;
   }

   void LocalColumn::setFaultStatus (FaultStatus newFaultStatus)
   {
      m_faultStatus = newFaultStatus;
   }

   FaultStatus LocalColumn::getFaultStatus (void)
   {
      return m_faultStatus;
   }

   unsigned int LocalColumn::getColumnStatus (void)
   {
      unsigned int status = 0;
      if (isWasting (GAS)) status += 1;
      if (isSealing (GAS)) status += 2;
      if (isWasting (OIL)) status += 10;
      if (isSealing (OIL)) status += 20;
      return status;
   }

   void LocalColumn::setTemperature (double newTemperature)
   {
      m_temperature = newTemperature;
   }

   double LocalColumn::getTemperature (void) const
   {
      return m_temperature;
   }

   void LocalColumn::setViscosity (double viscosity)
   {
      m_viscosity = viscosity;
   }

   double LocalColumn::getViscosity (void) const
   {
      return m_viscosity;
   }

   double LocalColumn::getPreviousTemperature (void) const
   {
      return m_temperaturePrevious;
   }

   void LocalColumn::setPressure (double newPressure)
   {
      m_pressure = newPressure;
   }

   double LocalColumn::getPressure (void) const
   {
      return m_pressure;
   }

   double LocalColumn::getPreviousPressure (void) const
   {
      return m_pressurePrevious;
   }

   void LocalColumn::setHydrostaticPressure (double newHydrostaticPressure)
   {
      m_hydrostaticPressure = newHydrostaticPressure;
   }

   double LocalColumn::getHydrostaticPressure (void) const
   {
      return m_hydrostaticPressure;
   }

   void LocalColumn::setLithostaticPressure (double newLithostaticPressure)
   {
      m_lithostaticPressure = newLithostaticPressure;
   }

   double LocalColumn::getLithostaticPressure (void) const
   {
      return m_lithostaticPressure;
   }

   bool LocalColumn::isMinimum (const PhaseId phase)
   {
      return m_adjacentColumn[phase] == this;
   }

   void LocalColumn::setWasting (PhaseId phase)
   {
      setBit (BASESEALINGSET + phase, false);
      setBit (BASEWASTINGSET + phase, true);
   }

   bool LocalColumn::isWasting (PhaseId phase)
   {
      return isTrue (BASEWASTINGSET + phase);
   }

   void LocalColumn::setSealing (PhaseId phase)
   {
      setBit (BASEWASTINGSET + phase, false);
      setBit (BASESEALINGSET + phase, true);
   }

   bool LocalColumn::isSealing (PhaseId phase)
   {
      return isTrue (BASESEALINGSET + phase);
   }

   void LocalColumn::setValue (ColumnValueRequest & request)
   {
      switch (request.valueSpec)
      {
      case SETGLOBALTRAPID:
         setGlobalTrapId ((int)request.value);
         break;
      case SETTOPDEPTH:
         setTopDepth (request.value);
         break;
      case SETBOTTOMDEPTH:
         setBottomDepth (request.value);
         break;
      case SETDIFFUSIONSTARTTIME:
         setDiffusionStartTime (request.value);
         break;
      case SETFILLDEPTH:
         setFillDepth ((PhaseId)request.phase, request.value);
         break;
      case SETCHARGEDENSITY:
         setChargeDensity ((PhaseId)request.phase, request.value);
         break;
      case SETPASTEURIZATIONSTATUS:
         setPasteurizationStatus ((int)request.value);
         break;
      case ADDMIGRATED:
         addMigrated ((PhaseId)request.phase, request.value);
         break;
      case ADDFLUX:
         addFlux (request.value);
         break;
      case REGISTER:
         addProxy ((int)request.value);
         break;
      case DEREGISTER:
         deleteProxy ((int)request.value);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "ERROR in LocalColumn::setValue: illegal request: " << request.valueSpec;
         assert (0);
      }
   }

   void LocalColumn::getValue (ColumnValueRequest & request, ColumnValueRequest & response)
   {
      response.reservoirIndex = m_reservoir->getIndex ();
      response.valueSpec = request.valueSpec;

      Column * targetColumn;

      switch (request.valueSpec)
      {
      case TOPDEPTH:
         response.i = getI ();
         response.j = getJ ();
         response.value = getTopDepth ();
         break;
      case BOTTOMDEPTH:
         response.i = getI ();
         response.j = getJ ();
         response.value = getBottomDepth ();
         break;
      case DIFFUSIONSTARTTIME:
         response.i = getI ();
         response.j = getJ ();
         response.value = getDiffusionStartTime ();
         break;
      case FILLDEPTH:
         response.i = getI ();
         response.j = getJ ();
         response.phase = request.phase;
         response.value = getFillDepth ((PhaseId)request.phase);
         break;
      case THICKNESS:
         response.i = getI ();
         response.j = getJ ();
         response.value = getThickness ();
         break;
      case NETTOGROSS:
         response.i = getI ();
         response.j = getJ ();
         response.value = getNetToGross ();
         break;
      case POROSITY:
         response.i = getI ();
         response.j = getJ ();
         response.value = getPorosity ();
         break;
#ifdef USEOTGC
      case IMMOBILESVOLUME:
         response.i = getI ();
         response.j = getJ ();
         response.value = getImmobilesVolume ();
         break;
#endif
      case FAULTSTATUS:
         response.i = getI ();
         response.j = getJ ();
         response.value = (double)getFaultStatus ();
         break;
      case COLUMNSTATUS:
         response.i = getI ();
         response.j = getJ ();
         response.value = (double)getColumnStatus ();
         break;
      case ISSPILLING:
         response.i = getI ();
         response.j = getJ ();
         response.value = (double)isSpilling ();
         break;
      case ISUNDERSIZED:
         response.i = getI ();
         response.j = getJ ();
         response.value = (double)isUndersized ();
         break;
      case ISSEALING:
         response.i = getI ();
         response.j = getJ ();
         response.phase = request.phase;
         response.value = (double)isSealing ((PhaseId)request.phase);
         break;
      case ISWASTING:
         response.i = getI ();
         response.j = getJ ();
         response.phase = request.phase;
         response.value = (double)isWasting ((PhaseId)request.phase);
         break;
      case PASTEURIZATIONSTATUS:
         response.i = getI ();
         response.j = getJ ();
         response.value = getPasteurizationStatus ();
         break;
      case GLOBALTRAPID:
         response.i = getI ();
         response.j = getJ ();
         response.value = getGlobalTrapId ();
         break;
      case DRAINAGEAREAID:
         response.i = getI ();
         response.j = getJ ();
         response.value = getDrainageAreaId ();
         break;
      case ADJACENTCOLUMN:
         response.phase = request.phase;
         targetColumn = getAdjacentColumn ((PhaseId)request.phase);
         if (targetColumn)
         {
            response.i = targetColumn->getI ();
            response.j = targetColumn->getJ ();
         }
         else
         {
            response.i = -1;
            response.j = -1;
         }
         response.value = 0;
         break;
      case TARGETCOLUMN:
         response.phase = request.phase;
         targetColumn = m_targetColumn[request.phase];
         if (targetColumn)
         {
            response.i = targetColumn->getI ();
            response.j = targetColumn->getJ ();
         }
         else
         {
            response.i = -1;
            response.j = -1;
         }
         response.value = 0;
         break;
      case SPILLTARGET:
         response.phase = request.phase;
         targetColumn = getSpillTarget ((PhaseId)request.phase);
         if (targetColumn)
         {
            response.i = targetColumn->getI ();
            response.j = targetColumn->getJ ();
         }
         else
         {
            response.i = -1;
            response.j = -1;
         }
         response.value = 0;
         break;
      case TRAPSPILLCOLUMN:
         targetColumn = getTrapSpillColumn ();
         if (targetColumn)
         {
            response.i = targetColumn->getI ();
            response.j = targetColumn->getJ ();
         }
         else
         {
            response.i = -1;
            response.j = -1;
         }
         response.value = 0;
         break;
      case OWCTEMPERATURE:
         response.i = getI ();
         response.j = getJ ();
         response.phase = request.phase;
         response.value = getOWCTemperature (request.value);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "ERROR in LocalColumn::getValue: illegal request: " << request.valueSpec;
         assert (false);
      }
   }

   void LocalColumn::setValue (ColumnValueArrayRequest & request)
   {
      switch (request.valueSpec)
      {
      case SETPENETRATIONDISTANCE:
         setPenetrationDistances (request.value);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "ERROR in LocalColumn::setValue: illegal request: " << request.valueSpec;
         assert (0);
      }
   }

   void LocalColumn::getValue (ColumnValueArrayRequest & request, ColumnValueArrayRequest & response)
   {
      response.valueSpec = request.valueSpec;

      switch (request.valueSpec)
      {
      case PENETRATIONDISTANCE:
         response.i = getI ();
         response.j = getJ ();
         std::copy (getPenetrationDistances (), getPenetrationDistances () + DiffusionComponentSize, response.value);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "ERROR in LocalColumn::getValue: illegal request: " << request.valueSpec;
         assert (false);
      }
   }

   double LocalColumn::getValue (ValueSpec valueSpec, PhaseId phase)
   {
      switch (valueSpec)
      {
      case TOPDEPTH:
         return getTopDepth ();
      case BOTTOMDEPTH:
         return getBottomDepth ();
      case CAPACITY:
         return getCapacity ();
      case THICKNESS:
         return getThickness ();
      case NETTOGROSS:
         return getNetToGross ();
      case POROSITY:
         return getPorosity ();
#ifdef USEOTGC
      case IMMOBILESVOLUME:
         return getImmobilesVolume ();
      case IMMOBILESDENSITY:
         return getImmobilesDensity ();
#endif
      case POROSITYPERCENTAGE:
         return getPorosityPercentage ();
      case FAULTSTATUS:
         return (double)getFaultStatus ();
      case COLUMNSTATUS:
         return (double)getColumnStatus ();
      case PRESSURE:
         return getPressure ();
      case TEMPERATURE:
         return getTemperature ();
      case OVERBURDEN:
         return getOverburden ();
      case FLUX:
         return getFlux ();
      case FLOW:
         return getFlow ();
      case DRAINAGEAREAID:
         return getDrainageAreaId (phase);
      case GLOBALTRAPID:
         return getGlobalTrapId ();
      case DIFFUSIONSTARTTIME:
         return getDiffusionStartTime ();
      case FILLDEPTH:
         return getFillDepth (phase);
      case CHARGEDENSITY:
         return getChargeDensity (phase);
      case LATERALCHARGEDENSITY:
         return getLateralChargeDensity (phase);
      case LEAKAGEQUANTITY:
         return getCompositionWeight ();
      case SEEPAGEQUANTITY:
         return getPhaseWeight (phase);
      case CHARGEQUANTITY:
         return getChargeQuantity (phase);
      case PASTEURIZATIONSTATUS:
         return getPasteurizationStatus ();
      case FLOWDIRECTION:
         return getFlowDirection (phase);
      case FLOWDIRECTIONIJ:
         return 10.0 * getFlowDirectionJ (phase) + 1.0 * getFlowDirectionI (phase);
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "ERROR in LocalColumn::getValue: illegal request: " << valueSpec;
         assert (false);
         return 0;
      }
   }

   void LocalColumn::setTrap (Trap * trap)
   {
      m_trap = trap;
   }

   Trap * LocalColumn::getTrap (void)
   {
      return m_trap;
   }

   void LocalColumn::setAdjacentColumn (PhaseId phase, Column * column)
   {
      if (adjacentColumnSet (phase))
         return;

      m_adjacentColumn[phase] = column;
      setBit (BASEADJACENTCOLUMNSET + phase, true);
   }

   bool LocalColumn::adjacentColumnSet (PhaseId phase)
   {
      return isTrue (BASEADJACENTCOLUMNSET + phase);
   }

   Column * LocalColumn::getAdjacentColumn (PhaseId phase)
   {
      return m_adjacentColumn[phase];
   }

   bool LocalColumn::computeTargetColumn (PhaseId phase)
   {
      if (!m_targetColumn[phase])
      {
         // Wasting or zero thickness should result in upward leakage
         if (isWasting (phase) or getThickness () < MinimumThickness)
         {
            // the stuff is not going anywhere else.
            m_targetColumn[phase] = this;
         }
         else
         {
            // Even if it is sealing, the adjacent column has been set in Reservoir::getAdjacentColumn()
            Column * adjacentColumn = getAdjacentColumn (phase);

            if (adjacentColumn == 0 || adjacentColumn == this)
            {
               m_targetColumn[phase] = this;
            }
            else
            {
               adjacentColumn->computeTargetColumn (phase);
               m_targetColumn[phase] = adjacentColumn->getTargetColumn (phase);
            }
         }
      }

      return (m_targetColumn[phase] != 0);
   }

   void LocalColumn::setTargetColumn (PhaseId phase, Column * column)
   {
      m_targetColumn[phase] = column;
   }

   Column * LocalColumn::getTargetColumn (PhaseId phase)
   {
      assert (IsValid (this));
      return m_targetColumn[phase];
   }

   Column * LocalColumn::getSpillTarget (const PhaseId phase)
   {
      if (!m_trap)
      {
         return 0;
      }
      else
      {
         Column * targetColumn = m_trap->getSpillTarget (phase);
         return targetColumn;
      }
   }

   Column * LocalColumn::getTrapSpillColumn ()
   {
      assert (m_trap);
      Column * spillColumn = m_trap->getSpillColumn ();
      assert (spillColumn);

      return spillColumn;
   }

   void LocalColumn::resetFillDepths (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         setFillDepth ((PhaseId)phase, -1e8);
      }
   }

   void LocalColumn::resetChargeDensities (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         setChargeDensity ((PhaseId)phase, 0);
      }
   }

#ifdef USEOTGC
   void LocalColumn::crackChargesToBeMigrated (OilToGasCracker & otgc, double startTime, double endTime)
   {
      Composition compositionCracked;

      Immobiles immobilesOut;
      Immobiles immobilesIn;


      if (!m_compositionToBeMigrated || m_compositionToBeMigrated->isEmpty ()) return;

      immobilesIn = getImmobiles ();

      if (getPreviousGlobalTrapId () < 0)
      {
         LogHandler (LogHandler::ERROR_SEVERITY) << "Error: Trying to crack " << m_compositionToBeMigrated->getWeight () << " kg in column " << this << " outside of any trap";
      }

      otgc.compute (*m_compositionToBeMigrated, immobilesIn, startTime, endTime,
         getPreviousPressure (), getPressure (), getPreviousTemperature (), getTemperature (),
         compositionCracked, immobilesOut);

      Composition compositionLost;
      Composition compositionGained;

      for (unsigned int component = 0; component < NumComponents; ++component)
      {
         double gained = compositionCracked.getWeight ((ComponentId)component) - m_compositionToBeMigrated->getWeight ((ComponentId)component);
         if (gained > 0)
         {
            // stuff got generated
            compositionGained.set ((ComponentId)component, gained);
         }
         else
         {
            // stuff got lost
            compositionLost.set ((ComponentId)component, -gained);
         }
      }

      double inWeight = immobilesIn.getWeight () + m_compositionToBeMigrated->getWeight ();
      double outWeight = immobilesOut.getWeight () + compositionCracked.getWeight ();
      double percentageWeightDeviation = 100.0 * (inWeight - outWeight) / inWeight;
      if (percentageWeightDeviation < -0.5)
      {
         LogHandler (LogHandler::WARNING_SEVERITY) << "Error in Oil to Gas Cracking of " << this;
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Immobiles In:             " << immobilesIn.getWeight ();
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Sara In:                  " << m_compositionToBeMigrated->getWeight ();
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Immobiles Out:            " << immobilesOut.getWeight ();
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Sara Out:                 " << compositionCracked.getWeight ();
         LogHandler (LogHandler::WARNING_SEVERITY) << "    ------------------------- ";
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Balance:	             " << immobilesIn.getWeight () + m_compositionToBeMigrated->getWeight ()
            - (immobilesOut.getWeight () + compositionCracked.getWeight ());
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Weight Deviation in %:    " << percentageWeightDeviation;
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Temperatures:             " << getPreviousTemperature () << " C - " << getTemperature () << " C";
         LogHandler (LogHandler::WARNING_SEVERITY) << "    Pressures:                " << getPreviousPressure () << " MPa - " << getPressure () << " MPa";
      }

      *m_compositionToBeMigrated = compositionCracked;
      setImmobiles (immobilesOut);

      // add the amounts lost and gained to the mass balance
      m_reservoir->addToCrackingLoss (compositionLost);
      m_reservoir->addToCrackingGain (compositionGained);

      // report cracking loss for migrationiotbl
      m_reservoir->reportCrackingLoss (getPreviousGlobalTrapId (), compositionLost);

      // report cracking gain for migrationiotbl
      m_reservoir->reportCrackingGain (getPreviousGlobalTrapId (), compositionGained);
   }
#endif

   void LocalColumn::migrateChargesToBeMigrated (void)
   {
      if (!m_compositionToBeMigrated || m_compositionToBeMigrated->isEmpty ())
         return;

      Composition phaseCompositions[NumPhases];

      flashChargesToBeMigrated (phaseCompositions);

          // calculate the global position
          int numI = m_reservoir->getGrid()->numIGlobal();
          int position = getI() + getJ() * numI;

      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         if (phaseCompositions[phase].isEmpty ()) continue;

         Column * targetColumn = getFinalTargetColumn ((PhaseId)phase);
         assert (IsValid (targetColumn));

         double weight = phaseCompositions[phase].getWeight ();
         targetColumn->addTargetCompositionToBuffer((PhaseId)phase, position, phaseCompositions[phase]);

         if (getPreviousGlobalTrapId () >= 0)
            m_reservoir->reportLateralMigration (getPreviousGlobalTrapId (), targetColumn, phaseCompositions[phase]);
         else
            m_reservoir->reportVerticalMigration (this, targetColumn, phaseCompositions[phase]);

         addMigrated ((PhaseId)phase, weight);
      }
   }

   void LocalColumn::manipulateComposition (ValueSpec valueSpec, int phase, Composition & composition)
   {
      switch (valueSpec)
      {
      case ADDCOMPOSITIONTOBEMIGRATED:
         addCompositionToBeMigrated (composition);
         break;
      case SETCHARGESTOBEMIGRATED:
         setChargesToBeMigrated ((PhaseId)phase, composition);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "Error in LocalColumn::manipulateComposition: Unknown request " << valueSpec;
         assert (0);
      }
   }

   void LocalColumn::manipulateCompositionPosition(ValueSpec valueSpec, int phase, int position, Composition & composition)
   {
           switch (valueSpec)
           {
           case INCREASEBUFFERTARGET:
                   addTargetCompositionToBuffer((PhaseId)phase, position, composition);
                   break;
           case INCREASEBUFFERWASTE:
                   addWasteCompositionToBuffer((PhaseId)phase, position, composition);
                   break;
           case INCREASEBUFFERSPILL:
                   addSpillCompositionToBuffer((PhaseId)phase, position, composition);
                   break;
           case INCREASEBUFFEMERGE:
                   addMergingCompositionToBuffer(position, composition);
                   break;
           default:
                   LogHandler(LogHandler::ERROR_SEVERITY) << "Error in LocalColumn::manipulateCompositionPosition: Unknown request " << valueSpec;
                   assert(0);
           }
   }

   void LocalColumn::getComposition (ValueSpec valueSpec, int phase, Composition & composition)
   {
      switch (valueSpec)
      {
      case GETCHARGES:
         composition = getComposition ();
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "Error in LocalColumn::getComposition: Unknown request " << valueSpec;
         assert (0);
      }
   }

   void LocalColumn::resetCompositionState ()
   {
      m_compositionState = 0;
   }

   void LocalColumn::addComposition (const Composition & composition)
   {
      if (!m_composition) m_composition = new Composition;
      m_composition->add (composition);
      m_compositionState |= INITIAL;
   }

   // Add target composition
   void LocalColumn::addTargetCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           if ((PhaseId)phase == GAS)
           {
                   m_vaporTargetBuffer.push_back(std::make_pair(position,composition));
           }
           if ((PhaseId)phase == OIL)
           {
                   m_liquidTargetBuffer.push_back(std::make_pair(position, composition));
           }
   }

   void LocalColumn::addTargetBuffer()
   {
           bufferCompositionSorter bufferCompositionSorter;
           std::sort(m_vaporTargetBuffer.begin(), m_vaporTargetBuffer.end(), bufferCompositionSorter);
           for (size_t i=0;i<m_vaporTargetBuffer.size();++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_vaporTargetBuffer[i].second);
                   m_compositionState |= INITIAL;
           }

           std::sort(m_liquidTargetBuffer.begin(), m_liquidTargetBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_liquidTargetBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_liquidTargetBuffer[i].second);
                   m_compositionState |= INITIAL;
           }
           m_vaporTargetBuffer.clear();
           m_liquidTargetBuffer.clear();
   }

   // Add waste composition
   void LocalColumn::addWasteCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           if ((PhaseId)phase == GAS)
           {
                   m_vaporWasteBuffer.push_back(std::make_pair(position, composition));
           }
           if ((PhaseId)phase == OIL)
           {
                   m_liquidWasteBuffer.push_back(std::make_pair(position, composition));
           }
   }

   void LocalColumn::addWasteBuffer()
   {
           bufferCompositionSorter bufferCompositionSorter;
           std::sort(m_vaporWasteBuffer.begin(), m_vaporWasteBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_vaporWasteBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_vaporWasteBuffer[i].second);
                   m_compositionState |= WASTED;
           }

           std::sort(m_liquidWasteBuffer.begin(), m_liquidWasteBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_liquidWasteBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_liquidWasteBuffer[i].second);
                   m_compositionState |= WASTED;
           }
           m_vaporWasteBuffer.clear();
           m_liquidWasteBuffer.clear();
   }

   // Add spill composition
   void LocalColumn::addSpillCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           if ((PhaseId)phase == GAS)
           {
                   m_vaporSpillBuffer.push_back(std::make_pair(position, composition));
           }
           if ((PhaseId)phase == OIL)
           {
                   m_liquidSpillBuffer.push_back(std::make_pair(position, composition));
           }
   }

   void LocalColumn::addSpillBuffer()
   {
           bufferCompositionSorter bufferCompositionSorter;
           std::sort(m_vaporSpillBuffer.begin(), m_vaporSpillBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_vaporSpillBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_vaporSpillBuffer[i].second);
                   m_compositionState |= SPILLED_IN;
           }

           std::sort(m_liquidSpillBuffer.begin(), m_liquidSpillBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_liquidSpillBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_liquidSpillBuffer[i].second);
                   m_compositionState |= SPILLED_IN;
           }
           m_vaporSpillBuffer.clear();
           m_liquidSpillBuffer.clear();
   }

   // Add merging composition
   void LocalColumn::addMergingCompositionToBuffer(int position, Composition & composition)
   {
           m_mergingBuffer.push_back(std::make_pair(position, composition));
   }

   void LocalColumn::addMergedBuffer(const int *compositionState)
   {
           bufferCompositionSorter bufferCompositionSorter;
           std::sort(m_mergingBuffer.begin(), m_mergingBuffer.end(), bufferCompositionSorter);
           for (size_t i = 0; i<m_mergingBuffer.size(); ++i)
           {
                   if (!m_composition) m_composition = new Composition;
                   m_composition->add(m_mergingBuffer[i].second);
                   if (compositionState) {
                       m_compositionState |= MERGED;
                   }
                   else
                   {
                       m_compositionState |= INITIAL; /// whats the implication of changeing this is yet to be determined.
                   }                   
           }
           m_mergingBuffer.clear();
   }

   void LocalColumn::addLeakComposition (Composition & composition)
   {
      if (!m_composition) m_composition = new Composition;
      m_composition->add (composition);
      m_compositionState |= LEAKED;
   }

   void LocalColumn::flashChargesToBeMigrated (Composition * compositionsOut)
   {
      if (!m_compositionToBeMigrated or m_compositionToBeMigrated->isEmpty ())
         return;
      m_compositionToBeMigrated->computePVT (getTemperature (), getPressure (), compositionsOut);

      // Check that weights of phases add up to total weight
      const double vapourWeight = compositionsOut[GAS].getWeight ();
      const double liquidWeight = compositionsOut[OIL].getWeight ();
      const double totalWeight = m_compositionToBeMigrated->getWeight ();

      if ((liquidWeight + vapourWeight - totalWeight) / totalWeight > 0.01)
         LogHandler (LogHandler::WARNING_SEVERITY) << "Warning: something is wrong with composition of seepage at i=" << getI () << ", j=" << getJ () <<
            "\nTotal weight: " << totalWeight << "\nVapor weight: " << vapourWeight << "\nLiquid weight: " << liquidWeight;

      double pvtMassError = totalWeight - (liquidWeight + vapourWeight);

      m_reservoir->accumulateErrorInPVT (-pvtMassError);

      m_compositionToBeMigrated->reset ();
   }

   double LocalColumn::getPhaseWeight (PhaseId phase)
   {
      if (phase == GAS)
      {
         Composition phaseCompositions[NumPhases];

         double temperature = getTemperature ();
         double pressure = getPressure ();

         if (!m_composition or m_composition->isEmpty ())
            return 0.0;
         else
            m_composition->computePVT (temperature, pressure, phaseCompositions);

         // Check that weights of phases add up to total weight
         const double vapourWeight = phaseCompositions[GAS].getWeight ();
         const double liquidWeight = phaseCompositions[OIL].getWeight ();
         const double totalWeight = m_composition->getWeight ();

         if ((liquidWeight + vapourWeight - totalWeight) / totalWeight > 0.01)
            LogHandler (LogHandler::WARNING_SEVERITY) << "Warning: something is wrong with composition of seepage at i=" << getI () << ", j=" << getJ () <<
            "\nTotal weight: " << totalWeight << "\nVapor weight: " << vapourWeight << "\nLiquid weight: " << liquidWeight;

         // m_composition will contain only the liquid phase now
         m_composition->reset ();
         *m_composition = phaseCompositions[OIL];

         return vapourWeight;
      }
      else
      {
         if (!m_composition or m_composition->isEmpty ())
            return 0.0;

         double liquidWeight = m_composition->getWeight ();
         m_composition->reset ();
         return liquidWeight;
      }
   }

   void LocalColumn::computePVT (Composition * compositionsOut)
   {
      assert (m_composition);
      m_composition->computePVT (getTemperature (), getPressure (), compositionsOut);

      // Check that weights of phases add up to total weight
      const double vapourWeight = compositionsOut[GAS].getWeight ();
      const double liquidWeight = compositionsOut[OIL].getWeight ();
      const double totalWeight = m_composition->getWeight ();

      if (m_composition->isEmpty ())
      {
         if (!compositionsOut[OIL].isEmpty () or !compositionsOut[GAS].isEmpty ())
            LogHandler (LogHandler::WARNING_SEVERITY) << "Warning: something is wrong with composition of seepage at i=" << getI () << ", j=" << getJ () <<
            "\nTotal weight: " << totalWeight << "\nVapor weight: " << vapourWeight << "\nLiquid weight: " << liquidWeight;
      }
      else
      {
         if ((liquidWeight + vapourWeight - totalWeight) / totalWeight > 0.01)
            LogHandler (LogHandler::WARNING_SEVERITY) << "Warning: something is wrong with composition of seepage at i=" << getI () << ", j=" << getJ () <<
            "\nTotal weight: " << totalWeight << "\nVapor weight: " << vapourWeight << "\nLiquid weight: " << liquidWeight;
      }

      double pvtMassError = totalWeight - (liquidWeight + vapourWeight);

      m_reservoir->accumulateErrorInPVT (-pvtMassError);

      // here the crest column composition is re-set to 0
      m_composition->reset ();
   }

   void LocalColumn::manipulateColumn (ValueSpec valueSpec, unsigned int i, unsigned int j)
   {
      switch (valueSpec)
      {
      case ADDTOYOURTRAP:
         addToYourTrap (i, j);
         break;
      default:
         LogHandler (LogHandler::ERROR_SEVERITY) << "Error in LocalColumn::manipulateColumn: Unknown request " << valueSpec;
         assert (0);
      }
   }

   void LocalColumn::addToYourTrap (unsigned int i, unsigned int j)
   {
      assert (m_trap);
      m_trap->addToToBeAdded (i, j);
   }

   void LocalColumn::addProxy (int rank)
   {
      m_proxies.push_back (rank);
   }

   void LocalColumn::deleteProxy (int rank)
   {
      for (vector<int>::iterator proxyIter = m_proxies.begin (); proxyIter != m_proxies.end (); ++proxyIter)
      {
         if (*proxyIter == rank)
         {
            m_proxies.erase (proxyIter);
            return;
         }
      }
   }

   void LocalColumn::resetProxy (int rank)
   {
      ColumnValueRequest valueRequest;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = RESETPROXY;
      RequestHandling::SendProxyReset (rank, valueRequest);
   }

   void LocalColumn::resetProxies (void)
   {
      for (vector<int>::iterator proxyIter = m_proxies.begin (); proxyIter != m_proxies.end (); ++proxyIter)
      {
         resetProxy (*proxyIter);
      }
   }

   ProxyColumn::ProxyColumn (unsigned int i, unsigned int j, MigrationReservoir * reservoir) : Column (i, j, reservoir)
   {
      clearProperties ();
   }

   ProxyColumn::~ProxyColumn (void)
   {
      deregisterWithLocal ();
      if (m_composition)
      {
         delete m_composition;
         m_composition = 0;
      }
   }

   void ProxyColumn::clearProperties ()
   {
      clearCache ();

      m_topDepth = m_reservoir->getUndefinedValue ();
      m_bottomDepth = m_reservoir->getUndefinedValue ();
      m_porosity = m_reservoir->getUndefinedValue ();

      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         m_adjacentColumn[phase] = 0;
         m_targetColumn[phase] = 0;
      }

      Column::clearProperties ();
   }

   void ProxyColumn::clearPreviousProperties ()
   {
      Column::clearPreviousProperties ();
   }

   void ProxyColumn::clearCache ()
   {
      m_cachedValues.clearAll ();
   }

   bool ProxyColumn::isCached (CacheBit bit) const
   {
      return m_cachedValues.valueIsTrue ((unsigned int)bit);
   }

   void ProxyColumn::setCached (CacheBit bit) const
   {
      m_cachedValues.setValue ((unsigned int)bit, true);
   }

   void ProxyColumn::resetCached (CacheBit bit) const
   {
      m_cachedValues.setValue ((unsigned int)bit, false);
   }

   Composition & ProxyColumn::getComposition (void)
   {
      if (!m_composition) m_composition = new Composition;
      ColumnCompositionRequest chargesRequest;
      ColumnCompositionRequest chargesResponse;

      chargesRequest.i = getI ();
      chargesRequest.j = getJ ();

      chargesRequest.reservoirIndex = m_reservoir->getIndex ();
      chargesRequest.valueSpec = GETCHARGES;

      RequestHandling::SendRequest (chargesRequest, chargesResponse);
      (*m_composition) = chargesResponse.composition;
      return (*m_composition);
   }

   // Add target composition
   void ProxyColumn::addTargetCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           ColumnCompositionPositionRequest chargesRequest;
           ColumnCompositionPositionRequest chargesResponse;

           chargesRequest.i = getI();
           chargesRequest.j = getJ();
           chargesRequest.phase = phase;
           chargesRequest.position = position;
           chargesRequest.reservoirIndex = m_reservoir->getIndex();
           chargesRequest.valueSpec = INCREASEBUFFERTARGET;
           chargesRequest.composition = composition;

           RequestHandling::SendRequest(chargesRequest, chargesResponse);

   }

   // Add waste composition
   void ProxyColumn::addWasteCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           ColumnCompositionPositionRequest chargesRequest;
           ColumnCompositionPositionRequest chargesResponse;

           chargesRequest.i = getI();
           chargesRequest.j = getJ();
           chargesRequest.phase = phase;
           chargesRequest.position = position;
           chargesRequest.reservoirIndex = m_reservoir->getIndex();
           chargesRequest.valueSpec = INCREASEBUFFERWASTE;
           chargesRequest.composition = composition;

           RequestHandling::SendRequest(chargesRequest, chargesResponse);

   }

   // Add spill composition
   void ProxyColumn::addSpillCompositionToBuffer(PhaseId phase, int position, Composition & composition)
   {
           ColumnCompositionPositionRequest chargesRequest;
           ColumnCompositionPositionRequest chargesResponse;

           chargesRequest.i = getI();
           chargesRequest.j = getJ();
           chargesRequest.phase = phase;
           chargesRequest.position = position;
           chargesRequest.reservoirIndex = m_reservoir->getIndex();
           chargesRequest.valueSpec = INCREASEBUFFERSPILL;
           chargesRequest.composition = composition;

           RequestHandling::SendRequest(chargesRequest, chargesResponse);

   }

   // Add merged composition. Phase is not relevant here.
   void ProxyColumn::addMergingCompositionToBuffer(int position, Composition & composition)
   {
           ColumnCompositionPositionRequest chargesRequest;
           ColumnCompositionPositionRequest chargesResponse;

           chargesRequest.i = getI();
           chargesRequest.j = getJ();
           chargesRequest.position = position;
           chargesRequest.reservoirIndex = m_reservoir->getIndex();
           chargesRequest.valueSpec = INCREASEBUFFEMERGE;
           chargesRequest.composition = composition;

           RequestHandling::SendRequest(chargesRequest, chargesResponse);

   }

   void ProxyColumn::setChargesToBeMigrated (PhaseId phase, Composition & composition)
   {
      ColumnCompositionRequest chargesRequest;
      ColumnCompositionRequest chargesResponse;

      chargesRequest.i = getI ();
      chargesRequest.j = getJ ();

      chargesRequest.phase = phase;

      chargesRequest.reservoirIndex = m_reservoir->getIndex ();
      chargesRequest.valueSpec = SETCHARGESTOBEMIGRATED;

      chargesRequest.composition = composition;

      RequestHandling::SendRequest (chargesRequest, chargesResponse);
   }

   void ProxyColumn::addCompositionToBeMigrated (const Composition & composition)
   {
      ColumnCompositionRequest chargesRequest;
      ColumnCompositionRequest chargesResponse;

      chargesRequest.i = getI ();
      chargesRequest.j = getJ ();

      chargesRequest.reservoirIndex = m_reservoir->getIndex ();
      chargesRequest.valueSpec = ADDCOMPOSITIONTOBEMIGRATED;

      chargesRequest.composition = composition;

      RequestHandling::SendRequest (chargesRequest, chargesResponse);
   }

   void LocalColumn::addCompositionToBeMigrated (const Composition & composition)
   {
      if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
      m_compositionToBeMigrated->add (composition);
   }

   void ProxyColumn::addToYourTrap (unsigned int i, unsigned int j)
   {
      ColumnColumnRequest columnRequest;

      columnRequest.i = getI ();
      columnRequest.j = getJ ();

      columnRequest.valueI = i;
      columnRequest.valueJ = j;

      columnRequest.reservoirIndex = m_reservoir->getIndex ();
      columnRequest.valueSpec = ADDTOYOURTRAP;

      RequestHandling::SendRequest (columnRequest);
   }

   void ProxyColumn::setGlobalTrapId (int trapId)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETGLOBALTRAPID;
      valueRequest.value = (double)trapId;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   int ProxyColumn::getGlobalTrapId (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = GLOBALTRAPID;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (int)valueResponse.value;
   }

   int ProxyColumn::getDrainageAreaId (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = DRAINAGEAREAID;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (int)valueResponse.value;
   }

   void ProxyColumn::setTopDepth (double depth)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETTOPDEPTH;
      valueRequest.value = (double)depth;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   double ProxyColumn::getTopDepth (void) const
   {
      if (!isCached (TOPDEPTHCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = TOPDEPTH;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_topDepth = valueResponse.value;
         setCached (TOPDEPTHCACHE);
      }

      return m_topDepth;
   }

   double ProxyColumn::getOWCTemperature (const double hydrocarbonWaterContactdepth) const
   {

      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      //no need to cache this value, it needs to be computed all times
      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = OWCTEMPERATURE;
      valueRequest.value = hydrocarbonWaterContactdepth;
      RequestHandling::SendRequest (valueRequest, valueResponse);

      return valueResponse.value;
   }

   void ProxyColumn::setBottomDepth (double depth)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETBOTTOMDEPTH;
      valueRequest.value = (double)depth;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   double ProxyColumn::getBottomDepth (void) const
   {
      if (!isCached (BOTTOMDEPTHCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = BOTTOMDEPTH;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_bottomDepth = valueResponse.value;
         setCached (BOTTOMDEPTHCACHE);
      }

      return m_bottomDepth;
   }

   double ProxyColumn::getNetToGross (void) const
   {
      if (!isCached (NETTOGROSSCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = NETTOGROSS;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_netToGross = valueResponse.value;
         setCached (NETTOGROSSCACHE);
      }

      return m_netToGross;
   }

   double ProxyColumn::getPorosity (void) const
   {
      if (!isCached (POROSITYCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = POROSITY;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_porosity = valueResponse.value;
         setCached (POROSITYCACHE);
      }

      return m_porosity;
   }

#ifdef USEOTGC
   double ProxyColumn::getImmobilesVolume (void) const
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = IMMOBILESVOLUME;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return valueResponse.value;
   }
#endif

   FaultStatus ProxyColumn::getFaultStatus (void)
   {
      if (!isCached (FAULTSTATUSCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = FAULTSTATUS;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_faultStatus = (FaultStatus)(static_cast<int>((valueResponse.value)));
         setCached (FAULTSTATUSCACHE);
      }

      return m_faultStatus;
   }

   bool ProxyColumn::isSpilling (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = ISSPILLING;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (valueResponse.value != 0.0);
   }

   bool ProxyColumn::isUndersized (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = ISUNDERSIZED;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (valueResponse.value != 0.0);
   }

   bool ProxyColumn::isSealing (PhaseId phase)
   {
      if (!isCached ((CacheBit)(BASESEALINGCOLUMNCACHE + phase)))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = ISSEALING;
         valueRequest.phase = (int)phase;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         setBit (BASESEALINGSET + phase, (valueResponse.value != 0.0));
         setCached ((CacheBit)(BASESEALINGCOLUMNCACHE + phase));
      }

      return isTrue (BASESEALINGSET + phase);
   }

   bool ProxyColumn::isWasting (PhaseId phase)
   {
      if (!isCached ((CacheBit)(BASEWASTINGCOLUMNCACHE + phase)))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = ISWASTING;
         valueRequest.phase = (int)phase;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         setBit (BASEWASTINGSET + phase, (valueResponse.value != 0.0));
         setCached ((CacheBit)(BASEWASTINGCOLUMNCACHE + phase));
      }

      return isTrue (BASEWASTINGSET + phase);
   }

   double ProxyColumn::getDiffusionStartTime ()
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = DIFFUSIONSTARTTIME;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (double)valueResponse.value;
   }

   void ProxyColumn::setDiffusionStartTime (double diffusionStartTime)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETDIFFUSIONSTARTTIME;
      valueRequest.value = (double)diffusionStartTime;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   const double * ProxyColumn::getPenetrationDistances ()
   {
      // this function is not implemented
      assert (false);
      return 0;
   }

   void ProxyColumn::setPenetrationDistances (const double * penetrationDistances)
   {
      ColumnValueArrayRequest valueArrayRequest;
      ColumnValueArrayRequest valueArrayResponse;

      valueArrayRequest.i = getI ();
      valueArrayRequest.j = getJ ();
      valueArrayRequest.reservoirIndex = m_reservoir->getIndex ();
      valueArrayRequest.valueSpec = SETPENETRATIONDISTANCE;
      std::copy (penetrationDistances, penetrationDistances + DiffusionComponentSize, valueArrayRequest.value);
      RequestHandling::SendRequest (valueArrayRequest, valueArrayResponse);
   }

   double ProxyColumn::getFillDepth (PhaseId phase)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = FILLDEPTH;
      valueRequest.phase = (int)phase;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      return (double)valueResponse.value;
   }

   void ProxyColumn::setFillDepth (PhaseId phase, double fillDepth)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETFILLDEPTH;
      valueRequest.phase = (int)phase;
      valueRequest.value = (double)fillDepth;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   void ProxyColumn::setChargeDensity (PhaseId phase, double chargeDensity)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETCHARGEDENSITY;
      valueRequest.phase = (int)phase;
      valueRequest.value = (double)chargeDensity;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   void ProxyColumn::addMigrated (PhaseId phase, double weight)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = ADDMIGRATED;
      valueRequest.phase = (int)phase;
      valueRequest.value = (double)weight;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   void ProxyColumn::addFlux (double weight)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = ADDFLUX;
      valueRequest.value = (double)weight;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   void ProxyColumn::setPasteurizationStatus (int status)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SETPASTEURIZATIONSTATUS;
      valueRequest.value = (double)status;
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   int ProxyColumn::getPasteurizationStatus () const
   {
      if (!isCached (PASTEURIZATIONSTATUSCACHE))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = PASTEURIZATIONSTATUS;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         m_pasteurizationStatus = (int)valueResponse.value;
         setCached (PASTEURIZATIONSTATUSCACHE);
      }

      return m_pasteurizationStatus;
   }

   Column * ProxyColumn::getAdjacentColumn (PhaseId phase)
   {
      if (!isCached ((CacheBit)(BASEADJACENTCOLUMNCACHE + phase)))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = ADJACENTCOLUMN;
         valueRequest.phase = (int)phase;
         RequestHandling::SendRequest (valueRequest, valueResponse);
         if (valueResponse.i >= 0 && valueResponse.j >= 0)
         {
            m_adjacentColumn[phase] = m_reservoir->getColumn (valueResponse.i, valueResponse.j);
         }
         setCached ((CacheBit)(BASEADJACENTCOLUMNCACHE + phase));
      }

      return m_adjacentColumn[phase];
   }

   Column * ProxyColumn::getTargetColumn (PhaseId phase)
   {
      if (!isCached ((CacheBit)(BASETARGETCOLUMNCACHE + phase)))
      {
         ColumnValueRequest valueRequest;
         ColumnValueRequest valueResponse;

         valueRequest.i = getI ();
         valueRequest.j = getJ ();
         valueRequest.reservoirIndex = m_reservoir->getIndex ();
         valueRequest.valueSpec = TARGETCOLUMN;
         valueRequest.phase = (int)phase;
         RequestHandling::SendRequest (valueRequest, valueResponse);

         if (valueResponse.i >= 0 && valueResponse.j >= 0) // may not have been calculated yet!!
         {
            m_targetColumn[phase] = m_reservoir->getColumn (valueResponse.i, valueResponse.j);
            setCached ((CacheBit)(BASETARGETCOLUMNCACHE + phase));
         }
      }

      return m_targetColumn[phase];
   }

   bool ProxyColumn::computeTargetColumn (PhaseId phase)
   {
      return false;
   }

   Column * ProxyColumn::getSpillTarget (const PhaseId phase)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = SPILLTARGET;
      valueRequest.phase = (int)phase;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      if (valueResponse.i >= 0 && valueResponse.j >= 0)
      {
         return m_reservoir->getColumn (valueResponse.i, valueResponse.j);
      }
      else
      {
         return 0;
      }
   }

   Column * ProxyColumn::getTrapSpillColumn (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = TRAPSPILLCOLUMN;
      RequestHandling::SendRequest (valueRequest, valueResponse);
      if (valueResponse.i >= 0 && valueResponse.j >= 0)
      {
         return m_reservoir->getColumn (valueResponse.i, valueResponse.j);
      }
      else
      {
         return 0;
      }
   }

   void ProxyColumn::registerWithLocal (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = REGISTER;
      valueRequest.value = (double)GetRank ();
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   void ProxyColumn::deregisterWithLocal (void)
   {
      ColumnValueRequest valueRequest;
      ColumnValueRequest valueResponse;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.reservoirIndex = m_reservoir->getIndex ();
      valueRequest.valueSpec = DEREGISTER;
      valueRequest.value = (double)GetRank ();
      RequestHandling::SendRequest (valueRequest, valueResponse);
   }

   ColumnArray::ColumnArray (MigrationReservoir * reservoir,
      int numIGlobal, int numJGlobal,
      int firstILocal, int lastILocal,
      int firstJLocal, int lastJLocal) :
      m_reservoir (reservoir),
      m_numIGlobal (numIGlobal), m_numJGlobal (numJGlobal),
      m_firstILocal (firstILocal),
      m_lastILocal (lastILocal),
      m_firstJLocal (firstJLocal),
      m_lastJLocal (lastJLocal)
   {
      //casting to Column*** to avoid ambiguous call with intel compiler
      m_columns = Array<Column*>::create2d (m_numIGlobal, m_numJGlobal, static_cast<Column*>(nullptr));
      m_numberOfProxyColumns = 0;

      for (unsigned int i = m_firstILocal; i <= m_lastILocal; ++i)
      {
         for (unsigned int j = m_firstJLocal; j <= m_lastJLocal; ++j)
         {
            m_columns[i][j] = new LocalColumn (i, j, m_reservoir);
         }
      }
   }

   ColumnArray::~ColumnArray (void)
   {
      if (m_columns)
      {
         for (unsigned int i = firstILocal (); i <= lastILocal (); ++i)
         {
            for (unsigned int j = firstJLocal (); j <= lastJLocal (); ++j)
            {
               if (m_columns[i][j])
               {
                  delete m_columns[i][j];
                  m_columns[i][j] = 0;
               }
            }
         }

         Array < Column * >::delete2d (m_columns);

         m_columns = 0;
      }
   }

   Column * ColumnArray::getColumn (unsigned int i, unsigned int j)
   {
      if (i >= m_firstILocal && i <= m_lastILocal &&
         j >= m_firstJLocal && j <= m_lastJLocal)
      {
         return getLocalColumn (i, j);
      }
      else
      {
         return getProxyColumn (i, j);
      }
   }

   LocalColumn * ColumnArray::getLocalColumn (unsigned int i, unsigned int j)
   {
      LocalColumn * localColumn = 0;

      if (i >= m_firstILocal && i <= m_lastILocal &&
         j >= m_firstJLocal && j <= m_lastJLocal)
      {
         Column * column = m_columns[i][j];
         assert (column);
         localColumn = dynamic_cast<LocalColumn *> (column);
         assert (localColumn);
      }

      return localColumn;
   }

   ProxyColumn * ColumnArray::getProxyColumn (unsigned int i, unsigned int j)
   {
      ProxyColumn * proxyColumn = 0;

      if (i < m_numIGlobal && j < m_numJGlobal &&
         !(i >= m_firstILocal && i <= m_lastILocal && j >= m_firstJLocal && j <= m_lastJLocal))
      {
         Column * column = m_columns[i][j];
         if (!column)
         {
            column = m_columns[i][j] = proxyColumn = new ProxyColumn (i, j, m_reservoir);
            ++m_numberOfProxyColumns;
            // need to do this AFTER proxycolumn has been created and added to the datastructure.
            proxyColumn->registerWithLocal ();
         }
         else
         {
            proxyColumn = dynamic_cast<ProxyColumn *> (column);
            assert (proxyColumn);
         }
      }
      return proxyColumn;
   }

   bool ColumnArray::clearProperties (void)
   {
      for (unsigned int i = 0; i < m_numIGlobal; ++i)
      {
         for (unsigned int j = 0; j < m_numJGlobal; ++j)
         {
            Column * column = m_columns[i][j];
            if (!column)
            {
               continue;
            }
            column->clearProperties ();
         }
      }

      return true;
   }

   bool ColumnArray::clearPreviousProperties (void)
   {
      for (unsigned int i = 0; i < m_numIGlobal; ++i)
      {
         for (unsigned int j = 0; j < m_numJGlobal; ++j)
         {
            Column * column = m_columns[i][j];
            if (!column)
            {
               continue;
            }
            column->clearPreviousProperties ();
         }
      }

      return true;
   }

   void ColumnArray::retainPreviousProperties (void)
   {
      for (unsigned int i = firstILocal (); i <= lastILocal (); ++i)
      {
         for (unsigned int j = firstJLocal (); j <= lastJLocal (); ++j)
         {
            getLocalColumn (i, j)->retainPreviousProperties ();
         }
      }
   }

   ostream & operator<< (ostream & stream, Column &column)
   {
      return stream << &column;
   }

   ostream & operator<< (ostream & stream, Column * column)
   {
      if (!column)
      {
         stream << "null";
      }
      else
      {
         stream << "("
            << column->getI ()
            << ", "
            << column->getJ ()
            << ", "
            << column->getTopDepth ()
            << ", "
            << column->getBottomDepth ()
            << (column->isSealing (GAS) ? ",S(g)" : "")
            << (column->isSealing (OIL) ? ",S(o)" : "")
            << (column->isWasting (GAS) ? ",W(g)" : "")
            << (column->isWasting (OIL) ? ",W(o)" : "")
            << ")";
      }
      return stream;
   }

} // namespace migration
