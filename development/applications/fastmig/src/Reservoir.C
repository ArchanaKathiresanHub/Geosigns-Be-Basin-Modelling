//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
#include "MigrationPropertyManager.h"

#include "overburden_MPI.h"
#include "RetrieveAndRestoreSurfaceGridMapContainer.h"
#include "RequestHandling.h"
#include "ObjectFactory.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/PropertyValue.h"
#include "Interface/BiodegradationParameters.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "Interface/DiffusionLeakageParameters.h"
#include "Interface/MapWriter.h"


#include "ConstantsGenex.h"
#include "FormationPropertyAtSurface.h"

#include "petscvec.h"
#include "petscdmda.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

// std library
#include <vector>
#include <assert.h>
#include <math.h>
#include<sstream>
using namespace std;
using std::ostringstream;
extern ostringstream cerrstrstr;
#define MAXDOUBLE std::numeric_limits<double>::max()

//utilities library
#include "LogHandler.h"
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;

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

   Reservoir::Reservoir (ProjectHandle * projectHandle, Migrator * const migrator, database::Record * record)
      : Interface::Reservoir (projectHandle, record),
      m_start (0),
      m_end (0),
      m_migrator (migrator),
      m_index (-1)
   {
      m_chargeDistributionCount = 0;
      m_columnArray = 0;
      createColumns ();

      setSourceReservoir (0);
      setSourceFormation (0);

      m_lowResEqualsHighRes =
         (*m_projectHandle->getLowResolutionOutputGrid () == *m_projectHandle->getHighResolutionOutputGrid ());

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

   int Reservoir::getIndex (void)
   {
      m_index = m_migrator->getIndex (this);
      return m_index;
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

      RequestHandling::StartRequestHandling (m_migrator, "computePathways");

      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
            {
               computeAdjacentColumn ((PhaseId)phase, i, j);
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

      RequestHandling::StartRequestHandling (m_migrator, "computeTargetColumns");

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
                  allComputed &= computeTargetColumn ((PhaseId)phase, i, j);
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

      RequestHandling::StartRequestHandling (m_migrator, "computeTrapTops");

      for (unsigned int phase = 0; phase < NumPhases; ++phase)
      {
         for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
            {
               computeTrapTop ((PhaseId)phase, i, j);
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

         RequestHandling::StartRequestHandling (m_migrator, "computeFluxes");

         for (unsigned int phase = 0; phase < NumPhases; ++phase)
         {
            for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
            {
               for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
               {
                  computeFlux ((PhaseId)phase, i, j);
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

      RequestHandling::StartRequestHandling (m_migrator, "computeTrapExtents");

      TrapVector::iterator trapIter;

      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         (*trapIter)->computeArea ();
         (*trapIter)->computeDepthToVolumeFunction ();
#ifdef COMPUTECAPACITY
         (* trapIter)->computeCapacity ();
#endif
         (*trapIter)->computeWasteColumns ();
         (*trapIter)->computeSpillTarget ();
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
      if (!isOilToGasCrackingOn ()) return false;

      RequestHandling::StartRequestHandling (m_migrator, "recomputeTrapDepthToVolumeFunctions");
      TrapVector::iterator trapIter;

      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         (*trapIter)->computeDepthToVolumeFunction ();
      }
      RequestHandling::FinishRequestHandling ();

      return true;
   }


   /// Initialize the atributes of the traps.
   bool Reservoir::initializeTraps (void)
   {
      bool result = true;

      RequestHandling::StartRequestHandling (m_migrator, "initializeTraps");

      TrapVector::iterator trapIter;

      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         (*trapIter)->initialize ();
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

   // compute the column to which column (i,j) spills to
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

   // find the column to which column (i,j) spills to and that is not in the trap
   Column *  Reservoir::getAdjacentColumn (PhaseId phase, Column * column, Trap * trap)
   {
      assert (IsValid (column));

      double depth = column->getTopDepth ();

      assert (depth != getUndefinedValue ());

      double minGradient = SealDepth;

      Column * adjacentColumn = column;

      // Try to avoid going through a sealing column
      if (trap and column->isSealing (phase))
      {
         adjacentColumn = avoidSealingColumn (phase, column, trap);
         return (adjacentColumn ? adjacentColumn : column);
      }

      // try to find a higher lying column
      for (int n = 0; n < NumNeighbours; ++n)
      {
         Column *neighbourColumn = getColumn (column->getI () + NeighbourOffsets2D[n][I], column->getJ () + NeighbourOffsets2D[n][J]);

         if (!IsValid (neighbourColumn))
         {
            // column lies on the edge, no adjacentColumn
            adjacentColumn = 0;
            break;
         }

         if (trap and trap->contains (neighbourColumn))
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
            if (!adjacentColumn or adjacentColumn->isDeeperThan (neighbourColumn))
            {
               minGradient = 0;
               adjacentColumn = neighbourColumn;
            }
         }
      }

      return adjacentColumn;
   }

   // Try to find a non-sealing column among the neighbours
   Column * Reservoir::avoidSealingColumn (PhaseId phase, Column * column, Trap * trap)
   {
      int kappa = 1; // Iterating from 1 to MaximumNeighbourOffset
      Column * nonSealingAdjacentColumn = 0;
      Column * columnToReturn = 0;
      double lowerDepth = 199999.0;

      if (MaximumNeighbourOffset == 0)
         return columnToReturn;

      do
      {
         for (int n = 0; n < NumNeighbours; ++n)
         {
            nonSealingAdjacentColumn = findNonSealingColumn (kappa, n, phase, column, trap);

            if (IsValid (nonSealingAdjacentColumn) and nonSealingAdjacentColumn->getTopDepth () < lowerDepth and
               !nonSealingAdjacentColumn->isSealing (phase) and (trap ? !trap->contains (nonSealingAdjacentColumn) : true))
            {
               columnToReturn = nonSealingAdjacentColumn;
               lowerDepth = nonSealingAdjacentColumn->getTopDepth ();
            }
         }

         if (++kappa > MaximumNeighbourOffset)
            return columnToReturn;
      }
      while (!columnToReturn);

      return columnToReturn;
   }

   // Decomposing the neighbour space to cases
   Column * Reservoir::findNonSealingColumn (int kappa, int n, PhaseId phase, Column * column, Trap * trap)
   {
      Column * nonSealingAdjacentColumn = 0;
      Column * columnToReturn = 0;
      double lowerDepth = 199999.0;
      int originalKappa = kappa;

      // Checking neighbours for which:
      // 1) DeltaI=DeltaJ (diagonal)
      // 2) Either DeltaI or DeltaJ is zero (orthogonal)
      nonSealingAdjacentColumn = getColumn (column->getI () + kappa * NeighbourOffsets2D[n][I], column->getJ () + kappa * NeighbourOffsets2D[n][J]);

      if (IsValid (nonSealingAdjacentColumn) and nonSealingAdjacentColumn->getTopDepth () < lowerDepth and
         !nonSealingAdjacentColumn->isSealing (phase) and (trap ? !trap->contains (nonSealingAdjacentColumn) : true))
      {
         columnToReturn = nonSealingAdjacentColumn;
         lowerDepth = nonSealingAdjacentColumn->getTopDepth ();
      }

      switch (n)
      {
         // These are the diagonal cases (see NeighbourOffsets[][] in migration.h). For these cases: abs(DeltaI)=abs(DeltaJ)
      case 0:
      case 2:
      case 5:
      case 7:
      {
         // Checking neighbours off the diagonal but for which both DeltaI and DeltaJ are non-zero
         for (; kappa > 1; --kappa)
         {
            nonSealingAdjacentColumn = getColumn (column->getI () + (kappa - 1) * NeighbourOffsets2D[n][I], column->getJ () + originalKappa * NeighbourOffsets2D[n][J]);

            if (IsValid (nonSealingAdjacentColumn) and nonSealingAdjacentColumn->getTopDepth () < lowerDepth and
               !nonSealingAdjacentColumn->isSealing (phase) and (trap ? !trap->contains (nonSealingAdjacentColumn) : true))
            {
               columnToReturn = nonSealingAdjacentColumn;
               lowerDepth = nonSealingAdjacentColumn->getTopDepth ();
            }

            nonSealingAdjacentColumn = getColumn (column->getI () + originalKappa * NeighbourOffsets2D[n][I], column->getJ () + (kappa - 1) * NeighbourOffsets2D[n][J]);

            if (IsValid (nonSealingAdjacentColumn) and nonSealingAdjacentColumn->getTopDepth () < lowerDepth and
               !nonSealingAdjacentColumn->isSealing (phase) and (trap ? !trap->contains (nonSealingAdjacentColumn) : true))
            {
               columnToReturn = nonSealingAdjacentColumn;
               lowerDepth = nonSealingAdjacentColumn->getTopDepth ();
            }
         }
         break;
      }
      // These are the orthogonal cases
      case 1:
      case 3:
      case 4:
      case 6:
      default:
         break;
      }

      return columnToReturn;
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
      while (adjacentColumn != 0 and adjacentColumn != column and IsValid (adjacentColumn))
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

      if (!IsValid (column) or !column->isMinimum () or
         column->isWasting (phase) or column->isSealing (phase) or
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
      RequestHandling::StartRequestHandling (m_migrator, "retainPreviousProperties");
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

      RequestHandling::StartRequestHandling (m_migrator, "clearProperties");
      bool result = true;
      result = m_columnArray->clearProperties ();
      RequestHandling::FinishRequestHandling ();

      return result;
   }

   bool Reservoir::clearPreviousProperties (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "clearPreviousProperties");
      bool result = m_columnArray->clearPreviousProperties ();
      RequestHandling::FinishRequestHandling ();
      return result;
   }

   bool Reservoir::computeProperties (void)
   {
      RequestHandle requestHandle (m_migrator, "computeProperties");

      if (!computeDepths ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeDepths done" << endl;
      }
#endif
      if (!computeSeaBottomPressures ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeSeaBottomPressures done" << endl;
      }
#endif

      if (!computeOverburdens ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeOverburdens done" << endl;
      }
#endif

      if (!adaptOverburdens ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "adaptOverburdens done" << endl;
      }
#endif

      if (!computeFaults ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeFaultss done" << endl;
      }
#endif

      if (!computePorosities ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computePorosities done" << endl;
      }
#endif

      if (!computePermeabilities ())
         return false;

#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computePermeabilities done" << endl;
      }
#endif
      if (!computeTemperatures ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeTemperatures done" << endl;
      }
#endif

      // Optimization for May 2016 Release
      if (!m_migrator->performLegacyMigration ())
      {
         if (!computeViscosities ())
            return false;
#if DEBUG
         if( GetRank() == 0 ) {
            cout << "computeViscosities done" << endl;
         }
#endif
      }

      if (!computePressures ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computePressures done" << endl;
      }
#endif

      if (!computeOverburdenGridMaps ())
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "computeOverburdenGridMaps done" << endl;
      }
#endif

      // And if the type of fracturePressureFunctionParameters is given by Interface::
      // FunctionOfLithostaticPressure, also the hydrostatic and lithostatic pressures are needed.  
      const FracturePressureFunctionParameters*
         fracturePressureFunctionParameters = getProjectHandle ()->getFracturePressureFunctionParameters ();
      if (!fracturePressureFunctionParameters)
         return false;
#if DEBUG
      if( GetRank() == 0 ) {
         cout << "fracturePressureFunctionParameters ON" << endl;
      }
#endif

      if (fracturePressureFunctionParameters->type () == Interface::FunctionOfLithostaticPressure)
      {
         if (!computeHydrostaticPressures ())
            return false;
#if DEBUG
         if( GetRank() == 0 ) {
            cout << "computeHydrostaticPressures done" << endl;
         }
#endif

         if (!computeLithostaticPressures ())
            return false;
#if DEBUG
         if( GetRank() == 0 ) {
            cout << "computeLithostaticPressures done" << endl;
         }
#endif

      }

      return true;
   }

   /// compute the top and bottom depths of the reservoir.
   bool Reservoir::computeDepths (void)
   {
      DerivedProperties::FormationPropertyPtr formationGridMap = getFormationPropertyPtr ("Depth", getEnd ());

      if (!formationGridMap)
      {
         cerr << "ERROR: " << getName () <<
            "::computing of reservoir depths failed, could not find the formation depth map at " << getEnd ()->getTime () << endl;
         cerr.flush ();
         return false;
      }

      unsigned int depth = formationGridMap->lengthK ();
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

               topIndex = Max ((double)0, topIndex);
               topIndex = Min ((double)depth - 1, topIndex);

               bottomIndex = Max ((double)0, bottomIndex);
               bottomIndex = Min ((double)depth - 1, bottomIndex);

               double topValue = formationGridMap->interpolate (i, j, topIndex);
               double bottomValue = formationGridMap->interpolate (i, j, bottomIndex);

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

               if (bottomValue < topValue)
                  bottomValue = topValue;
               column->setBottomDepth (bottomValue);
            }
         }
      }
      else
      {
         DerivedProperties::SurfacePropertyPtr topSurfaceGridMap = getTopSurfaceProperty ("DepthHighRes", getEnd ());
         DerivedProperties::SurfacePropertyPtr bottomSurfaceGridMap = getBottomSurfaceProperty ("DepthHighRes", getEnd ());

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

               double topSurfaceDepth = topSurfaceGridMap->get (i, j);
               double bottomSurfaceDepth = bottomSurfaceGridMap->get (i, j);
               double lowResTopSurfaceDepth = formationGridMap->get (i, j, depth - 1);

               if (lowResTopSurfaceDepth == formationGridMap->getUndefinedValue () or
                  topSurfaceDepth == topSurfaceGridMap->getUndefinedValue () or
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

                  bottomValue = Min (bottomValue, bottomSurfaceDepth);

                  if (bottomValue < topValue)
                     bottomValue = topValue;
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

      DerivedProperties::SurfacePropertyPtr gridMap = getSeaBottomProperty (depthPropertyName (), getEnd ());

      if (gridMap == 0)
      {
         // If there is no surface property, use formation property
         DerivedProperties::FormationPropertyPtr gridFormMap = getSeaBottomFormationProperty (depthPropertyName (), getEnd ());
         const Interface::Formation* seaFormation = getSeaBottomFormation (getEnd ());

         gridMap = DerivedProperties::SurfacePropertyPtr (new DerivedProperties::FormationPropertyAtSurface (gridFormMap, seaFormation->getTopSurface ()));
         if (gridMap == 0)
         {
            return false;
         }
      }

      gridMap->retrieveData ();

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column))
            {
               column->setOverburden (column->getTopDepth () - gridMap->get (i, j));
            }
         }
      }
      gridMap->restoreData ();

      return true;
   }

   bool Reservoir::computeSeaBottomPressures (void)
   {

      DerivedProperties::SurfacePropertyPtr gridMap = getSeaBottomProperty ("HydroStaticPressure", getEnd ());
      //   DerivedProperties::FormationPropertyPtr gridMap = getSeaBottomFormationProperty ( "HydroStaticPressure", getEnd () );

      if (gridMap == 0)
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
               //	    column->setSeaBottomPressure (column->getTopDepth () - gridMap->get (i, j, gridMap->lastK() ));
               column->setSeaBottomPressure (column->getTopDepth () - gridMap->get (i, j));
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
      DerivedProperties::SurfacePropertyPtr gridMap = getTopSurfaceProperty (depthPropertyName (), getEnd ());

      if (gridMap == 0)
      {
         DerivedProperties::FormationPropertyPtr gridFormMap = getTopFormationProperty (depthPropertyName (), getEnd ());
         gridMap = DerivedProperties::SurfacePropertyPtr (new DerivedProperties::FormationPropertyAtSurface (gridFormMap, getTopFormation (getEnd ())->getTopSurface ()));

         if (gridMap == 0)
         {
            if (GetRank ())
            {
               LogHandler (LogHandler::ERROR_SEVERITY) << "Cannot allocate " << depthPropertyName () << " FormationPropertyAtSurface " << getEnd ()->getTime ();
            }
            return false;
         }

      }

      double gridUndefinedValue = gridMap->getUndefinedValue ();

      gridMap->retrieveData ();

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);

            if (column->getTopDepth () != getUndefinedValue ())
            {
               double formationTopDepth = gridMap->get (i, j);

               double reservoirDepthOffset = 0;

               if (formationTopDepth != gridUndefinedValue)
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

      m_diffusionOverburdenGridMaps.clear ();
      m_sealPressureLeakageGridMaps.clear ();

      // Determine the overburden formations.  Include first the formation of the reservoir, for 
      // the trap may be sealed inside the formation:
      vector<const Formation*> formations;
      formations.push_back (dynamic_cast<Formation *>(const_cast<Interface::Formation*>(getFormation ())));
      overburden::OverburdenFormations overburden (formations, true);

      // Add the formation overburdens:
      overburden::OverburdenFormations overburdenFormations = overburden::getOverburdenFormations
         (dynamic_cast<Formation *> (const_cast<Interface::Formation*>(getFormation ())), true);
      overburden.append (overburdenFormations.formations ());

      // We need the depth for both diffusion leakage and seal pressure leakage.  (Note, we put depths 
      // in vector<FormationSurfaceGridMaps> even though the depth is a continuous property.  This is done 
      // just for convenience.   The algorithms work out easier.)
      const ProjectHandle* projectHandle = getFormation ()->getProjectHandle ();
      const Property* depthProp = lowResEqualsHighRes () ?
         projectHandle->findProperty ("Depth") :
         projectHandle->findProperty ("DepthHighRes");

      vector<FormationSurfaceGridMaps> depthGridMaps = overburden_MPI::getFormationSurfaceGridMaps (
         overburden.formations (), depthProp, getEnd ());

      // If diffusion leakages is included, initialize m_diffusionOverburdenGridMaps with 
      // the necessary grid maps:
      // Optimization for May 2016 Release
      if (isDiffusionOn () and !m_migrator->performLegacyMigration ())
      {
         vector<SurfaceGridMapFormations> temperatureGridMaps = overburden_MPI::getAdjacentSurfaceGridMapFormations (
            overburden, "Temperature", getEnd ());
         vector<FormationSurfaceGridMaps> porosityGridMaps = overburden_MPI::getFormationSurfaceGridMaps (
            overburden.formations (), "Porosity", getEnd ());

         vector<FormationSurfaceGridMaps> brineViscosityGridMaps = overburden_MPI::getFormationSurfaceGridMaps (
            overburden.formations (), "BrineViscosity", getEnd ());

         m_diffusionOverburdenGridMaps.setDiscontinuous (SurfaceGridMapContainer::DISCONTINUOUS_DEPTH, depthGridMaps);
         m_diffusionOverburdenGridMaps.setContinuous (SurfaceGridMapContainer::CONTINUOUS_TEMPERATURE, temperatureGridMaps);
         m_diffusionOverburdenGridMaps.setDiscontinuous (SurfaceGridMapContainer::DISCONTINUOUS_POROSITY, porosityGridMaps);
         m_diffusionOverburdenGridMaps.setDiscontinuous (SurfaceGridMapContainer::DISCONTINUOUS_BRINEVISCOSITY, brineViscosityGridMaps);
      }

      // Include the grid maps for seal failure. Seal failure is always on.  We need the grid maps 
      // for permeability and the percentage maps for LithoType 1, 2 and 3.  We read permeability by 
      // means of overburden_MPI::getFormationSurfaceGridMapss.  We can't use formation_MPI::surfaceTopPropertyMap, 
      // because permeability is not a continuous property, so we must get the permeability from the 
      // seal formation.  And at this moment in time, it is not clear what the seal formation is, 
      // so we read in all formations:
      vector<FormationSurfaceGridMaps> permeabilityGridMaps = overburden_MPI::getFormationSurfaceGridMaps (
         overburden.formations (), "Permeability", getEnd ());

      m_sealPressureLeakageGridMaps.setDiscontinuous (SurfaceGridMapContainer::DISCONTINUOUS_DEPTH, depthGridMaps);
      m_sealPressureLeakageGridMaps.setDiscontinuous (SurfaceGridMapContainer::DISCONTINUOUS_PERMEABILITY, permeabilityGridMaps);

      // Get the percentage maps for LithoType 1, 2 and 3. 3 and sometimes also 2 may not exist for
      // some formations:
      SurfaceGridMapContainer::constant_properties lithoType1PercentGridMaps;
      SurfaceGridMapContainer::constant_properties lithoType2PercentGridMaps;
      SurfaceGridMapContainer::constant_properties lithoType3PercentGridMaps;

      for (overburden::OverburdenFormations::formations_type::const_iterator f = overburden.formations ().begin ();
         f != overburden.formations ().end (); ++f)
      {
         if ((*f)->getBottomSurface()->getSnapshot()->getTime() < getEnd()->getTime())
            break;
            
         // Create the following grid map only once:
         const GridMap* litho1PercentMap = (*f)->getLithoType1PercentageMap ();
         assert ((*f)->getLithoType1 () and litho1PercentMap);
         lithoType1PercentGridMaps.push_back (make_pair (*f, SurfaceGridMap (litho1PercentMap,
            (unsigned int)0)));

         if ((*f)->getLithoType2 ())
         {
            // Make sure (*f)->getLithoType2PercentageMap() is consistent with (*f)->getLithoType2(). 
            // If (*f)->getLithoType2() exists, so does (*f)->getLithoType2PercentageMap():
            const GridMap* litho2PercentMap = (*f)->getLithoType2PercentageMap ();
            assert (litho2PercentMap);
            lithoType2PercentGridMaps.push_back (make_pair (*f, SurfaceGridMap (litho2PercentMap,
               (unsigned int)0)));
         }

         if ((*f)->getLithoType3 ())
         {
            // Check also for consistency of (*f)->getLithoType2PercentageMap() and (*f)->getLithoType2():
            const GridMap* litho3PercentMap = (*f)->getLithoType3PercentageMap ();
            assert (litho3PercentMap);
            lithoType3PercentGridMaps.push_back (make_pair (*f, SurfaceGridMap (litho3PercentMap,
               (unsigned int)0)));
         }
      }

      // Include all percentage maps, even if they are empty:
      m_sealPressureLeakageGridMaps.setConstants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE1PERCENT,
         lithoType1PercentGridMaps);
      m_sealPressureLeakageGridMaps.setConstants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE2PERCENT,
         lithoType2PercentGridMaps);
      m_sealPressureLeakageGridMaps.setConstants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE3PERCENT,
         lithoType3PercentGridMaps);

      return true;
   }

   double Reservoir::getMassStoredInColumns (void)
   {
      double totalMass = 0;
      RequestHandling::StartRequestHandling (m_migrator, "getMassStoredInColumns");
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column))
            {
               for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
               {
                  totalMass += column->getChargeQuantity ((PhaseId)phase);
               }
            }
         }
      }

      RequestHandling::FinishRequestHandling ();

      return SumAll (totalMass);
   }

   void Reservoir::putSeepsInColumns (const Formation * seepsFormation)
   {
      // index of top formation nodes in seepsFormation (top in stratigraphy as well)
      int k = seepsFormation->getNodeDepth () - 1;

      for (int i = (int)m_columnArray->firstILocal (); i <= (int) m_columnArray->lastILocal (); ++i)
      {
         for (int j = (int) m_columnArray->firstJLocal (); j <= (int) m_columnArray->lastJLocal (); ++j)
         {
			LocalColumn * seepColumn =dynamic_cast<LocalColumn*> (getColumn ((unsigned int) i, (unsigned int) j));
            if (!seepColumn)
               continue;

            LocalFormationNode * localFormationNode = seepsFormation->getLocalFormationNode (i, j, k);
            if (!IsValid (localFormationNode))
               continue;

            Composition composition = localFormationNode->getComposition ();

            seepColumn->addComposition (composition);
         }
      }

      return;
   }

   bool Reservoir::saveComputedInputProperties (const bool saveSnapshot)
   {
      bool result = true;

      if (saveSnapshot)
      {
         result &= saveComputedProperty ("ResRockTop", TOPDEPTH);
         result &= saveComputedProperty ("ResRockBottom", BOTTOMDEPTH);
         result &= saveComputedProperty ("ResRockThickness", THICKNESS);
         result &= saveComputedProperty ("ResRockPorosity", POROSITYPERCENTAGE);
         result &= saveComputedProperty ("ResRockFaultCutEdges", FAULTSTATUS);
         result &= saveComputedProperty ("ResRockBarriers", COLUMNSTATUS);
         result &= saveComputedProperty ("ResRockPressure", PRESSURE);
         result &= saveComputedProperty ("ResRockTemperature", TEMPERATURE);
         result &= saveComputedProperty ("ResRockOverburden", OVERBURDEN);
      }
      return result;
   }

   bool Reservoir::saveComputedOutputProperties (const bool saveSnapshot)
   {
      bool result = true;
      if (saveSnapshot)
      {

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

            result &= saveComputedProperty (propName + "Phase", DRAINAGEAREAID, (PhaseId)phase);
         }

         for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
         {
            string propName = "ResRockFill";
            propName += PhaseNames[phase];

            result &= saveComputedProperty (propName + "PhaseDensity", LATERALCHARGEDENSITY, (PhaseId)phase);
            result &= saveComputedProperty (propName + "PhaseQuantity", CHARGEQUANTITY, (PhaseId)phase);
         }

         if (!m_migrator->performLegacyMigration ())
         {
            for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
            {
               string propName = "ResRockFlowDirection";
               propName += PhaseNames[phase];

               result &= saveComputedProperty (propName + "IJ", FLOWDIRECTIONIJ, (PhaseId)phase);
            }
         }
      }
      return result;
   }

   bool Reservoir::saveComputedProperty (const string & name, ValueSpec valueSpec, PhaseId phase)
   {
      RequestHandling::StartRequestHandling (m_migrator, "saveComputedProperty");

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

   bool Reservoir::saveSeepageProperties (const Interface::Snapshot * end)
   {
      RequestHandling::StartRequestHandling (m_migrator, "saveSeepageProperties");

      // Need pressures and temperatures at the top of the basin to flash the seeping HCs
      computeTemperatures ();
      computePressures ();

      PropertyValue * propertyValue_Gas = getProjectHandle ()->createMapPropertyValue ("SeepageBasinTop_Gas", end, this, 0, 0);
      PropertyValue * propertyValue_Oil = getProjectHandle ()->createMapPropertyValue ("SeepageBasinTop_Oil", end, this, 0, 0);
      assert (propertyValue_Gas);
      assert (propertyValue_Oil);

      GridMap * gridMap_Gas = propertyValue_Gas->getGridMap ();
      GridMap * gridMap_Oil = propertyValue_Oil->getGridMap ();
      assert (gridMap_Gas);
      assert (gridMap_Oil);

      gridMap_Gas->retrieveData ();
      gridMap_Oil->retrieveData ();

      // Using formation to assign temperature and pressure to the columns at the basin top
      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation);
      int k = formation->getNodeDepth () - 1;

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);

            if (column)
            {// Order is important here. Gas needs to be set before oil
               gridMap_Gas->setValue (i, j, column->getValue (SEEPAGEQUANTITY, GAS));
               gridMap_Oil->setValue (i, j, column->getValue (SEEPAGEQUANTITY, OIL));
            }
            else
            {
               gridMap_Gas->setValue (i, j, gridMap_Gas->getUndefinedValue ());
               gridMap_Oil->setValue (i, j, gridMap_Oil->getUndefinedValue ());
            }
         }
      }
      gridMap_Gas->restoreData ();
      gridMap_Oil->restoreData ();

      RequestHandling::FinishRequestHandling ();

      return true;
   }

   bool Reservoir::saveComputedPathways (const Interface::Snapshot * end) const
   {
      ((Interface::Snapshot *) end)->setAppendFile (true);

      int nodeDepth = (dynamic_cast<const migration::Formation *>(getFormation ()))->getNodeDepth ();

      Interface::PropertyValue * propertyValue =
         m_migrator->getProjectHandle ()->createVolumePropertyValue ("ResRockFlowDirectionGasIJ", end, 0, getFormation (), nodeDepth + 1);
      assert (propertyValue);

      GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap);

      gridMap->retrieveData ();

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < (unsigned int) nodeDepth; ++k)
            {
               double gridMapValue;
               Column * adjacentColumn = getColumn (i, j)->getAdjacentColumn (GAS);
               if (!adjacentColumn)
                  gridMapValue = 0.0;
               else
                  gridMapValue =
                  10 * ((int)(adjacentColumn->getJ ()) - (int)j) +     // J
                  1 * ((int)(adjacentColumn->getI ()) - (int)i);      // I

               gridMap->setValue (i, j, k, gridMapValue);
            }
         }
      }
      gridMap->restoreData ();

      return true;
   }

   bool Reservoir::computePorosities (void)
   {

      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("Porosity", getEnd ());

      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setPorosity (gridMap->interpolate (i, j, index) * Percentage2Fraction);
         }
      }
      gridMap->restoreData ();
      return true;
   }

   bool Reservoir::computePermeabilities (void)
   {

      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("Permeability", getEnd ());

      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setPermeability (gridMap->interpolate (i, j, index));
         }
      }
      gridMap->restoreData ();
      return true;
   }

   bool Reservoir::computeTemperatures (void)
   {
      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("Temperature", getEnd ());
      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setTemperature (gridMap->interpolate (i, j, index));
         }
      }
      gridMap->restoreData ();

      return true;
   }

   bool Reservoir::computeViscosities (void)
   {
      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("BrineViscosity", getEnd ());
      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setViscosity (gridMap->interpolate (i, j, index));
         }
      }
      gridMap->restoreData ();

      return true;

   }

   bool Reservoir::computePressures (void)
   {
      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("Pressure", getEnd ());
      if (gridMap)
      {
         unsigned int depth = gridMap->lengthK ();
         assert (depth > 1);

         gridMap->retrieveData ();
         for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
            {
               LocalColumn *column = getLocalColumn (i, j);
               double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);

               index = Max ((double)0, index);
               index = Min ((double)depth - 1, index);

               column->setPressure (gridMap->interpolate (i, j, index));
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
               double hydrostaticPressure = column->getTopDepth () * AccelerationDueToGravity * WaterDensity * PaToMegaPa;

               column->setPressure (hydrostaticPressure);
            }
         }
      }
      return true;
   }

   bool Reservoir::computeHydrostaticPressures (void)
   {

      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("HydroStaticPressure", getEnd ());

      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setHydrostaticPressure (gridMap->interpolate (i, j, index));
         }

      }
      gridMap->restoreData ();

      return true;
   }

   bool Reservoir::computeLithostaticPressures (void)
   {
      DerivedProperties::FormationPropertyPtr gridMap = getFormationPropertyPtr ("LithoStaticPressure", getEnd ());

      if (gridMap == 0)
      {
         return false;
      }

      unsigned int depth = gridMap->lengthK ();
      assert (depth > 1);

      gridMap->retrieveData ();
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            double index = (depth - 1) - column->getTopDepthOffset () * (depth - 1);
            index = Max ((double)0, index);
            index = Min ((double)depth - 1, index);

            column->setLithostaticPressure (gridMap->interpolate (i, j, index));
         }
      }
      gridMap->restoreData ();

      return true;
   }

   /// Any zero-thickness column is to become wasting
   /// Any zero-thickness neighbour of a non-zero-thickness column is to become sealing.
   bool Reservoir::refineGeometryZeroThicknessAreas (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "refineGeometryZeroThicknessAreas");

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation);

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column))
            {
               //set zero thickness areas to wasting
               if (column->getThickness () < MinimumThickness)
               {
                  column->setWasting (GAS);
                  column->setWasting (OIL);

                  for (int n = 0; n < NumNeighbours; ++n)
                  {
                     Column * neighbourColumn = getColumn (i + NeighbourOffsets2D[n][I], j + NeighbourOffsets2D[n][J]);
                     if (IsValid (neighbourColumn) and neighbourColumn->getThickness () >= MinimumThickness)
                     {
                        column->setSealing (GAS);
                        column->setSealing (OIL);
                        break;
                     }
                  }
               }
            }
         }
      }

      RequestHandling::FinishRequestHandling ();
      return true;
   }

   /// Also includes the faults into the geometry
   bool Reservoir::refineGeometrySetFaulStatus (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "refineGeometrySetFaulStatus");

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation);

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column))
            {
               //set fault status
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
               }
            }
         }
      }
      RequestHandling::FinishRequestHandling ();
      return true;
   }

   /// reset Proxies before refine
   bool Reservoir::resetProxiesBeforeRefine (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "resetProxiesBeforeRefine");

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation);

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column) and (column->getThickness () < MinimumThickness or column->getFaultStatus () != NOFAULT))
            {
               column->resetProxies ();
            }
         }
      }

      RequestHandling::FinishRequestHandling ();
      return true;
   }

   // Only for detected reservoirs! Sets columns corresponding to top nodes
   // without the reservoir flag to wasting.

   // TO DO: Account for zero-thickness elements
   void Reservoir::wasteNonReservoirColumns (const Snapshot * snapshot)
   {
      RequestHandling::StartRequestHandling (m_migrator, "wasteNonReservoirCOlumns");

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation);
      int depthIndex = formation->getNodeDepth () - 1;

      // Grid is different for nodes and columns: one more column in the x and y direction.
      for (unsigned int i = m_columnArray->firstILocal (); i <= Min (m_columnArray->lastILocal (), (unsigned int) getGrid ()->numIGlobal () - 2); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= Min (m_columnArray->lastJLocal (), (unsigned int) getGrid ()->numJGlobal () - 2); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            // Skip invalid, sealing and already wasting columns
            if (IsValid (column) and
               !column->isSealing (GAS) and !column->isSealing (OIL) and
               !column->isWasting (GAS) and !column->isWasting (OIL))
            {
               int depth = depthIndex;
               LocalFormationNode * formationNode = formation->getLocalFormationNode ((int)i, (int)j, depth);

               // Move to deeper elements until a non-zero-thickness element is found
               while (!formationNode->hasThickness () and depth > 0)
               {
                  --depth;
                  formationNode = formation->getLocalFormationNode ((int)i, (int)j, depth);
               }

               assert (formationNode->hasThickness () or depth == 0);

               if (!formationNode->getReservoirVapour () or !formationNode->getReservoirLiquid ())
               {
                  column->resetProxies ();

                  if (!formationNode->getReservoirVapour ())
                     column->setWasting (GAS);

                  if (!formationNode->getReservoirLiquid ())
                     column->setWasting (OIL);
               }
            }
         }
      }

      RequestHandling::FinishRequestHandling ();
      return;
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

   const Interface::Formation* Reservoir::getSeaBottomFormation (const Interface::Snapshot * snapshot) const
   {

      FormationList * formations = m_projectHandle->getFormations (snapshot);

      assert (formations);
      assert (formations->size () > 0);

      const Interface::Formation* topFormation = 0;

      for (size_t i = 0; i < formations->size (); ++i)
      {
         // continue downward until a gridMap is returned

         if ((*formations)[i]->getTopSurface ()->getSnapshot ()->getTime () <= snapshot->getTime ())
         {
            topFormation = (*formations)[i];
         }

      }
      delete formations;
      return topFormation;
   }

   DerivedProperties::SurfacePropertyPtr Reservoir::getSeaBottomProperty (const string & propertyName, const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      const Interface::Formation* seaFormation = getSeaBottomFormation (snapshot);

      DerivedProperties::SurfacePropertyPtr theProperty = m_migrator->getPropertyManager ().getSurfaceProperty (property, snapshot,
         (seaFormation ? seaFormation->getTopSurface () : 0));

#ifdef DEBUG   
      if( GetRank() == 0 ) {
         if( theProperty ) {
            cout << "getSeaBottomProperty " << propertyName << " for " << seaFormation->getName() << " at " << snapshot->getTime () << " at " << seaFormation->getTopSurface ()->getName()  << endl;
         } else {
            cout << "NO getSeaBottomProperty " << propertyName << " for " << seaFormation->getName() << " at " << snapshot->getTime () << " at " << seaFormation->getTopSurface ()->getName()  << endl;
         }
      }
#endif

      return theProperty;

   }

   DerivedProperties::FormationPropertyPtr Reservoir::getSeaBottomFormationProperty (const string & propertyName, const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      const Interface::Formation* seaFormation = getSeaBottomFormation (snapshot);

      DerivedProperties::FormationPropertyPtr theProperty = m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, seaFormation);

      return theProperty;
   }

   DerivedProperties::FormationPropertyPtr Reservoir::getFormationPropertyPtr (const string &              propertyName,
      const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      DerivedProperties::FormationPropertyPtr theProperty = m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, getFormation ());

      return theProperty;
   }

   const Interface::Formation* Reservoir::getTopFormation (const Interface::Snapshot * snapshot) const
   {

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      assert (formation != 0);

      const Interface::Formation* topFormation = 0;

      for (; topFormation == 0 and formation != 0; formation = formation->getTopFormation ())
      {

         if (formation->getTopSurface ()->getSnapshot ()->getTime () >= snapshot->getTime ())
         {
            topFormation = formation;
         }
      }
      return topFormation;

   }

   DerivedProperties::FormationPropertyPtr Reservoir::getTopFormationProperty (const string & propertyName, const Snapshot * snapshot) const
   {
      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      const Interface::Formation* topFormation = getTopFormation (snapshot);

      DerivedProperties::FormationPropertyPtr theProperty = m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, topFormation);

      return theProperty;
   }

   DerivedProperties::SurfacePropertyPtr Reservoir::getTopSurfaceProperty (const string & propertyName, const Snapshot * snapshot) const
   {
      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      const Interface::Formation* topFormation = getTopFormation (snapshot);

      DerivedProperties::SurfacePropertyPtr theProperty = m_migrator->getPropertyManager ().getSurfaceProperty (property, snapshot, topFormation ? topFormation->getTopSurface () : 0);

      return theProperty;
   }


   DerivedProperties::SurfacePropertyPtr Reservoir::getBottomSurfaceProperty (const string & propertyName, const Snapshot * snapshot) const
   {
      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());

      const Interface::Formation* bottomFormation = 0;

      for (; bottomFormation == 0 and formation != 0; formation = formation->getBottomFormation ())
      {

         if (formation->getBottomSurface ()->getSnapshot ()->getTime () >= snapshot->getTime ())
         {
            bottomFormation = formation;
         }
      }

      DerivedProperties::SurfacePropertyPtr theProperty = m_migrator->getPropertyManager ().getSurfaceProperty (property, snapshot, (bottomFormation ? bottomFormation->getBottomSurface () : 0));

      return theProperty;
   }

   DerivedProperties::FormationPropertyPtr Reservoir::getVolumeProperty (const Formation * formation,
      const string & propertyName,
      const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_projectHandle->findProperty (propertyName);

      DerivedProperties::FormationPropertyPtr result;


      if (property != 0)
      {
         result = m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, formation);
      }

      return result;
   }


   const GridMap * Reservoir::getPropertyGridMap (const string & propertyName,
      const Snapshot * snapshot,
      const Interface::Reservoir * reservoir, const Interface::Formation * formation, const Interface::Surface * surface) const
   {
      int selectionFlags = 0;

      if (reservoir) selectionFlags |= Interface::RESERVOIR;
      if (formation and !surface) selectionFlags |= Interface::FORMATION;
      if (surface and !formation) selectionFlags |= Interface::SURFACE;
      if (formation and surface) selectionFlags |= Interface::FORMATIONSURFACE;

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

   const Grid * Reservoir::getGrid (void) const
   {
      return getProjectHandle ()->getHighResolutionOutputGrid (); // not to be changed to getActivityOutputGrid ()!!
   }

   double Reservoir::getNeighbourDistance (int n)
   {
      double & distance = m_neighbourDistances[n];
      if (distance < 0)
      {
         double deltaI = Abs (NeighbourOffsets2D[n][0]) * getGrid ()->deltaIGlobal ();
         double deltaJ = Abs (NeighbourOffsets2D[n][1]) * getGrid ()->deltaJGlobal ();

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

   LocalColumn * Reservoir::getLocalColumn (unsigned int i, unsigned int j) const
   {
      return m_columnArray->getLocalColumn (i, j);
   }

   ProxyColumn * Reservoir::getProxyColumn (unsigned int i, unsigned int j)
   {
      return m_columnArray->getProxyColumn (i, j);
   }

   Column * Reservoir::getColumn (unsigned int i, unsigned int j) const
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

   /// See whether distribution has finished on all processors
   bool Reservoir::allProcessorsFinished (bool finished)
   {
      return (AndAll ((int)finished) != 0);
   }

   int Reservoir::computeMaximumTrapCount (bool countUndersized)
   {
      int numberOfTraps = (int)m_traps.size ();
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
      DerivedProperties::SurfacePropertyPtr formationTopDepthMap = getTopSurfaceProperty (depthPropertyName (), presentDay);
      DerivedProperties::SurfacePropertyPtr formationBottomDepthMap = getBottomSurfaceProperty (depthPropertyName (), presentDay);

      if (!formationTopDepthMap)
      {
         if (GetRank () == 0)
         {
            cerr << "WARNING: property value '" << depthPropertyName () << "' does not exist for top surface of formation "
               << getFormation ()->getName () << " at snapshot " << presentDay->getTime () <<
               ",\n\tcannot compute depth offsets for reservoir " << getName () << endl;
            cerr.flush ();
         }
         return false;
      }

      if (!formationBottomDepthMap)
      {
         if (GetRank () == 0)
         {
            cerr << "WARNING: property value '" << depthPropertyName () << "' does not exist for bottom surface of formation "
               << getFormation ()->getName () << " at snapshot " << presentDay->getTime () <<
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

	  //By default the m_topDepthOffset= m_bottomDepthOffset = 0
	  if (!m_migrator->performLegacyMigration() and (depthOffsetMap or thicknessMap))
	  {
        LogHandler (LogHandler::WARNING_SEVERITY) << "Reservoir " << getName() << ": Offset and/or thickness maps for reservoirs cannot be used in non-legacy mode.\nThese inputs will be ignored in this run.";
		  	  
        depthOffsetMap = nullptr;
        thicknessMap   = nullptr;
	  }

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);

            double formationTopDepth = formationTopDepthMap->get (i, j);
            double formationBottomDepth = formationBottomDepthMap->get (i, j);

            if (formationTopDepth == formationTopDepthMap->getUndefinedValue () or
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

      const Formation * formation = dynamic_cast<const Formation *>(getFormation ());
      const Surface * topSurface = (const Surface *)formation->getTopSurface ();
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
      if (!isOilToGasCrackingOn ()) return false;

      setSourceReservoir (this);

      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            column->crackChargesToBeMigrated (otgc, m_start->getTime (), m_end->getTime ());
         }
      }

      processMigrationRequests ();
      setSourceReservoir (0);
      return true;
   }
#endif



   bool Reservoir::collectExpelledCharges (const Formation * formation, unsigned int direction, Barrier * barrier)
   {
      if (direction == EXPELLEDNONE) return true;

      PetscBool genexMinorSnapshots;
      PetscBool genexFraction;
      PetscBool printDebug;

      PetscOptionsHasName (PETSC_NULL, "-genex", &genexMinorSnapshots);
      PetscOptionsHasName (PETSC_NULL, "-genexf", &genexFraction);
      PetscOptionsHasName (PETSC_NULL, "-debug", &printDebug);

      const double depoTime = formation->getTopSurface ()->getSnapshot ()->getTime ();
      bool sourceRockIsActive = (depoTime > getStart ()->getTime ()) or fabs (depoTime - getStart ()->getTime ()) < Genex6::Constants::Zero;

      Formation * srFormation = const_cast<Formation *>(formation);

      if (genexMinorSnapshots)
      {
         if (sourceRockIsActive)
         {
            if (!formation->isPreprocessed ())
            {
               bool status = srFormation->preprocessSourceRock (getStart ()->getTime (), printDebug);
               if (!status and GetRank () == 0)
               {
                  LogHandler (LogHandler::ERROR_SEVERITY) << "Cannot preprocess " << formation->getName () << ", depoage= " << depoTime << " at " << getStart ()->getTime ();
               }

            }
            bool status = srFormation->calculateGenexTimeInterval (getStart (), getEnd (), printDebug);

            if (status)
            {
               double fraction = (direction == EXPELLEDUPANDDOWNWARD ? 1.0 : 0.5);

               addChargesToBeMigrated (formation->getGenexData (), fraction, barrier);

            }
            else
            {
               if (GetRank () == 0)
               {
                  LogHandler (LogHandler::ERROR_SEVERITY) << "Cannot calculate genex " << getName () << ", depoage= " << depoTime << " at " << getStart ()->getTime ();
               }

            }
            return status;
         }
         return true;
      }

      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId]) continue;

         string propertyName = CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId );
         propertyName += "ExpelledCumulative";

         const GridMap * gridMapEnd = getPropertyGridMap (propertyName, getEnd (), 0, formation, 0);
         const GridMap * gridMapStart = getPropertyGridMap (propertyName, getStart (), 0, formation, 0);

         double fraction = (direction == EXPELLEDUPANDDOWNWARD ? 1.0 : 0.5);

         // Initialized to stop the VS runtime error. The valuer is -1 because there is no snapshot at that time
         // so we make sure that these variables are assigned real paleo times.
         double startTime = -1.0, endTime = -1.0, fractionToMigrate = 1.0;

         if (gridMapStart)
         {
            startTime = getStart ()->getTime ();
         }
         else  if (genexFraction)
         {
            const Snapshot * startSnapshot = m_projectHandle->findPreviousSnapshot (getStart ()->getTime ());
            startTime = startSnapshot->getTime ();
            gridMapStart = getPropertyGridMap (propertyName, startSnapshot, 0, formation, 0);
         }
         if (gridMapEnd)
         {
            endTime = getEnd ()->getTime ();
         }
         else if (genexFraction)
         {
            const Snapshot * endSnapshot = m_projectHandle->findNextSnapshot (getEnd ()->getTime ());
            endTime = endSnapshot->getTime ();
            gridMapEnd = getPropertyGridMap (propertyName, endSnapshot, 0, formation, 0);
         }

         if (genexFraction and startTime - endTime > 0)
         {
            if (gridMapStart and gridMapEnd)
            {
               fractionToMigrate = (getStart ()->getTime () - getEnd ()->getTime ()) / (startTime - endTime);
               if (endTime == getEnd ()->getTime ())
               {
                  //fractionToMigrate =  the rest
               }
            }
         }

         if (gridMapStart and gridMapEnd)
         {
            if (GetRank () == 0 and  printDebug)
            {
               if (propertyName == "asphaltenesExpelledCumulative")
               {
                  cout << formation->getName () << ": " << propertyName << ": Start = " << startTime << "(" << getStart ()->getTime () << "), End = " << endTime
                     << "(" << getEnd ()->getTime () << "),fraction = " << fractionToMigrate << endl;
               }
            }
         }
         else
         {
            if (gridMapStart and !gridMapEnd)
            {
               if (GetRank () == 0 and  printDebug)
               {
                  if (propertyName == "asphaltenesExpelledCumulative")
                  {
                     cout << formation->getName () << ": " << propertyName << ": only start " << startTime << " and no " << getEnd ()->getTime () << " depotime = " << depoTime << endl;
                  }
               }
            }
            else if (!gridMapStart and gridMapEnd)
            {
               if (GetRank () == 0 and  printDebug)
               {
                  if (propertyName == "asphaltenesExpelledCumulative")
                  {
                     cout << formation->getName () << ": " << propertyName << ": only end " << endTime << " and no " << getStart ()->getTime () << " depotime = " << depoTime << endl;
                  }
               }
            }
         }

         if (gridMapEnd)
         {
            gridMapEnd->retrieveData ();
            addChargesToBeMigrated ((ComponentId)componentId, gridMapEnd, fraction * fractionToMigrate, barrier);
            gridMapEnd->restoreData ();
         }

         if (gridMapStart)
         {
            gridMapStart->retrieveData ();
            subtractChargesToBeMigrated ((ComponentId)componentId, gridMapStart, fraction * fractionToMigrate, barrier);
            gridMapStart->restoreData ();
         }
      }

      return true;
   }


   bool Reservoir::saveGenexMaps (const string & speciesName, DataAccess::Interface::GridMap * aMap, const Formation * formation, const Snapshot * aSnapshot)
   {

      const Interface::Surface   * topMap = formation->getTopSurface ();
      const string topSurfaceName = topMap->getName ();

      float time = (float)aSnapshot->getTime ();

      const string extensionString = ".HDF";
      Interface::MapWriter * mapWriter = m_projectHandle->getFactory ()->produceMapWriter ();

      const string dirToOutput = m_projectHandle->getProjectName () + "_CauldronOutputDir/";

      // string outputFileName = projectHandle->getProjectName() + "_" + outputMapsNames[i] + string(ageString) + extensionString;
      string outputFileName = dirToOutput + formation->getName () + "_" + speciesName + "_" + aSnapshot->asString () + extensionString;

      // Put 0 as a DataSetName to make comparison with regression tests results easier. Also 0 should be there if we want to re-use the map in fastcauldron
      string dataSetName = speciesName; //"0"; //outputMapsNames[i];
      dataSetName += "_";
      dataSetName += aSnapshot->asString ();
      dataSetName += "_";
      dataSetName += topSurfaceName;

      mapWriter->open (outputFileName, false);
      mapWriter->saveDescription (m_projectHandle->getActivityOutputGrid ());

      mapWriter->writeMapToHDF (aMap, time, time, dataSetName, topSurfaceName);
      mapWriter->close ();

      if (GetRank () == 0)
      {
         cout << "Map " << speciesName << " at " << time << " is saved into " << outputFileName << endl;
      }
      delete mapWriter;
      return true;
   }

   void Reservoir::deleteExpelledChargeMaps (const Formation * formation)
   {
      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId]) continue;

         string propertyName = CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId );
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
            if (IsValid (column) and IsValid (leakingColumn) and
               !leakingColumn->isOnBoundary ())
            {
               if (barrier and barrier->isBlocking (i, j))
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

      RequestHandling::StartRequestHandling (m_migrator, "migrateChargesToBeMigrated");
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


	  RequestHandling::StartRequestHandling(m_migrator, "addTargetBuffer");
	  for (unsigned int i = m_columnArray->firstILocal(); i <= m_columnArray->lastILocal(); ++i)
	  {
		  for (unsigned int j = m_columnArray->firstJLocal(); j <= m_columnArray->lastJLocal(); ++j)
		  {
			  LocalColumn * column = getLocalColumn(i, j);
			  column->addTargetBuffer();
		  }
	  }
	  RequestHandling::FinishRequestHandling();

      processMigrationRequests ();

      setSourceFormation (0);
      setSourceReservoir (0);


      return true;
   }

   double Reservoir::getTotalToBeStoredCharges (bool onBoundaryOnly)
   {
      double total = 0;

      RequestHandling::StartRequestHandling (m_migrator, "getTotalToBeStoredCharges");
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column) and (!onBoundaryOnly or column->isOnBoundary ()))
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

      RequestHandling::StartRequestHandling (m_migrator, "getTotalChargesToBeMigrated");
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

      if (!computeDistributionParameters ())
         return false;

      m_biodegraded = 0;
      if (isBioDegradationOn ())
      {
         if (!m_migrator->performLegacyMigration ())
         {
            if (!computeHydrocarbonWaterContactDepth () or
                !computeHydrocarbonWaterTemperature () or 
                !needToComputePasteurizationStatusFromScratch() or
                !pasteurizationStatus() or
                !setPasteurizationStatus())
               return false;
         }
         m_biodegraded = biodegradeCharges ();
      }

      do
      {
         collectAndSplitCharges ();
         distributeCharges ();

         mergeSpillingTraps ();
         processMigrationRequests ();
      }
      while (!allProcessorsFinished (distributionHasFinished ()));

      // Optimization for May 2016 Release
      if (isDiffusionOn () and !m_migrator->performLegacyMigration ())
      {
         broadcastTrapFillDepthProperties ();
         if (!diffusionLeakCharges ())
         {
            return false;
         }

         // To get the fill heights, etc. correct again!
         collectAndSplitCharges (true);
         distributeCharges (true);
      }

      reportLeakages ();

      processMigrationRequests ();
      setSourceReservoir (0);
      return true;
   }

   /// See whether distribution of charge has finished on this processor.
   bool Reservoir::distributionHasFinished (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "distributionHasFinished");

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
      RequestHandle requestHandle (m_migrator, "computeDistributionParameters");

      bool isLegacy = m_migrator->performLegacyMigration();

      const FracturePressureFunctionParameters *fracturePressureFunctionParameters =
         getProjectHandle ()->getFracturePressureFunctionParameters ();

      {
         RetrieveAndRestoreSurfaceGridMapContainer retrieve (m_sealPressureLeakageGridMaps);

         for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
         {
            if (!(*trapIter)->computeDistributionParameters (fracturePressureFunctionParameters,
               m_sealPressureLeakageGridMaps, m_end, isLegacy ) )
               return false;
         }

      }                            // ~retrieve

      return true;
   }

   bool Reservoir::computeHydrocarbonWaterContactDepth ()
   {
      RequestHandling::StartRequestHandling (m_migrator, "computeHydrocarbonWaterContactDepth");
      bool succeded;

      if (m_traps.size () > 0)
         succeded = false;
      else
         succeded = true;

      for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         succeded = (*trapIter)->computeHydrocarbonWaterContactDepth ();
         if (!succeded) break;
      }
      RequestHandling::FinishRequestHandling ();

      return succeded;
   };

   bool Reservoir::computeHydrocarbonWaterTemperature ()
   {
      RequestHandling::StartRequestHandling (m_migrator, "computeHydrocarbonWaterTemperature");
      bool succeded;

      if (m_traps.size () > 0)
         succeded = false;
      else
         succeded = true;

      for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         succeded = (*trapIter)->computeHydrocarbonWaterContactTemperature ();
         if (!succeded) break;
      }
      RequestHandling::FinishRequestHandling ();

      return succeded;
   };
   
   bool Reservoir::needToComputePasteurizationStatusFromScratch()
   {
	   RequestHandling::StartRequestHandling(m_migrator, "needToComputePasteurizationStatusFromScratch");

	   const BiodegradationParameters* biodegradationParameters =
		   getProjectHandle()->getBiodegradationParameters();

	   Biodegrade biodegrade(biodegradationParameters);

	   // If the user has toggle on the pasteurization effect
	   if (biodegrade.pasteurizationInd())
	   {
		   for (TrapVector::iterator trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter)
		   {
			   (*trapIter)->needToComputePasteurizationStatusFromScratch();
		   }
	   }
	   RequestHandling::FinishRequestHandling();

	   return true;
   };


   bool Reservoir::pasteurizationStatus()
   {
	   RequestHandling::StartRequestHandling(m_migrator, "pasteurizationStatus");

	   const BiodegradationParameters* biodegradationParameters =
		   getProjectHandle()->getBiodegradationParameters();

	   Biodegrade biodegrade(biodegradationParameters);

	   // If the user has toggle on the pasteurization effect
	   if (biodegrade.pasteurizationInd())
	   {
		   for (TrapVector::iterator trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter)
		   {
			  (*trapIter)->pasteurizationStatus(biodegrade.maxBioTemp());
		   }
	   }

	   RequestHandling::FinishRequestHandling();

	   return true;
   };


   bool Reservoir::setPasteurizationStatus()
   {
	   RequestHandling::StartRequestHandling(m_migrator, "setPasteurizationStatus");

	   const BiodegradationParameters* biodegradationParameters =
		   getProjectHandle()->getBiodegradationParameters();

	   Biodegrade biodegrade(biodegradationParameters);

	   bool succeded = true;

	   // If the user has toggle on the pasteurization effect
	   if (biodegrade.pasteurizationInd())
	   {
		   for (TrapVector::iterator trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter)
		   {
			   succeded = (*trapIter)->setPasteurizationStatus(biodegrade.maxBioTemp());
			   if (!succeded) break;
		   }
	   }
	   RequestHandling::FinishRequestHandling();

	   return succeded;
   }; 

   double Reservoir::biodegradeCharges ()
   {
      RequestHandling::StartRequestHandling (m_migrator, "biodegradeCharges");

      double biodegraded = 0;

      const BiodegradationParameters* biodegradationParameters =
         getProjectHandle ()->getBiodegradationParameters ();
      double timeInterval = m_start->getTime () - m_end->getTime ();

      if (timeInterval >= 30 and !m_migrator->performLegacyMigration ())
      {
         getProjectHandle ()->getMessageHandler ().print ("WARNING: The time interval between the two snapshots ");
         getProjectHandle ()->getMessageHandler ().print (m_start->getTime ());
         getProjectHandle ()->getMessageHandler ().print (" Ma and ");
         getProjectHandle ()->getMessageHandler ().print (m_end->getTime ());
         getProjectHandle ()->getMessageHandler ().printLine (" Ma involving biodegradation is bigger than 30 Ma, the biodegradation results can be questionable due to the large time interval");
      }
      Biodegrade biodegrade (biodegradationParameters);

      if (m_migrator->performLegacyMigration ())
      {
         for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
         {
            biodegraded += (*trapIter)->biodegradeChargesLegacy (timeInterval, biodegrade);
         }
      }
      else
      {
         for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
         {
            biodegraded += (*trapIter)->biodegradeCharges (timeInterval, biodegrade);
         }
      }

      RequestHandling::FinishRequestHandling ();
      return biodegraded;
   }

   bool Reservoir::isDiffusionOn (void)
   {
      return Interface::Reservoir::isDiffusionOn ();
   }

   bool Reservoir::diffusionLeakCharges ()
   {
      RequestHandle requestHandle (m_migrator, "diffusionLeakCharges");

      // maximum error in the flux (kg/m2/Ma)
      double maxFluxError = 0.05;
      char *envStr = getenv ("Diffusion_MaxFluxError");

      if (envStr and strlen (envStr) != 0)
         maxFluxError = atof (envStr);

      // maximum timestep
      double maxTimeStep = 50;

      envStr = getenv ("Diffusion_MaxTimeStep");
      if (envStr and strlen (envStr) != 0)
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

   bool Reservoir::distributeCharges (bool always)
   {
      bool distributionFinished = true;
      RequestHandling::StartRequestHandling (m_migrator, "distributeCharges");
      TrapVector::iterator trapIter;
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         if (always and !(*trapIter)->diffusionLeakageOccoured ())
            continue;
         else
            distributionFinished &= (*trapIter)->distributeCharges ();
      }
      RequestHandling::FinishRequestHandling ();

	  RequestHandling::StartRequestHandling(m_migrator, "addSpillBuffer");
	  for (unsigned int i = m_columnArray->firstILocal(); i <= m_columnArray->lastILocal(); ++i)
	  {
		  for (unsigned int j = m_columnArray->firstJLocal(); j <= m_columnArray->lastJLocal(); ++j)
		  {
			  LocalColumn * column = getLocalColumn(i, j);
			  column->addSpillBuffer();
		  }
	  }
	  RequestHandling::FinishRequestHandling();


	  RequestHandling::StartRequestHandling(m_migrator, "addWasteBuffer");
	  for (unsigned int i = m_columnArray->firstILocal(); i <= m_columnArray->lastILocal(); ++i)
	  {
		  for (unsigned int j = m_columnArray->firstJLocal(); j <= m_columnArray->lastJLocal(); ++j)
		  {
			  LocalColumn * column = getLocalColumn(i, j);
			  column->addWasteBuffer();
		  }
	  }
	  RequestHandling::FinishRequestHandling();


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
      return (columnA->isUndersized () or columnB->isUndersized ());
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
      }
      while (!allProcessorsFinished (noTrapsWereMerged));

      return true;
   }
#endif

   /// determine whether and identify traps that need to merge
   /// returns true if no traps needed to merge
   bool Reservoir::determineTrapsToMerge (ConditionTest conditionTest)
   {
      bool noTrapsToMerge = true;

      RequestHandling::StartRequestHandling (m_migrator, "determineTrapsToMerge");
      for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         LocalColumn * crestColumn = trap->getCrestColumn ();
         for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
         {
            Column * spillBackColumn;
            if (((spillBackColumn = crestColumn->getSpillBackTarget ((PhaseId)phase)) != 0) and
               (*conditionTest) (crestColumn, spillBackColumn))
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


	  RequestHandling::StartRequestHandling(m_migrator, "addMergedBuffer");
	  for (unsigned int i = m_columnArray->firstILocal(); i <= m_columnArray->lastILocal(); ++i)
	  {
		  for (unsigned int j = m_columnArray->firstJLocal(); j <= m_columnArray->lastJLocal(); ++j)
		  {
			  LocalColumn * column = getLocalColumn(i, j);
			  column->addMergedBuffer();
		  }
	  }
	  RequestHandling::FinishRequestHandling();

      return noTrapsToMerge;
   }

   static bool TrapsAreSpilling (Column * columnA, Column * columnB)
   {
      return (columnA->isSpilling () and columnB->isSpilling ());
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
      RequestHandling::StartRequestHandling (m_migrator, "absorbTraps");
      TrapVector::iterator trapIter;
      for (trapIter = m_traps.begin (); trapIter != m_traps.end ();)
      {
         Trap * trap = *trapIter;
         if (trap->isToBeAbsorbed ())
         {
            trap->beAbsorbed ();

            delete trap;
            trapIter = m_traps.erase (trapIter);
            continue;
         }
         ++trapIter;
      }
      RequestHandling::FinishRequestHandling ();
   }

   void Reservoir::completeTrapExtensions (void)
   {
      RequestHandling::StartRequestHandling (m_migrator, "completeTrapExtensions");
      for (TrapVector::iterator trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         if (trap->hasBeenExtended ())
         {
            trap->completeExtension ();
         }
      }
      RequestHandling::FinishRequestHandling ();
   }

   void Reservoir::broadcastTrapDiffusionStartTimes (void)
   {
      // Optimization for May 2016 Release
      if (!isDiffusionOn () or m_migrator->performLegacyMigration ()) return;

      TrapVector::iterator trapIter;

      RequestHandling::StartRequestHandling (m_migrator, "broadcastTrapDiffusionStartTimes");
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         trap->broadcastDiffusionStartTimes ();
      }
      RequestHandling::FinishRequestHandling ();
   }

   void Reservoir::broadcastTrapPenetrationDistances (void)
   {
      // Optimization for May 2016 Release
      if (!isDiffusionOn () or m_migrator->performLegacyMigration ()) return;

      const DiffusionLeakageParameters *parameters = getProjectHandle ()->getDiffusionLeakageParameters ();

      if (parameters->transientModel () != Interface::Transient) return;

      TrapVector::iterator trapIter;

      RequestHandling::StartRequestHandling (m_migrator, "broadcastTrapPenetrationDistances");
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         trap->broadcastPenetrationDistances ();
      }
      RequestHandling::FinishRequestHandling ();

   }

   void Reservoir::broadcastTrapFillDepthProperties (void)
   {
      TrapVector::iterator trapIter;

      RequestHandling::StartRequestHandling (m_migrator, "broadcastTrapFillDepthProperties");
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         trap->broadcastFillDepthProperties ();
      }
      RequestHandling::FinishRequestHandling ();
   }

   void Reservoir::broadcastTrapChargeProperties (void)
   {
      TrapVector::iterator trapIter;

      RequestHandling::StartRequestHandling (m_migrator, "broadcastTrapChargeProperties");
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         Trap * trap = *trapIter;
         trap->broadcastChargeProperties ();
      }
      RequestHandling::FinishRequestHandling ();
   }

   void Reservoir::collectAndSplitCharges (bool always)
   {
      RequestHandling::StartRequestHandling (m_migrator, "collectAndSplitCharges");
      TrapVector::iterator trapIter;

      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         if (always and !(*trapIter)->diffusionLeakageOccoured ())
            continue;
         else
            (*trapIter)->collectAndSplitCharges (always);
      }
      RequestHandling::FinishRequestHandling ();
   }

   /// debugging function
   bool Reservoir::checkDistribution (void)
   {
      bool result = true;
      RequestHandling::StartRequestHandling (m_migrator, "checkDistribution");
      for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
         {
            LocalColumn * column = getLocalColumn (i, j);
            if (IsValid (column))
            {
               for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
               {
                  if (column->getFinalTargetColumn ((PhaseId)phase) != column)
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
                  if (barrier and barrier->isBlocking (i, j))
                     addBlocked (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
                  else
                     column->addComponentToBeMigrated (componentId, gridMap->getValue (i, j) * fraction * getSurface (i, j));
               }
            }
         }
      }
      return true;
   }
   /// add charges to the reservoir
   bool Reservoir::addChargesToBeMigrated (const Interface::GridMap * gridMap, double fraction, Barrier * barrier)
   {
      gridMap->retrieveData ();

      for (unsigned int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId]) continue;

         for (unsigned int i = m_columnArray->firstILocal (); i <= m_columnArray->lastILocal (); ++i)
         {
            for (unsigned int j = m_columnArray->firstJLocal (); j <= m_columnArray->lastJLocal (); ++j)
            {

               LocalColumn * column = getLocalColumn (i, j);
               double value = gridMap->getValue (i, j, componentId);

               if (value != gridMap->getUndefinedValue ())
               {
                  if (IsValid (column))
                  {
                     if (barrier and barrier->isBlocking (i, j))
                     {
                        addBlocked ((ComponentId)componentId, value * fraction * getSurface (i, j));
                     }
                     else
                     {
                        column->addComponentToBeMigrated ((ComponentId)componentId, value * fraction * getSurface (i, j));
                     }
                  }
               }
            }
         }
      }
      gridMap->restoreData ();

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
            if (IsValid (column) and gridMap->getValue (i, j) != gridMap->getUndefinedValue ())
            {
               if (barrier and barrier->isBlocking (i, j))
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
                     << ": expelled weight of " << CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId ) << " is negative (" << column->getComponent (componentId) << ")" << endl;
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
         Trap * trap = *trapIter;
         if (trap->getGlobalId () == globalId)
            return trap;
      }
      return 0;
   }

   void Reservoir::addTrap (Trap * trap)
   {
      m_traps.push_back (trap);
      trap->setLocalId ((int)m_traps.size ());
   }

   void Reservoir::processTrapProperties (TrapPropertiesRequest & tpRequest)
   {
      assert (tpRequest.valueSpec == SAVETRAPPROPERTIES);

      m_migrator->addTrapRecord (this, tpRequest);
   }

   double Reservoir::getUndefinedValue (void)
   {
      return Interface::DefaultUndefinedMapValue;
   }

#ifdef __cplusplus
   extern "C" {
#endif
      static int TrapPropertiesCompare (const void * elem1, const void * elem2);
#ifdef __cplusplus
   }
#endif

   static int TrapPropertiesCompare (const void * elem1, const void * elem2)
   {
      const TrapPropertiesRequest * tpr1 = (const TrapPropertiesRequest *)elem1;
      const TrapPropertiesRequest * tpr2 = (const TrapPropertiesRequest *)elem2;
      double weight1 = tpr1->composition.getWeight ();
      double weight2 = tpr2->composition.getWeight ();

      if (tpr1->rank < 0 and tpr2->rank >= 0) return 1;
      if (tpr1->rank >= 0 and tpr2->rank < 0) return -1;
      if (tpr1->rank < 0 and tpr2->rank < 0) return 0;

      if (weight1 > weight2) return -1;
      else if (weight1 < weight2) return 1;
      else if (tpr1->capacity > tpr2->capacity) return -1;
      else if (tpr1->capacity < tpr2->capacity) return 1;
      else return 0;
   }

   void Reservoir::collectTrapProperties (TrapPropertiesRequest * tpRequests, unsigned int maxNumberOfRequests)
   {
      RequestHandling::StartRequestHandling (m_migrator, "collectTrapProperties");

      TrapVector::iterator trapIter;
      unsigned int i = 0;
      for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
      {
         assert (i < maxNumberOfRequests);
         Trap * trap = *trapIter;
         trap->collectProperties (tpRequests[i]);
         ++i;
      }

      RequestHandling::FinishRequestHandling ();
   }

   /// renumber traps according to the amount of stored charge
   /// save the trap properties to the TrapIoTbl
   bool Reservoir::saveTrapProperties (const bool saveSnapshot)
   {
      if (saveSnapshot)
      {
         unsigned int maxLocalNumberOfTraps = computeMaximumTrapCount (true);
         TrapPropertiesRequest * trapsIn = new TrapPropertiesRequest[maxLocalNumberOfTraps];

         collectTrapProperties (trapsIn, maxLocalNumberOfTraps);

         unsigned int maxGlobalNumberOfTraps = maxLocalNumberOfTraps * NumProcessors ();
         TrapPropertiesRequest * trapsOut = new TrapPropertiesRequest[maxGlobalNumberOfTraps];

         // collect trap info from all processors and sort it according to charge content
         AllGatherFromAll (trapsIn, maxLocalNumberOfTraps, TrapPropertiesType,
            trapsOut, maxLocalNumberOfTraps, TrapPropertiesType);

         delete[] trapsIn;

         // sort the traps according to the weight of their content, largest weight first
         qsort (trapsOut, maxGlobalNumberOfTraps, sizeof (TrapPropertiesRequest), TrapPropertiesCompare);

         populateMigrationTables (trapsOut, maxGlobalNumberOfTraps);
         eliminateUndersizedTraps (trapsOut, maxGlobalNumberOfTraps);

         delete[] trapsOut;
      }
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
               m_migrator->renumberMigrationRecordTrap (getEnd (), tpRequests[i].id,
                  tpRequests[i].id + trapNumberOffset);
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
               m_migrator->renumberMigrationRecordTrap (getEnd (), tpRequests[i].id, i + 1);
            }

            changeTrapPropertiesRequestId (tpRequests, maxNumberOfRequests, tpRequests[i].id, i + 1);

            if (GetRank () == 0 and tpRequests[i].capacity >= minimumCapacity)
            {
               m_migrator->addTrapRecord (this, tpRequests[i]);
            }
         }
      }

      // renumber the Traps themselves to new id's
      RequestHandling::StartRequestHandling (m_migrator, "populateMigrationTables");

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
      RequestHandling::StartRequestHandling (m_migrator, "eliminateUndersizedTraps");

      vector<int> rank;
      vector<int> from;
      vector<int> to;

      std::vector<int> finalIds;
      finalIds.resize (maxNumberOfRequests);

      const int maxIterations = 100;

      double minimumCapacity = getTrapCapacity ();
      unsigned int i;
      for (i = 0; i < maxNumberOfRequests; ++i)
      {
         int originatingId = tpRequests[i].id;
         int currentId = tpRequests[i].id;
         double currentCapacity = tpRequests[i].capacity;
         if (currentId < 0 or currentCapacity >= minimumCapacity)
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
         while (++iteration < maxIterations and currentlySpilling and currentId > 0 and currentCapacity < minimumCapacity)
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

         if (currentId == tpRequests[i].id or currentCapacity < minimumCapacity) // Nothing came by, not spilling or too small.
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

            m_migrator->renumberMigrationRecordTrap (getEnd (), originatingId, currentId);
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
      assert (m_sourceReservoir or m_sourceFormation);

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
      {
         m_migrator->addMigrationRecord (m_sourceReservoir ? m_sourceReservoir->getName () : string (""),
            m_sourceFormation ? m_sourceFormation->getName () : string (""),
            getName (), mr);

         return true;
         break;
      }
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
      {
         m_migrator->renumberMigrationRecordTrap (getEnd (), mr.source.trapId, mr.destination.trapId);
         return true;
         break;
      }
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
         MigrationRequest & thisRequest = *mrIter;
         if (request == thisRequest)
         {
            return &thisRequest;
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
      int localSize = (int)m_migrationRequests.size ();

      // determine maximum number of requests per processor
      int localMaximumSize = MaximumAll (localSize);
      if (localMaximumSize == 0) return;

#if 0
      ReportProgress ("starting processing migration requests");
#endif

      int globalMaximumSize = NumProcessors () * localMaximumSize;

      // put all requests into a C array
      MigrationRequest * localMigrationRequestArray = new MigrationRequest[localMaximumSize];

      vector<MigrationRequest>::iterator mrIter;

      int i = 0;
      if (GetRank () != 0)
      {
         for (mrIter = m_migrationRequests.begin (), i = 0; mrIter != m_migrationRequests.end (); ++mrIter, ++i)
         {
            localMigrationRequestArray[i] = *mrIter;
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
      PetscSynchronizedFlush (PETSC_COMM_WORLD, PETSC_STDOUT);
#endif

      if (useAllGather or GetRank () == 0)
      {
         globalMigrationRequestArray = new MigrationRequest[globalMaximumSize];
      }

#ifdef DEBUG_GATHER
      PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Gathering... %d (%d) elements\n", GetRank (), localSize, localMaximumSize);
      PetscSynchronizedFlush (PETSC_COMM_WORLD, PETSC_STDOUT);
#endif

      if (useAllGather)
      {
#ifdef DEBUG_GATHER
         PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Using AllGatherFromAll...\n", GetRank());
         PetscSynchronizedFlush (PETSC_COMM_WORLD, PETSC_STDOUT);
#endif
         AllGatherFromAll (localMigrationRequestArray, localMaximumSize, MigrationType,
            globalMigrationRequestArray, localMaximumSize, MigrationType);
      }
      else
      {
#ifdef DEBUG_GATHER
         PetscSynchronizedPrintf (PETSC_COMM_WORLD, "%d: Using RootGatherFromAll...\n", GetRank());
         PetscSynchronizedFlush (PETSC_COMM_WORLD, PETSC_STDOUT);
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
            processMigration (*mrIter);
         }

         for (mrIter = m_migrationRequests.begin (), i = 0; mrIter != m_migrationRequests.end (); ++mrIter, ++i)
         {
            processAbsorption (*mrIter);
         }
      }

#ifdef DEBUG_GATHER
      PetscPrintf (PETSC_COMM_WORLD, "Deleting...\n");
#endif

      if (useAllGather or GetRank () == 0)
         delete[] globalMigrationRequestArray;

      m_migrationRequests.clear ();
      delete[] localMigrationRequestArray;

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

   void Reservoir::manipulateColumnCompositionPosition(ColumnCompositionPositionRequest & compositionPositionRequest)
   {
	   getLocalColumn(compositionPositionRequest.i, compositionPositionRequest.j)->manipulateCompositionPosition(compositionPositionRequest.valueSpec,
		   compositionPositionRequest.phase, compositionPositionRequest.position, compositionPositionRequest.composition);
   }

   void Reservoir::getColumnComposition (ColumnCompositionRequest & compositionRequest, ColumnCompositionRequest & compositionResponse)
   {
      compositionResponse.reservoirIndex = getIndex ();
      compositionResponse.valueSpec = compositionRequest.valueSpec;

      getLocalColumn (compositionRequest.i, compositionRequest.j)->getComposition (compositionRequest.valueSpec,
         compositionRequest.phase, compositionResponse.composition);
   }
}
