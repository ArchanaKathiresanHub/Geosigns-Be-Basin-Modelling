#include "Reservoir.h"
#include "OilToGasCracker.h"
#include "Trap.h"
#include "Column.h"
#include "Formation.h"
#include "Surface.h"
#include "Migrator.h"
#include "Barrier.h"

#include "rankings.h"
#include "migration.h"

#include "overburden_MPI.h"
#include "RetrieveAndRestoreSurfaceGridMapContainer.h"
#include "RequestHandling.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/PropertyValue.h"
#include "Interface/BiodegradationParameters.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "Interface/DiffusionLeakageParameters.h"

#include "CBMGenerics/src/consts.h"

#include "petscvec.h"
#include "petscdmda.h"

#define MAXDOUBLE std::numeric_limits<double>::max()

#include <vector>
#include <assert.h>
#include <math.h>

#include<sstream>
using std::ostringstream;
extern ostringstream cerrstrstr;


using namespace std;
using namespace CBMGenerics;

using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::FormationList;
using Interface::Surface;
using Interface::PropertyValue;
using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FracturePressureFunctionParameters;
using Interface::BiodegradationParameters;
using Interface::DiffusionLeakageParameters;

namespace migration
{

Reservoir::Reservoir (ProjectHandle * projectHandle, database::Record * record)
   : Interface::Reservoir (projectHandle, record),
     m_start(0),
     m_end(0)
{
   m_chargeDistributionCount = 0;
   m_averageDepth = Interface::DefaultUndefinedScalarValue;
   m_columnArray = 0;
   createColumns ();

   setSourceReservoir (0);
   setSourceFormation (0);

   m_lowResEqualsHighRes =
      (* m_projectHandle->getLowResolutionOutputGrid () == * m_projectHandle->getHighResolutionOutputGrid ());

   for (int n = 0; n < 8; ++n)
   {
      m_neighbourDistances[n] = -1;
   }
}

Reservoir::~Reservoir (void)
{
   removePreviousTraps ();
   removeTraps ();
   destroyColumns ();
}

void Reservoir::createColumns (void)
{
   m_columnArray = new ColumnArray (this,
	 getGrid ()->numIGlobal (), getGrid ()->numJGlobal (),
	 getGrid ()->firstI (), getGrid ()->lastI (),
	 getGrid ()->firstJ (), getGrid ()->lastJ ());
}

void Reservoir::destroyColumns (void)
{
   if (m_columnArray)
   {
      delete m_columnArray;
      m_columnArray = 0;
   }
}

void Reservoir::retainPreviousTraps (void)
{
   TrapVector::iterator trapIter;

   assert (m_previousTraps.size () == 0);
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      m_previousTraps.push_back (*trapIter);
   }

   m_traps.clear ();
}

void Reservoir::removePreviousTraps (void)
{
   TrapVector::iterator trapIter;

   for (trapIter = m_previousTraps.begin (); trapIter != m_previousTraps.end (); ++trapIter)
   {
      delete * trapIter;
   }

   m_previousTraps.clear ();
}

void Reservoir::removeTraps (void)
{
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      delete * trapIter;
   }

   m_traps.clear ();
}

bool Reservoir::computeTraps (void)
{
   bool result = true;

   if (result) result = computeTrapTops ();
   if (result) m_maximumTrapCount = computeMaximumTrapCount ();
   if (result) result = computeTrapExtents ();
   if (result) result = initializeTraps ();


#ifdef MERGEUNDERSIZEDTRAPSAPRIORI
   if (result) result = mergeUndersizedTraps ();
#endif

   return result;
}

/// For each column,determine the adjacent column to migrate charge to.
bool Reservoir::computePathways (void)
{
   bool result = true;

   RequestHandling::StartRequestHandling (this, "computePathways");

   for (unsigned int phase = 0; phase < NumPhases; ++phase)
   {
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
	 for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
	 {
	    computeAdjacentColumn ((PhaseId) phase, i, j);
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   return result;
}

/// For each column,determine the end of the path of adjacent columns to migrate charge to.
bool Reservoir::computeTargetColumns (void)
{
   bool result = true;

   RequestHandling::StartRequestHandling (this, "computeTargetColumns");

   for (unsigned int phase = 0; phase < NumPhases; ++phase)
   {
      bool allComputed = false;
      // depends on computations performed on other processors.
      // hence, keep on going until all target columns have been computed as it may not go right the first time
      while (!allComputed)
      {
	 allComputed = true;
	 for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
	 {
	    for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
	    {
	       allComputed &= computeTargetColumn ((PhaseId) phase, i, j);
	    }
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   return result;
}

/// find the trap tops and create a trap for each of them.
bool Reservoir::computeTrapTops (void)
{
   bool result = true;

   RequestHandling::StartRequestHandling (this, "computeTrapTops");

   for (unsigned int phase = 0; phase < NumPhases; ++phase)
   {
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
	 for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
	 {
	    computeTrapTop ((PhaseId) phase, i, j);
	 }
      }
   }
   RequestHandling::FinishRequestHandling ();

   return result;
}

bool Reservoir::computeFluxes (void)
{
   bool result = true;

   int iteration = 0;

   do
   {
      m_computeFluxesHasFinished = true;
      ++iteration;

      RequestHandling::StartRequestHandling (this, "computeFluxes");

      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
            {
               computeFlux ((PhaseId) phase, i, j);
            }
         }
      }

      RequestHandling::FinishRequestHandling ();
   }
   while (!allProcessorsFinished (m_computeFluxesHasFinished));
   return result;
}

/// Compute the extents of the traps.
bool Reservoir::computeTrapExtents (void)
{
   bool result = true;

   RequestHandling::StartRequestHandling (this, "computeTrapExtents");

   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (* trapIter)->computeArea ();
      (* trapIter)->computeDepthToVolumeFunction ();
#ifdef COMPUTECAPACITY
      (* trapIter)->computeCapacity ();
#endif
      (* trapIter)->computeWasteColumns ();
      (* trapIter)->computeSpillTarget ();
#if 0
      (*trapIter)->printInterior ();
      (*trapIter)->printPerimeter ();

      cerr << GetRankString () << ": " << getName () << "::spillColumn = " << (*trapIter)->getSpillColumn () << endl;
#endif
   }

   RequestHandling::FinishRequestHandling ();
   return result;
}

bool Reservoir::recomputeTrapDepthToVolumeFunctions (void)
{
   if (!isOilToGasCrackingOn()) return false;

   RequestHandling::StartRequestHandling (this, "recomputeTrapDepthToVolumeFunctions");
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (* trapIter)->computeDepthToVolumeFunction ();
   }
   RequestHandling::FinishRequestHandling ();

   return true;
}


/// Initialize the atributes of the traps.
bool Reservoir::initializeTraps (void)
{
   bool result = true;

   RequestHandling::StartRequestHandling (this, "initializeTraps");

   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (* trapIter)->initialize ();
   }

   RequestHandling::FinishRequestHandling ();
   return result;
}


bool Reservoir::computeTargetColumn (PhaseId phase, unsigned int i, unsigned int j)
{
   LocalColumn * column = getLocalColumn (i, j);

   if (!IsValid (column))
      return true;
   else
      return column->computeTargetColumn (phase);
}
   
// compute the column to which column (i,j) spillls to
bool Reservoir::computeAdjacentColumn (PhaseId phase, unsigned int i, unsigned int j)
{
   LocalColumn *column = getLocalColumn (i, j);

   if (!IsValid (column))
      return true;

   if (column->adjacentColumnSet (phase))
      return true;

   Column * adjacentColumn = getAdjacentColumn (phase, column);

   column->setAdjacentColumn (phase, adjacentColumn);

   return (adjacentColumn != 0);
}

// find the column to which column (i,j) spillls to and that is not in the trap
Column *  Reservoir::getAdjacentColumn (PhaseId phase, Column * column, Trap * trap)
{
   assert (IsValid (column));

   double depth = column->getTopDepth ();

   assert (depth != getUndefinedValue ());

   double minGradient = SealDepth;

#if 0
   if (column->isWasting(phase)) // charge will go upward
   {
      return column;
   }
#endif

   Column * adjacentColumn = column;

   // try to find a higher lying column
   for (int n = 0; n < NumNeighbours; ++n)
   {
      Column *neighbourColumn = getColumn (column->getI () + NeighbourOffsets[n][I], column->getJ () + NeighbourOffsets[n][J]);

      if (!IsValid (neighbourColumn))
      {
	 // column lies on the edge, no adjacentColumn
	 adjacentColumn = 0;
	 break;
      }

      if (trap && trap->contains (neighbourColumn))
      {
	 continue;
      }

      if (neighbourColumn->isSealing (phase))
      {
	 // a sealing column cannot be in a migration pathway.
	 continue;
      }

      double neighbourDepth = neighbourColumn->getTopDepth ();

      if (column->isShallowerThan (neighbourColumn))
      {
	 // neighbour is deeper
	 continue;
      }
      else if (column->isDeeperThan (neighbourColumn, false))
      {
	 // neighbour is shallower in absolute terms
	 double gradient = (neighbourDepth - depth) /
	    getNeighbourDistance (n);

	 if (gradient <= minGradient)
	 {
	    // neighbourColumn has steepest upward gradient from column, so far.
	    // set it to be the adjacent column.
	    minGradient = gradient;
	    adjacentColumn = neighbourColumn;
	 }
      }
      else
      {
	 assert (depth == neighbourDepth);
	 if (!adjacentColumn || adjacentColumn->isDeeperThan (neighbourColumn))
	 {
	    minGradient = 0;
	    adjacentColumn = neighbourColumn;
	 }
      }
   }

   return adjacentColumn;
}

bool Reservoir::computeFlux (PhaseId phase, unsigned int i, unsigned int j)
{
   LocalColumn *column = getLocalColumn (i, j);

   if (!IsValid (column))
      return true;

   double weight = column->getMigrated (phase);
   column->resetMigrated (phase);
   assert (weight != getUndefinedValue ());
   if (weight < MinimumMass) return true;

   if (column->isWasting (phase)) // everything leaks upward from here
      return true;

   column->addFlux (weight);

   Column * adjacentColumn = column->getAdjacentColumn (phase);
   while (adjacentColumn != 0 && adjacentColumn != column && IsValid (adjacentColumn))
   {
      if (adjacentColumn->isWasting (phase)) // everything leaks upward from here
	 break;

      if ((column = dynamic_cast<LocalColumn *> (adjacentColumn)) != 0)
      {
	 adjacentColumn->addFlux (weight);
	 adjacentColumn = adjacentColumn->getAdjacentColumn (phase);
      }
      else
      {
	 // let the other processor deal with the rest
	 m_computeFluxesHasFinished = false;
	 adjacentColumn->addMigrated (phase, weight);
	 break;
      }
   }
   return true;
}


/// determine whether specified indices are of a trap top and create a trap for the associated column if so.
void Reservoir::computeTrapTop (PhaseId phase, unsigned int i, unsigned int j)
{
   LocalColumn * column = getLocalColumn (i, j);

   if (!IsValid (column) || !column->isMinimum () ||
	 column->isWasting (phase) || column->isSealing (phase) ||
	 column->getTrap ())
   {
      return;
   }

   // column does not have neigbours with smaller depth and left bottom neighbours all have greater depth

   Trap * trap = new Trap (column);
   addTrap (trap);
   column->setGlobalTrapId (trap->getGlobalId ());
}

double Reservoir::getSurface (unsigned int i, unsigned int j) const
{
   return getGrid ()->getSurface (i, j);
}

void Reservoir::retainPreviousProperties (void)
{
   RequestHandling::StartRequestHandling (this, "retainPreviousProperties");
   m_columnArray->retainPreviousProperties ();
   RequestHandling::FinishRequestHandling ();
}

bool Reservoir::clearProperties (void)
{
   m_lossPVT = 0;
   m_errorPVT = 0;

   m_compositionBlocked.reset ();

   m_crackingLoss.reset ();
   m_crackingGain.reset ();

   m_chargeDistributionCount = 0;

   RequestHandling::StartRequestHandling (this, "clearProperties");
   bool result = true;
   result = m_columnArray->clearProperties ();
   RequestHandling::FinishRequestHandling ();

   return result;
}

bool Reservoir::clearPreviousProperties (void)
{
   RequestHandling::StartRequestHandling (this, "clearPreviousProperties");
   bool result = m_columnArray->clearPreviousProperties ();
   RequestHandling::FinishRequestHandling ();
   return result;
}

bool Reservoir::computeProperties (void)
{
   RequestHandle requestHandle (this, "computeProperties");

   if (!computeDepths ())
      return false;

   if (!computeSeaBottomPressures ())
      return false;

   if (!computeOverburdens ())
      return false;

   if (!adaptOverburdens ())
      return false;

   if (!computeFaults ())
      return false;

   if (!computePorosities ())
      return false;

   if (!computePermeabilities ())
      return false;

   if (!computeTemperatures ())
      return false;

   if (!computePressures ())
      return false;

   if (!computeOverburdenGridMaps ())
      return false;

   // And if the type of fracturePressureFunctionParameters is given by Interface::
   // FunctionOfLithostaticPressure, also the hydrostatic and lithostatic pressures are needed.  
   const FracturePressureFunctionParameters* 
      fracturePressureFunctionParameters = getProjectHandle()->getFracturePressureFunctionParameters();
   if (!fracturePressureFunctionParameters)
      return false;

      if (fracturePressureFunctionParameters->type () == Interface::FunctionOfLithostaticPressure)
      {
      if (!computeHydrostaticPressures ())
         return false;

      if (!computeLithostaticPressures ())
         return false;
   }

   return true;
}

/// compute the top and bottom depths of the reservoir.
bool Reservoir::computeDepths (void)
{
   const GridMap * formationGridMap = getPropertyGridMap ("Depth", getEnd ());

   if (!formationGridMap)
   {
      cerr << "ERROR: " << getName () <<
	 "::computing of reservoir depths failed, could not find the formation depth map" << endl;
	 cerr.flush ();
      return false;
   }

   unsigned int depth = formationGridMap->getDepth ();
   assert (depth > 1);

   formationGridMap->retrieveData ();

   if (lowResEqualsHighRes ())
   {
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn *column = getLocalColumn (i, j);

            double topIndex = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	    double bottomIndex = column->getBottomDepthOffset () * (depth - 1);

            topIndex = Max ((double) 0, topIndex);
            topIndex = Min ((double) depth - 1, topIndex);

	    bottomIndex = Max ((double) 0, bottomIndex);
	    bottomIndex = Min ((double) depth - 1, bottomIndex);

            double topValue = formationGridMap->getValue (i, j, topIndex);
	    double bottomValue = formationGridMap->getValue (i, j, bottomIndex);

            if (topValue == formationGridMap->getUndefinedValue ())
            {
	       topValue = getUndefinedValue ();
            }
	    column->setTopDepth (topValue);

	    if (bottomValue == formationGridMap->getUndefinedValue ())
	    {
	       assert (topValue == getUndefinedValue ());

	       bottomValue = getUndefinedValue ();
	    }
	    column->setBottomDepth (bottomValue);
         }
      }
   }
   else
   {
      const GridMap * topSurfaceGridMap = getTopSurfacePropertyGridMap ("DepthHighRes", getEnd ());
      const GridMap * bottomSurfaceGridMap = getBottomSurfacePropertyGridMap ("DepthHighRes", getEnd ());

      if (!topSurfaceGridMap)
      {
         cerr << "ERROR: " << getName () <<
               "::computing of reservoir depths failed, could not find the DepthHighRes map for surface "
	       << getFormation ()->getTopSurface ()->getName () << " or higher at age " << getEnd ()->getTime () << endl;
	 cerr.flush ();
         return false;
      }

      if (!bottomSurfaceGridMap)
      {
         cerr << "ERROR: " << getName () <<
               "::computing of reservoir depths failed, could not find the DepthHighRes map for surface " <<
	       getFormation ()->getBottomSurface ()->getName () << " or lower at age " << getEnd ()->getTime () << endl;
	 cerr.flush ();
         return false;
      }

      topSurfaceGridMap->retrieveData ();
      bottomSurfaceGridMap->retrieveData ();

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn *column = getLocalColumn (i, j);

	    double topSurfaceDepth = topSurfaceGridMap->getValue (i, j);
	    double bottomSurfaceDepth = bottomSurfaceGridMap->getValue (i, j);
            double lowResTopSurfaceDepth = formationGridMap->getValue (i, j, depth - 1);


	    if (lowResTopSurfaceDepth == formationGridMap->getUndefinedValue () ||
		  topSurfaceDepth == topSurfaceGridMap->getUndefinedValue () ||
		  bottomSurfaceDepth == bottomSurfaceGridMap->getUndefinedValue ())
	    {
               column->setTopDepth (getUndefinedValue ());
               column->setBottomDepth (getUndefinedValue ());
	    }
	    else
	    {
	       double topValue = column->getTopDepthOffset () * bottomSurfaceDepth +
		  (1 - column->getTopDepthOffset ()) * topSurfaceDepth;

	       topValue = Max (topValue, topSurfaceDepth);
               column->setTopDepth (topValue);

	       double bottomValue = column->getBottomDepthOffset () * topSurfaceDepth +
		  (1 - column->getBottomDepthOffset ()) * bottomSurfaceDepth;

	       bottomValue = Min(bottomValue, bottomSurfaceDepth);
               column->setBottomDepth (bottomValue);

               assert (column->isValid ());
	    }
	 }
      }

      topSurfaceGridMap->restoreData ();
      bottomSurfaceGridMap->restoreData ();
   }

      formationGridMap->restoreData ();
   return true;
}

/// Compute the overburdens assuming the reservoir lies at the top of the formation.
/// Additional overburdens are calculated in adaptOverburdens ().
bool Reservoir::computeOverburdens (void)
{
   const GridMap * gridMap = getSeaBottomDepthMap (getEnd ());
   if (!gridMap)
   {
      return false;
   }

   gridMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    column->setOverburden (column->getTopDepth () - gridMap->getValue (i, j));
	 }
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computeSeaBottomPressures (void)
{
   const GridMap * gridMap = getSeaBottomPressureMap (getEnd ());
   if (!gridMap)
   {
      return false;
   }

   gridMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    column->setSeaBottomPressure (column->getTopDepth () - gridMap->getValue (i, j));
	 }
      }
   }
   gridMap->restoreData ();

   return true;
}

/// Increase the overburden with the difference between
/// the reservoir top depth and the reservoir's formation top depth.
bool Reservoir::adaptOverburdens (void)
{
   const GridMap * gridMap;

   if (lowResEqualsHighRes ())
      gridMap = getTopSurfacePropertyGridMap ("Depth", getEnd ());
   else
      gridMap = getTopSurfacePropertyGridMap ("DepthHighRes", getEnd ());

   gridMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);

	 if (column->getTopDepth () != getUndefinedValue ())
	 {
	    double formationTopDepth = gridMap->getValue (i, j);
	    double reservoirDepthOffset = 0;

	    if (formationTopDepth != gridMap->getUndefinedValue ())
	    {
	       reservoirDepthOffset = column->getTopDepth () - formationTopDepth;
	       reservoirDepthOffset = Max (0.0, reservoirDepthOffset);
	    }
	    column->setOverburden (column->getOverburden () + reservoirDepthOffset);
	 }
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computeFaults (void)
{
   const GridMap * gridMap = getFormation ()->computeFaultGridMap (getGrid (), getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth (); // should be 1

   gridMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 FaultStatus fs;

	 double gmValue = gridMap->getValue (i, j, depth - 1);
	 if (gmValue == gridMap->getUndefinedValue ())
	 {
	    fs = NOFAULT;
	 }
	 else
	 {
	    fs = FaultStatus (static_cast<int>(gmValue));
	 }
	 getLocalColumn (i, j)->setFaultStatus (fs);
      }
   }
   gridMap->restoreData ();
   // required, not a PropertyValue GridMap
   delete gridMap;

   return true;
}

bool Reservoir::computeOverburdenGridMaps (void)
{

   m_diffusionOverburdenGridMaps.clear();
   m_sealPressureLeakageGridMaps.clear();

   // Determine the overburden formations.  Include first the formation of the reservoir, for 
   // the trap may be sealed inside the formation:
   vector<const Formation*> formations;
   formations.push_back( dynamic_cast<Formation *>( const_cast<Interface::Formation*>(getFormation())));
   overburden::OverburdenFormations overburden(formations, true);

   // Add the formation overburdens:
   overburden::OverburdenFormations overburdenFormations = overburden::getOverburdenFormations( dynamic_cast<Formation *> ( const_cast<Interface::Formation*>(getFormation())), true);
   overburden.append(overburdenFormations.formations());

   // We need the depth for both diffusion leakage and seal pressure leakage.  (Note, we put depths 
   // in vector<FormationSurfaceGridMaps> even though the depth is a continuous property.  This is done 
   // just for convenience.   The algorithms work out easier.)
   const ProjectHandle* projectHandle = getFormation()->getProjectHandle();
   const Property* depthProp = lowResEqualsHighRes() ? 
      projectHandle->findProperty("Depth") :
      projectHandle->findProperty("DepthHighRes");
   
   vector<FormationSurfaceGridMaps> depthGridMaps = overburden_MPI::getFormationSurfaceGridMaps(
      overburden.formations(), depthProp, getEnd());

   // If diffusion leakages is included, initialize m_diffusionOverburdenGridMaps with 
   // the necessary grid maps:
   if (isDiffusionOn())
   {
      vector<SurfaceGridMapFormations> temperatureGridMaps = overburden_MPI::getAdjacentSurfaceGridMapFormations(
         overburden, "Temperature", getEnd());
      vector<FormationSurfaceGridMaps> porosityGridMaps = overburden_MPI::getFormationSurfaceGridMaps(
	 overburden.formations(), "Porosity", getEnd());

      m_diffusionOverburdenGridMaps.setDiscontinuous(SurfaceGridMapContainer::DISCONTINUOUS_DEPTH, depthGridMaps);
      m_diffusionOverburdenGridMaps.setContinuous(SurfaceGridMapContainer::CONTINUOUS_TEMPERATURE, temperatureGridMaps);
      m_diffusionOverburdenGridMaps.setDiscontinuous(SurfaceGridMapContainer::DISCONTINUOUS_POROSITY, porosityGridMaps);
   }

   // Include the grid maps for seal failure. Seal failure is always on.  We need the grid maps 
   // for permeability and the percentage maps for LithoType 1, 2 and 3.  We read permeability by 
   // means of overburden_MPI::getFormationSurfaceGridMapss.  We can't use formation_MPI::surfaceTopPropertyMap, 
   // because permeability is not a continuous property, so we must get the permeability from the 
   // seal formation.  And at this moment in time, it is not clear what the seal formation is, 
   // so we read in all formations:
   vector<FormationSurfaceGridMaps> permeabilityGridMaps = overburden_MPI::getFormationSurfaceGridMaps(
      overburden.formations(), "Permeability", getEnd());

   m_sealPressureLeakageGridMaps.setDiscontinuous(SurfaceGridMapContainer::DISCONTINUOUS_DEPTH, depthGridMaps);
   m_sealPressureLeakageGridMaps.setDiscontinuous(SurfaceGridMapContainer::DISCONTINUOUS_PERMEABILITY, permeabilityGridMaps);

   // Get the percentage maps for LithoType 1, 2 and 3. 3 and sometimes also 2 may not exist for
   // some formations:
   SurfaceGridMapContainer::constant_properties lithoType1PercentGridMaps;
   SurfaceGridMapContainer::constant_properties lithoType2PercentGridMaps;
   SurfaceGridMapContainer::constant_properties lithoType3PercentGridMaps;

   for (overburden::OverburdenFormations::formations_type::const_iterator f = overburden.formations().begin();
      f != overburden.formations().end(); ++f)
   {
      // Create the following grid map only once:
      const GridMap* litho1PercentMap = (*f)->getLithoType1PercentageMap();
      assert((*f)->getLithoType1() && litho1PercentMap);
      lithoType1PercentGridMaps.push_back(make_pair(*f, SurfaceGridMap(litho1PercentMap,
         (unsigned int)0)));

      if ((*f)->getLithoType2()) {
         // Make sure (*f)->getLithoType2PercentageMap() is consistent with (*f)->getLithoType2(). 
         // If (*f)->getLithoType2() exists, so does (*f)->getLithoType2PercentageMap():
         const GridMap* litho2PercentMap = (*f)->getLithoType2PercentageMap();
         assert(litho2PercentMap);
         lithoType2PercentGridMaps.push_back(make_pair(*f, SurfaceGridMap(litho2PercentMap,
            (unsigned int)0)));
      }

      if ((*f)->getLithoType3()) {
         // Check also for consistency of (*f)->getLithoType2PercentageMap() and (*f)->getLithoType2():
         const GridMap* litho3PercentMap = (*f)->getLithoType3PercentageMap();
         assert(litho3PercentMap);
         lithoType3PercentGridMaps.push_back(make_pair(*f, SurfaceGridMap(litho3PercentMap,
            (unsigned int)0)));
      }
   }

   // Include all percentage maps, even if they are empty:
   m_sealPressureLeakageGridMaps.setConstants(SurfaceGridMapContainer::CONSTANT_LITHOTYPE1PERCENT,
      lithoType1PercentGridMaps);
   m_sealPressureLeakageGridMaps.setConstants(SurfaceGridMapContainer::CONSTANT_LITHOTYPE2PERCENT,
      lithoType2PercentGridMaps);
   m_sealPressureLeakageGridMaps.setConstants(SurfaceGridMapContainer::CONSTANT_LITHOTYPE3PERCENT,
      lithoType3PercentGridMaps);

   return true;
}

double Reservoir::getMassStoredInColumns (void)
{
   double totalMass = 0;
   RequestHandling::StartRequestHandling (this, "getMassStoredInColumns");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
	    {
	       totalMass += column->getChargeQuantity ((PhaseId) phase);
	    }
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   return SumAll (totalMass);
}

bool Reservoir::saveComputedInputProperties (void)
{
   bool result = true;
   result &= saveComputedProperty ("ResRockTop", TOPDEPTH);
   result &= saveComputedProperty ("ResRockBottom", BOTTOMDEPTH);
   result &= saveComputedProperty ("ResRockThickness", THICKNESS);
   result &= saveComputedProperty ("ResRockPorosity", POROSITYPERCENTAGE);
   result &= saveComputedProperty ("ResRockFaultCutEdges", FAULTSTATUS);
   result &= saveComputedProperty ("ResRockBarriers", COLUMNSTATUS);
   result &= saveComputedProperty ("ResRockPressure", PRESSURE);
   result &= saveComputedProperty ("ResRockTemperature", TEMPERATURE);
   result &= saveComputedProperty ("ResRockOverburden", OVERBURDEN);

   return result;
}

bool Reservoir::saveComputedOutputProperties (void)
{
   bool result = true;

   result &= saveComputedProperty ("ResRockTrapId", GLOBALTRAPID);
   result &= saveComputedProperty ("ResRockLeakage", LEAKAGEQUANTITY);
   result &= saveComputedProperty ("ResRockCapacity", CAPACITY);
#ifdef USEOTGC
   result &= saveComputedProperty ("ResRockImmobilesDensity", IMMOBILESDENSITY);
#endif
   result &= saveComputedProperty ("ResRockFlux", FLUX);
   result &= saveComputedProperty ("ResRockFlow", FLOW);

   int phase;
   for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      string propName = "ResRockDrainageId";
      propName += PhaseNames[phase];

      result &= saveComputedProperty (propName + "Phase", DRAINAGEAREAID, (PhaseId) phase);
   }

   for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      string propName = "ResRockFill";
      propName += PhaseNames[phase];

      result &= saveComputedProperty (propName + "PhaseDensity", LATERALCHARGEDENSITY, (PhaseId) phase);
      result &= saveComputedProperty (propName + "PhaseQuantity", CHARGEQUANTITY, (PhaseId) phase);
   }

   return result;
}

bool Reservoir::saveComputedProperty (const string & name, ValueSpec valueSpec, PhaseId phase)
{
   RequestHandling::StartRequestHandling (this, "saveComputedProperty");

   PropertyValue * propertyValue = getProjectHandle ()->createMapPropertyValue (name, getEnd (), this, 0, 0);
   assert (propertyValue);

   GridMap * gridMap = propertyValue->getGridMap ();
   assert (gridMap);

   gridMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 if (IsValid (getLocalColumn (i, j)))
	 {
	    gridMap->setValue (i, j, getLocalColumn (i, j)->getValue (valueSpec, phase));
	 }
	 else
	 {
	    gridMap->setValue (i, j, gridMap->getUndefinedValue ());
	 }
      }
   }
   gridMap->restoreData ();

   // do not delete gridMap, it is our (temporary) storageDevice !!!
   RequestHandling::FinishRequestHandling ();
   return true;
}

bool Reservoir::computePorosities (void)
{
   const GridMap * gridMap = getPropertyGridMap ("Porosity", getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth ();
   assert (depth > 1);

   gridMap->retrieveData ();
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	 index = Max ((double) 0, index);
	 index = Min ((double) depth - 1, index);

	 column->setPorosity (gridMap->getValue (i, j, index) * Percentage2Fraction);
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computePermeabilities (void)
{
   const GridMap * gridMap = getPropertyGridMap ("Permeability", getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth ();
   assert (depth > 1);

   gridMap->retrieveData ();
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	 index = Max ((double) 0, index);
	 index = Min ((double) depth - 1, index);

	 column->setPermeability (gridMap->getValue (i, j, index));
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computeTemperatures (void)
{
   const GridMap * gridMap = getPropertyGridMap ("Temperature", getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth ();
   assert (depth > 1);

   gridMap->retrieveData ();
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	 index = Max ((double) 0, index);
	 index = Min ((double) depth - 1, index);

	 column->setTemperature (gridMap->getValue (i, j, index));
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computePressures (void)
{
   const GridMap *gridMap = getPropertyGridMap ("Pressure", getEnd ());

   if (gridMap)
   {
      unsigned int depth = gridMap->getDepth ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn *column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);

            index = Max ((double) 0, index);
            index = Min ((double) depth - 1, index);

            column->setPressure (gridMap->getValue (i, j, index));
         }
      }
      gridMap->restoreData ();
   }
   else
   {
      if (GetRank () == 0)
      {
	 cerr << "WARNING: 3D property 'Pressure' does not exist for Formation "
	    << getFormation ()->getName () << " at snapshot " << getEnd ()->getTime ()
	    << "," << endl
	    << "\t using fallback values" << endl;
	 cerr.flush ();
      }

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn *column = getLocalColumn (i, j);
            double hydrostaticPressure = column->getTopDepth () * Gravity * WaterDensity * Pa2MPa;

            column->setPressure (hydrostaticPressure);
         }
      }
   }
   return true;
}

bool Reservoir::computeHydrostaticPressures (void)
{
   const GridMap * gridMap = getPropertyGridMap ("HydroStaticPressure", getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth ();
   assert (depth > 1);

   gridMap->retrieveData ();
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	 index = Max ((double) 0, index);
	 index = Min ((double) depth - 1, index);

	 column->setHydrostaticPressure (gridMap->getValue (i, j, index));
      }
   }
   gridMap->restoreData ();

   return true;
}

bool Reservoir::computeLithostaticPressures (void)
{
   const GridMap * gridMap = getPropertyGridMap ("LithoStaticPressure", getEnd ());
   if (!gridMap) return false;

   unsigned int depth = gridMap->getDepth ();
   assert (depth > 1);

   gridMap->retrieveData ();
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
	 index = Max ((double) 0, index);
	 index = Min ((double) depth - 1, index);

	 column->setLithostaticPressure (gridMap->getValue (i, j, index));
      }
   }
   gridMap->restoreData ();

   return true;
}

/// Any zero-thickness column is to become wasting
/// Any zero thickness neighbour of a non-zero-thickness column is to become sealing.
/// Also includes the faults into the geometry
bool Reservoir::refineGeometry (void)
{
   RequestHandling::StartRequestHandling (this, "refineGeometry");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    if (column->getThickness () < MinimumThickness)
	    {
	       column->setWasting (GAS);
	       column->setWasting (OIL);

	       for (int n = 0; n < NumNeighbours; ++n)
	       {
		  Column * neighbourColumn = getColumn (i + NeighbourOffsets[n][I], j + NeighbourOffsets[n][J]);
		  if (IsValid (neighbourColumn) && neighbourColumn->getThickness () >= MinimumThickness)
		  {
		     column->setSealing (GAS);
		     column->setSealing (OIL);
		     break;
		  }
	       }
	       column->resetProxies ();
	    }
	    if (column->getFaultStatus () != NOFAULT)
	    {
	       switch (column->getFaultStatus ())
	       {
		  case SEAL:
		     column->setSealing (GAS);
		     column->setSealing (OIL);
		     break;
		  case PASS:
		     break;
		  case WASTE:
		     column->setWasting (GAS);
		     column->setWasting (OIL);
		     break;
		  case SEALOIL:
		     column->setWasting (GAS);
		     column->setSealing (OIL);
		     break;
		  case PASSOIL:
		     column->setWasting (GAS);
		     break;
		  default:
		     assert (false);
	       }
	       column->resetProxies ();
	    }
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();
   return true;
}

void Reservoir::setSourceFormation (const Formation * formation)
{
   m_sourceFormation = formation;
}

const Formation * Reservoir::getSourceFormation (void)
{
   return m_sourceFormation;
}

void Reservoir::setSourceReservoir (const Reservoir * reservoir)
{
   m_sourceReservoir = reservoir;
}

const Reservoir * Reservoir::getSourceReservoir (void)
{
   return m_sourceReservoir;
}

const GridMap * Reservoir::getSeaBottomDepthMap (const Snapshot * snapshot) const
{
   string propertyName;
   if (lowResEqualsHighRes ())
      propertyName = "Depth";
   else
      propertyName = "DepthHighRes";

   FormationList * formations = m_projectHandle->getFormations (snapshot);
   assert (formations);
   assert (formations->size () > 0);

   Interface::FormationList::iterator formationIter;
   const GridMap * gridMap = 0;
   // continue downward until a gridMap is returned
   for (formationIter = formations->begin (); gridMap == 0 && formationIter != formations->end (); ++formationIter)
   {
      const Formation * topFormation = dynamic_cast<const Formation *>( * formationIter);

      gridMap = getPropertyGridMap (propertyName, snapshot, 0, 0, topFormation->getTopSurface ());
   }

   delete formations;
   return gridMap;
}

const GridMap * Reservoir::getSeaBottomPressureMap (const Snapshot * snapshot) const
{
   string propertyName = "HydroStaticPressure";

   FormationList * formations = m_projectHandle->getFormations (snapshot);
   assert (formations);
   assert (formations->size () > 0);

   Interface::FormationList::iterator formationIter;
   const GridMap * gridMap = 0;
   // continue downward until a gridMap is returned
   for (formationIter = formations->begin (); gridMap == 0 && formationIter != formations->end (); ++formationIter)
   {
      const Formation * topFormation = dynamic_cast<const Formation *>( * formationIter );

      gridMap = getPropertyGridMap (propertyName, snapshot, 0, 0, topFormation->getTopSurface ());
   }

   delete formations;
   return gridMap;
}

const GridMap * Reservoir::getPropertyGridMap (const string & propertyName,
      const Snapshot * snapshot) const
{
   return getVolumePropertyGridMap ( dynamic_cast<const Formation *>( getFormation ()), propertyName, snapshot);
}

const GridMap * Reservoir::getVolumePropertyGridMap (const Formation * formation, const string & propertyName,
      const Snapshot * snapshot) const
{
   int selectionFlags = Interface::FORMATION;

   PropertyValueList * propertyValues =
      m_projectHandle->getPropertyValues (selectionFlags,
	    m_projectHandle->findProperty (propertyName),
	    snapshot, 0, formation, 0,
	    Interface::VOLUME);

   if (propertyValues->size () != 1)
   {
      return 0;
   }

   const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}

const GridMap * Reservoir::getPropertyGridMap (const string & propertyName,
      const Snapshot * snapshot,
      const Interface::Reservoir * reservoir, const Interface::Formation * formation, const Interface::Surface * surface) const
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation && !surface) selectionFlags |= Interface::FORMATION;
   if (surface && !formation) selectionFlags |= Interface::SURFACE;
   if (formation && surface) selectionFlags |= Interface::FORMATIONSURFACE;

   PropertyValueList * propertyValues =
      m_projectHandle->getPropertyValues (selectionFlags,
	    m_projectHandle->findProperty (propertyName),
	    snapshot, reservoir, formation, surface,
	    Interface::MAP);

   if (propertyValues->size () != 1)
   {
      delete propertyValues;
      return 0;
   }

   const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}

const GridMap * Reservoir::getReservoirPropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   return getPropertyGridMap (propertyName, snapshot, this, 0, 0);
}

const GridMap * Reservoir::getFormationPropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   return getPropertyGridMap (propertyName, snapshot, 0, getFormation (), 0);
}

const GridMap * Reservoir::getTopSurfacePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   const Formation * formation;
   const GridMap * formationTopMap = 0;

   for (formation = dynamic_cast<const Formation *>( getFormation ());
	 formationTopMap == 0 && formation != 0;
	 formation = formation->getTopFormation ())
   {
      formationTopMap = getPropertyGridMap (propertyName, snapshot, 0, 0, formation->getTopSurface ());
   }

   return formationTopMap;
}

const GridMap * Reservoir::getBottomSurfacePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   const Formation * formation;
   const GridMap * formationBottomMap = 0;

   for (formation = dynamic_cast<Formation *> ( const_cast<Interface::Formation*>(getFormation ()));
	 !formationBottomMap && formation;
	 formation = formation->getBottomFormation ())
   {
      formationBottomMap = getPropertyGridMap (propertyName, snapshot, 0, 0, formation->getBottomSurface ());
   }

   return formationBottomMap;
}

const GridMap * Reservoir::getTopFormationSurfacePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   return getPropertyGridMap (propertyName, snapshot, 0, getFormation (), getFormation ()->getTopSurface ());
}

const GridMap * Reservoir::getBottomFormationSurfacePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
{
   return getPropertyGridMap (propertyName, snapshot, 0, getFormation (), getFormation ()->getBottomSurface ());
}

const Grid * Reservoir::getGrid (void) const
{
   return getProjectHandle ()->getHighResolutionOutputGrid (); // not to be changed to getActivityOutputGrid ()!!
}

double Reservoir::getNeighbourDistance (int n)
{
   double & distance = m_neighbourDistances[n];
   if (distance < 0)
   {
      double deltaI = Abs (NeighbourOffsets[n][0]) * getGrid ()->deltaIGlobal ();
      double deltaJ = Abs (NeighbourOffsets[n][1]) * getGrid ()->deltaJGlobal ();

      if (deltaI == 0)
      {
            distance = deltaJ;
      }
      else if (deltaJ == 0)
      {
	 distance = deltaI;
      }
      else
      {
	 distance = sqrt (Square (deltaI) + Square (deltaJ));
      }
   }

   return distance;
}

LocalColumn * Reservoir::getLocalColumn (unsigned int i, unsigned int j)
{
   return m_columnArray->getLocalColumn (i, j);
}

ProxyColumn * Reservoir::getProxyColumn (unsigned int i, unsigned int j)
{
   return m_columnArray->getProxyColumn (i, j);
}

Column * Reservoir::getColumn (unsigned int i, unsigned int j)
{
   return m_columnArray->getColumn (i, j);
}

void Reservoir::accumulateErrorInPVT (double error)
{
   m_lossPVT += error;
   m_errorPVT += Abs (error);
}

double Reservoir::getErrorPVT (void)
{
   return m_errorPVT;
}

double Reservoir::getLossPVT (void)
{
   return m_lossPVT;
}

double Reservoir::getAverageDepth (void)
{
   if (m_averageDepth == Interface::DefaultUndefinedScalarValue)
   {
      determineAverageDepth ();
   }
   return m_averageDepth;
}

bool Reservoir::determineAverageDepth (void)
{
   const GridMap * gridMap = getTopSurfacePropertyGridMap ("Depth", m_projectHandle->findSnapshot (0));
   if (!gridMap) return false;

   gridMap->retrieveData ();
   double average = gridMap->getAverageValue ();
   gridMap->restoreData ();

   setAverageDepth (average);

   return true;
}

/// See whether distribution has finished on all processors
bool Reservoir::allProcessorsFinished (bool finished)
{
   return (bool) AndAll ((int) finished);
}

int Reservoir::computeMaximumTrapCount (bool countUndersized)
{
   int numberOfTraps = m_traps.size ();
   if (!countUndersized)
   {
      TrapVector::iterator trapIter;
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
	 if ((*trapIter)->isUndersized ())
	    --numberOfTraps;
      }
   }
   return MaximumAll (numberOfTraps);
}

bool Reservoir::computeDepthOffsets (const Snapshot * presentDay)
{
   string depthPropertyName;

   if (lowResEqualsHighRes ())
   {
      depthPropertyName = "Depth";
   }
   else
   {
      depthPropertyName = "DepthHighRes";
   }

   const GridMap * formationTopDepthMap = getTopSurfacePropertyGridMap (depthPropertyName, presentDay);
   const GridMap * formationBottomDepthMap = getBottomSurfacePropertyGridMap (depthPropertyName, presentDay);

   if (!formationTopDepthMap)
   {
      if (GetRank () == 0)
      {
	 cerr << "WARNING: property value '" << depthPropertyName << "' does not exist for top surface of formation "
	    << getFormation ()->getName () << " at snapshot " << presentDay->getTime ()  <<
	    ",\n\tcannot compute depth offsets for reservoir " << getName () << endl;
	 cerr.flush ();
      }
      return false;
   }

   if (!formationBottomDepthMap)
   {
      if (GetRank () == 0)
      {
	 cerr << "WARNING: property value '" << depthPropertyName << "' does not exist for bottom surface of formation "
	    << getFormation ()->getName () << " at snapshot " << presentDay->getTime ()  <<
	    ",\n\tcannot compute depth offsets for reservoir " << getName () << endl;
	 cerr.flush ();
      }
      return false;
   }

   const GridMap * depthOffsetMap = getMap (Interface::DepthOffset); // may be 0

   const GridMap * thicknessMap = getMap (Interface::ReservoirThickness); // may be 0

   formationTopDepthMap->retrieveData ();
   formationBottomDepthMap->retrieveData ();
   if (depthOffsetMap) depthOffsetMap->retrieveData ();
   if (thicknessMap) thicknessMap->retrieveData ();


   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);

	 double formationTopDepth = formationTopDepthMap->getValue (i, j);
	 double formationBottomDepth = formationBottomDepthMap->getValue (i, j);

	 if (formationTopDepth == formationTopDepthMap->getUndefinedValue () || 
	       formationBottomDepth == formationBottomDepthMap->getUndefinedValue ())
	 {
	    // use default values 0, 0
	    continue;
	 }

	 double formationThickness = formationBottomDepth - formationTopDepth;
	 formationThickness = Max (0.001, formationThickness);

	 double depthOffset = 0;
	 if (depthOffsetMap)
	 {
	    depthOffset = depthOffsetMap->getValue (i, j);
	    if (depthOffset == depthOffsetMap->getUndefinedValue ())
	    {
	       depthOffset = 0;
	    }
	 }

	 double thickness = formationThickness;
	 if (thicknessMap)
	 {
	    thickness = thicknessMap->getValue (i, j);
	    if (thickness == thicknessMap->getUndefinedValue ())
	    {
	       thickness = formationThickness;
	    }
	 }

	 column->setTopDepthOffset (depthOffset / formationThickness);
	 column->setBottomDepthOffset ((formationThickness - (depthOffset + thickness)) / formationThickness);
      }
   }

   formationTopDepthMap->restoreData ();
   formationBottomDepthMap->restoreData ();
   if (depthOffsetMap) depthOffsetMap->restoreData ();
   if (thicknessMap) thicknessMap->restoreData ();

   delete formationTopDepthMap;
   delete formationBottomDepthMap;
   if (depthOffsetMap) delete depthOffsetMap;
   if (thicknessMap) delete thicknessMap;

   return true;
}

bool Reservoir::computeNetToGross (void)
{
   const GridMap * netToGrossMap = getMap (Interface::NetToGross); // may be 0

   if (netToGrossMap) netToGrossMap->retrieveData ();

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);

	 double netToGross = 100.0;
	 if (netToGrossMap)
	 {
	    netToGross = netToGrossMap->getValue (i, j);
	    if (netToGross == netToGrossMap->getUndefinedValue ())
	    {
	       netToGross = 100.0;
	    }
	 }

	 column->setNetToGross (netToGross / 100.0);
      }
   }

   if (netToGrossMap) netToGrossMap->restoreData ();

   return true;
}

bool Reservoir::isActive (const Snapshot * snapshot) const
{
   if (getActivityMode () == "NeverActive") return false; // not active anyway

   const Formation * formation = dynamic_cast<const Formation *>( getFormation ());
   const Surface * topSurface = (const Surface *) formation->getTopSurface ();
   const Snapshot * depoSnapshot = topSurface->getSnapshot ();

   if (depoSnapshot->getTime () < snapshot->getTime ()) return false; // not active anyway

   if (getActivityMode () == "AlwaysActive") return true;

   if (getActivityMode () == "ActiveFrom")
   {
      const Snapshot * activeFromSnapshot = m_projectHandle->findSnapshot (getActivityStart ());
      return (activeFromSnapshot->getTime () >= snapshot->getTime ());
   }

   if (GetRank () == 0)
   {
      cerr << "ERROR: Undefined activity mode '" << getActivityMode () << "' for Reservoir " << getName () << endl;
      cerr.flush ();
   }
   return true;
}

#ifdef USEOTGC
bool Reservoir::crackChargesToBeMigrated (OilToGasCracker & otgc)
{
   if (!isOilToGasCrackingOn()) return false;

   setSourceReservoir (this);

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 column->crackChargesToBeMigrated (otgc, m_start->getTime(), m_end->getTime());
      }
   }

   processMigrationRequests ();
   setSourceReservoir (0);
   return true;
}
#endif

bool Reservoir::collectExpelledCharges (const Formation * formation,
      unsigned int direction, Barrier * barrier)
{

   if (direction == EXPELLEDNONE) return true;

   for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
   {
      if (!ComponentsUsed[componentId]) continue;

      string propertyName = ComponentNames[componentId];
      propertyName += "ExpelledCumulative";

      const GridMap * gridMapEnd = getPropertyGridMap (propertyName, getEnd (), 0, formation, 0);
      const GridMap * gridMapStart = getPropertyGridMap (propertyName, getStart (), 0, formation, 0);

      double fraction = (direction == EXPELLEDUPANDDOWNWARD ? 1.0 : 0.5);

      if (gridMapEnd)
      {
	 gridMapEnd->retrieveData ();
	 addChargesToBeMigrated ((ComponentId) componentId, gridMapEnd, fraction, barrier);
	 gridMapEnd->restoreData ();
      }

      if (gridMapStart)
      {
	 gridMapStart->retrieveData ();
	 subtractChargesToBeMigrated ((ComponentId) componentId, gridMapStart, fraction, barrier);
	 gridMapStart->restoreData ();
      }

   }

   return true;
}

void Reservoir::deleteExpelledChargeMaps (const Formation * formation)
{
   for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
   {
      if (!ComponentsUsed[componentId]) continue;

      string propertyName = ComponentNames[componentId];
      propertyName += "ExpelledCumulative";

      const GridMap * gridMapStart = getPropertyGridMap (propertyName, getStart (), 0, formation, 0);

      if (gridMapStart) delete gridMapStart;
   }
}

bool Reservoir::collectLeakedCharges (Reservoir * leakingReservoir, Barrier * barrier)
{
   if (!leakingReservoir)
      return true;

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 LocalColumn * leakingColumn = leakingReservoir->getLocalColumn (i, j);
	 if (IsValid (column) && IsValid (leakingColumn) &&
	       !leakingColumn->isOnBoundary ())
	 {
	    if (barrier && barrier->isBlocking (i, j))
	       addBlocked (leakingColumn->getComposition ());
	    else
	       column->addCompositionToBeMigrated (leakingColumn->getComposition ());
	 }
      }
   }
   return true;
}

bool Reservoir::migrateChargesToBeMigrated (const Formation * srcFormation, const Reservoir * srcReservoir)
{
   setSourceFormation (srcFormation);
   setSourceReservoir (srcReservoir);

   RequestHandling::StartRequestHandling (this, "migrateChargesToBeMigrated");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    column->migrateChargesToBeMigrated ();
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   processMigrationRequests ();

   setSourceFormation (0);
   setSourceReservoir (0);


   return true;
}

double Reservoir::getTotalToBeStoredCharges (bool onBoundaryOnly)
{
   double total = 0;

   RequestHandling::StartRequestHandling (this, "getTotalToBeStoredCharges");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column) && (!onBoundaryOnly || column->isOnBoundary ()))
	 {
	    total += column->getCompositionWeight ();
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   return SumAll (total);
}

double Reservoir::getTotalChargesToBeMigrated (void)
{
   double total = 0;

   RequestHandling::StartRequestHandling (this, "getTotalChargesToBeMigrated");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    total += column->getWeightToBeMigrated ();
	 }
      }
   }

   RequestHandling::FinishRequestHandling ();

   return SumAll (total);
}

#ifdef USEOTGC
double Reservoir::getTotalImmobiles (void)
{
   double total = 0;

   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 total += column->getImmobilesWeight ();
      }
   }

   return SumAll (total);
}
#endif

int Reservoir::getTotalNumberOfProxyColumns (void)
{
   return SumAll (getNumberOfProxyColumns ());
}

int Reservoir::getNumberOfProxyColumns (void)
{
   return m_columnArray->getNumberOfProxyColumns ();
}

void Reservoir::printInconsistentTrapVolumes (void)
{
   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (*trapIter)->printInconsistentVolumes ();
   }
}

double Reservoir::getTotalStoredCharges (void)
{
   double total = 0;

   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      total += (*trapIter)->getWeight ();
   }

   return SumAll (total);
}

double Reservoir::getTotalSealPressureLeakedCharges (void)
{
   double total = 0;

   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      total += (*trapIter)->getSealPressureLeakages ();
   }

   return SumAll (total);
}

double Reservoir::getTotalBiodegradedCharges (void)
{
   double total = m_biodegraded;

   return SumAll (total);
}

double Reservoir::getTotalDiffusionLeakedCharges (void)
{
   double total = 0;

   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      total += (*trapIter)->getDiffusionLeakages ();
   }

   return SumAll (total);
}

double Reservoir::getTotalToBeDistributedCharges (void)
{
   double total = 0;

   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      total += (*trapIter)->getWeightToBeDistributed ();
   }

   return SumAll (total);
}

/// perform the filling and spilling process.
bool Reservoir::fillAndSpill ()
{

   setSourceReservoir (this);

   if (!computeDistributionParameters())
      return false;

   m_biodegraded = 0;
   if (isBioDegradationOn())
   {
      m_biodegraded = biodegradeCharges ();
   }

   // bool distributionFinished = false;

   do
   {
#ifndef MOVEDTODISTRIBUTECHARGES
      collectAndSplitCharges ();
#endif

      distributeCharges ();

#if 0
      checkDistribution ();
#endif
      mergeSpillingTraps ();
      processMigrationRequests ();
   } while (!allProcessorsFinished (distributionHasFinished ()));

   if (isDiffusionOn())
   {
      broadcastTrapFillDepthProperties ();
      if (!diffusionLeakCharges ())
      {
         return false;
      }

      // To get the fill heights, etc. correct again!
      collectAndSplitCharges (true);
      distributeCharges ();
   }

   reportLeakages ();

   processMigrationRequests ();
   setSourceReservoir (0);
   return true;
}

/// See whether distribution of charge has finished on this processor.
bool Reservoir::distributionHasFinished (void)
{
   RequestHandling::StartRequestHandling (this, "distributionHasFinished");

   bool value = true;
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      if ((*trapIter)->requiresDistribution ())
      {
	 value = false;
	 break;
      }
   }

   RequestHandling::FinishRequestHandling ();
   return value;
}

void Reservoir::reportLeakages ()
{
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (*trapIter)->reportLeakage ();
   }
}

bool Reservoir::computeDistributionParameters ()
{
   RequestHandle requestHandle (this, "computeDistributionParameters");

   const FracturePressureFunctionParameters *fracturePressureFunctionParameters =
         getProjectHandle ()->getFracturePressureFunctionParameters ();

   {
      RetrieveAndRestoreSurfaceGridMapContainer retrieve (m_sealPressureLeakageGridMaps);

      for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         if (!(*trapIter)->computeDistributionParameters (fracturePressureFunctionParameters,
                                                          m_sealPressureLeakageGridMaps, m_end))
            return false;
      }

   }                            // ~retrieve

   return true;
}

double Reservoir::biodegradeCharges ()
{
   RequestHandling::StartRequestHandling (this, "biodegradeCharges");

   double biodegraded = 0;

   const BiodegradationParameters* biodegradationParameters = 
      getProjectHandle()->getBiodegradationParameters();
   double timeInterval = m_start->getTime() - m_end->getTime();
   Biodegrade biodegrade(biodegradationParameters);

   for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
     biodegraded += (*trapIter)->biodegradeCharges(timeInterval, biodegrade);
   }

   RequestHandling::FinishRequestHandling ();
   return biodegraded;
}

bool Reservoir::isDiffusionOn (void)
{
   static int diffusionOn = -1;

   bool diffusionTurnedOn = Interface::Reservoir::isDiffusionOn ();

   if (diffusionTurnedOn && diffusionOn < 0)
   {
      char *envStr = getenv ("Diffusion_On");
      if (envStr)
      {
	 diffusionOn = 1;
      }
      else
      {
	 if (GetRank () == 0)
	 {
	    cerr << "WARNING: Diffusion currently disabled" << endl;
	 }
	 diffusionOn = 0;
      }
   }
   return (diffusionOn && diffusionTurnedOn);
}

bool Reservoir::diffusionLeakCharges ()
{
   RequestHandle requestHandle (this, "diffusionLeakCharges");

   // maximum error in the flux (kg/m2/Ma)
   double maxFluxError = 0.05;
   char *envStr = getenv ("Diffusion_MaxFluxError");

   if (envStr && strlen (envStr) != 0)
      maxFluxError = atof (envStr);

   // maximum timestep
   double maxTimeStep = 50;

   envStr = getenv ("Diffusion_MaxTimeStep");
   if (envStr && strlen (envStr) != 0)
      maxTimeStep = atof (envStr);

   const DiffusionLeakageParameters *parameters = getProjectHandle ()->getDiffusionLeakageParameters ();

   RetrieveAndRestoreSurfaceGridMapContainer retrieve (m_diffusionOverburdenGridMaps);

   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      if (!(*trapIter)->computeDiffusionOverburden (m_diffusionOverburdenGridMaps, getEnd (),
                                                    parameters->maximumSealThickness (),
                                                    numeric_limits < int >::max ()))
             return false;
   }

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (*trapIter)->diffusionLeakCharges (m_start->getTime (), m_end->getTime (), getProjectHandle ()->
                                         getDiffusionLeakageParameters (), maxTimeStep, maxFluxError);
   }

   return true;
}

bool Reservoir::distributeCharges ()
{
#if 0
   cerrstrstr << GetRankString () << ": " << getName () << "::distributeCharges ()" << endl;
#endif

   bool distributionFinished = true;
   RequestHandling::StartRequestHandling (this, "distributeCharges");

   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
     distributionFinished &= (*trapIter)->distributeCharges ();
   }

   RequestHandling::FinishRequestHandling ();

   return distributionFinished;
}

void Reservoir::incrementChargeDistributionCount (void)
{
   ++m_chargeDistributionCount;
}

void Reservoir::reportChargeDistributionCount (void)
{
   long globalChargeDistributionCount = SumAll (m_chargeDistributionCount);

   if (GetRank () == 0)
   {
      cout << getName () << ": # charge distributions (0) = " << m_chargeDistributionCount << endl;
      cout << getName () << ": # charge distributions (all) = " << globalChargeDistributionCount << endl;
   }
}

#ifdef MERGEUNDERSIZEDTRAPSAPRIORI
static bool TrapIsUndersized (Column * columnA, Column * columnB)
{
   return (columnA->isUndersized () || columnB->isUndersized ());
}

/// Try to merge traps that are too small with other traps to form traps that are not too small
/// No longer used as undersized traps should not fill but rather spill everything.
/// Once two traps of any size are spilling to each other, they will merge anyway.
/// We are keeping this function to remind us not to use it.
///
/// Decided to use it anyway to display 'large' but unfilled traps constructed from smaller, undersized ones which
/// would otherwise be obscured.
bool Reservoir::mergeUndersizedTraps (void)
{
   bool noTrapsWereMerged;
   do
   {
      noTrapsWereMerged = determineTrapsToMerge (TrapIsUndersized);
      absorbTraps ();
      completeTrapExtensions ();
   } while (!allProcessorsFinished (noTrapsWereMerged));

   return true;
}
#endif

/// determine whether and identify traps that need to merge
/// returns true if no traps needed to merge
bool Reservoir::determineTrapsToMerge (ConditionTest conditionTest)
{
   bool noTrapsToMerge = true;

   RequestHandling::StartRequestHandling (this, "determineTrapsToMerge");
   for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      LocalColumn * crestColumn = trap->getCrestColumn ();
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
	 Column * spillBackColumn;
	 if (((spillBackColumn = crestColumn->getSpillBackTarget ((PhaseId) phase)) != 0) &&
	       (* conditionTest) (crestColumn, spillBackColumn))
	 {
	    noTrapsToMerge = false;

	    assert (spillBackColumn != crestColumn);

	    if (crestColumn->isShallowerThan (spillBackColumn))
	    {
	       Column * trapSpillColumn = trap->getSpillColumn ();
	       assert (trapSpillColumn);

	       trap->moveBackToCrestColumn ();
	       trap->extendWith (spillBackColumn, trapSpillColumn->getTopDepth ());
	    }
	    else
	    {
	       // trap is absorbed
	       trap->migrateTo (spillBackColumn);
	       trap->setToBeAbsorbed ();
	    }
	    break;
	 }
      }
   }
   RequestHandling::FinishRequestHandling ();

   return noTrapsToMerge;
}

static bool TrapsAreSpilling (Column * columnA, Column * columnB)
{
   return (columnA->isSpilling () && columnB->isSpilling ());
}

/// First we determine which traps need to merge into which and move the charge of the traps to be absorbed into the absorbing traps.
/// Next the traps to be absorbed extend the absorbing traps with their columns and are subsequently destroyed.
/// Finally the areas and other properties of the absorbing traps are re-computed.
void Reservoir::mergeSpillingTraps (void)
{
   determineTrapsToMerge (TrapsAreSpilling);
   absorbTraps ();
   completeTrapExtensions ();
}

/// Transfer interiors of traps to be absorbed to the absorbing traps.
void Reservoir::absorbTraps (void)
{
   RequestHandling::StartRequestHandling (this, "absorbTraps");
   TrapVector::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      if (trap->isToBeAbsorbed ())
      {
	 trap->beAbsorbed ();

	 delete trap;
	 trapIter = m_traps.erase (trapIter);
	 --trapIter;
      }
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::completeTrapExtensions (void)
{
   RequestHandling::StartRequestHandling (this, "completeTrapExtensions");
   for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      if (trap->hasBeenExtended ())
      {
	 trap->completeExtension ();
      }
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::broadcastTrapDiffusionStartTimes (void)
{
   if (!isDiffusionOn ()) return;

   TrapVector::iterator trapIter;

   RequestHandling::StartRequestHandling (this, "broadcastTrapDiffusionStartTimes");
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      trap->broadcastDiffusionStartTimes ();
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::broadcastTrapPenetrationDistances (void)
{
   if (!isDiffusionOn ()) return;

   const DiffusionLeakageParameters *parameters = getProjectHandle ()->getDiffusionLeakageParameters ();

   if (parameters->transientModel() != Interface::Transient) return;

   TrapVector::iterator trapIter;

   RequestHandling::StartRequestHandling (this, "broadcastTrapPenetrationDistances");
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      trap->broadcastPenetrationDistances ();
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::broadcastTrapFillDepthProperties (void)
{
   TrapVector::iterator trapIter;

   RequestHandling::StartRequestHandling (this, "broadcastTrapFillDepthProperties");
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      trap->broadcastFillDepthProperties ();
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::broadcastTrapChargeProperties (void)
{
   TrapVector::iterator trapIter;

   RequestHandling::StartRequestHandling (this, "broadcastTrapChargeProperties");
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      trap->broadcastChargeProperties ();
   }
   RequestHandling::FinishRequestHandling ();
}

void Reservoir::collectAndSplitCharges (bool always)
{
   RequestHandling::StartRequestHandling (this, "collectAndSplitCharges");
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      (*trapIter)->collectAndSplitCharges (always);
   }
   RequestHandling::FinishRequestHandling ();
}

/// debugging function
bool Reservoir::checkDistribution (void)
{
   bool result = true;
   RequestHandling::StartRequestHandling (this, "checkDistribution");
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
	    {
	       if (column->getFinalTargetColumn ((PhaseId) phase) != column)
	       {
		  if (column->containsComposition ())
		  {
		     cerr << GetRankString () << ":: ERROR: " << column << " contains charge ("
			<< column->getCompositionWeight () << ") but shouldn't" << endl;
		     cerr.flush ();
		     result = false;
		  }
	       }
	    }
	 }
      }
   }
   RequestHandling::FinishRequestHandling ();
   return result;
}

/// add charges to the reservoir
bool Reservoir::addChargesToBeMigrated (ComponentId componentId, const GridMap * gridMap, double fraction, Barrier * barrier)
{
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (gridMap->getValue (i, j) != gridMap->getUndefinedValue ())
	 {
	    if (IsValid (column))
	    {
	       if (barrier && barrier->isBlocking (i, j))
		  addBlocked (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
	       else
		  column->addComponentToBeMigrated (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
	    }
	 }
      }
   }
   return true;
}

/// subtract charges from the reservoir
bool Reservoir::subtractChargesToBeMigrated (ComponentId componentId, const GridMap * gridMap, double fraction, Barrier * barrier)
{
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column) && gridMap->getValue (i, j) != gridMap->getUndefinedValue ())
	 {
	    if (barrier && barrier->isBlocking (i, j))
	       subtractBlocked (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
	    else
	       column->subtractComponentToBeMigrated (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
	 }
      }
   }
   return true;
}

bool Reservoir::checkChargesToBeMigrated (ComponentId componentId)
{
   bool result = true;
   for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
   {
      for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
      {
	 LocalColumn * column = getLocalColumn (i, j);
	 if (IsValid (column))
	 {
	    if (column->getComponentToBeMigrated (componentId) < 0)
	    {
	       cerr << "Error in " << column << " of Reservoir " << getName () << " at age " << getEnd ()->getTime ()
		  << ": expelled weight of " << ComponentNames[componentId] << " is negative (" << column->getComponent (componentId) << ")" << endl;
	       result = false;
	    }
	 }
      }
   }
   return result;
}

void Reservoir::addBlocked (ComponentId componentId, double mass)
{
   m_compositionBlocked.add (componentId, mass);
}

void Reservoir::subtractBlocked (ComponentId componentId, double mass)
{
   m_compositionBlocked.subtract (componentId, mass);
}

void Reservoir::addBlocked (const Composition & composition)
{
   m_compositionBlocked.add (composition);
}

void Reservoir::subtractBlocked (const Composition & composition)
{
   m_compositionBlocked.subtract (composition);
}

const Composition & Reservoir::getBlocked (void) const
{
   return m_compositionBlocked;
}

double Reservoir::getTotalBlocked (void) const
{
   return SumAll (getBlocked ().getWeight ());
}

void Reservoir::addToCrackingLoss (const Composition & composition)
{
   m_crackingLoss.add (composition);
}

void Reservoir::addToCrackingGain (const Composition & composition)
{
   m_crackingGain.add (composition);
}

const Composition & Reservoir::getCrackingLoss (void) const
{
   return m_crackingLoss;
}

double Reservoir::getTotalCrackingLoss (void) const
{
   return SumAll (getCrackingLoss ().getWeight ());
}

const Composition & Reservoir::getCrackingGain (void) const
{
   return m_crackingGain;
}

double Reservoir::getTotalCrackingGain (void) const
{
   return SumAll (getCrackingGain ().getWeight ());
}


Trap * Reservoir::findTrap (int globalId)
{
   TrapVector::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
      if (trap->getGlobalId () == globalId)
	 return trap;
   }
   return 0;
}

void Reservoir::addTrap (Trap * trap)
{
   m_traps.push_back (trap);
   trap->setLocalId (m_traps.size ());
}

void Reservoir::processTrapProperties (TrapPropertiesRequest & tpRequest)
{
   assert (tpRequest.valueSpec == SAVETRAPPROPERTIES);
   ((Migrator *) m_projectHandle)->addTrapRecord (this, tpRequest);
}

double Reservoir::getUndefinedValue (void)
{
   return Interface::DefaultUndefinedMapValue;
}

#ifdef __cplusplus
extern "C" {
#endif
static int TrapPropertiesCompare (const void * elem1, const void * elem2 );
#ifdef __cplusplus
}
#endif

static int TrapPropertiesCompare (const void * elem1, const void * elem2 )
{
   const TrapPropertiesRequest * tpr1 = (const TrapPropertiesRequest *) elem1;
   const TrapPropertiesRequest * tpr2 = (const TrapPropertiesRequest *) elem2;
   double weight1 = tpr1->composition.getWeight ();
   double weight2 = tpr2->composition.getWeight ();

   if (tpr1->rank < 0 && tpr2->rank >= 0) return 1;
   if (tpr1->rank >= 0 && tpr2->rank < 0) return -1;
   if (tpr1->rank < 0 && tpr2->rank < 0) return 0;

   if (weight1 > weight2) return -1;
   else if (weight1 < weight2) return 1;
   else if (tpr1->capacity > tpr2->capacity) return -1;
   else if (tpr1->capacity < tpr2->capacity) return 1;
   else return 0;
}
   
void Reservoir::collectTrapProperties (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests)
{
   RequestHandling::StartRequestHandling (this, "collectTrapProperties");

   TrapVector::iterator trapIter;
   unsigned int i = 0;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      assert (i < maxNumberOfRequests);
      Trap * trap = * trapIter;
      trap->collectProperties (tpRequests[i]);
      ++i;
   }

   RequestHandling::FinishRequestHandling ();
}

/// renumber traps according to the amount of stored charge
/// save the trap properties to the TrapIoTbl
bool Reservoir::saveTrapProperties (void)
{
   unsigned int maxLocalNumberOfTraps = computeMaximumTrapCount (true);
   TrapPropertiesRequest * trapsIn = new TrapPropertiesRequest [maxLocalNumberOfTraps];

   collectTrapProperties (trapsIn, maxLocalNumberOfTraps);

   unsigned int maxGlobalNumberOfTraps = maxLocalNumberOfTraps * NumProcessors ();
   TrapPropertiesRequest * trapsOut = new TrapPropertiesRequest [maxGlobalNumberOfTraps];

   // collect trap info from all processors and sort it according to charge content
   AllGatherFromAll (trapsIn, maxLocalNumberOfTraps, TrapPropertiesType,
	 trapsOut, maxLocalNumberOfTraps, TrapPropertiesType);

   delete [] trapsIn;

   // sort the traps according to the weight of their content, largest weight first
   qsort (trapsOut, maxGlobalNumberOfTraps, sizeof (TrapPropertiesRequest), TrapPropertiesCompare);

   populateMigrationTables (trapsOut, maxGlobalNumberOfTraps);
   eliminateUndersizedTraps (trapsOut, maxGlobalNumberOfTraps);

   delete [] trapsOut;

   return true;
}

void Reservoir::changeTrapPropertiesRequestId (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests, int oldId, int newId)
{
   unsigned int i;

   for (i = 0; i < maxNumberOfRequests; ++i)
   {
      if (tpRequests[i].id == oldId)
         tpRequests[i].id = newId;

      if (tpRequests[i].spillid == oldId)
         tpRequests[i].spillid = newId;
   }
}

/// renumber the traps in the MigrationIoTbl and fill the TrapIoTbl
void Reservoir::populateMigrationTables (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests)
{
   // renumber the traps according to their position in the array
   // and add them to the trapiotbl

   // sufficiently large number
   const int trapNumberOffset = 1000000;

   // first renumber each trap id in the MigrationIoTbl to an id we are not going to use later
   unsigned int i;
   for (i = 0; i < maxNumberOfRequests; ++i)
   {
      if (tpRequests[i].id >= 0) // globally valid id 
      {
	 if (GetRank () == 0)
	 {
	    // map each number to another number that we are not going to use later
	    ((Migrator *) m_projectHandle)->renumberMigrationRecordTrap (getEnd (),
	       tpRequests[i].id, tpRequests[i].id + trapNumberOffset);
	 }

	 int oldId = tpRequests[i].id;
	 changeTrapPropertiesRequestId (tpRequests, maxNumberOfRequests, tpRequests[i].id, tpRequests[i].id + trapNumberOffset);
	 assert (oldId + trapNumberOffset == tpRequests[i].id);
      }
   }

   // determine the new id's for the traps and retain a mapping for the traps that are present locally
   // renumber the traps in the MigrationIoTbl and add the renumbered traps to the TrapIoTbl
   TrapVector trapList;
   vector<int> idList;

   double minimumCapacity = getTrapCapacity ();

   for (i = 0; i < maxNumberOfRequests; ++i)
   {
      if (tpRequests[i].id >= 0) // globally valid id 
      {
	 if (tpRequests[i].rank == GetRank ()) // locally available trap
	 {
	    // id change with trapNumberOffset has not been applied to the traps themselves
	    Trap * trap = findTrap (tpRequests[i].id - trapNumberOffset); 
	    assert (trap);

	    trapList.push_back (trap);
	    // this is to be the new id
	    idList.push_back (i + 1);
	 }

	 if (GetRank () == 0)
	 {
	    ((Migrator *) m_projectHandle)->renumberMigrationRecordTrap (getEnd (),
	       tpRequests[i].id, i + 1);
	 }

	 changeTrapPropertiesRequestId (tpRequests, maxNumberOfRequests, tpRequests[i].id, i + 1);

	 if (GetRank () == 0 && tpRequests[i].capacity >= minimumCapacity)
	 {
	    ((Migrator *) m_projectHandle)->addTrapRecord (this, tpRequests[i]);
	 }
      }
   }


   // renumber the Traps themselves to new id's
   RequestHandling::StartRequestHandling (this, "populateMigrationTables");

   TrapVector::iterator trapIter;
   vector<int>::iterator idIter;
   for (trapIter = trapList.begin (), idIter = idList.begin (); trapIter != trapList.end (); ++trapIter, ++idIter)
   {
      (*trapIter)->setGlobalId (*idIter);
   }

   RequestHandling::FinishRequestHandling ();
}

/// remove all undersized traps from the equation by giving them as id the id of the oversized trap they finally spill to,
/// or the NoTrapId if they spill into nothingness or the UnknownTrapId if no charge has come by.
/// Can be done more efficiently.
void Reservoir::eliminateUndersizedTraps (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests)
{
   RequestHandling::StartRequestHandling (this, "eliminateUndersizedTraps");

   vector<int> rank;
   vector<int> from;
   vector<int> to;

   std::vector<int> finalIds;
   finalIds.resize(maxNumberOfRequests);

   const int maxIterations = 100;

   double minimumCapacity = getTrapCapacity ();
   unsigned int i;
   for (i = 0; i < maxNumberOfRequests; ++i)
   {
      int originatingId = tpRequests[i].id;
      int currentId = tpRequests[i].id;
      double currentCapacity = tpRequests[i].capacity;
      if (currentId < 0 || currentCapacity >= minimumCapacity)
      {
	 finalIds[i] = currentId;
	 continue;
      }

// #define SHOWTRAPELIMINATION

#ifdef SHOWTRAPELIMINATION
      if (GetRank () == 0)
	 cerr << "Eliminating trap " << originatingId;
#endif

      int currentSpillId = tpRequests[i].spillid;
      int currentlySpilling = tpRequests[i].spilling;

      int iteration = 0; // making sure we won't end up in a loop ....
      // find out where the trap is spilling to if it is spilling

      bool finalIdFound = false;
      while (++iteration < maxIterations && currentlySpilling && currentId > 0 && currentCapacity < minimumCapacity)
      {
	 unsigned int j;
	 for (j = 0; j < maxNumberOfRequests; ++j)
	 {
	    if (tpRequests[j].id == currentSpillId)
	    {
	       if (j < i)
	       {
#ifdef SHOWTRAPELIMINATION
		  if (GetRank () == 0)
		  {
		     cerr << "\t----->\t" << tpRequests[j].id;
		  }
#endif
		  currentId = finalIds[j];
		  currentCapacity = MAXDOUBLE;
		  finalIdFound = true;
	       }
	       else
	       {
		  currentId = tpRequests[j].id;
		  currentCapacity = tpRequests[j].capacity;
		  currentSpillId = tpRequests[j].spillid;
		  currentlySpilling = tpRequests[j].spilling;
	       }
	       break;
	    }
	 }

	 if (j == maxNumberOfRequests) // nothing was found, leaking
	 {
	    currentId = NoTrapId;
	    currentCapacity = MAXDOUBLE;
	    currentSpillId = NoTrapId;
	    currentlySpilling = 0;
	 }

#ifdef SHOWTRAPELIMINATION
	 if (GetRank () == 0)
	 {
	    cerr << "\t=====>\t" << currentId;
	 }
#endif

	 if (finalIdFound) break;
      }

      if (currentId == tpRequests[i].id || currentCapacity < minimumCapacity) // Nothing came by, not spilling or too small.
      {
	 currentId = NoTrapId;
#ifdef SHOWTRAPELIMINATION
	 if (GetRank () == 0)
	 {
	    cerr << "\t=====>\t" << currentId;
	 }
#endif
      }

      finalIds[i] = currentId;

      from.push_back (tpRequests[i].id);
      to.push_back (currentId);
      rank.push_back (tpRequests[i].rank);

      if (GetRank () == 0)
      {
#ifdef SHOWTRAPELIMINATION
	 cerr << endl;
#endif
	 ((Migrator *) m_projectHandle)->renumberMigrationRecordTrap (getEnd (), originatingId, currentId);
      }

   }

   unsigned int p;
   for (p = 0; p < from.size (); ++p)
   {
      if (rank[p] == GetRank ())
      {
	 Trap * trap = findTrap (from[p]);
	 assert (trap);
	 trap->setGlobalId (NoTrapId); // for the TrapId Map
	 trap->setDrainageAreaId (to[p]); // for the DrainageAreaId Map
      }
   }

   RequestHandling::FinishRequestHandling ();
}

/// report migration from a trap at a previous snapshot
void Reservoir::reportLateralMigration (int sourceTrapId, Column * targetColumn, const Composition & composition)
{
   MigrationRequest mr;

   mr.composition = composition;

   mr.source.age = getStart ()->getTime ();
   mr.source.trapId = sourceTrapId;
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;


   mr.destination.age = getEnd ()->getTime ();
   mr.destination.trapId = targetColumn->getGlobalTrapId ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.process = (mr.destination.trapId == NoTrapId) ? REMIGRATIONLEAKAGE : REMIGRATION;


   collectMigrationRequest (mr);
}

/// report migration from an underling reservoir or from a source rock
void Reservoir::reportVerticalMigration (Column * sourceColumn, Column * targetColumn, const Composition & composition)
{
   assert (m_sourceReservoir || m_sourceFormation);

   MigrationRequest mr;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = NoTrapId;
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.trapId = targetColumn->getGlobalTrapId ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;

   mr.process = (m_sourceReservoir) ?
      ((mr.destination.trapId == NoTrapId) ? THROUGHLEAKAGE : LEAKAGETOTRAP) :
      ((mr.destination.trapId == NoTrapId) ? EXPULSIONLEAKAGE : EXPULSION);
   
   collectMigrationRequest (mr);
}

void Reservoir::reportTrapAbsorption (Trap * sourceTrap, Column * targetColumn, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = ABSORPTION;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = sourceTrap->getGlobalId ();
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = targetColumn->getGlobalTrapId ();
   assert (mr.destination.trapId > 0);
   
   collectMigrationRequest (mr);
}

/// report leakage from a trap
void Reservoir::reportWaste (Trap * sourceTrap, Column * targetColumn, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = LEAKAGEFROMTRAP;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = sourceTrap->getGlobalId ();
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.trapId = NoTrapId;
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   
   collectMigrationRequest (mr);
}

void Reservoir::reportSpill (Trap * sourceTrap, Column * targetColumn, const Composition & composition)
{
   MigrationRequest mr;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = sourceTrap->getGlobalId ();

   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = targetColumn->getGlobalTrapId ();

   mr.process = (mr.destination.trapId == NoTrapId) ? SPILLUPOROUT : SPILL;
   
   collectMigrationRequest (mr);
}

void Reservoir::reportBiodegradationLoss (Trap * sourceTrap, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = BIODEGRADATION;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = sourceTrap->getGlobalId ();
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = NoTrapId;
   
   collectMigrationRequest (mr);
}

void Reservoir::reportCrackingLoss (int trapId, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = OILTOGASCRACKINGLOST;

   mr.composition = composition;

   mr.source.age = getStart ()->getTime ();
   mr.source.trapId = trapId;
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getStart ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = NoTrapId;
   
   collectMigrationRequest (mr);
}

void Reservoir::reportCrackingGain (int trapId, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = OILTOGASCRACKINGGAINED;

   mr.composition = composition;

   mr.source.age = getStart ()->getTime ();
   mr.source.trapId = trapId;
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getStart ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = trapId;
   
   collectMigrationRequest (mr);
}

void Reservoir::reportDiffusionLoss (Trap * sourceTrap, const Composition & composition)
{
   MigrationRequest mr;

   mr.process = DIFFUSION;

   mr.composition = composition;

   mr.source.age = getEnd ()->getTime ();
   mr.source.trapId = sourceTrap->getGlobalId ();
   mr.source.x = Interface::DefaultUndefinedScalarValue;
   mr.source.y = Interface::DefaultUndefinedScalarValue;

   mr.destination.age = getEnd ()->getTime ();
   mr.destination.x = Interface::DefaultUndefinedScalarValue;
   mr.destination.y = Interface::DefaultUndefinedScalarValue;
   mr.destination.trapId = NoTrapId;
   
   collectMigrationRequest (mr);
}

bool Reservoir::processMigration (MigrationRequest &mr)
{
   switch (mr.process)
   {
      case REMIGRATION:
      case REMIGRATIONLEAKAGE:
      case LEAKAGEFROMTRAP:
      case LEAKAGETOTRAP:
      case EXPULSION:
      case EXPULSIONLEAKAGE:
      case SPILL:
      case SPILLUPOROUT:
      case THROUGHLEAKAGE:
      case DIFFUSION:
      case BIODEGRADATION:
      case OILTOGASCRACKINGLOST:
      case OILTOGASCRACKINGGAINED:
	 ((Migrator *) m_projectHandle)->addMigrationRecord (m_sourceReservoir ? m_sourceReservoir->getName () : string (""),
	    m_sourceFormation ? m_sourceFormation->getName () : string (""), getName (), mr);
	 return true;
	 break;
      case ABSORPTION:
	 return true;
	 break;
      case NUMBEROFPROCESSES:
      case NOPROCESS:
	 return false;
	 break;
   }
   return false;
}

bool Reservoir::processAbsorption (MigrationRequest &mr)
{
   switch (mr.process)
   {
      case REMIGRATION:
      case REMIGRATIONLEAKAGE:
      case LEAKAGEFROMTRAP:
      case LEAKAGETOTRAP:
      case EXPULSION:
      case EXPULSIONLEAKAGE:
      case SPILL:
      case SPILLUPOROUT:
      case THROUGHLEAKAGE:
      case DIFFUSION:
      case BIODEGRADATION:
      case OILTOGASCRACKINGLOST:
      case OILTOGASCRACKINGGAINED:
	 return true;
	 break;
      case ABSORPTION:
	 ((Migrator *) m_projectHandle)->renumberMigrationRecordTrap (getEnd (), mr.source.trapId, mr.destination.trapId);
	 return true;
	 break;
      case NUMBEROFPROCESSES:
      case NOPROCESS:
	 return false;
	 break;
   }
   return false;
}

MigrationRequest * Reservoir::findMigrationRequest (MigrationRequest & request)
{
   vector<MigrationRequest>::iterator mrIter;
   for (mrIter = m_migrationRequests.begin (); mrIter != m_migrationRequests.end (); ++mrIter)
   {
      MigrationRequest & thisRequest = * mrIter;
      if (request == thisRequest)
      {
	 return & thisRequest;
      }
   }
   return 0;
}

/// collect a migration to be ultimately sent to processor 0 to be ultimately saved in the MigrationIoTbl
void Reservoir::collectMigrationRequest (MigrationRequest & request)
{
   if (request.process == NOPROCESS) return;

   MigrationRequest * foundRequest = findMigrationRequest (request);

   if (foundRequest)
   {
      foundRequest->composition.add (request.composition);
   }
   else
   {
      m_migrationRequests.push_back (request);
   }
}

/// send all collected migration requests to processor 0 to be added to the MigrationIoTbl
void Reservoir::processMigrationRequests (void)
{
   int localSize = m_migrationRequests.size ();

   // determine maximum number of requests per processor
   int localMaximumSize = MaximumAll (localSize);
   if (localMaximumSize == 0) return;

#if 0
   ReportProgress ("starting processing migration requests");
#endif

   int globalMaximumSize = NumProcessors () * localMaximumSize;

   // put all requests into a C array
   MigrationRequest * localMigrationRequestArray = new MigrationRequest [localMaximumSize];

   vector<MigrationRequest>::iterator mrIter;

   int i = 0;
   if (GetRank () != 0)
   {
      for (mrIter = m_migrationRequests.begin (), i = 0; mrIter != m_migrationRequests.end (); ++mrIter, ++i)
      {
	 localMigrationRequestArray[i] = * mrIter;
	 assert (localMigrationRequestArray[i].process != NOPROCESS);
      }
   }

   for (i = i; i < localMaximumSize; ++i)
   {
      localMigrationRequestArray[i].process = NOPROCESS;
   }

   MigrationRequest * globalMigrationRequestArray = 0;

   // RootGather used to crash but no longer ...
#define USEROOTGATHER
#ifdef USEROOTGATHER
   bool useAllGather = false;
#else
   bool useAllGather = true;
#endif

// #define DEBUG_GATHER
#ifdef DEBUG_GATHER
   PetscPrintf (PETSC_COMM_WORLD, "%d: Collecting...\n", GetRank());
   PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif

   if (useAllGather || GetRank () == 0)
   {
      globalMigrationRequestArray = new MigrationRequest [globalMaximumSize];
   }

#ifdef DEBUG_GATHER
   PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Gathering... %d (%d) elements\n", GetRank (), localSize, localMaximumSize);
   PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif

   if (useAllGather)
   {
#ifdef DEBUG_GATHER
      PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Using AllGatherFromAll...\n", GetRank());
      PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif
      AllGatherFromAll (localMigrationRequestArray, localMaximumSize, MigrationType,
	    globalMigrationRequestArray, localMaximumSize, MigrationType);
   }
   else
   {
#ifdef DEBUG_GATHER
      PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Using RootGatherFromAll...\n", GetRank());
      PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif
   RootGatherFromAll (localMigrationRequestArray, localMaximumSize, MigrationType,
	 globalMigrationRequestArray, localMaximumSize, MigrationType);
   }

#ifdef DEBUG_GATHER
   PetscPrintf (PETSC_COMM_WORLD, "Processing...\n");
#endif
   if (GetRank () == 0)
   {
#ifdef DEBUG_GATHER
      ReportProgress ("exchanged migration requests");
#endif

      // process the the migration requests
      // by adding them to m_migrationRequests
      assert (globalMigrationRequestArray != 0);

      for (i = 0; i < globalMaximumSize; ++i)
      {
	 collectMigrationRequest (globalMigrationRequestArray[i]);
      }


      for (mrIter = m_migrationRequests.begin (), i = 0; mrIter != m_migrationRequests.end (); ++mrIter, ++i)
      {
	 processMigration (* mrIter);
      }

      for (mrIter = m_migrationRequests.begin (), i = 0; mrIter != m_migrationRequests.end (); ++mrIter, ++i)
      {
	 processAbsorption (* mrIter);
      }
   }

#ifdef DEBUG_GATHER
   PetscPrintf (PETSC_COMM_WORLD, "Deleting...\n");
#endif

   if (useAllGather || GetRank () == 0)
      delete [] globalMigrationRequestArray;

   m_migrationRequests.clear ();
   delete [] localMigrationRequestArray;

#ifdef DEBUG_GATHER
   ReportProgress ("finished processing migration requests");
#endif
}

void Reservoir::getValue (ColumnValueRequest & valueRequest, ColumnValueRequest & valueResponse)
{
   getLocalColumn (valueRequest.i, valueRequest.j)->getValue (valueRequest, valueResponse);
}

void Reservoir::setValue (ColumnValueRequest & valueRequest)
{
   getLocalColumn (valueRequest.i, valueRequest.j)->setValue (valueRequest);
}

void Reservoir::getValue (ColumnValueArrayRequest & valueArrayRequest, ColumnValueArrayRequest & valueArrayResponse)
{
   getLocalColumn (valueArrayRequest.i, valueArrayRequest.j)->getValue (valueArrayRequest, valueArrayResponse);
}

void Reservoir::setValue (ColumnValueArrayRequest & valueArrayRequest)
{
   getLocalColumn (valueArrayRequest.i, valueArrayRequest.j)->setValue (valueArrayRequest);
}

void Reservoir::clearProxyProperties (ColumnValueRequest & valueRequest)
{
   getProxyColumn (valueRequest.i, valueRequest.j)->clearProperties ();
}

void Reservoir::manipulateColumn (ColumnColumnRequest & columnRequest)
{
   getLocalColumn (columnRequest.i, columnRequest.j)->manipulateColumn (columnRequest.valueSpec, columnRequest.valueI, columnRequest.valueJ);
}

void Reservoir::manipulateColumnComposition (ColumnCompositionRequest & compositionRequest)
{
   getLocalColumn (compositionRequest.i, compositionRequest.j)->manipulateComposition (compositionRequest.valueSpec,
	 compositionRequest.phase, compositionRequest.composition);
}
}
