//
// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RequestHandling.h"
#include "Trap.h"
#ifdef USEOTGC
#include "OilToGasCracker.h"
#include "Immobiles.h"
#endif
#include "Column.h"
#include "Reservoir.h"
#include "Composition.h"
#include "rankings.h"
#include "migration.h"
#include "MassBalance.h"
#include "DiffusionLeak.h"
#include "Interpolator.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "methaneSolubility.h"
#include "capillarySealStrength.h"
#include "fracturePressure.h"
#include "waterDensity.h"
#include "Tuple2.h"
#include "depthToVolume.h"
#include "Interface/FluidType.h"
#include "GeoPhysicsFluidType.h"
#include "translateProps.h"
#include "LeakWasteAndSpillDistributor.h"
#include "LeakAllGasAndOilDistributor.h"
#include "SpillAllGasAndOilDistributor.h"
#include "utils.h"

// std library
#include <assert.h>
#include <algorithm>
#include <vector>
#include <sstream>
using namespace std;
using std::ostringstream;
extern ostringstream cerrstrstr;
#define MAXDOUBLE std::numeric_limits<double>::max()

using Interface::Formation;
using Interface::Snapshot;
using functions::Tuple2;

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::CelciusToKelvin;

// #define DEBUG_TRAP
// #define DEBUG_BIODEGRADATION
using namespace CBMGenerics;


namespace migration
{

   Trap::Trap (LocalColumn * column) :
      m_levelToVolume (0),
#ifdef DEBUG_TRAP
      m_volumeToDepth2(0),
#endif
      m_distributor (0),
      m_globalId (UnknownTrapId),
      m_drainageAreaId (UnknownTrapId),
      m_diffusionStartTime (-1),
      m_diffusionOverburdenProps (0),
      m_hydrocarbonWaterContactDepth (-199999),
      m_isPasteurized (false),
      m_biodegraded (false)
   {
      column->setTrap (this);
      m_reservoir = column->getReservoir ();
      addToInterior (column);
      assert (isInInterior (getCrestColumn ()));
      closePerimeter (column);
      assert (isOnPerimeter (getSpillColumn ()));

#ifdef THISISNOTRIGHT
      m_minimumSpillDepth = WasteDepth;
      setMinimumSpillDepth (column->getTopDepth ());
#endif

      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         m_spillTarget[phase] = 0;

         m_wasteColumns[phase] = 0;
         m_wasteDepths[phase] = WasteDepth;

         m_distributed[phase].reset ();
         m_distributed[phase].setDensity (0);

         m_toBeDistributed[phase].reset ();
         m_toBeDistributed[phase].setDensity (0);

         m_fillDepth[phase] = column->getTopDepth ();
      }

      m_diffusionLeaked = 0;
      m_leakedBeforeBiodegDiffusion = 0;
      m_sealPermeability = -1;
      m_fracturePressure = -1;

#ifdef COMPUTECAPACITY
      m_capacity = -1;
#endif
      m_volumeToDepth2 = 0;

      m_toBeAbsorbed = false;
      m_computedPVT = false;
      m_extended = false;
      m_spilling = false;

#ifdef DETAILED_MASS_BALANCE
      string filename = utils::getProjectBaseName(getReservoir()->getProjectHandle()->getName());
      filename += "_MassBalance";
      ostringstream strstream;
      strstream << getReservoir()->getName() << " at " << getReservoir()->getEnd()->asString();
      strstream << " crest " << getCrestColumn();
      filename += "_"; filename += strstream.str();
      filename += ".log";
      m_massBalanceFile.open(utils::replaceSpaces(filename).c_str(), ios::out);
      m_massBalance = new MassBalance<ofstream>(m_massBalanceFile);
#endif

#ifdef DETAILED_VOLUME_BALANCE
      string filename = utils::getProjectBaseName(getReservoir()->getProjectHandle()->getName());
      filename += "_VolumeBalance";
      ostringstream strstream;
      strstream << getReservoir()->getName() << " at " << getReservoir()->getEnd()->asString();
      strstream << " crest " << getCrestColumn();
      filename += "_"; filename += strstream.str();
      filename += ".log";
      m_volumeBalanceFile.open(utils::replaceSpaces(filename).c_str(), ios::out);
      m_volumeBalance = new MassBalance<ofstream>(m_volumeBalanceFile);
#endif

      /// Set penetration distances to 0
      for (size_t i = 0; i != DiffusionComponentSize; ++i) m_penetrationDistances[i] = 0;

   }

   Trap::~Trap ()
   {
#ifdef DONTDOTHIS
      getCrestColumn ()->setTrap (0);
#endif

#ifdef DETAILED_MASS_BALANCE
      if (m_massBalanceFile)
         m_massBalanceFile.close();
#endif

#ifdef DETAILED_VOLUME_BALANCE
      if (m_volumeBalanceFile)
         m_volumeBalanceFile.close();
#endif

      delete m_levelToVolume;
      delete m_distributor;
      if (m_diffusionLeaked)
      {
         delete[] m_diffusionLeaked;
         m_diffusionLeaked = 0;
      }
      if (m_leakedBeforeBiodegDiffusion)
      {
         delete m_leakedBeforeBiodegDiffusion;
         m_leakedBeforeBiodegDiffusion = 0;
      }
   }

   size_t Trap::getSize (void)
   {
      return m_interior.size ();
   }

#ifdef COMPUTECAPACITY
   /// compute the trap's storage capacity
   void Trap::computeCapacity (void)
   {
      m_capacity = getVolumeBetweenDepths (getTopDepth (), getBottomDepth ());
      cerr << this << ": capacity: " << m_capacity << endl;
   }
#endif

#ifdef DEBUG_TRAP
   const double TOLERANCE = 0.01;
#endif

   double Trap::getVolumeBetweenDepths (double upperDepth, double lowerDepth) const
   {
      assert (m_levelToVolume);
      double topDepth = upperDepth - getTopDepth ();
      double baseDepth = lowerDepth - getTopDepth ();
      double volume = m_levelToVolume->operator()(baseDepth);
      volume -= m_levelToVolume->operator()(topDepth);

#ifdef DEBUG_TRAP
      assert(m_volumeToDepth2);
      double comparisonVolume = 0.0;
      ConstColumnIterator iter;
      for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = * iter;
         comparisonVolume += column->getVolumeBetweenDepths (upperDepth, lowerDepth);
      }
      assert(!(volume == 0.0 && comparisonVolume != 0.0));
      assert(!(volume != 0.0 && comparisonVolume == 0.0));
      assert(volume == 0.0 || fabs(1.0 - comparisonVolume / volume) < TOLERANCE);
#endif

      return volume;
   }

   double Trap::getVolumeBetweenDepths2 (double upperDepth, double lowerDepth) const
   {
      double volume = 0.0;
      ConstColumnIterator iter;
      for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         const Column * column = *iter;
         volume += column->getVolumeBetweenDepths (upperDepth, lowerDepth);
      }
      return volume;
   }

   /// Compute the function that returns a fill depth for a volume to be stored.
   /// If the number of interior columns is smaller than MaxInteriorSize, this function is pretty much exact.
   /// Otherwise, this function will consist of between 1 / MaximumVolumeIncrease and 1 / MinimumVolumeIncrease points.
   void Trap::computeDepthToVolumeFunction (void)
   {
      computeVolumeToDepthFunction ();
#ifdef DEBUG_TRAP
      computeVolumeToDepthFunction2();
#endif
   }

   /// Compute the function that returns a fill depth for a volume to be stored.
   /// If the number of interior columns is smaller than MaxInteriorSize, this function is pretty much exact.
   /// Otherwise, this function will consist of between 1 / MaximumVolumeIncrease and 1 / MinimumVolumeIncrease points.
   void Trap::computeVolumeToDepthFunction2 (void)
   {
      const unsigned int MaxInteriorSize = 200;
      const double NumDepthIncrements = 200;

      const double MaximumVolumeIncrease = 0.005;
      const double MinimumVolumeIncrease = 0.0025;

      delete m_volumeToDepth2;
      m_volumeToDepth2 = 0;

      double pointsTested = 0;
      double pointsUsed = 0;

      m_volumeToDepth2 = new Interpolator (Interpolator::Ascending);

      if (m_interior.size () < MaxInteriorSize)
      {
         // compute the 'exact' function

         // To be on the safe side, add a point to the volume function at minimumDepth ()
         m_volumeToDepth2->addPoint ((double) 0.0, (double)getTopDepth ());

         // process columns with respect to their topDepth () and their bottomDepth
         ConstColumnIterator iter;
         for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * column = *iter;

            // dirty trick to process both bottom and top depths in one block
            for (int i = 0; i < 2; i++)
            {
               double depth = (i == 0 ? column->getTopDepth () : column->getBottomDepth ());

               if (depth >= getBottomDepth () || depth <= getTopDepth ())
               {
                  // not required.
                  continue;
               }

               double volume = getVolumeBetweenDepths2 (getTopDepth (), depth);

               m_volumeToDepth2->addPoint (volume, depth);
               ++pointsTested;
               ++pointsUsed;
            }
         }
      }
      else
      {
         // compute an 'approximate' function, faster for traps with many (> ~500) columns.
         // keeps the increase in volume between two points between Min- and MaxVolumeIncrease.
         double previousVolume = 0;

         // the increment we are starting with
         double depthIncrement = (getBottomDepth () - getTopDepth ()) / NumDepthIncrements;

         double depth = getTopDepth ();
         while (depth < getBottomDepth ())
         {
            ++pointsTested;

            double currentDepth = Min (getBottomDepth (), depth + depthIncrement);
            double volume = getVolumeBetweenDepths2 (getTopDepth (), currentDepth);

            double volumeIncrease = (volume - previousVolume) / getCapacity ();

            if (volumeIncrease > MaximumVolumeIncrease)
            {
               // step is too large, decrease step size and try again.
               depthIncrement /= 2;
               continue;
            }
            else
            {
               depth = currentDepth;

               if (volumeIncrease < MinimumVolumeIncrease)
               {
                  depthIncrement *= 2;
               }
            }

            ++pointsUsed;

            m_volumeToDepth2->addPoint (volume, depth);

            previousVolume = volume;
         }
      }

      // To be on the safe side, add a point to the volume function at maximumDepth ()
      m_volumeToDepth2->addPoint (getVolumeBetweenDepths2 (getTopDepth (), getBottomDepth ()), getBottomDepth ());

      // also add a point at (inf, inf) to ensure increasing depths with increasing volumes
      m_volumeToDepth2->addPoint ((double)MAXDOUBLE, (double)MAXDOUBLE);

   }

   /// Compute the function that maps capacity to depth. This function is always exact.
   void Trap::computeVolumeToDepthFunction (void)
   {
      // Create a new MonotonicIncreasingPiecewiseLinearInvertableFunction function:
      delete m_levelToVolume;
      if (m_distributor)
         m_distributor->setLevelToVolume (0);

      m_levelToVolume = depthToVolume::compute (getTopDepth (), m_interior.begin (), m_interior.end (), getWasteColumn (OIL) ? getWasteDepth (OIL) : getSpillDepth ());

#ifdef DEBUG_LEVELTOVOLUME
      string filename = utils::getProjectBaseName(getReservoir()->getProjectHandle()->getName());
      filename += "_LevelToVolume";
      ostringstream strstream;
      strstream << getReservoir()->getName() << " at " << getReservoir()->getEnd()->asString();
      strstream << " crest " << getCrestColumn();
      filename += "_"; filename += strstream.str();
      filename += ".log";
      ofstream levelToVolumeFile; levelToVolumeFile.open(utils::replaceSpaces(filename).c_str(), ios::out);

      for (int i = 0; i < m_levelToVolume->size(); ++i) {
         Tuple2<Tuple2<double> > limits = m_levelToVolume->piece(i);
         levelToVolumeFile << "index: " << i << " =  [" << limits[0][0] << "," << limits[0][1] << "] -> [" <<
            limits[1][0] << "," << limits[1][1] << "]." << endl;
      }
      levelToVolumeFile.close();
#endif

      if (m_distributor)
         m_distributor->setLevelToVolume (m_levelToVolume);
   }

   double Trap::getDepthForVolume (double volume)
   {
      assert (m_levelToVolume);
      double depth = getTopDepth () + m_levelToVolume->apply (volume);

#ifdef DEBUG_TRAP
      assert (m_volumeToDepth2);
      if (depth != 0.0)
         assert(fabs(1.0 - m_volumeToDepth2->compute(volume) / depth) < TOLERANCE);
#endif
      return depth;
   }

   void Trap::computeArea (void)
   {
      // this function assumes that the perimeter is sorted, highest lying column first

      getCrestColumn ()->setGlobalTrapId (getGlobalId ());

      // The perimeter is sorted with the sealing columns at the end.
      // So, if the first column of the perimeter (given by getSpillColumn ()) is sealing, all columns in the perimeter are sealing
      assert (getSpillColumn ());
      if (getSpillColumn ()->isSealing ())
      {
         return;
      }

      bool spillPointFound = false;
      while (!spillPointFound && m_perimeter.size () > 0)
      {
         Column * spillColumn = getSpillColumn ();

         // Should this column be in the perimeter or in the interior ?
         if (spillColumn->isSealing () || isSpillPoint (spillColumn))
         {
            // current spill point is sealing or is spilling to another point outside of the trap
            // The Trap's perimeter and interior are now complete.
            // This column is automatically the highest point of the Trap's perimeter,
            // and thus the point where the Trap spills.
            // It is also the first column in the Trap's perimeter.

            spillPointFound = true;
         }
         else
         {
            // It's not a spill point, hence it is an interior point
            // Move column from perimeter to the interior.

            if (spillColumn->getThickness () == 0)
            {
               cerr << "WARNING from Trap::computeArea (): Adding zero thickness column " << spillColumn << " to trap " << this << endl;
               cerr << "                                   isSealing () = " << spillColumn->isSealing () << endl;
               cerr << "                                   spillColumn = " << getSpillColumn () << endl;
            }

            removeFromPerimeter (spillColumn);
            addToInterior (spillColumn);

            // By removing this column from the perimeter and adding it to the interior,
            // we created a hole in the perimeter that we have to close again.
            closePerimeter (spillColumn);
         }
      }
   }

   void Trap::becomeObsolete (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         // the target of this trap's crest column must be re-pointed to its spill target.
         Column * targetColumn = getSpillTarget ((PhaseId)phase);

         getCrestColumn ()->setTargetColumn ((PhaseId)phase, targetColumn);
      }
      getCrestColumn ()->resetProxies ();

      double spillDepth = getSpillDepth ();

      for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = *iter;
         double thickness = column->getThickness ();
         column->setTopDepth (spillDepth);
         column->setBottomDepth (spillDepth + thickness);
      }
   }

   /// Transfer interiors of trap to be absorbed to the absorbing trap.
   void Trap::beAbsorbed (void)
   {
      assert (isToBeAbsorbed ());

      // the target of this trap's crest column must be re-pointed to its spill target.
      Column * absorbingCrestColumn = 0;

      int phase;
      for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if ((absorbingCrestColumn = getCrestColumn ()->getSpillBackTarget ((PhaseId)phase)) != 0)
            break;
      }

      assert (phase < NUM_PHASES);
      assert (absorbingCrestColumn);

      for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         getCrestColumn ()->setTargetColumn ((PhaseId)phase, absorbingCrestColumn);
         getCrestColumn ()->setAdjacentColumn ((PhaseId)phase, getSpillTarget ((PhaseId)phase));
      }

      getCrestColumn ()->resetProxies ();

      // Add columns to the absorbing trap
      ConstColumnIterator iter;
      for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = *iter;

         absorbingCrestColumn->addToYourTrap (column->getI (), column->getJ ());
      }
   }

   void Trap::extendWith (Column * column, double minimumSpillDepth)
   {
      if (DebugOn)
      {
         cerr << GetRankString () << ": " << m_reservoir->getName () << this
              << ": extendWith (" << column << ", " << minimumSpillDepth << ")" << endl;
      }

      // addToPerimeter (column);

#ifdef THISISNOTRIGHT
      setMinimumSpillDepth (minimumSpillDepth);
#endif
      setExtended ();
   }

   void Trap::completeExtension (void)
   {
      if (hasBeenExtended ())
      {
         LocalColumn *curCrestColumn = getCrestColumn ();

         Column * spillColumn = getSpillColumn ();

         // add the columns that were passed on by the trap to be absorbed
         addColumnsToBeAdded ();

         // complete the area computation
         computeArea ();
         computeVolumeToDepthFunction ();
         computeWasteColumns ();
         // find the new spill target
         computeSpillTarget ();
#ifdef COMPUTECAPACITY
         computeCapacity ();
#endif

         resetSpilling ();

         resetExtended ();

         if (getCrestColumn () != curCrestColumn)
         {
            cerr << GetRankString () << ": ERROR in " << this << ": old crest column " << curCrestColumn
                 << " != new crest column " << getCrestColumn () << endl;
         }
         assert (getCrestColumn () == curCrestColumn);
      }
   }

   void Trap::initialize (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         setFillDepth ((PhaseId)phase, getTopDepth ());
      }
   }

   bool Trap::isSpillPoint (Column * column)
   {
      if (column->isSealing ())
      {
#ifdef THISISNOTRIGHT
         cerr << "    " << " -> is sealing (true)" << endl;
#endif
         return true;
      }

#ifdef THISISNOTRIGHT
      double topDepth = column->getTopDepth ();

      // only columns not above m_minimumSpillDepth can be spill points

      if (topDepth < getMinimumSpillDepth ())
      {
         return false;
      }
#endif

      for (int k = 0; k < NumNeighbours; ++k)
      {
         int iNeighbour = column->getI () + NeighbourOffsets2D[k][I];
         int jNeighbour = column->getJ () + NeighbourOffsets2D[k][J];

         Column * neighbourColumn = m_reservoir->getColumn (iNeighbour, jNeighbour);
         if (!IsValid (neighbourColumn))
         {
            return true;
         }

         if (!neighbourColumn->isSealing () && !isInInterior (neighbourColumn) && !isOnPerimeter (neighbourColumn) &&
             neighbourColumn->isShallowerThan (column))
         {
            return true;
         }
      }
      return false;
   }

   // find out if a column is in the Trap's interior
   bool Trap::isInInterior (Column * column) const
   {
      // probably more efficient to look backward
      ConstColumnReverseIterator riter;
      for (riter = m_interior.rbegin (); riter != m_interior.rend (); ++riter)
      {
         if (*riter == column)
         {
            return true;
         }

         // m_interior is sorted, lower depth first.
         if ((*riter)->isShallowerThan (column)) break;
      }
      return false;
   }

   // find out if a column is on the Trap's perimeter
   bool Trap::isOnPerimeter (Column * column) const
   {
      if (DebugOn)
      {
         cerr << "        " << GetRankString () << ": Checking if on perimeter "
              << *column
              << endl;
      }

      ConstColumnIterator iter;
      for (iter = m_perimeter.begin (); iter != m_perimeter.end (); ++iter)
      {
         if (*iter == column)
         {
            return true;
         }

         // m_perimeter is sorted, lower depth first.
         if ((*iter)->isDeeperThan (column)) break;
      }
      return false;
   }

   void Trap::closePerimeter (Column * column)
   {
      for (int k = 0; k < NumNeighbours; ++k)
      {
         int iOffset = NeighbourOffsets2D[k][I];
         int jOffset = NeighbourOffsets2D[k][J];

         unsigned int iNeighbour = column->getI () + iOffset;
         unsigned int jNeighbour = column->getJ () + jOffset;
         Column * neighbourColumn = m_reservoir->getColumn (iNeighbour, jNeighbour);
         if (!IsValid (neighbourColumn))
            continue;

         if (!isInInterior (neighbourColumn))
         {
            addToPerimeter (neighbourColumn);
         }
      }
   }

   void Trap::addToPerimeter (Column * column)
   {
      ColumnIterator iter;

      for (iter = m_perimeter.begin (); iter != m_perimeter.end (); ++iter)
      {
         Column * perimeterColumn = *iter;
         if (column == perimeterColumn)
            return;
         if ((perimeterColumn->isSealing () and column->isSealing () and perimeterColumn->isDeeperThan (column)) or
             (!perimeterColumn->isSealing () and !column->isSealing () and perimeterColumn->isDeeperThan (column)) or
             (perimeterColumn->isSealing () and !column->isSealing ()))
         {
            break;
         }
      }

      m_perimeter.insert (iter, column);
   }

   // removes a column from the Trap's perimeter.
   void Trap::removeFromPerimeter (Column * column)
   {
      bool found = false;
      ColumnIterator iter;
      for (iter = m_perimeter.begin (); iter != m_perimeter.end (); ++iter)
      {
         if (*iter == column)
         {
            found = true;
            break;
         }
      }

      if (found)
      {
         m_perimeter.erase (iter);
      }
   }

   // Interior is sorted, highest first
   void Trap::addToInterior (Column * column)
   {
      // Sealing columns can't be part of a trap (not stored in the trap Interior),
      // Wasting columns can.
      assert (!column->isSealing ());

      if (column->getThickness () == 0)
      {
         cerr << "WARNING from Trap::addToInterior (): Adding zero thickness column " << column << " to trap " << this << endl;
      }

      ColumnIterator iter;

      // probably more efficient to look backward, as we start out with a highest point and
      // successively add lower points until a spillpoint is found.
      for (iter = m_interior.end (); iter != m_interior.begin (); --iter)
      {
         ColumnIterator interiorPtr = iter;
         Column * interiorColumn = *--interiorPtr;

         if (column == interiorColumn)
         {
            // already in the list, may happen if we construct a trap from 2 others
            // that share a flat area.
            return;
         }

         if (interiorColumn->isShallowerThan (column))
         {
            break;
         }
      }

      m_interior.insert (iter, column);

      column->setGlobalTrapId (getGlobalId ());
   }

   void Trap::addColumnsToBeAdded ()
   {
      IntPairVector::iterator iter;
      for (iter = m_toBeAdded.begin (); iter != m_toBeAdded.end (); ++iter)
      {
         Column * column = m_reservoir->getColumn ((*iter).first, (*iter).second);

         removeFromPerimeter (column);
         addToInterior (column);
         closePerimeter (column);
      }
      m_toBeAdded.clear ();
   }

   void Trap::addToToBeAdded (int i, int j)
   {
      m_toBeAdded.push_back (IntPair (i, j));
   }

   bool Trap::contains (Column * column) const
   {
      ConstColumnIterator iter;
      for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * interiorColumn = *iter;
         if (column == interiorColumn)
         {
            return true;
         }
      }
      return false;
   }

   Reservoir * Trap::getReservoir (void)
   {
      return m_reservoir;
   }

   LocalColumn * Trap::getCrestColumn (void) const
   {
      if (!m_interior.empty ())
      {
         return dynamic_cast<LocalColumn *> (*m_interior.begin ());
      }
      else if (!m_perimeter.empty ())
      {
         return dynamic_cast<LocalColumn *> (*m_perimeter.begin ());
      }
      else
         return 0;
   }

   Column * Trap::getSpillColumn (void) const
   {
      return m_perimeter.front ();
   }

   double Trap::getSpillDepth () const
   {
      // The perimeter is sorted with the sealing columns at the end.
      // So, if the first column of the perimeter (given by getSpillColumn ()) is sealing, all columns in the perimeter are sealing
      if (getSpillColumn ()->isSealing ())
         return SealDepth;
      else
         return getSpillColumn ()->getTopDepth ();
   }

   Column * Trap::getColumnToSpillTo (const PhaseId phase)
   {
      Column * spillColumn = getSpillColumn ();

      return getReservoir ()->getAdjacentColumn (phase, spillColumn, this);
   }

   // find the column this trap spills to
   void Trap::computeSpillTarget (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         Column * spillingTo = getColumnToSpillTo ((PhaseId)phase);
         if (spillingTo)
         {
            m_spillTarget[phase] = spillingTo;
         }
         else
         {
            m_spillTarget[phase] = getSpillColumn ();
         }
         assert (m_spillTarget[phase]);

         assert (m_spillTarget[phase] != getCrestColumn ());

         if (getFinalSpillTarget ((PhaseId)phase) == getCrestColumn ())
         {
            cerr << "ERROR: final spill target(): "
                 << getFinalSpillTarget ((PhaseId)phase) << " == crest column: " << getCrestColumn () << endl;
            cerr << GetRankString () << ": " << this << "->getSpillColumn () = " << m_spillTarget[phase] << endl;
            printSpillTrajectory ((PhaseId)phase);
            printInterior ();
            printPerimeter ();

            assert (getFinalSpillTarget ((PhaseId)phase) != getCrestColumn ());
         }
      }
   }

   /// Return the column where charge spilled from this trap finally ends up
   Column * Trap::getFinalSpillTarget (PhaseId phase)
   {
      return getSpillTarget ((PhaseId)phase)->getFinalTargetColumn (phase);
   }

   Column * Trap::getSpillTarget (const PhaseId phase)
   {
      assert (m_spillTarget[phase] != 0);
      return m_spillTarget[phase];
   }

   /// compute the columns at which the trap will waste
   void Trap::computeWasteColumns (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         ColumnIterator iter;
         double wasteDepth = SealDepth;
         Column * wasteColumn = NULL;

         for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * interiorColumn = *iter;
            if (interiorColumn->isWasting ((PhaseId)phase))
            {
#ifdef USEADJACENTCOLUMN
               double curWasteDepth = interiorColumn->getAdjacentColumn ((PhaseId) phase)->getTopDepth ();
#else
               double curWasteDepth = interiorColumn->getTopDepth ();
#endif
               if (curWasteDepth < wasteDepth)
               {
                  wasteDepth = curWasteDepth;
                  wasteColumn = interiorColumn;
               }
            }
         }

         m_wasteColumns[phase] = wasteColumn;
         m_wasteDepths[phase] = wasteDepth;
      }
   }

   Column * Trap::getWasteColumn (PhaseId phase)
   {
      return m_wasteColumns[phase];
   }

   double Trap::getWasteDepth (PhaseId phase)
   {
      return m_wasteDepths[phase];
   }

   unsigned int Trap::getI (void)
   {
      return getCrestColumn ()->getI ();
   }

   unsigned int Trap::getJ (void)
   {
      return getCrestColumn ()->getJ ();
   }

   double Trap::getTopDepth (void) const
   {
      return getCrestColumn ()->getTopDepth ();
   }

   double Trap::getBottomDepth () const
   {
      return getSpillColumn ()->getTopDepth ();
   }

   double Trap::getTemperature (void) const
   {
      return getCrestColumn ()->getTemperature ();
   }

   double Trap::getPreviousTemperature (void) const
   {
      return getCrestColumn ()->getPreviousTemperature ();
   }

   double Trap::getPermeability (void) const
   {
      return getCrestColumn ()->getPermeability ();
   }

   void Trap::setSealPermeability (double permeability)
   {
      m_sealPermeability = permeability;
   }

   double Trap::getSealPermeability (void) const
   {
      return m_sealPermeability;
   }

   void Trap::setFracturePressure (double pressure)
   {
      m_fracturePressure = pressure;
   }

   double Trap::getFracturePressure (void) const
   {
      return m_fracturePressure;
   }

   double Trap::getPressure (void) const
   {
      return getCrestColumn ()->getPressure ();
   }

   double Trap::getPreviousPressure (void) const
   {
      return getCrestColumn ()->getPreviousPressure ();
   }

   double Trap::getHydrostaticPressure (void) const
   {
      return getCrestColumn ()->getHydrostaticPressure ();
   }

   double Trap::getLithostaticPressure (void) const
   {
      return getCrestColumn ()->getLithostaticPressure ();
   }

   double Trap::getNetToGross (void) const
   {
      return getCrestColumn ()->getNetToGross ();
   }

   double Trap::getWeight (void) const
   {
      double weight = 0;
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         weight += getWeight ((PhaseId)phase);
      }
      return weight;
   }

   double Trap::getWeight (PhaseId phase, ComponentId component) const
   {
      return m_distributed[phase].getWeight (component);
   }

   double Trap::getWeight (PhaseId phase) const
   {
      return m_distributed[phase].getWeight ();
   }

   double Trap::getWeight (ComponentId component) const
   {
      double weight = 0;
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         weight += getWeight ((PhaseId)phase, component);
      }
      return weight;
   }

   double Trap::getVolume (PhaseId phase) const
   {
      double weight = getWeight (phase);
      if (weight == 0) return 0;

      return weight / getDensity (phase);
   }

   double Trap::getWeightToBeDistributed (void) const
   {
      double weight = 0;
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         weight += getWeightToBeDistributed ((PhaseId)phase);
      }
      return weight;
   }

   double Trap::getWeightToBeDistributed (PhaseId phase) const
   {
      return m_toBeDistributed[phase].getWeight ();
   }


   double Trap::getDiffusionLeakages (void) const
   {
      if (!m_diffusionLeaked) return 0.0;
      return m_diffusionLeaked[GAS].getWeight ();
   }

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* Trap::levelToVolume () const
   {
      return m_levelToVolume;
   }

   double Trap::getCapacity (void) const
   {
#ifdef COMPUTECAPACITY
      return m_capacity;
#else
      if (m_levelToVolume)
         return m_levelToVolume->apply (numeric_limits<double>::max ());
      else
         return -1;
#endif
   }

   void Trap::setSpilling (void)
   {
      m_spilling = true;
   }

   void Trap::resetSpilling (void)
   {
      m_spilling = false;
   }

   bool Trap::isSpilling (void) const
   {
      return m_spilling;
   }

   bool Trap::isUndersized (void) const
   {
      return (getCapacity () == 0 || getCapacity () < m_reservoir->getMinTrapCapacity ());
   }

   void Trap::setGlobalId (int id)
   {
      m_globalId = id;
      for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = *iter;
         column->setGlobalTrapId (id);
      }

      setDrainageAreaId (id);
   }

   void Trap::setDrainageAreaId (int id)
   {
      m_drainageAreaId = id;
   }

   int Trap::getDrainageAreaId (void)
   {
      return m_drainageAreaId;
   }

   int Trap::getGlobalId (void)
   {
      return ((m_globalId != UnknownTrapId) ? m_globalId : GetRank () + getLocalId () * NumProcessors ());
   }

   void Trap::setLocalId (int id)
   {
      m_id = id;
   }

   int Trap::getLocalId (void)
   {
      return m_id;
   }

   double Trap::getSurface (PhaseId phase)
   {
      double surface = 0;
      for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = *iter;
         if (column->getFillDepth (phase) > column->getTopDepth ())
         {
            surface += column->getSurface ();
         }
      }

      return surface;
   }

   bool Trap::biodegradationOccurred () const
   {
      return m_biodegraded;
   }

   bool Trap::diffusionLeakageOccurred () const
   {
      if (!m_diffusionLeaked) return false;

      double diffusedWeight = 0;

      diffusedWeight = m_diffusionLeaked[GAS].getWeight (ComponentId::C1) +
                       m_diffusionLeaked[GAS].getWeight (ComponentId::C2) +
                       m_diffusionLeaked[GAS].getWeight (ComponentId::C3) +
                       m_diffusionLeaked[GAS].getWeight (ComponentId::C4) +
                       m_diffusionLeaked[GAS].getWeight (ComponentId::C5);

      if (diffusedWeight > 0)
         return true;
      else
         return false;
   }

   /// if charge got spilled here, collect all charge at the crest column for re-distribution and perform PVT.
   void Trap::collectAndSplitCharges (bool always)
   {
      if (always || requiresDistribution ())
      {
         // Biodegradation or diffusion occurred, only the trap content will be flashed and re-distributed.
         // But if we're in the post-biodegradation/diffusion stage and spillage has occurred (2nd iteration)
         // don't save crest-column content in m_leakageBeforeBiodegDiffusion, as it is spillage from other traps.
         if (always and (biodegradationOccurred () or diffusionLeakageOccurred ()))
         {
            moveDistributedToCrestColumn ();
         }
         // If there is no spillage, but there is leakage, then crest column only has leaked HCs.
         // Put those temporarily in leakedBeforeBiodegDiffusion to reflash trap.
         else if ((getCrestColumn ()->getCompositionState () & SPILLED) == 0 and (getCrestColumn ()->getCompositionState () & LEAKED) != 0)
         {
            moveDistributedToCrestColumn ();
         }
         else
         {
            moveBackToCrestColumn ();
         }

         m_computedPVT = false;
      }

#ifndef INCLUDE_COX
      assert (m_toBeDistributed[GAS].getWeight (ComponentId::COX) == 0.0);
      assert (m_toBeDistributed[OIL].getWeight (ComponentId::COX) == 0.0);
#endif

      if (requiresPVT ())
         computePVT ();

      getCrestColumn ()->resetCompositionState ();
   }

   /// this function moves the content of this trap to the crest of the trap it merges with and
   /// resets the target column of its own crest column to the crest column of the trap it merges with
   void Trap::migrateTo (Column * column)
   {
      int numI = getReservoir()->getGrid()->numIGlobal();
      int position = getCrestColumn()->getI() + getCrestColumn()->getJ() * numI;
      moveBackToCrestColumn ();
      column->addMergingCompositionToBuffer(position, getCrestColumn ()->getComposition ());
      m_reservoir->reportTrapAbsorption (this, column, getCrestColumn ()->getComposition ());
      getCrestColumn ()->resetComposition ();
   }

   double Trap::biodegradeCharges (const double& timeInterval, const Biodegrade& biodegrade)
   {
      // Charge has been placed in m_toBeDistributed in computeHydrocarbonWaterContactDepth()
      // It was flashed at the end of the initial distribution. Ready for biodegradation.
      Composition biodegraded;

      // Only biodegrade if there is charge in m_toBeDistributed and the trap is not pasteurized
      if ((!m_toBeDistributed[GAS].isEmpty() or !m_toBeDistributed[OIL].isEmpty()) and !m_isPasteurized)
      {

#ifdef DEBUG_BIODEGRADATION
         cerr << endl << ">>> DEBUG_BIODEGRADATION <<<<" << endl;
         cerr << "Initial Gas mass = " << m_toBeDistributed[GAS].getWeight() << "; Initial Gas volume = " << m_toBeDistributed[GAS].getVolume() << endl;
         cerr << "Initial Oil mass = " << m_toBeDistributed[OIL].getWeight() << "; Initial Oil volume = " << m_toBeDistributed[OIL].getVolume() << endl;
#endif

         Composition biodegradedGas;
         Composition biodegradedOil;

         // compute the volume proportions
         double volumeFractionOfGasBiodegraded = 0.0;
         double volumeFractionOfOilBiodegraded = 0.0;
         computePhaseVolumeProportionInBiodegradadedZone(timeInterval, volumeFractionOfGasBiodegraded, volumeFractionOfOilBiodegraded, biodegrade);

         // Compute biodegradation for the GAS phase
         if (volumeFractionOfGasBiodegraded > 0.0)
         {
            m_toBeDistributed[GAS].computeBiodegradation (timeInterval, m_hydrocarbonWaterContactTemperature, biodegrade,
                                                          biodegradedGas, volumeFractionOfGasBiodegraded);
            m_toBeDistributed[GAS].subtract (biodegradedGas);
            assert (m_toBeDistributed[GAS].getWeight () >= 0.0);
         }
         // Compute biodegradation for the OIL phase
         if (volumeFractionOfOilBiodegraded > 0.0)
         {
            m_toBeDistributed[OIL].computeBiodegradation (timeInterval, m_hydrocarbonWaterContactTemperature, biodegrade,
                                                          biodegradedOil, volumeFractionOfOilBiodegraded);
            m_toBeDistributed[OIL].subtract (biodegradedOil);
            assert (m_toBeDistributed[OIL].getWeight () >= 0.0);
         }

         // Addition of the mass biodegraded in the two phases in order to report the global biodegradation
         biodegraded.add (biodegradedGas);
         biodegraded.add (biodegradedOil);

         // Report the biodegradation
         m_reservoir->reportBiodegradationLoss (this, biodegraded);

#ifdef DETAILED_MASS_BALANCE
         m_massBalance->subtractFromBalance("biodegraded", biodegraded.getWeight());
#endif

#ifdef DETAILED_VOLUME_BALANCE
         m_volumeBalance->subtractFromBalance("biodegraded", biodegraded.getWeight());
#endif
      }

      // Biodegradation or not, charge needs to be put in m_distributed 
      // so that it can be diffused and/or re-distributed
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if (m_toBeDistributed[phase].isEmpty())
            continue;
         m_distributed[phase].add(m_toBeDistributed[phase]);
         m_distributed[phase].setDensity(m_toBeDistributed[phase].getDensity());
         m_toBeDistributed[phase].reset();
      }

      // If biodegraded trap, just set m_biodegraded to true and exit.
      // Re-flashing and re-distributing will happen later
      if (biodegraded.getWeight() > 0.0)
      {
         m_biodegraded = true;
      }
      // But if nothing was biodegraded, there are two possibilities:
      // a) Either nothing is in the trap, in which case there's nothing to do. OR
      // b) There is something in the trap and it didn't get biodegraded due to temperature,
      //    pasteurization, whatever. In that case, no redistribution is needed but we need
      //    to re-establish volumes and fill depths to get the mass and volume balances right.
      //    There won't be a chance to do this later because no-redistribution is needed.
      else
      {
         if (!m_distributed[GAS].isEmpty () || !m_distributed[OIL].isEmpty ())
         {
            double volume[2] = {0.0};
            for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
            {
               if (!m_distributed[phase].isEmpty() and m_distributed[phase].getDensity() > 0.0)
                  volume[phase] = m_distributed[phase].getWeight() / m_distributed[phase].getDensity();
            }
            double finalVapourLevel = m_levelToVolume->invert (volume[GAS]);
            double finalLiquidLevel = m_levelToVolume->invert (volume[GAS] + volume[OIL]);            

            setFillDepth (GAS, finalVapourLevel + getTopDepth ());
            setFillDepth (OIL, finalLiquidLevel + getTopDepth ());
         }
      }

      return biodegraded.getWeight ();
   }

   void Trap::needToComputePasteurizationStatusFromScratch()
   {

#ifdef DEBUG_BIODEGRADATION
	   cerr << endl << "==== Compute pasteurization status ====" << endl;
	   // Count the number of columns for each different status
	   int countNotPasteurized = 0;
	   int countNeutral = 0;
	   int countPasteurized = 0;
#endif

	   // Evaluation of the state of the columns
	   // Loop to check if some columns are already identified as pasteurized in the trap. If yes, no need to re-compute the pasteurization status
	   m_computePasteurizationStatusFromScratch = true;
	   ConstColumnIterator iter;
	   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
	   {
		   Column * column = *iter;

#ifdef DEBUG_BIODEGRADATION
		   if (column->getPasteurizationStatus() == -1)
			   countNotPasteurized++;
		   if (column->getPasteurizationStatus() == 0)
			   countNeutral++;
		   if (column->getPasteurizationStatus() == 1)
			   countPasteurized++;
#endif

		   // If at least one column of the trap is identify as pasteurized, it means that this trap already existed at the previous snapshot,
		   // and that there is no need to assess again the pasteurization status. The trap will be pasteurized, except if it merges with a not-pasteurized trap.
		   if (column->getPasteurizationStatus() == 1)
		   {
			   m_computePasteurizationStatusFromScratch = false;
			   break;
		   }
	   }
#ifdef DEBUG_BIODEGRADATION
	   cerr << "Number of not-Pasteurized / neutral / Pasteurized columns: " << countNotPasteurized << ", " << countNeutral << ", " << countPasteurized << endl;
	   cerr << "Need to compute the pasteurization status ? " << m_computePasteurizationStatusFromScratch << endl;
#endif
   };

   void Trap::pasteurizationStatus(const double maxBiodegradationTemperature)
   {
	   // Need to define the status of columns according to the temperature at the OWC (new trap or existing trap with not-pasteurized columns)
	   if (m_computePasteurizationStatusFromScratch == true)
	   {
	      ConstColumnIterator iter;
		   if (m_hydrocarbonWaterContactTemperature >= maxBiodegradationTemperature) // Trap pasteurized if temperature higher or equal to the maximum temperature allowed for biodegradation
		   {
			   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
			   {
				   Column * column = *iter;
				   column->setPasteurizationStatus(1); // columns pasteurized
			   }
			   m_isPasteurized = true;
		   }
		   else // Trap not-pasteurized
		   {
			   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
			   {
				   Column * column = *iter;
				   column->setPasteurizationStatus(-1); // columns not-pasteurized
			   }
			   m_isPasteurized = false;
		   }
	   }
	   else
	   {
		   // Loop to check what kind of columns are present in the trap: pasteurized and/or not-pasteurized
		   // Remark: as their name is a clear indication, neutral columns won't change the behavior of the trap
		   m_includeNotPasteurizedColumn = false;
		   m_includePasteurizedColumn = false;
		   ConstColumnIterator iter;

		   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
		   {
			   Column * column = *iter;
			   // Break the loop if the predominant kind of column have already been found (pasteurized and not-pasteurized)
			   // Indeed, if those two kinds of columns have already been found inside the trap, the behaviour of the trap is already known (see next section)
			   if (m_includeNotPasteurizedColumn == true && m_includePasteurizedColumn == true)
				   break;

			   // Check if the trap includes not-pasteurized column
			   if (column->getPasteurizationStatus() == -1)
			   {
				   m_includeNotPasteurizedColumn = true;
				   continue;
			   }
			   // Check if the trap includes pasteurized column
			   if (column->getPasteurizationStatus() == 1)
			   {
				   m_includePasteurizedColumn = true;
				   continue;
			   }
		   }
	   }
   }

   bool Trap::setPasteurizationStatus(const double maxBiodegradationTemperature)
   {
	   if (m_computePasteurizationStatusFromScratch == false)
	   {
		   ConstColumnIterator iter;
		   // Give the expected behavior of the trap according to the kind of columns that it possesses
		   // 1) If the trap includes only NOT-pasteurized columns (and possibly neutral columns) => the trap is NOT pasteurized
		   if (m_includeNotPasteurizedColumn == true && m_includePasteurizedColumn == false)
		   {
			   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
			   {
				   Column * column = *iter;
				   column->setPasteurizationStatus(-1);   //set all the columns of this trap as not-pasteurized
			   }
			   m_isPasteurized = false;
			   return true;
		   }

		   // 2) If the trap includes only pasteurized columns (and possibly neutral columns) => the trap is pasteurized
		   // Rationale behind this: if the trap grows and (neutral) columns are added to the trap, those columns will be at a deeper depth and so already pasteurized
		   if (m_includeNotPasteurizedColumn == false && m_includePasteurizedColumn == true)
		   {
			   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
			   {
				   Column * column = *iter;
				   column->setPasteurizationStatus(1);   //set all the columns of this trap as pasteurized
			   }
			   m_isPasteurized = true;
			   return true;
		   }

		   // 3) If the trap is a mix of pasteurized and not-pasteurized columns => the trap is not-pasteurized if the temperature is below the pasteurization temperature
		   // Rationale behind this: merging of two traps, so the bacteria in one (not pasteurized) trap can migrate to the other (previously pasteurized) trap
		   if (m_includeNotPasteurizedColumn == true && m_includePasteurizedColumn == true)
		   {
			   // The temperature at the OWC is not high enough at this snapshot to pasteurized the trap
			   if (m_hydrocarbonWaterContactTemperature <= maxBiodegradationTemperature)
			   {
				   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
				   {
					   Column * column = *iter;
					   column->setPasteurizationStatus(-1);   //set all the columns of this trap as not-pasteurized
				   }
				   m_isPasteurized = false;
				   return true;
			   }
			   else // The temperature at this snapshot is too high and pasteurized the trap
			   {
				   for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
				   {
					   Column * column = *iter;
					   column->setPasteurizationStatus(1);   //set all the columns of this trap as pasteurized
				   }
				   m_isPasteurized = true;
				   return true;
			   }
		   }

		   // No reason to reach this point of the function, so the "return false" is just an extra security
		   return false;
	   }

	   return true;
   }

   /// If depths contains a vector of formations starting with the formation containing
   /// this trap, return iterators pointing to the formations which constitute the
   /// overburden of this trap:
   void Trap::iterateToFirstOverburdenFormation (const vector<FormationSurfaceGridMaps>& depths,
                                                 vector<FormationSurfaceGridMaps>::const_iterator& begin,
                                                 vector<FormationSurfaceGridMaps>::const_iterator& end) const
   {
      // Determine first whether the seal of the trap lies inside this formation, or
      // whether the seal is formed by the next formation.  In the last case, the first
      // is the second formation of depths etc:
      begin = depths.begin (); end = depths.end ();
      ++begin;
      
      if (begin == end) LogHandler(LogHandler::ERROR_SEVERITY) << "The begin and the end of the depth overbuden array coincide in Trap::iterateToFirstOverburdenFormation";
   }

   bool Trap::computeDiffusionOverburden (const SurfaceGridMapContainer& fullOverburden,
                                          const Snapshot* snapshot, const double& maxSealThickness, int maxFormations)
   {
      delete m_diffusionOverburdenProps;
      m_diffusionOverburdenProps = 0;

      bool sealPresent;
      double sealFluidDensity;
      if (!computeSealFluidDensity (fullOverburden, snapshot, sealPresent, sealFluidDensity))
         return false;

      // If there is no seal, there is no diffusion leakage, so leave m_diffusionOverburdenProps 0:
      if (!sealPresent)
         return true;

      // Below we copy the property information of the relevant formations to
      // m_diffusionOverburdenProps. Clear first the old information in m_diffusionOverburdenProps:
      vector<DiffusionLeak::OverburdenProp> diffusionOverburdenProps;

      if (!computeDiffusionOverburdenImpl (fullOverburden, snapshot, maxSealThickness, maxFormations,
                                           diffusionOverburdenProps))
         return false;

      m_diffusionOverburdenProps = new DiffusionOverburdenProperties (diffusionOverburdenProps,
                                                                      sealFluidDensity);
      return true;
   }

   bool Trap::computeSealFluidDensity (const SurfaceGridMapContainer& fullOverburden, const Snapshot* snapshot,
                                       bool& sealPresent, double& sealFluidDensity) const
   {
      // Get the overburden formation depths:
      const vector<FormationSurfaceGridMaps>& depths = fullOverburden.discontinuous (SurfaceGridMapContainer::DISCONTINUOUS_DEPTH);
      vector<FormationSurfaceGridMaps>::const_iterator begin;
      vector<FormationSurfaceGridMaps>::const_iterator end;
      iterateToFirstOverburdenFormation (depths, begin, end);

      // Get the first overburden formation which does exist, i.e. for which the thickness is larger
      // than zero:
      unsigned int i = getCrestColumn ()->getI ();
      unsigned int j = getCrestColumn ()->getJ ();
      vector<const Formation*> formations;
      if (!overburden_MPI::getRelevantOverburdenFormations (begin, end, snapshot,
                                                            i, j, numeric_limits<double>::max (), 1, true, formations))
         return false;

      sealPresent = formations.size () != 0;
      if (!sealPresent)
      {
         sealFluidDensity = 0.0;
         return true;
      }

      // Casting to GeoPhysics::FluidType so that the phase-change implementation for brine density can be used
      const GeoPhysics::FluidType* fluidType = dynamic_cast<const GeoPhysics::FluidType *> (formations[0]->getFluidType ());
      assert (fluidType);
      sealFluidDensity = fluidType->density (getTemperature (), getPressure ());
      return true;
   }

   bool Trap::computeDiffusionOverburdenImpl (const SurfaceGridMapContainer& fullOverburden,
                                              const Snapshot* snapshot, const double& maxSealThickness, int maxFormations,
                                              vector<DiffusionLeak::OverburdenProp>& diffusionOverburdenProps) const
   {
      const SurfaceGridMapContainer::discontinuous_properties& depths = fullOverburden.discontinuous (
         SurfaceGridMapContainer::DISCONTINUOUS_DEPTH);
      const SurfaceGridMapContainer::continuous_properties& temperatures = fullOverburden.continuous (
         SurfaceGridMapContainer::CONTINUOUS_TEMPERATURE);
      const SurfaceGridMapContainer::discontinuous_properties& porosities = fullOverburden.discontinuous (
         SurfaceGridMapContainer::DISCONTINUOUS_POROSITY);

      const SurfaceGridMapContainer::discontinuous_properties& brineViscosity = fullOverburden.discontinuous (SurfaceGridMapContainer::DISCONTINUOUS_BRINEVISCOSITY);

      // There must be one extra element in temperatures in comparison to depths and porosities:
      assert (temperatures.size () == porosities.size () + 1);
      assert (temperatures.size () == depths.size () + 1);

      // Get the overburden formation depths:
      vector<FormationSurfaceGridMaps>::const_iterator begin;
      vector<FormationSurfaceGridMaps>::const_iterator end;
      iterateToFirstOverburdenFormation (depths, begin, end);

      // Get the formations which do exist, i.e. for which the thickness is larger than zero,
      // and which are within maxSealThickness and maxFormations:
      unsigned int i = getCrestColumn ()->getI ();
      unsigned int j = getCrestColumn ()->getJ ();
      vector<const Formation*> formations;
      if (!overburden_MPI::getRelevantOverburdenFormations (begin, depths.end (), snapshot,
                                                            i, j, maxSealThickness, maxFormations, true, formations))
         return false;

      // In case the trap lies inside the formation, the thickness, porosity and base temperature
      // are not provided by that of the overburden formation, but by the values from the Trap itelf.
      // The reason is that if we would get the values from the overburden formation, we would
      // the values from the base of the formation instead of the top of the trap:
      double baseDepth = getTopDepth ();
      double basePorosity = getCrestColumn ()->getPorosity ();
      double baseTemperature = getTemperature ();
      double baseBrineViscosity = getCrestColumn ()->getViscosity ();

      vector<const Formation*>::const_iterator f = formations.begin ();
      SurfaceGridMapContainer::discontinuous_properties::const_iterator d = depths.begin ();
      SurfaceGridMapContainer::continuous_properties::const_iterator t = temperatures.begin ();
      SurfaceGridMapContainer::discontinuous_properties::const_iterator p = porosities.begin ();
      SurfaceGridMapContainer::discontinuous_properties::const_iterator bv = brineViscosity.begin ();

      // There must be a minimum of 2 entries in fullOverburden.continuous(CONTINUOUS_TEMPERATURE):
      assert (t != temperatures.end ());

      // Iterate over the included formations and the property GridMaps and copy the
      // information for each formation within formations:
      ++t;
      while (f != formations.end ())
      {
         assert (d != depths.end ());
         assert (t != temperatures.end ());
         assert (p != porosities.end ());
         assert (bv != brineViscosity.end ());

         // The formations of d, p and t should match:
         assert ((*d).formation () == (*t).base ());
         assert ((*d).formation () == (*p).formation ());

         // Check whether the formation of d, t and p is included:
         if ((*d).formation () == *f)
         {

            if (!(*d).base ().valid ())
            {
               cerr << "Trap.C:1369: Exiting as no valid depth property found for base of overburden formation: '" <<
                  (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
               cerr.flush ();

               return false;
            }

            double thickness = (*d).base ()[functions::Tuple2<unsigned int>(i, j)] - (*d).top ()[functions::Tuple2<unsigned int>(i, j)];

#ifdef DEBUG_TRAP
            string name = (*f)->getName();

            bool validTopDepth = (*d).top().valid();
            bool validBaseDepth = (*d).base().valid();
            double compTopDepth = 0.0;
            if (validTopDepth)
               compTopDepth = (*d).top()[functions::tuple(i,j)];
            double compBaseDepth = 0.0;
            if (validBaseDepth)
               compBaseDepth = (*d).base()[functions::tuple(i,j)];

            bool validTopPorosity = (*p).top().valid();
            bool validBasePorosity = (*p).base().valid();
            double compTopPorosity = 0.0;
            if (validTopPorosity)
               compTopPorosity = (*p).top()[functions::tuple(i,j)];
            double compBasePorosity = 0.0;
            if (validBasePorosity)
               compBasePorosity = (*p).base()[functions::tuple(i,j)];

            double snapshotAge = snapshot->getTime();
#endif
            assert (thickness > 0.0);

            if (!(*t).valid ())
            {
               cerr << "Trap.C:1404: Exiting as no valid temperature property found for base of overburden formation: '" <<
                  (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
               cerr.flush ();

               return false;
            }
            double topTemperature = (*t)[functions::Tuple2<unsigned int>(i, j)];

            if (!(*p).top ().valid ())
            {
               cerr << "Trap.C:1413: Exiting as no valid permeability property found for base of overburden formation: '" <<
                  (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
               cerr.flush ();

               return false;
            }
            double topPorosity = (*p).top ()[functions::Tuple2<unsigned int>(i, j)];

            if (!(*bv).top ().valid ())
            {
               cerr << "Exiting as no valid brine viscosity property found for base of overburden formation: '" <<
                  (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
               cerr.flush ();

               return false;
            }
            double topBrineViscosity = (*bv).top ()[functions::Tuple2<unsigned int>(i, j)];

            diffusionOverburdenProps.push_back (DiffusionLeak::OverburdenProp (thickness, topPorosity, basePorosity,
                                                                               topTemperature, baseTemperature,
                                                                               topBrineViscosity, baseBrineViscosity));

            // Goto the next included formation:
            ++f;
         }

         if (!(*d).base ().valid ())
         {
            cerr << "Trap.C:1429: Exiting as no valid depth property found for base of overburden formation: '" <<
               (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
            cerr.flush ();

            return false;
         }
         baseDepth = (*d).base ()[functions::Tuple2<unsigned int>(i, j)];

         if (!(*p).top ().valid ())
         {
            cerr << "Trap.C:1438: Exiting as no valid permeability property found for top of overburden formation: '" <<
               (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
            cerr.flush ();

            return false;
         }

         basePorosity = (*p).top ()[functions::Tuple2<unsigned int>(i, j)];

         if (!(*t).valid ())
         {
            cerr << "Trap.C:1448: Exiting as no valid temperature property found for top of overburden formation: '" <<
               (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
            cerr.flush ();

            return false;
         }
         baseTemperature = (*t)[functions::Tuple2<unsigned int>(i, j)];

         if (!(*bv).top ().valid ())
         {
            cerr << "Exiting as no valid brine viscosity property found for top of overburden formation: '" <<
               (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
            cerr.flush ();

            return false;
         }

         baseBrineViscosity = (*bv).top ()[functions::Tuple2<unsigned int>(i, j)];

         ++d; ++p; ++t;
      }
      return true;
   }

   /// Calculate charge loss due to diffusion leakages.
   ///
   /// @param[in] timeInterval: The time interval between the previous and current snapshot, i.e.
   ///                          previous - current. As is customary in fastmig, all times are positive
   ///                          and because timeInterval = previous - current timeInterval is also
   ///                          positive.
   /// @param[in] parameters:
   /// @param[in] maxTimeStep:
   /// @param[in] maxFluxError:
   ///
   void Trap::diffusionLeakCharges (const double& intervalStartTime, const double & intervalEndTime, const Interface::DiffusionLeakageParameters*
                                    parameters, const double& maxTimeStep, const double& maxFluxError)
   {
      // If there are no m_diffusionOverburdenProps, there wasn't a overburden.  (Diffusion directly
      // to the air is leaking, not diffusion.).  In that case do nothing:
      if (!m_diffusionOverburdenProps)
         return;

      int c;
      double totalWeight = 0;

      int size = (int)parameters->concentrationConsts ().size ();
      size = DiffusionComponentSize;

      for (c = 0; c < size; ++c)
      {
         totalWeight += getWeight (GAS, (ComponentId)(ComponentId::C1 - c));
      }

      if (totalWeight < 1)
      {
         setDiffusionStartTime (-1);
         return;
      }

      double diffusionStartTime = getCrestColumn ()->getDiffusionStartTime ();
      if (diffusionStartTime < 0)
      {
         diffusionStartTime = intervalStartTime;
      }
      setDiffusionStartTime (diffusionStartTime);

      // Calculate the solubility of methane, i.e. C1:
      double methaneSolubilityPerKgH2O = CBMGenerics::methaneSolubility::compute (parameters->salinity (), getTemperature (), getPressure ());

      assert (methaneSolubilityPerKgH2O >= 0.0);

      if (methaneSolubilityPerKgH2O == 0.0)
         return;

      // The number of concentrationConsts should be the same as the number of diffusionConsts:
      assert ((int)parameters->diffusionConsts ().size () >= size);

      // kg C1/ m3 water
      double methaneSolubilityPerM3 = methaneSolubilityPerKgH2O * m_diffusionOverburdenProps->
         sealFluidDensity ();

      vector<double> solubilities (parameters->concentrationConsts ());

      for (c = 0; c < size; ++c)
      {
         solubilities[c] *= methaneSolubilityPerM3;
      }

      vector<DiffusionLeak*> diffusionLeaks;
      diffusionLeaks.reserve (size);

      ///Retrive a pointer to the m_penetrationDistances vector from the previous snapshot
      const double * oldPenetrationDistance = getCrestColumn ()->getPenetrationDistances ();

      setDiffusionStartTime (diffusionStartTime);

      for (c = 0; c < size; ++c)
      {
         DiffusionCoefficient coefficient (parameters->diffusionConsts ()[c], parameters->activationEnergy ());

         double penetrationDistance (0.0);

         if (parameters->transientModel () == Interface::Transient)
         {
            if (oldPenetrationDistance[c] < 0)
            {
               penetrationDistance = 0;
            }
            else
            {
               penetrationDistance = oldPenetrationDistance[c];
            }
         }
         else
         {
            penetrationDistance = parameters->maximumSealThickness ();
         }

         assert (penetrationDistance >= 0);

         DiffusionLeak*  diffusionLeak = new DiffusionLeak (m_diffusionOverburdenProps->properties (),
                                                            m_diffusionOverburdenProps->sealFluidDensity (), penetrationDistance,
                                                            parameters->maximumSealThickness (), coefficient, maxTimeStep, maxFluxError);

         diffusionLeaks.push_back (diffusionLeak);
      }

      assert (diffusionLeaks.size () <= solubilities.size ());

      // if not present create a new m_diffusionLeaked
      if (!m_diffusionLeaked) m_diffusionLeaked = new Composition[2];

      m_diffusionLeaked[GAS].reset ();
      m_diffusionLeaked[GAS].setDensity (0);

      m_diffusionLeaked[OIL].reset ();
      m_diffusionLeaked[OIL].setDensity (0);

      m_distributed[GAS].computeDiffusionLeakages (diffusionStartTime, intervalStartTime, intervalEndTime, solubilities, getSurface (GAS), diffusionLeaks,
                                                   computeGorm (m_distributed[GAS], m_distributed[OIL]), &m_distributed[GAS], &m_diffusionLeaked[GAS]);

      /// \brief set the penetration distance of the trap for the next snapshot
      if (parameters->transientModel () == Interface::Transient)
      {
         for (c = 0; c < size; ++c)
         {
            setPenetrationDistance ((ComponentId)c, diffusionLeaks[c]->penetrationDistance ());
         }
      }

      m_reservoir->reportDiffusionLoss (this, m_diffusionLeaked[GAS]);

#ifdef DETAILED_MASS_BALANCE
      m_massBalance->subtractFromBalance("diffusion leaked", m_diffusionLeaked[GAS].getWeight());
#endif

#ifdef DETAILED_MASS_BALANCE
      m_volumeBalance->subtractFromBalance("diffusion leaked", m_diffusionLeaked[GAS].getVolume());
#endif
      // delete diffusionLeak objects pointed in diffusionLeaks vector
      for (int i = 0; i != diffusionLeaks.size (); ++i) delete diffusionLeaks[i];
      delete m_diffusionOverburdenProps;
   }

   bool Trap::computeDistributionParameters (const Interface::FracturePressureFunctionParameters*
                                             parameters, const SurfaceGridMapContainer& fullOverburden,
                                             const Snapshot* snapshot)
   {
      delete m_distributor;
      m_distributor = 0;

      // Calculate the following overburden parameters:
      bool sealPresent;
      double fracPressure;
      double sealFluidDensity;
      vector< vector<translateProps::CreateCapillaryLithoProp::output> > lithProps;
      vector< vector<double> > lithFracs;
      vector<CBMGenerics::capillarySealStrength::MixModel> mixModel;
      vector<double> permeability;

      if (!computeSealPressureLeakParametersImpl (parameters, fullOverburden, snapshot, sealPresent,
                                                  fracPressure, sealFluidDensity, lithProps, lithFracs,
                                                  mixModel, permeability) )
         return false;

      setFracturePressure (fracPressure);
      setSealPermeability( 0 ); // we set a non-zero seal permeability only if the seal is present

      if (sealPresent)
      {
         if ( permeability.size( ) == 2 )
         {
            //set the seal permeability only if a seal formation is present
            setSealPermeability( permeability[1] );
         }

         if (!isUndersized ())
         {
            // The fracture seal strength is provided by the fracturePressure minus the pressure:
            double porePressure = getPressure ();
            double fracSealStrength = fracPressure - porePressure / PaToMegaPa;
#if 0
            cerr << "trap = " << this << endl;
            cerr << "porePressure = " << porePressure / PaToMegaPa << endl;
            cerr << "fracPressure = " << fracPressure << endl;
            cerr << "fracSealStrength = " << fracSealStrength << endl;
#endif
            if (fracSealStrength < 0) fracSealStrength = 0;

#ifdef DEBUG
            if (GetRank () == 0)
            {
               cerr << "fracPressure = " << fracPressure <<
                  " Pa, pressure = " << getPressure () * MegaPaToPa <<
                  " Pa, fracSealStrength = " << fracSealStrength << " Pa" << endl;
            }
#endif

#ifdef DETAILED_MASS_BALANCE
            ostringstream strstream1;
            strstream1 << "fracture seal strength: " << fracSealStrength << endl;
            m_massBalance->addComment(strstream1.str());
            ostringstream strstream2;
            strstream2 << "seal fluid density: " << sealFluidDensity << endl;
            m_massBalance->addComment(strstream2.str());
            ostringstream strstream3;
            strstream2 << "permeability: " << permeability << endl;
            m_massBalance->addComment(strstream3.str());
#endif

#ifdef DETAILED_VOLUME_BALANCE
            ostringstream strstream1;
            strstream1 << "fracture seal strength: " << fracSealStrength << endl;
            m_volumeBalance->addComment(strstream1.str());
            ostringstream strstream2;
            strstream2 << "seal fluid density: " << sealFluidDensity << endl;
            m_volumeBalance->addComment(strstream2.str());
            ostringstream strstream3;
            strstream2 << "permeability: " << permeability << endl;
            m_volumeBalance->addComment(strstream3.str());
#endif

            // With these parameters, create the algorithm which determines the max gas and oil column.
            // However, some critical parameters that are necessary for the calculation of the capillary
            // entry seal strength, such as the density and composition of the oil and gas phases, are
            // right now not available. So we create here a CapillarySealStrength object and provide this
            // object the parameters which we already do know. The CapillarySealStrength then calculates
            // the capillary seal strength at the moment the missing data becomes available.

            // to compute leakage you need to compute the Brooks Corey correction. Retrive the lambda at the crest location and pass it to the distributor
            double lambdaPC = Interface::DefaultUndefinedScalarValue;

            const Formation * formation = dynamic_cast<const Formation *> ( getReservoir( )->getFormation( ) );
            const GeoPhysics::CompoundLithology* compoundLithology = formation->getCompoundLithology( getCrestColumn( )->getI( ), getCrestColumn( )->getJ( ) );
            if ( compoundLithology ) lambdaPC = compoundLithology->LambdaPc( );

            // If the project file does not contain values for Lambda_Pc assign an 'avarage' value of 1.
            if ( lambdaPC == Interface::DefaultUndefinedMapValue or lambdaPC == Interface::DefaultUndefinedScalarValue )
               lambdaPC = 1.0;

            double overPressureContrast = 0.0;
            bool overpressuredLeakage = m_reservoir->getMigrator()->isOverpressuredLeakageOn();
            if (!m_reservoir->getMigrator()->isHydrostaticCalculation() and overpressuredLeakage)
            {
               boost::array<double,2> overPressures = {0.0, 0.0};

               int i,j;
               i = getCrestColumn()->getI();
               j = getCrestColumn()->getJ();
               const Formation * const reservoirFormation = dynamic_cast<const Formation *> (m_reservoir->getFormation());

               reservoirFormation->getTopBottomOverpressures(i, j, overPressures);
               overPressureContrast = Utilities::Maths::MegaPaToPa * (overPressures[0] - overPressures[1]); 
            }

            double crestColumnThickness = getCrestColumn()->getThickness();

            m_distributor = new LeakWasteAndSpillDistributor( sealFluidDensity, fracSealStrength, overPressureContrast, crestColumnThickness,
                                                              CapillarySealStrength( lithProps, lithFracs, mixModel, permeability, sealFluidDensity, lambdaPC ),
                                                              m_levelToVolume );
         }
         else
         {

#ifdef DETAILED_MASS_BALANCE
            m_massBalance->addComment("Trap undersized!");
#endif

#ifdef DETAILED_VOLUME_BALANCE
            m_volumeBalance->addComment("Trap undersized!");
#endif

            m_distributor = new SpillAllGasAndOilDistributor (m_levelToVolume);
         }
      }
      else
      {

#ifdef DETAILED_MASS_BALANCE
         m_massBalance->addComment("No seal present!");
#endif

#ifdef DETAILED_VOLUME_BALANCE
         m_volumeBalance->addComment("No seal present!");
#endif

         m_distributor = new LeakAllGasAndOilDistributor (m_levelToVolume);
      }

      return true;
   }

   bool Trap::computeSealPressureLeakParametersImpl(
      const Interface::FracturePressureFunctionParameters* fracturePressureParameters,
      const SurfaceGridMapContainer& fullOverburden,
      const Snapshot* snapshot,
      bool& sealPresent,
      double& fracPressure,
      double& sealFluidDensity,
      vector< vector<translateProps::CreateCapillaryLithoProp::output> >& lithProps,
      vector< vector<double> >& lithFracs,
      vector<CBMGenerics::capillarySealStrength::MixModel>& mixModel,
      vector<double>& permeability) const
   // to implement //
   {
      const SurfaceGridMapContainer::discontinuous_properties & depths =
         fullOverburden.discontinuous (SurfaceGridMapContainer::DISCONTINUOUS_DEPTH);
      const SurfaceGridMapContainer::discontinuous_properties & permeabilities =
         fullOverburden.discontinuous (SurfaceGridMapContainer::DISCONTINUOUS_PERMEABILITY);
      const SurfaceGridMapContainer::constant_properties & lithoType1Percents =
         fullOverburden.constants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE1PERCENT);
      const SurfaceGridMapContainer::constant_properties & lithoType2Percents =
         fullOverburden.constants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE2PERCENT);
      const SurfaceGridMapContainer::constant_properties & lithoType3Percents =
         fullOverburden.constants (SurfaceGridMapContainer::CONSTANT_LITHOTYPE3PERCENT);

      // There must be one extra element in depths in comparison to the discontiuous data
      // and the number of percentage map properties decrease from LithoType 1 to LithType 2 and
      // LithoType 3:
      assert (depths.size () == permeabilities.size ());
      assert (depths.size () == lithoType1Percents.size ());
      assert (lithoType1Percents.size () >= lithoType2Percents.size ());
      assert (lithoType2Percents.size () >= lithoType3Percents.size ());

      // Get the overburden formation depths:
      vector < FormationSurfaceGridMaps >::const_iterator begin;
      vector < FormationSurfaceGridMaps >::const_iterator end;
      iterateToFirstOverburdenFormation (depths, begin, end);

      // Get the first formation which does exist, i.e. for which the thickness is larger than zero:
      unsigned int i = getCrestColumn ()->getI ();
      unsigned int j = getCrestColumn ()->getJ ();
      vector < const Formation *>formations;

      // Get the reservoir (formations[0]) and the seal formation (formations[1])
      if ( !overburden_MPI::getRelevantOverburdenFormations( depths.begin( ), depths.end( ), snapshot, i, j,
                                                             numeric_limits < double >::max( ), 2, true, formations ) )
         return false;

      // It can happen there are no seal formations.  In that case return:
      sealPresent = formations.size () != 0;
      if (!sealPresent)
         return true;

      vector < const Formation *>::const_iterator f = formations.begin ();
      SurfaceGridMapContainer::discontinuous_properties::const_iterator d = depths.begin ();
      SurfaceGridMapContainer::discontinuous_properties::const_iterator p = permeabilities.begin ();
      SurfaceGridMapContainer::constant_properties::const_iterator l0 = lithoType1Percents.begin ();
      SurfaceGridMapContainer::constant_properties::const_iterator l1 = lithoType2Percents.begin ();
      SurfaceGridMapContainer::constant_properties::const_iterator l2 = lithoType3Percents.begin ();

      // Find the with the one overburden formation corresponding GridMap's of the
      // properties.  In most cases this will be the first GridMap, but in order to be safe,
      // we iterate until depths.end() (to prevent an eternal loop), even though we shouldn't get
      // at depths.end():

      // the fracture pressure result
      bool result = true;

      while ( d != depths.end() )
      {
         assert( d != depths.end() );
         assert( p != permeabilities.end() );
         assert( l0 != lithoType1Percents.end() );
         // l1 and l2 may be empty, so no asserts for l1 and l2.

         // The formations of d, p and l0 should match:
         assert( ( *d ).formation() == ( *p ).formation() );
         assert( ( *d ).formation() == ( *l0 ).first );

         // Check whether the formation of d, p and l0 is the seal formation:
         if ( ( *d ).formation() == *f )
         {

            // The top of the one overburden formation must be larger than both
            // the top of the trap and the base of the overburden formation:
#ifdef DEBUG_TRAP
            bool validTop = (*d).top ().valid ();
            bool validBase = (*d).base ().valid ();
            double compTopFormationDepth = 0.0;

            if (validTop)
               compTopFormationDepth = (*d).top ()[functions::tuple (i, j)];
            double compBaseFormationDepth = 0.0;

            if (validBase)
               compBaseFormationDepth = (*d).base ()[functions::tuple (i, j)];
            double compTopDepth = getTopDepth ();
            string name = (*f)->getName ();
            double snapshotAge = snapshot->getTime ();
#endif
            //should always be valid for reservoir and seal;
            if ( !( *d ).base().valid() )
            {
               cerr << "Trap::computeSealPressureLeakParametersImpl : Exiting as no valid depth property found for base of formation: '" <<
                  ( *f )->getName() << "' at time: " << snapshot->getTime() << "." << endl;
               cerr.flush();

               return false;
            }

            //the formation has thickness at the specific i,j
            assert( ( *d ).base()[functions::Tuple2<unsigned int>( i, j )] - ( *d ).top()[functions::Tuple2<unsigned int>( i, j )] > 0.0 );
            assert( ( *f )->getLithoType1() );
            assert( l0 != lithoType1Percents.end() );
            assert( l0->first == *f );

            // So we have found the right d, p, l0, and possibly l1 and l2 iterators if they
            // do exist.  Get the fractions of the LithoTypes:
            vector<double> formLithFracs;

            formLithFracs.push_back( 0.01 * ( *l0 ).second[functions::Tuple2<unsigned int>( i, j )] );
            if ( ( *f )->getLithoType2() && l1 != lithoType2Percents.end() && ( *l1 ).first == ( *f ) )
               formLithFracs.push_back( 0.01 * ( *l1 ).second[functions::Tuple2<unsigned int>( i, j )] );
            if ( ( *f )->getLithoType3() && l2 != lithoType3Percents.end() && ( *l2 ).first == ( *f ) )
               formLithFracs.push_back( 0.01 * ( *l2 ).second[functions::Tuple2<unsigned int>( i, j )] );

            // store the formation litho fraction
            lithFracs.push_back( formLithFracs );

            // For the capillary entry pressure, we need to know what mixing model to apply when calculating
            // the effective capillary parameters:
            mixModel.push_back( ( *f )->getMixModel() );

            // Permeability is the permeability of the base of the overburden
            // formation:
            if ( *f == formations.front() && ( *p ).top().valid() )
            {
               //reservoir (in case the only valid formation is the reservoir only this is executed)
               permeability.push_back( ( *p ).top()[functions::Tuple2<unsigned int>( i, j )] );
            }
            else if ( formations.size() == 2 && *f == formations.back() && ( *p ).base().valid() )
            {
               //seal
               permeability.push_back( ( *p ).base()[functions::Tuple2<unsigned int>( i, j )] );
            }
            else
            {
               cerr << "Trap::computeSealPressureLeakParametersImpl : Exiting as no valid permeability property found for formation: '" <<
                  ( *f )->getName() << "' at time: " << snapshot->getTime() << "." << endl;
               cerr.flush();
               return false;
            }
            

            // Finally use the gathered information to calculate the capillary seal strength of gas and oil:
            vector<translateProps::CreateCapillaryLithoProp::output> formlithProps;
            translateProps::translate < translateProps::CreateCapillaryLithoProp >( *f, translateProps::CreateCapillaryLithoProp(),
                                                                                    formlithProps );
            lithProps.push_back( formlithProps );

            // -- Fracture pressure calculations --//
            // Compute sealFluidDensity, fracPressure and the fracture pressure only for the seal.
            if ( ( *f ) == formations.back() )
            {

               // But as depth and temperatureC are of course continuous properties, we may get them from
               // the Trap:
               double depth = getTopDepth();
               double depthWrtSedimentSurface = getCrestColumn()->getOverburden();
               double temperatureC = getTemperature();

               // Get the water density:
               const Interface::FluidType * fluidType = ( *f )->getFluidType();
               sealFluidDensity = CBMGenerics::waterDensity::compute( fluidType->fluidDensityModel(),
                                                                      fluidType->density(), fluidType->salinity(),
                                                                      temperatureC, getPressure() );
               // Compute the fracture pressure:
               fracPressure = numeric_limits < double >::max();

               switch ( fracturePressureParameters->type() )
               {
               case Interface::None:
                  break;
               case Interface::FunctionOfDepthWrtSeaLevelSurface:
                  fracPressure =
                     CBMGenerics::fracturePressure::computeForFunctionOfDepthWrtSeaLevelSurface( fracturePressureParameters->
                                                                                                 coefficients(), depth );
                  break;
               case Interface::FunctionOfDepthWrtSedimentSurface:
               {
                  fracPressure =
                     CBMGenerics::fracturePressure::computeForFunctionOfDepthWrtSedimentSurface( fracturePressureParameters->coefficients(),
                                                                                                 depthWrtSedimentSurface,
                                                                                                 getCrestColumn()->getSeaBottomPressure() );
                  break;
               }
               case Interface::FunctionOfLithostaticPressure:
                  result = computeForFunctionOfLithostaticPressure( fullOverburden, *f, formLithFracs, fracPressure );
                  break;
               default:
                  assert( 0 );
               }

               //the end of the formation vector is reached, no need to loop again
               break;
            }

            // increment the formation iterator
            ++f;
         }

         // If the formation of l1 and l2 matches that of d, go to the next l1 and l2:
         if ( l1 != lithoType2Percents.end() && ( *l1 ).first == ( *d ).formation() )
            ++l1;
         if ( l2 != lithoType3Percents.end() && ( *l2 ).first == ( *d ).formation() )
            ++l2;
         ++d;
         ++p;
         ++l0;
      }

      return result;
   }

   bool Trap::computeForFunctionOfLithostaticPressure (const SurfaceGridMapContainer& fullOverburden,
                                                       const Formation* formation, const vector<double>& lithFracs,
                                                       double& fracPressure) const
   {
      vector<double> lithHydraulicFracturingFracs;
      translateProps::translate<translateProps::CreateLithHydraulicFracturingFrac> (formation, translateProps::CreateLithHydraulicFracturingFrac (),
                                                                                    lithHydraulicFracturingFracs);
      double hydraulicFracture = CBMGenerics::fracturePressure::hydraulicFracturingFrac (lithHydraulicFracturingFracs, lithFracs);

      fracPressure = CBMGenerics::fracturePressure::computeForFunctionOfLithostaticPressure (hydraulicFracture, getLithostaticPressure (),
                                                                                             getHydrostaticPressure ());
      return true;
   }

   bool Trap::distributeCharges (void)
   {
      // requiresDistribution () won't work as it also looks at the crest column,
      // which is not required/advisable as it is not used further on.
      // Also, Anton's distributor does not handle the case where there is nothing toBeDistributed very well.

      bool distributionNeeded = false;
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if (!m_toBeDistributed[phase].isEmpty ()) distributionNeeded = true;
      }

      if (!distributionNeeded)
         return true;

      incrementChargeDistributionCount ();

      assert (m_distributor);

#if 0
      cerrstrstr << GetRankString () << ": " << this << "::distributeCharges ("
                 << m_toBeDistributed[GAS].getWeight () << ", " << m_toBeDistributed[OIL].getWeight () << ")" << endl;
#endif

      // Check if wasting is possible:
      if (getWasteDepth (GAS) != WasteDepth)
      {
         double wasteLevel = getWasteDepth (GAS) - getTopDepth ();
         if (wasteLevel < m_levelToVolume->invert (numeric_limits<double>::max ()))
         {
            m_distributor->setWasteLevel (wasteLevel);
            m_distributor->setWasting (true);
         }
      }

      // Check if leaking is necessary:
      m_distributor->setLeaking (!isUndersized ());

      Composition gasLeaked;
      Composition gasWasted;
      Composition gasSpilled;
      Composition oilLeaked;
      Composition oilSpilledOrWasted;

      double finalGasLevel;
      double finalHCLevel;

      double crestPressure = getCrestColumn()->getPressure();

      m_distributor->distribute (m_toBeDistributed[GAS], m_toBeDistributed[OIL],
                                 getTemperature () + CelciusToKelvin, m_distributed[GAS], m_distributed[OIL], gasLeaked, gasWasted,
                                 gasSpilled, oilLeaked, oilSpilledOrWasted, finalGasLevel, finalHCLevel, crestPressure);

      // If after distribution the weights of the vapour and the liquid phases are equal to the input weights
      // down to a billionth, then whatever processes took place are insignificant, and are ignored. In this way,
      // the possibility for hangs in fillAndSpill due to minute amounts of leakage, spillage and wasting
      // being calculated is eliminated.
      if ( NumericFunctions::isEqual(m_toBeDistributed[GAS].getWeight(), m_distributed[GAS].getWeight(), trapTolerance) and
           NumericFunctions::isEqual(m_toBeDistributed[OIL].getWeight(), m_distributed[OIL].getWeight(), trapTolerance) )
      {
         m_toBeDistributed[GAS].reset ();
         m_toBeDistributed[OIL].reset ();

         setFillDepth (GAS, finalGasLevel + getTopDepth ());
         setFillDepth (OIL, finalHCLevel + getTopDepth ());

         return true;
      }

#ifdef DETAILED_MASS_BALANCE
      m_massBalance->subtractFromBalance("gas leaked", gasLeaked.getWeight());
      m_massBalance->subtractFromBalance("gas wasted", gasWasted.getWeight());
      m_massBalance->subtractFromBalance("gas spilled", gasSpilled.getWeight());
      m_massBalance->subtractFromBalance("oil leaked", oilLeaked.getWeight());
      m_massBalance->subtractFromBalance("oil spilled or wasted", oilSpilledOrWasted.getWeight());
      m_massBalance->subtractFromBalance("gas remaining", m_distributed[GAS].getWeight());
      m_massBalance->subtractFromBalance("oil remaining", m_distributed[OIL].getWeight());

      m_massBalance->printBalance();

      assert(fabs(m_massBalance->balance()) < 10.0);
#endif

#ifdef DETAILED_VOLUME_BALANCE
      m_volumeBalance->subtractFromBalance("gas leaked", gasLeaked.getVolume());
      m_volumeBalance->subtractFromBalance("gas wasted", gasWasted.getVolume());
      m_volumeBalance->subtractFromBalance("gas spilled", gasSpilled.getVolume());
      m_volumeBalance->subtractFromBalance("oil leaked", oilLeaked.getVolume());
      m_volumeBalance->subtractFromBalance("oil spilled or wasted", oilSpilledOrWasted.getVolume());
      m_volumeBalance->subtractFromBalance("gas remaining", m_distributed[GAS].getVolume());
      m_volumeBalance->subtractFromBalance("oil remaining", m_distributed[OIL].getVolume());

      m_volumeBalance->printBalance();

      assert(fabs(m_volumeBalance->balance()) < 10.0);
#endif

      //trap position for buffered addition
      int numI = getReservoir()->getGrid()->numIGlobal();
      int position = getCrestColumn()->getI() + getCrestColumn()->getJ() * numI;

      if (!gasLeaked.isEmpty () || !oilLeaked.isEmpty ())
      {
         gasLeaked.add (oilLeaked);
         getCrestColumn ()->addLeakComposition (gasLeaked);
      }

      if (!gasWasted.isEmpty ())
      {
         getWasteColumn (GAS)->addWasteCompositionToBuffer(GAS, position, gasWasted);
         m_reservoir->reportWaste (this, getWasteColumn (GAS), gasWasted);
      }

      if (!oilSpilledOrWasted.isEmpty ())
      {
         if (getWasteColumn (OIL))
         {
            getWasteColumn (OIL)->addWasteCompositionToBuffer(OIL, position, oilSpilledOrWasted);
            m_reservoir->reportWaste (this, getWasteColumn (OIL), oilSpilledOrWasted);
         }
         else
         {
            Column* targetColumn = getFinalSpillTarget (OIL);

            targetColumn->addSpillCompositionToBuffer (OIL, position, oilSpilledOrWasted);
            m_reservoir->reportSpill (this, targetColumn, oilSpilledOrWasted);
            getSpillTarget (OIL)->addMigrated (OIL, oilSpilledOrWasted.getWeight ());
            setSpilling ();
         }
      }

      if (!gasSpilled.isEmpty ())
      {
         Column* targetColumn = getFinalSpillTarget (GAS);

         targetColumn->addSpillCompositionToBuffer (GAS, position, gasSpilled);
         m_reservoir->reportSpill (this, targetColumn, gasSpilled);
         getSpillTarget (GAS)->addMigrated (GAS, gasSpilled.getWeight ());
         setSpilling ();
      }

      // can be removed???
      // negotiateDensity(GAS);
      // negotiateDensity(OIL);

      m_toBeDistributed[GAS].reset ();
      m_toBeDistributed[OIL].reset ();

      if ( (finalGasLevel + getTopDepth () > getSpillDepth () and !NumericFunctions::isEqual(finalGasLevel + getTopDepth (), getSpillDepth (), trapTolerance)) ||
           (finalHCLevel  + getTopDepth () > getSpillDepth () and !NumericFunctions::isEqual(finalHCLevel  + getTopDepth (), getSpillDepth (), trapTolerance)) )
      {
         std::cerr.precision(17);
         std::cerr  << GetRankString () << ": " << this << ": finalGasDepth = " << getTopDepth () + finalGasLevel
              << ", finalHCDepth = " << getTopDepth () + finalHCLevel
              << ", spill depth = " << getSpillDepth () << endl;
      }

      setFillDepth (GAS, finalGasLevel + getTopDepth ());
      setFillDepth (OIL, finalHCLevel + getTopDepth ());

      return true;
   }

   void Trap::incrementChargeDistributionCount (void)
   {
      m_reservoir->incrementChargeDistributionCount ();
   }

   bool Trap::isSpillingBack (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if (getCrestColumn ()->isSpillingBack ((PhaseId)phase))
            return true;
      }
      return false;
   }

   bool Trap::requiresPVT (void)
   {
      return (!m_computedPVT && getCrestColumn ()->containsComposition ());
   }

   bool Trap::computePVT (void)
   {
      assert (requiresPVT ());

      double inputWeight = getCrestColumn ()->getCompositionWeight ();

      double balance = 0;
      balance += inputWeight;

      double outputWeightOld = m_toBeDistributed[GAS].getWeight () + m_toBeDistributed[OIL].getWeight ();
      assert (outputWeightOld < 1e5);

      // computes PVT and moves it to m_toBeDistributed[]
      getCrestColumn ()->computePVT (m_toBeDistributed);
      assert (!getCrestColumn ()->containsComposition ());

      double outputWeight = m_toBeDistributed[GAS].getWeight () + m_toBeDistributed[OIL].getWeight ();
      balance -= outputWeight;

      assert (inputWeight > 0 && Abs (balance / inputWeight) < 1e5);

      m_computedPVT = true;

#ifdef DETAILED_MASS_BALANCE
      {
         m_massBalance->addToBalance("distributed gas", m_toBeDistributed[GAS].getWeight());
         m_massBalance->addToBalance("distributed oil", m_toBeDistributed[OIL].getWeight());

         m_massBalance->addComment("\nTrap:\n");
         m_massBalance->addComment("----\n");
         m_massBalance->addComment("Reservoir   = ");
         m_massBalance->addComment(getReservoir()->getName() + "\n");
         m_massBalance->addComment("Crest column= ");
         ostringstream strstream; strstream << getCrestColumn() << endl;
         m_massBalance->addComment(strstream.str());
         m_massBalance->addComment("Snapshot    = ");
         m_massBalance->addComment(getReservoir()->getEnd()->asString() + "\n");
         m_massBalance->addComment("Size        = ");
         ostringstream strstream0; strstream0 << getSize() << endl;
         m_massBalance->addComment(strstream0.str());
         ostringstream strstream1; strstream1 << getTopDepth() << endl;
         m_massBalance->addComment("Depth       = ");
         m_massBalance->addComment(strstream1.str());

         m_massBalance->addComment("\nTrap Properties:\n");
         m_massBalance->addComment("---------------\n");
         ostringstream strstream2; strstream2 << "Temperature = " << getTemperature() << " C" << endl;
         m_massBalance->addComment(strstream2.str());
         ostringstream strstream3; strstream3 << "Pressure    = " << getPressure() << " MPa" << endl;
         m_massBalance->addComment(strstream3.str());
         ostringstream strstream4; strstream4 << "Capacity    = " << getCapacity() << " m^3" << endl;
         m_massBalance->addComment(strstream4.str());

         if (m_toBeDistributed[GAS].getWeight() > 0.0) {
            m_massBalance->addComment("\nGas Properties:\n");
            m_massBalance->addComment("--------------\n");
            ostringstream strstream5; strstream5 << "Density     = " << m_toBeDistributed[GAS].getDensity() << " kg/m^3" << endl;
            m_massBalance->addComment(strstream5.str());
            ostringstream strstream6; strstream6 << "Viscosity   = " << m_toBeDistributed[GAS].getViscosity() << endl;
            m_massBalance->addComment(strstream6.str());
         }

         if (m_toBeDistributed[OIL].getWeight() > 0.0) {
            m_massBalance->addComment("\nOil Properties:\n");
            m_massBalance->addComment("--------------\n");
            ostringstream strstream7; strstream7 << "Density     = " << m_toBeDistributed[OIL].getDensity() << endl;
            m_massBalance->addComment(strstream7.str());
            ostringstream strstream8; strstream8 << "Viscosity   = " << m_toBeDistributed[OIL].getViscosity() << endl;
            m_massBalance->addComment(strstream8.str());
         }

         m_massBalance->addComment("\nMass Balance:\n");
         m_massBalance->addComment("------------\n");
      }
#endif

#ifdef DETAILED_VOLUME_BALANCE
      {
         m_volumeBalance->addToBalance("distributed gas", m_toBeDistributed[GAS].getVolume());
         m_volumeBalance->addToBalance("distributed oil", m_toBeDistributed[OIL].getVolume());

         m_volumeBalance->addComment("\nTrap:\n");
         m_volumeBalance->addComment("----\n");
         m_volumeBalance->addComment("Reservoir   = ");
         m_volumeBalance->addComment(getReservoir()->getName() + "\n");
         m_volumeBalance->addComment("Crest column= ");
         ostringstream strstream; strstream << getCrestColumn() << endl;
         m_volumeBalance->addComment(strstream.str());
         m_volumeBalance->addComment("Snapshot    = ");
         m_volumeBalance->addComment(getReservoir()->getEnd()->asString() + "\n");
         m_volumeBalance->addComment("Size        = ");
         ostringstream strstream0; strstream0 << getSize() << endl;
         m_volumeBalance->addComment(strstream0.str());
         ostringstream strstream1; strstream1 << getTopDepth() << endl;
         m_volumeBalance->addComment("Depth       = ");
         m_volumeBalance->addComment(strstream1.str());

         m_volumeBalance->addComment("\nTrap Properties:\n");
         m_volumeBalance->addComment("---------------\n");
         ostringstream strstream2; strstream2 << "Temperature = " << getTemperature() << " C" << endl;
         m_volumeBalance->addComment(strstream2.str());
         ostringstream strstream3; strstream3 << "Pressure    = " << getPressure() << " MPa" << endl;
         m_volumeBalance->addComment(strstream3.str());
         ostringstream strstream4; strstream4 << "Capacity    = " << getCapacity() << " m^3" << endl;
         m_volumeBalance->addComment(strstream4.str());

         if (m_toBeDistributed[GAS].getVolume() > 0.0) {
            m_volumeBalance->addComment("\nGas Properties:\n");
            m_volumeBalance->addComment("--------------\n");
            ostringstream strstream5; strstream5 << "Density     = " << m_toBeDistributed[GAS].getDensity() << " kg/m^3" << endl;
            m_volumeBalance->addComment(strstream5.str());
            ostringstream strstream6; strstream6 << "Viscosity   = " << m_toBeDistributed[GAS].getViscosity() << endl;
            m_volumeBalance->addComment(strstream6.str());
         }

         if (m_toBeDistributed[OIL].getVolume() > 0.0) {
            m_volumeBalance->addComment("\nOil Properties:\n");
            m_volumeBalance->addComment("--------------\n");
            ostringstream strstream7; strstream7 << "Density     = " << m_toBeDistributed[OIL].getDensity() << endl;
            m_volumeBalance->addComment(strstream7.str());
            ostringstream strstream8; strstream8 << "Viscosity   = " << m_toBeDistributed[OIL].getViscosity() << endl;
            m_volumeBalance->addComment(strstream8.str());
         }

         m_volumeBalance->addComment("\nMass Balance:\n");
         m_volumeBalance->addComment("------------\n");
      }
#endif

      return true;
   }

   bool Trap::requiresDistribution (void)
   {
      bool result = false;
      if (getCrestColumn ()->containsComposition () &&
          ((getCrestColumn ()->getCompositionState () & (INITIAL+ LEAKED + SPILLED)) != 0))
         result = true;

      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         if (!m_toBeDistributed[phase].isEmpty ()) result = true;
      }

      return result;
   }

   double Trap::computethicknessAffectedByBiodegradationAboveOWC (const double timeInterval, const double bioRate) const
   {
      // Arbitrary value which states the thickness of the biodegradation impact above the OWC
      double const thicknessAffectedByBiodegradationAboveOWC = bioRate * timeInterval;  // Original value entered by the user or by default: 0.3m/Myr, but depends also on timeInterval

      return thicknessAffectedByBiodegradationAboveOWC;
   }

   bool Trap::computeHydrocarbonWaterContactDepth (void)
   {
      // Charge is flashed split in phases during distributeCarges()
      // Put it in m_toBeDistributed so it can be biodegraded
      // Must be put back to m_distributed after biodegradation
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         m_toBeDistributed[phase].add(m_distributed[phase]);
         m_toBeDistributed[phase].setDensity(m_distributed[phase].getDensity());
         m_distributed[phase].reset();
      }

      double const volumeOil = m_toBeDistributed[OIL].getVolume ();
      double const volumeGas = m_toBeDistributed[GAS].getVolume ();

      double const maximumCapacityOfTrap = getVolumeBetweenDepths2 (getTopDepth (), getBottomDepth ());
      m_hydrocarbonWaterContactDepth = -199999;

      if ((volumeGas + volumeOil) < maximumCapacityOfTrap)
      {
         m_hydrocarbonWaterContactDepth = (m_levelToVolume->invert (volumeGas + volumeOil));
         m_hydrocarbonWaterContactDepth += getTopDepth ();
      }
      else
      {
         m_hydrocarbonWaterContactDepth = getBottomDepth ();
      }

      // Assert to be add again after bug fix in Fastmig: spilling depth of a trap shallower than the top depth of the trap....
      //assert((m_hydrocarbonWaterContactDepth >= getTopDepth()) && (m_hydrocarbonWaterContactDepth <= getBottomDepth()));

      // Is this necessary?
      setFillDepth (OIL, m_hydrocarbonWaterContactDepth);

      return true;
   }

   double Trap::computeFractionVolumeBiodegraded (const double timeInterval, const PhaseId phase, const double bioRate)
   {
      double const volumePhase = m_toBeDistributed[phase].getVolume ();

      // Arbitrary value which states the thickness of the biodegradation impact above the OWC
      double const thicknessAffectedByBiodegradationAboveOWC = computethicknessAffectedByBiodegradationAboveOWC (timeInterval, bioRate);

      double const maximumCapacityOfTrap = getVolumeBetweenDepths2 (getTopDepth (), getBottomDepth ());
      double const volumeBiodegraded = min (getVolumeBetweenDepths2 (getHydrocarbonWaterContactDepth () - thicknessAffectedByBiodegradationAboveOWC, getHydrocarbonWaterContactDepth ()), volumePhase);

      assert (volumeBiodegraded >= 0.0);
      assert (volumeBiodegraded <= maximumCapacityOfTrap && volumeBiodegraded <= volumePhase);

      double const fractionVolumeBiodegraded = volumeBiodegraded / volumePhase;
      assert (fractionVolumeBiodegraded >= 0.0 && fractionVolumeBiodegraded <= 1.0);

#ifdef DEBUG_BIODEGRADATION
      cerr << endl << "==== Compute the fraction of volume which is biodegraded ====" << endl;
      cerr << "Volume of GAS in the Trap: " << m_toBeDistributed[GAS].getVolume() << " ; Volume of OIL in the Trap: " << m_toBeDistributed[OIL].getVolume() << endl;
      cerr << "Volume total of HC in the Trap: " << m_toBeDistributed[GAS].getVolume() + m_toBeDistributed[OIL].getVolume() << endl;
      cerr << "Top depth of the trap (crest column): " << getTopDepth() << endl;
      cerr << "Hydrocarbon - water contact depth: " << getHydrocarbonWaterContactDepth() << endl;
      cerr << "Bottom depth of the trap (spilling Point): " << getBottomDepth() << endl;
      cerr << "Maximum capacity of the trap: " << getVolumeBetweenDepths2(getTopDepth(), getBottomDepth()) << endl;
      cerr << "Volume impacted by biodegradation in the trap: " << volumeBiodegraded << endl;
      cerr << "Fraction of the volume impacted by biodegradation: " << fractionVolumeBiodegraded * 100 << " % " << endl;
#endif

      // Assert to be add again after bug fix in Fastmig: spilling depth of a trap shallower than the top depth of the trap....
      //assert(getTopDepth() < getBottomDepth());
      return fractionVolumeBiodegraded;
   }

   bool Trap::computeHydrocarbonWaterContactTemperature ()
   {
      LocalColumn const * crestColumn = getCrestColumn ();
      // Initialisation of the hydrocarbon - water contact temperature at the temperature at the top of crest column of the trap
      m_hydrocarbonWaterContactTemperature = crestColumn->getTemperature ();

      // If the hydrocarbon - water contact is included between the top and the bottom of the crest column
      // The interpolation of temperature will be done only on the crest column
      if (m_hydrocarbonWaterContactDepth <= crestColumn->getBottomDepth ())
      {
         m_hydrocarbonWaterContactTemperature = crestColumn->getOWCTemperature (m_hydrocarbonWaterContactDepth);
      }
      // Else we need to find another column which has a top and a bottom depth which surrounded the OWC
      else
      {
         // Find a column of the trap with a bottom depth deeper than the hydrocarbon - water contact
         // and a top depth shallower than the hydrocarbon - water contact
         ConstColumnIterator iter;
         for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * column = *iter;

            if (m_hydrocarbonWaterContactDepth <= column->getBottomDepth () && m_hydrocarbonWaterContactDepth >= column->getTopDepth ())
            {
               m_hydrocarbonWaterContactTemperature = column->getOWCTemperature (m_hydrocarbonWaterContactDepth);
               break;
            }
         }

      }

      return true;
   }

   void Trap::computePhaseVolumeProportionInBiodegradadedZone (const double timeInterval, double& volumeFractionOfGasBiodegraded, double& volumeFractionOfOilBiodegraded, const Biodegrade& biodegrade)
   {
#ifdef DEBUG_BIODEGRADATION
      cerr << endl << "==== Phase(s) impacted by bidoegradation ====" << endl;
#endif

      double const thicknessAffectedByBiodegradationAboveOWC = computethicknessAffectedByBiodegradationAboveOWC (timeInterval, biodegrade.bioRate ());

      if (m_toBeDistributed[GAS].isEmpty () && m_toBeDistributed[OIL].isEmpty ()) // nothing in the trap
      {
#ifdef DEBUG_BIODEGRADATION
         cerr << "0) Nothing " << endl;
#endif
         volumeFractionOfGasBiodegraded = 0;
         volumeFractionOfOilBiodegraded = 0;
      }
      else if (m_toBeDistributed[OIL].isEmpty ()) // only gas in the trap
      {
#ifdef DEBUG_BIODEGRADATION
         cerr << "1) Only GAS " << endl;
#endif
         volumeFractionOfGasBiodegraded = computeFractionVolumeBiodegraded (timeInterval, GAS, biodegrade.bioRate ());
         volumeFractionOfOilBiodegraded = 0;
      }
      else if (m_toBeDistributed[GAS].isEmpty ())  // only oil in the trap
      {
#ifdef DEBUG_BIODEGRADATION
         cerr << "2) Only OIL " << endl;
#endif
         volumeFractionOfGasBiodegraded = 0;
         volumeFractionOfOilBiodegraded = computeFractionVolumeBiodegraded (timeInterval, OIL, biodegrade.bioRate ());
      }
      else  // case of a mixed-fill trap
      {
         double const maxCapacityOfTrap = getVolumeBetweenDepths2 (getTopDepth (), getBottomDepth ());
         double const volumeBiodegraded = min (getVolumeBetweenDepths2 (getHydrocarbonWaterContactDepth () - thicknessAffectedByBiodegradationAboveOWC, getHydrocarbonWaterContactDepth ()), maxCapacityOfTrap);

         if (m_toBeDistributed[OIL].getVolume () >= volumeBiodegraded) // only oil in the biodegraded zone
         {
#ifdef DEBUG_BIODEGRADATION
            cerr << "3) OIL and GAS, but only OIL is biodegraded " << endl;
#endif
            volumeFractionOfGasBiodegraded = 0;
            volumeFractionOfOilBiodegraded = computeFractionVolumeBiodegraded (timeInterval, OIL, biodegrade.bioRate ());
         }
         else // oil and gas are present in the biodegraded zone
         {
#ifdef DEBUG_BIODEGRADATION
            cerr << "4) Both phases are biodegraded " << endl;
#endif
            volumeFractionOfOilBiodegraded = 1;
            volumeFractionOfGasBiodegraded = (volumeBiodegraded - m_toBeDistributed[OIL].getVolume ()) / m_toBeDistributed[GAS].getVolume ();
            volumeFractionOfGasBiodegraded = min (volumeFractionOfGasBiodegraded, 1.0);   // in case that the biodegradation zone extends above the volume of Gas
         }
      }
      assert (volumeFractionOfOilBiodegraded >= 0 && volumeFractionOfOilBiodegraded <= 1);
      assert (volumeFractionOfGasBiodegraded >= 0 && volumeFractionOfGasBiodegraded <= 1);

#ifdef DEBUG_BIODEGRADATION
      cerr << endl << "==== Computation of the phase volume proportions in the biodegraded impact zone ====" << endl;
      cerr << "Percent of Gas biodegraded: " << volumeFractionOfGasBiodegraded * 100 << " %" << endl;
      cerr << "Percent of Oil biodegraded: " << volumeFractionOfOilBiodegraded * 100 << " %" << endl;
#endif
   }

   void Trap::printInconsistentVolumes ()
   {
      if (getVolume (GAS) < 0.99 * getVolumeByColumns (GAS) || getVolume (GAS) > 1.01 * getVolumeByColumns (GAS))
      {
         cerr << "        " << this << " :: composition based volume (Gas) = " << getVolume (GAS) << endl;
         cerr << "        " << this << " :: column based volume (Gas) = " << getVolumeByColumns (GAS) << endl;
         cerr << endl;
      }

      if (getVolume (OIL) < 0.99 * getVolumeByColumns (OIL) || getVolume (OIL) > 1.01 * getVolumeByColumns (OIL))
      {
         cerr << "        " << this << " :: composition based volume (Oil) = " << getVolume (OIL) << endl;
         cerr << "        " << this << " :: column based volume (Oil) = " << getVolumeByColumns (OIL) << endl;
         cerr << endl;
      }
   }

   double Trap::getVolumeByColumns (PhaseId phase) const
   {
      double total = 0;
      for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = *iter;

         total += column->getVolumeBetweenDepths (column->getFillDepth (phase), column->getFillDepth (phase) + column->getFillHeight (phase));
      }

      return total;
   }

   void Trap::printSpillTrajectory (PhaseId phase)
   {
      Column * targetColumn = getFinalSpillTarget (phase);

      cerr << GetRankString () << ": " << m_reservoir->getName () << "->" << this
           << "->spills to " << targetColumn;

      Column * tmpColumn = getSpillTarget (phase);
      while (tmpColumn != targetColumn)
      {
         cerr << endl << "\tvia " << tmpColumn;
         tmpColumn = tmpColumn->getTargetColumn (phase);
      }

      cerr << endl;
   }

   void Trap::broadcastDiffusionStartTimes (void)
   {
      double diffusionStartTime = getDiffusionStartTime ();
      if (diffusionStartTime < 0) return;
      if (getCrestColumn ()->getDiffusionStartTime () == diffusionStartTime) return; // no reason for broadcasting

      for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         (*iter)->setDiffusionStartTime (diffusionStartTime);
      }
   }

   void Trap::broadcastPenetrationDistances (void)
   {
      const double * trapPenetrationDistances = getPenetrationDistances (); //get the trap penetration distance
      const double * columnPenetrationDistances = getCrestColumn ()->getPenetrationDistances ();

      if (std::equal (columnPenetrationDistances, columnPenetrationDistances + DiffusionComponentSize, trapPenetrationDistances))
      {
         return; // no reason for broadcasting
      }

      for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         (*iter)->setPenetrationDistances (trapPenetrationDistances);
      }
   }

   void Trap::broadcastFillDepthProperties (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         double fillDepth = getFillDepth ((PhaseId)phase);

         if (fillDepth > getSpillDepth () and !NumericFunctions::isEqual(fillDepth, getSpillDepth (), trapTolerance))
         {
            std::cerr.precision(17);
            std::cerr << "Error in " << this <<  ": TrapID " << getGlobalId() << " fillDepth (" << fillDepth << ") for phase " << PhaseNames[phase] << " is larger than spillDepth (" << getSpillDepth () << ")" << endl;
         }

         for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * column = *iter;

            column->setFillDepth ((PhaseId)phase, fillDepth);
         }
      }
   }

   void Trap::broadcastChargeProperties (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * column = *iter;

            column->setChargesToBeMigrated ((PhaseId)phase, m_distributed[phase]);
         }
      }
   }

   // move all charge heavier than the given phase that occupies the space upto the
   // given depth back to the trap for re-distribution.
   // This function assumes that the freed space is to be occupied by hydrocarbon of the
   // given phase.
   void Trap::moveBackToToBeDistributed (PhaseId lighterPhase)
   {
      for (int phase = lighterPhase + 1; phase < NUM_PHASES; ++phase)
      {
         m_toBeDistributed[phase].add (m_distributed[phase]);
         m_distributed[phase].reset ();
         setFillDepth ((PhaseId)phase, getFillDepth (PhaseId (phase - 1)));
      }
   }

   /// move all the stuff already in the trap back to the trap's crest column.
   /// To be executed if new stuff got spilled to this trap.
   void Trap::moveBackToCrestColumn (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         moveBackToCrestColumn ((PhaseId)phase);
      }
   }

   // move only the trap content (m_distributed) to the crest column
   void Trap::moveDistributedToCrestColumn (void)
   {
      // Temporarily store leaked composition in m_leakedBeforeBiodegDiffusion
      LocalColumn * crestColumn = getCrestColumn ();
      if (!m_leakedBeforeBiodegDiffusion) m_leakedBeforeBiodegDiffusion = new Composition;

      m_leakedBeforeBiodegDiffusion->setDensity (0);
      m_leakedBeforeBiodegDiffusion->add (crestColumn->getComposition ());

      // reset crest column composition
      crestColumn->getComposition ().reset ();

      // move only m_distributed to crest
      crestColumn->addComposition (m_distributed[GAS]);
      m_distributed[GAS].reset ();
      crestColumn->addComposition (m_distributed[OIL]);
      m_distributed[OIL].reset ();

      setFillDepth (GAS, getTopDepth ());
      setFillDepth (OIL, getTopDepth ());
   }

   /// move all the stuff already in the trap back to the trap's crest column.
   /// To be executed if new stuff got spilled to this trap.
   void Trap::moveBackToCrestColumn (PhaseId phase)
   {
      LocalColumn * crestColumn = getCrestColumn ();

      crestColumn->addComposition (m_toBeDistributed[phase]);
      m_toBeDistributed[phase].reset ();

      crestColumn->addComposition (m_distributed[phase]);
      m_distributed[phase].reset ();
      setFillDepth (phase, getTopDepth ());
   }

   void Trap::moveBackToBeMigrated (void)
   {
      for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
      {
         moveBackToBeMigrated ((PhaseId)phase);
      }
   }

   void Trap::moveBackToBeMigrated (PhaseId phase)
   {
      LocalColumn * crestColumn = getCrestColumn ();

      crestColumn->addCompositionToBeMigrated (m_toBeDistributed[phase]);
      m_toBeDistributed[phase].reset ();

      crestColumn->addCompositionToBeMigrated (m_distributed[phase]);
      m_distributed[phase].reset ();
      setFillDepth (phase, getTopDepth ());
   }

   void Trap::putInitialLeakageBack()
   {
      if (m_leakedBeforeBiodegDiffusion)
         getCrestColumn ()->getComposition ().add (*m_leakedBeforeBiodegDiffusion);

      return;
   }

   void Trap::negotiateDensity (PhaseId phase)
   {
      if (m_distributed[phase].getDensity () == 0)
      {
         m_distributed[phase].setDensity (m_toBeDistributed[phase].getDensity ());
      }
      else
      {
         m_toBeDistributed[phase].setDensity (m_distributed[phase].getDensity ());
      }
   }

   bool Trap::isFull (PhaseId phase)
   {
      return (getFillDepth (phase) >= getBottomDepth ());
   }

   void Trap::setDiffusionStartTime (double diffusionStartTime)
   {
      m_diffusionStartTime = diffusionStartTime;
   }

   double Trap::getDiffusionStartTime ()
   {
      return m_diffusionStartTime;
   }

   void Trap::setPenetrationDistances (const double * penetrationDistances)
   {
      std::copy (penetrationDistances, penetrationDistances + DiffusionComponentSize, m_penetrationDistances);
   }

   const double * Trap::getPenetrationDistances ()
   {
      return m_penetrationDistances;
   }

   void Trap::setPenetrationDistance (ComponentId c, double penetrationDistance)
   {
      m_penetrationDistances[c] = penetrationDistance;
   }

   double Trap::getPenetrationDistance (ComponentId c)
   {
      return m_penetrationDistances[c];
   }

   void Trap::setFillDepth (PhaseId phase, double fillDepth)
   {
      if (phase == FIRST_PHASE)
      {
         m_fillDepth[phase] = fillDepth;
      }
      else
      {
         m_fillDepth[phase] = Max (fillDepth, getFillDepth (PhaseId (phase - 1)));
      }
   }

   double Trap::getFillDepth (PhaseId phase)
   {
      double fillDepth;

      if (phase == FIRST_PHASE)
      {
         fillDepth = m_fillDepth[phase];
      }
      else
      {
         fillDepth = Max (m_fillDepth[phase], getFillDepth (PhaseId (phase - 1)));
      }
      return fillDepth;
   }


#ifdef THISISNOTRIGHT
   void Trap::setMinimumSpillDepth (double minimumSpillDepth)
   {
      m_minimumSpillDepth = Max (m_minimumSpillDepth, minimumSpillDepth);
   }

   double Trap::getMinimumSpillDepth (void)
   {
      return m_minimumSpillDepth;
   }
#endif

   double Trap::getDensity (PhaseId phase) const
   {
      return m_distributed[phase].getDensity ();
   }

   void Trap::printPerimeter (void)
   {
      cerr << GetRankString () << ":: Perimeter (" << this << "): ";
      ConstColumnIterator iter;
      int i;
      for (i = 0, iter = m_perimeter.begin (); iter != m_perimeter.end (); ++i, ++iter)
      {
         if (i > 0 && i % 4 == 0) cerr << endl << "\t";
         cerr << *iter << ", ";
      }
      cerr << endl;
   }

   void Trap::printInterior (void)
   {

      cerr << GetRankString () << ":: Interior (" << this << "): ";
      ConstColumnIterator iter;
      int i;
      for (i = 0, iter = m_interior.begin (); iter != m_interior.end (); ++i, ++iter)
      {
         if (i > 0 && i % 4 == 0) cerr << endl << "\t";
         cerr << *iter << ", ";
      }
      cerr << endl;
   }

   void Trap::resetExtended (void)
   {
      m_extended = false;
   }

   void Trap::setExtended (void)
   {
      m_extended = true;
   }

   bool Trap::hasBeenExtended (void)
   {
      return m_extended;
   }

   void Trap::resetToBeAbsorbed (void)
   {
      m_toBeAbsorbed = false;
   }

   void Trap::setToBeAbsorbed (void)
   {
      if (DebugOn)
      {
         cerr << GetRankString () << ": " << this << "->toBeAbsorbed (" << getSpillDepth () << ")" << endl;
      }
      m_toBeAbsorbed = true;
   }

   bool Trap::isToBeAbsorbed (void)
   {
      return m_toBeAbsorbed;
   }

   ostream & operator<< (ostream & stream, Trap & trap)
   {
      return stream << &trap;
   }

   ostream & operator<< (ostream & stream, Trap * trap)
   {
      if (trap->getSpillColumn ())
      {
         stream << "Trap (" << trap->getReservoir ()->getName () << ":" << trap->getCrestColumn () <<
            ", " << trap->getSpillColumn () << ", C = " << trap->getCapacity () << ", S = " << trap->getSize () << ")";
      }
      else
      {
         stream << "Trap (" << trap->getReservoir ()->getName () << ":" << trap->getCrestColumn () << ")";
      }

      return stream;
   }

   void Trap::reportLeakage ()
   {
      Composition & leakageComposition = getCrestColumn ()->getComposition ();
      if (!leakageComposition.isEmpty ())
      {
         m_reservoir->reportWaste (this, getCrestColumn (), leakageComposition);
      }
   }

   void Trap::collectProperties (TrapPropertiesRequest & tpRequest)
   {
      tpRequest.i = getI ();
      tpRequest.j = getJ ();

      tpRequest.id = getGlobalId ();
      tpRequest.spilling = m_spilling;

      if (m_spilling)
      {
         tpRequest.spillid = getFinalSpillTarget (GAS)->getGlobalTrapId ();
         if (tpRequest.spillid < 0)
         {
            tpRequest.spillid = getFinalSpillTarget (OIL)->getGlobalTrapId ();
         }
      }
      else
      {
         tpRequest.spillid = NoTrapId;
      }

      tpRequest.rank = GetRank ();
      tpRequest.capacity = getCapacity ();
      tpRequest.depth = getTopDepth ();
      tpRequest.spillDepth = getSpillDepth ();
      tpRequest.spillPointI = getSpillColumn ()->getI ();
      tpRequest.spillPointJ = getSpillColumn ()->getJ ();
      tpRequest.wcSurface = getSurface (LAST_PHASE);
      tpRequest.temperature = getTemperature ();
      tpRequest.pressure = getPressure ();
      tpRequest.permeability = getPermeability ();
      tpRequest.sealPermeability = getSealPermeability ();
      tpRequest.fracturePressure = getFracturePressure () * PaToMegaPa;
      tpRequest.netToGross = getNetToGross () * Fraction2Percentage;
      tpRequest.fractureSealStrength = -1;

      // funny code by nlamol
      LeakWasteAndSpillDistributor* leakDistributor = dynamic_cast<LeakWasteAndSpillDistributor*>(m_distributor);

      tpRequest.fractureSealStrength = -1;
      if (leakDistributor && leakDistributor->fractureSealStrength () != numeric_limits<double>::max ())
         tpRequest.fractureSealStrength = PaToMegaPa * leakDistributor->fractureSealStrength ();

      tpRequest.goc = getFillDepth (GAS);
      tpRequest.owc = getFillDepth (OIL);

      double gorm = computeGorm (m_distributed[GAS], m_distributed[OIL]);

      double capSealStrength_H2O_Gas;
      double capSealStrength_H2O_Oil;
      std::vector<Composition> trapComposition(2);
      trapComposition[0] = m_distributed[GAS];
      trapComposition[1] = m_distributed[OIL];
      double crestPressure = getCrestColumn()->getPressure();

      for (int phase = (int)FIRST_PHASE; phase < (int)NUM_PHASES; ++phase)
      {
         tpRequest.cep[phase] = -1;
         tpRequest.criticalTemperature[phase] = -1;
         tpRequest.interfacialTension[phase] = -1;

         if (leakDistributor && m_distributed[phase].getWeight () > 0.0)
         {
            leakDistributor->capillarySealStrength().compute(trapComposition, gorm, getTemperature() + CelciusToKelvin, crestPressure,
                                                             capSealStrength_H2O_Gas, capSealStrength_H2O_Oil);

            if ( phase == GAS && capSealStrength_H2O_Gas != numeric_limits<double>::max ())
               tpRequest.cep[phase] = PaToMegaPa * capSealStrength_H2O_Gas;
            else if ( phase == OIL && capSealStrength_H2O_Oil != numeric_limits<double>::max() )
               tpRequest.cep[phase] = PaToMegaPa * capSealStrength_H2O_Oil;

            double criticalTemperature = leakDistributor->capillarySealStrength ().criticalTemperature (m_distributed[phase], gorm) - CelciusToKelvin;
            tpRequest.criticalTemperature[phase] = criticalTemperature;

            double interfacialTension = leakDistributor->capillarySealStrength ().interfacialTension (m_distributed[phase], gorm, getTemperature () + CelciusToKelvin);
            tpRequest.interfacialTension[phase] = interfacialTension;
         }
         tpRequest.volume[phase] = getVolume ((PhaseId)phase);
      }

      for (int component = ComponentId::FIRST_COMPONENT; component < ComponentId::NUMBER_OF_SPECIES; ++component)
      {
         tpRequest.composition.set ((ComponentId)component, getWeight ((ComponentId)component));
      }
   }

   /// not used
   bool Trap::saveProperties (void)
   {
      if (isUndersized ())
      {

         for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
         {
            Column * column = *iter;
            column->setGlobalTrapId (UnknownTrapId);
         }
         return false;
      }
      else
      {
         TrapPropertiesRequest tpRequest;

         tpRequest.valueSpec = SAVETRAPPROPERTIES;
         collectProperties (tpRequest);

         RequestHandling::SendRequest (tpRequest);

         return true;
      }
   }

} // namespace migration
