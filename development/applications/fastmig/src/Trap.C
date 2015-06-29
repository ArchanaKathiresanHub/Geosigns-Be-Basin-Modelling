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
#include "CBMGenerics/src/methaneSolubility.h"
#include "CBMGenerics/src/capillarySealStrength.h"
#include "CBMGenerics/src/fracturePressure.h"
#include "CBMGenerics/src/waterDensity.h"
#include "CBMGenerics/src/ComponentManager.h"
#include "functions/src/Tuple2.h"
#include "depthToVolume.h"
#include "Interface/FluidType.h"
#include "translateProps.h"
#include "LeakWasteAndSpillDistributor.h"
#include "LeakAllGasAndOilDistributor.h"
#include "SpillAllGasAndOilDistributor.h"
#include "utils.h"
#include "CBMGenerics/src/consts.h"

#include "Migrator.h"

#include <assert.h>
#include <algorithm>
#include <vector>
#include <sstream>
using std::ostringstream;

extern ostringstream cerrstrstr;

using namespace std;
using namespace CBMGenerics;

#define MAXDOUBLE std::numeric_limits<double>::max()

using Interface::Formation;
using Interface::Snapshot;
using functions::Tuple2;
using functions::tuple;

// #define DEBUG_TRAP
// #define DEBUG_BIODEGRADATION

namespace migration {

Trap::Trap (LocalColumn * column):
   m_levelToVolume(0),
#ifdef DEBUG_TRAP
   m_volumeToDepth2(0),
#endif
   m_distributor(0),
   m_globalId (UnknownTrapId),
   m_drainageAreaId (UnknownTrapId),
   m_diffusionStartTime (-1),
   m_diffusionOverburdenProps(0)

{
   column->setTrap (this);
   m_reservoir = column->getReservoir ();
   addToInterior (column);
   closePerimeter (column);

#ifdef THISISNOTRIGHT
   m_minimumSpillDepth = WasteDepth;
   setMinimumSpillDepth (column->getTopDepth ());
#endif

   m_spillTarget = 0;

   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      m_wasteColumns[phase] = 0;
      m_wasteDepths[phase] = WasteDepth;

      m_distributed[phase].reset ();
      m_distributed[phase].setDensity (0);

      m_toBeDistributed[phase].reset ();
      m_toBeDistributed[phase].setDensity (0);

      m_fillDepth[phase] = column->getTopDepth ();
   }

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
}

int Trap::getSize (void)
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
   assert(m_levelToVolume);
   double topDepth = upperDepth - getTopDepth();
   double baseDepth = lowerDepth - getTopDepth();
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
      const Column * column = * iter;
      volume += column->getVolumeBetweenDepths (upperDepth, lowerDepth);
   }
   return volume;
}

/// Compute the function that returns a fill depth for a volume to be stored.
/// If the number of interior columns is smaller than MaxInteriorSize, this function is pretty much exact.
/// Otherwise, this function will consist of between 1 / MaximumVolumeIncrease and 1 / MinimumVolumeIncrease points.
void Trap::computeDepthToVolumeFunction (void)
{
   computeVolumeToDepthFunction();
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
      m_volumeToDepth2->addPoint ((double) 0.0, (double) getTopDepth ());

      // process columns with respect to their topDepth () and their bottomDepth
      ConstColumnIterator iter;
      for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
	 Column * column = * iter;

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
   m_volumeToDepth2->addPoint ((double) MAXDOUBLE, (double) MAXDOUBLE);

}

/// Compute the function that maps capacity to depth. This function is always exact.
void Trap::computeVolumeToDepthFunction (void)
{
   // Create a new MonotonicIncreasingPiecewiseLinearInvertableFunction function:
   delete m_levelToVolume;
   if (m_distributor)
      m_distributor->setLevelToVolume(0);

   m_levelToVolume = depthToVolume::compute(getTopDepth(), m_interior.begin(), m_interior.end(), getWasteColumn (OIL) ? getWasteDepth (OIL) : getSpillDepth ());

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
      m_distributor->setLevelToVolume(m_levelToVolume);
}

void Trap::deleteDepthToVolumeFunction (void)
{
   delete m_levelToVolume; // FIXME
   m_levelToVolume = 0;
   if (m_distributor)
      m_distributor->setLevelToVolume(0);
#ifdef DEBUG_TRAP
   delete m_volumeToDepth2;
   m_volumeToDepth2 = 0;
#endif
}

double Trap::getDepthForVolume (double volume)
{
   assert (m_levelToVolume);
   double depth = getTopDepth() + m_levelToVolume->apply(volume);

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
   assert (getSpillColumn ());
   if (getSpillColumn ()->isSealing ())
   {
      // all columns in the perimeter are sealing as sealing columns are always at the back of the exterior
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
   // the target of this trap's crest column must be re-pointed to its spill target.
   Column * targetColumn = getSpillTarget ();

   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      getCrestColumn ()->setTargetColumn ((PhaseId) phase, targetColumn);
   }
   getCrestColumn ()->resetProxies ();

   double spillDepth = getSpillDepth ();

   for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
   {
      Column * column = * iter;
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
      if ((absorbingCrestColumn = getCrestColumn ()->getSpillBackTarget ((PhaseId) phase)) != 0)
	 break;
   }

   assert (phase < NUM_PHASES);
   assert (absorbingCrestColumn);

   for (phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      getCrestColumn ()->setTargetColumn ((PhaseId) phase, absorbingCrestColumn);
      getCrestColumn ()->setAdjacentColumn ((PhaseId) phase, getSpillTarget ());
   }

   getCrestColumn ()->resetProxies ();

   // Add columns to the absorbing trap
   ConstColumnIterator iter;
   for (iter = m_interior.begin (); iter != m_interior.end (); ++iter)
   {
      Column * column = * iter;

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
      setFillDepth ((PhaseId) phase, getTopDepth ());
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
      int iNeighbour = column->getI () + NeighbourOffsets[k][I];
      int jNeighbour = column->getJ () + NeighbourOffsets[k][J];

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
      if (* riter == column)
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
	 << * column
	 << endl;
   }

   ConstColumnIterator iter;
   for (iter = m_perimeter.begin (); iter != m_perimeter.end (); ++iter)
   {
      if (* iter == column)
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
      int iOffset = NeighbourOffsets[k][I];
      int jOffset = NeighbourOffsets[k][J];

      unsigned int iNeighbour = column->getI () + iOffset;
      unsigned int jNeighbour = column->getJ () + jOffset;

      Column * neighbourColumn = m_reservoir->getColumn (iNeighbour, jNeighbour);
      if (!IsValid (neighbourColumn))
	 continue;

      if (!isInInterior (neighbourColumn) /* && !isOnPerimeter (neighbourColumn) */) 
      {
	 addToPerimeter (neighbourColumn);
      }
   }
}

/// add a column to the Trap's perimeter such that the perimeter is sorted,
/// highest lying column first.
/// Put sealing columns add the end
void Trap::addToPerimeter (Column * column)
{

   int pos;
   ColumnIterator iter;

   for (pos = 0, iter = m_perimeter.begin (); iter != m_perimeter.end (); ++pos, ++iter)
   {
      Column * perimeterColumn = * iter;
      if (column == perimeterColumn) return;
      if (perimeterColumn->isSealing () || (!column->isSealing () && perimeterColumn->isDeeperThan (column)))
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

   // Sealing columns can't be part of a trap,
   // Wasting columns can.
   assert (!column->isSealing ());

   if (column->getThickness () == 0)
   {
      cerr << "WARNING from Trap::addToInterior (): Adding zero thickness column " << column << " to trap " << this << endl;
   }

   ColumnIterator iter;

   // probably more efficient to look backward, as we start out with a highest point and
   // successively add lower points until a spillpoint is found.
   int position = m_interior.size () - 1;
   for (iter = m_interior.end (); iter != m_interior.begin (); --iter)
   {
      ColumnIterator interiorPtr = iter;
      Column * interiorColumn = * --interiorPtr;

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
      --position;
   }

   m_interior.insert (iter, column);

   column->setGlobalTrapId (getGlobalId ());
}

void Trap::addColumnsToBeAdded ()
{
   IntPairVector::iterator iter;
   for (iter = m_toBeAdded.begin (); iter != m_toBeAdded.end (); ++iter)
   {
      Column * column = m_reservoir->getColumn ((* iter).first, (* iter).second);

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
      Column * interiorColumn = * iter;
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
      return dynamic_cast<LocalColumn *> (* m_interior.begin ());
   else if (!m_perimeter.empty ())
      return dynamic_cast<LocalColumn *> (* m_perimeter.begin ());
   else
      return 0;
}

Column * Trap::getSpillColumn (void) const
{
   return m_perimeter.front ();
}

double Trap::getSpillDepth ()
{
   if (getSpillColumn ()->isSealing ())
      return SealDepth;
   else
      return getSpillColumn ()->getTopDepth ();
}

Column * Trap::getColumnToSpillTo (void)
{
   Column * spillColumn = getSpillColumn ();
   if (spillColumn->isSealing ()) return 0;

   return getReservoir ()->getAdjacentColumn (GAS, spillColumn, this);
}

// find the column this trap spills to
void Trap::computeSpillTarget (void)
{
   Column * spillingTo = getColumnToSpillTo ();
   if (spillingTo)
   {
      m_spillTarget = spillingTo;
   }
   else
   {
      m_spillTarget = getSpillColumn ();
   }
   assert (m_spillTarget);

   assert (m_spillTarget != getCrestColumn ());

   if (getFinalSpillTarget (GAS) == getCrestColumn ())
   {
      cerr << "ERROR: final spill target(): "
	 << getFinalSpillTarget (GAS) << " == crest column: " << getCrestColumn () << endl;
      cerr << GetRankString () << ": " << this << "->getSpillColumn () = " << m_spillTarget << endl;
      printSpillTrajectory (GAS);
      printInterior ();
      printPerimeter ();

      assert (getFinalSpillTarget (GAS) != getCrestColumn ());
   }
}

/// Return the column where charge spilled from this trap finally ends up
Column * Trap::getFinalSpillTarget (PhaseId phase)
{
   return getSpillTarget ()->getFinalTargetColumn (phase);
}

Column * Trap::getSpillTarget (void)
{
   assert (m_spillTarget != 0);
   return m_spillTarget;
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
         Column * interiorColumn = * iter;
         if (interiorColumn->isWasting ((PhaseId) phase))
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
      weight += getWeight ((PhaseId) phase);
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
      weight += getWeight ((PhaseId) phase, component);
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
      weight += getWeightToBeDistributed ((PhaseId) phase);
   }
   return weight;
}

double Trap::getWeightToBeDistributed (PhaseId phase) const
{
   return m_toBeDistributed[phase].getWeight ();
}

double Trap::getSealPressureLeakages(void) const
{
   double loss = 0.0;
   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      loss += getSealPressureLeakages((PhaseId) phase);
   }
   return loss;
}

double Trap::getSealPressureLeakages(PhaseId phase) const
{
  return m_sealPressureLeaked[phase].getWeight();
}

double Trap::getDiffusionLeakages(void) const
{
   return m_diffusionLeaked[GAS].getWeight();
}

const MonotonicIncreasingPiecewiseLinearInvertableFunction* Trap::levelToVolume() const
{
   return m_levelToVolume;
}

double Trap::getCapacity (void) const
{
#ifdef COMPUTECAPACITY
   return m_capacity;
#else
   if (m_levelToVolume)
      return m_levelToVolume->apply(numeric_limits<double>::max());
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
   // always merge zero-sized traps
   return (getCapacity () == 0 || getCapacity () < getTrapCapacity ());
}

double Trap::getTrapCapacity (void) const
{
   return m_reservoir->getTrapCapacity ();
}

void Trap::setGlobalId (int id)
{
   m_globalId = id;
   for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
   {
      Column * column = * iter;
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
      Column * column = * iter;
      if (column->getFillDepth (phase) > column->getTopDepth ())
      {
	 surface += column->getSurface ();
      }
   }
   return surface;
}

/// if charge got spilled here, collect all charge at the crest column for re-distribution and perform PVT.
void Trap::collectAndSplitCharges (bool always)
{
   if (always || requiresDistribution ())
   {
      moveBackToCrestColumn ();
      m_computedPVT = false;
   }

#ifndef INCLUDE_COX
   assert (m_toBeDistributed[GAS].getWeight(pvtFlash::COX) == 0.0);
   assert (m_toBeDistributed[OIL].getWeight(pvtFlash::COX) == 0.0);
#endif

   if (requiresPVT ())
      computePVT ();

   getCrestColumn()->resetCompositionState();
}

/// this function moves the content of this trap to the crest of the trap it merges with and
/// resets the target column of its own crest column to the crest column of the trap it merges with
void Trap::migrateTo (Column * column)
{
   moveBackToCrestColumn ();
   column->addComposition (getCrestColumn ()->getComposition ());
   m_reservoir->reportTrapAbsorption (this, column, getCrestColumn ()->getComposition ());
   getCrestColumn ()->resetComposition ();
}

double Trap::biodegradeCharges(const double& timeInterval, const Biodegrade& biodegrade)
{
   if (requiresPVT ())
      computePVT ();

   int phase = LAST_PHASE;
   if (m_toBeDistributed[phase].isEmpty()) 
   {
      phase = FIRST_PHASE;
      if (m_toBeDistributed[phase].isEmpty())
         return 0;
   }

   double biodegraded = biodegradeCharges(timeInterval, biodegrade, (PhaseId)phase);

#ifdef DETAILED_MASS_BALANCE
   m_massBalance->subtractFromBalance("biodegraded", biodegraded);
#endif

#ifdef DETAILED_VOLUME_BALANCE
   m_volumeBalance->subtractFromBalance("biodegraded", biodegraded);
#endif

   return biodegraded;
}

/// If depths contains a vector of formations starting with the formation containing 
/// this trap, return iterators pointing to the formations which constitute the 
/// overburden of this trap:
void Trap::iterateToFirstOverburdenFormation(const vector<FormationSurfaceGridMaps>& depths, 
   vector<FormationSurfaceGridMaps>::const_iterator& begin, 
   vector<FormationSurfaceGridMaps>::const_iterator& end) const
{
   // Determine first whether the seal of the trap lies inside this formation, or 
   // whether the seal is formed by the next formation.  In the last case, the first 
   // is the second formation of depths etc:
   begin = depths.begin(); end = depths.end();
   if (getCrestColumn()->getTopDepthOffset() == 0.0) 
      ++begin;
   assert(begin != end);
}

bool Trap::computeDiffusionOverburden(const SurfaceGridMapContainer& fullOverburden,
   const Snapshot* snapshot, const double& maxSealThickness, int maxFormations)
{
   delete m_diffusionOverburdenProps;
   m_diffusionOverburdenProps = 0;

   bool sealPresent;
   double sealFluidDensity;
   if (!computeSealFluidDensity(fullOverburden, snapshot, sealPresent, sealFluidDensity))
      return false;

   // If there is no seal, there is no diffusion leakage, so leave m_diffusionOverburdenProps 0:
   if (!sealPresent)
      return true;

   // Below we copy the property information of the relevant formations to 
   // m_diffusionOverburdenProps. Clear first the old information in m_diffusionOverburdenProps:
   vector<DiffusionLeak::OverburdenProp> diffusionOverburdenProps;

   if (!computeDiffusionOverburdenImpl(fullOverburden, snapshot, maxSealThickness, maxFormations,
      diffusionOverburdenProps))
      return false;

   m_diffusionOverburdenProps = new DiffusionOverburdenProperties(diffusionOverburdenProps, 
      sealFluidDensity);
   return true;
}

bool Trap::computeSealFluidDensity(const SurfaceGridMapContainer& fullOverburden, const Snapshot* snapshot,
   bool& sealPresent, double& sealFluidDensity) const
{
   // Get the overburden formation depths:
   const vector<FormationSurfaceGridMaps>& depths = fullOverburden.discontinuous(SurfaceGridMapContainer::DISCONTINUOUS_DEPTH);
   vector<FormationSurfaceGridMaps>::const_iterator begin;
   vector<FormationSurfaceGridMaps>::const_iterator end;
   iterateToFirstOverburdenFormation(depths, begin, end);

   // Get the first overburden formation which does exist, i.e. for which the thickness is larger 
   // than zero:
   unsigned int i = getCrestColumn()->getI();
   unsigned int j = getCrestColumn()->getJ();
   vector<const Formation*> formations;
   if (!overburden_MPI::getRelevantOverburdenFormations(begin, end, snapshot, 
      i,j, numeric_limits<double>::max(), 1, true, formations))
      return false;

   sealPresent = formations.size() != 0;
   if (!sealPresent) {
      sealFluidDensity = 0.0;
      return true;
   }

   const Interface::FluidType* parameters = formations[0]->getFluidType ();
   assert(parameters);
   sealFluidDensity = CBMGenerics::waterDensity::compute(parameters->fluidDensityModel(), 
      parameters->density(), parameters->salinity(), getTemperature(), getPressure());
   return true;
}

bool Trap::computeDiffusionOverburdenImpl(const SurfaceGridMapContainer& fullOverburden,
   const Snapshot* snapshot, const double& maxSealThickness, int maxFormations,
   vector<DiffusionLeak::OverburdenProp>& diffusionOverburdenProps) const
{
   const SurfaceGridMapContainer::discontinuous_properties& depths = fullOverburden.discontinuous(
      SurfaceGridMapContainer::DISCONTINUOUS_DEPTH);
   const SurfaceGridMapContainer::continuous_properties& temperatures = fullOverburden.continuous(
      SurfaceGridMapContainer::CONTINUOUS_TEMPERATURE);
   const SurfaceGridMapContainer::discontinuous_properties& porosities = fullOverburden.discontinuous(
      SurfaceGridMapContainer::DISCONTINUOUS_POROSITY);

   // There must be one extra element in temperatures in comparison to depths and porosities:
   assert(temperatures.size() == porosities.size()+1);
   assert(temperatures.size() == depths.size()+1);

   // Get the overburden formation depths:
   vector<FormationSurfaceGridMaps>::const_iterator begin;
   vector<FormationSurfaceGridMaps>::const_iterator end;
   iterateToFirstOverburdenFormation(depths, begin, end);

   // Get the formations which do exist, i.e. for which the thickness is larger than zero, 
   // and which are within maxSealThickness and maxFormations:
   unsigned int i = getCrestColumn()->getI();
   unsigned int j = getCrestColumn()->getJ();
   vector<const Formation*> formations;
   if (!overburden_MPI::getRelevantOverburdenFormations(begin, depths.end(), snapshot, 
      i,j, maxSealThickness, maxFormations, true, formations))
      return false;

   // In case the trap lies inside the formation, the thickness, porosity and base temperature 
   // are not provided by that of the overburden formation, but by the values from the Trap itelf.
   // The reason is that if we would get the values from the overburden formation, we would 
   // the values from the base of the formation instead of the top of the trap:
   double baseDepth = getTopDepth();
   double basePorosity = getCrestColumn()->getPorosity();
   double baseTemperature = getTemperature();

   vector<const Formation*>::const_iterator f = formations.begin();   
   SurfaceGridMapContainer::discontinuous_properties::const_iterator d = depths.begin();
   SurfaceGridMapContainer::continuous_properties::const_iterator t = temperatures.begin();
   SurfaceGridMapContainer::discontinuous_properties::const_iterator p =  porosities.begin();

   // There must be a minimum of 2 entries in fullOverburden.continuous(CONTINUOUS_TEMPERATURE):
   assert(t != temperatures.end());

   // Iterate over the included formations and the property GridMaps and copy the 
   // information for each formation within formations:
   ++t;
   while (f != formations.end())
   {
      assert(d != depths.end());
      assert(t != temperatures.end());
      assert(p != porosities.end());

      // The formations of d, p and t should match:
      assert((*d).formation() == (*t).base());
      assert((*d).formation() == (*p).formation());
  
      // Check whether the formation of d, t and p is included:
      if ((*d).formation() == *f) {

         if (!(*d).base().valid()) {
	    cerr << "Trap.C:1369: Exiting as no valid depth property found for base of overburden formation: '" << 
	       (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	    cerr.flush();

            return false;
	 }

         double thickness =  (*d).base()[functions::tuple(i,j)] - (*d).top()[functions::tuple(i,j)];

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
         assert(thickness > 0.0);

         if (!(*t).valid()) {
	    cerr << "Trap.C:1404: Exiting as no valid temperature property found for base of overburden formation: '" << 
	       (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	    cerr.flush();

            return false;
	 }
         double topTemperature = (*t)[functions::tuple(i,j)];

         if (!(*p).top().valid()) {
	    cerr << "Trap.C:1413: Exiting as no valid permeability property found for base of overburden formation: '" << 
	       (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	    cerr.flush();

            return false;
	 }
         double topPorosity = (*p).top()[functions::tuple(i,j)];

         diffusionOverburdenProps.push_back( DiffusionLeak::OverburdenProp(thickness, 
	    topPorosity, basePorosity, topTemperature, baseTemperature) );

         // Goto the next included formation:
         ++f;
      }

      if (!(*d).base().valid()) {
	 cerr << "Trap.C:1429: Exiting as no valid depth property found for base of overburden formation: '" << 
	    (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	 cerr.flush();

	 return false;
      }
      baseDepth = (*d).base()[functions::tuple(i,j)];

      if (!(*p).top().valid()) {
	 cerr << "Trap.C:1438: Exiting as no valid permeability property found for top of overburden formation: '" << 
	    (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	 cerr.flush();

	 return false;
      }

      basePorosity = (*p).top()[functions::tuple(i,j)];

      if (!(*t).valid()) {
	 cerr << "Trap.C:1448: Exiting as no valid temperature property found for top of overburden formation: '" << 
	    (*f)->getName() << "' at time: " << snapshot->getTime() << "." << endl;
	 cerr.flush();

	 return false;
      }
      baseTemperature = (*t)[functions::tuple(i,j)];

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
void Trap::diffusionLeakCharges(const double& intervalStartTime, const double & intervalEndTime, const Interface::DiffusionLeakageParameters* 
   parameters, const double& maxTimeStep, const double& maxFluxError)
{
   // If there are no m_diffusionOverburdenProps, there wasn't a overburden.  (Diffusion directly 
   // to the air is leaking, not diffusion.).  In that case do nothing:
   if (!m_diffusionOverburdenProps)
      return;

   int c;
   double totalWeight = 0;

   int size = (int)parameters->concentrationConsts().size();
   size = DiffusionComponentSize;
   for (c = 0; c < size; ++c)
   {
      totalWeight += getWeight (GAS, (ComponentId) c);
   }

   if (totalWeight < 1)
   {
      setDiffusionStartTime (-1);
      return;
   }

// #define DIFFUSIONDEBUG
#ifdef DIFFUSIONDEBUG
   cerr << "Diffusing " << * this << endl;
#endif
// #undef DIFFUSIONDEBUG

   double diffusionStartTime = getCrestColumn ()->getDiffusionStartTime ();
   if (diffusionStartTime < 0)
   {
      diffusionStartTime = intervalStartTime;
   }
   setDiffusionStartTime (diffusionStartTime);

   // Calculate the solubility of methane, i.e. C1:
   double methaneSolubilityPerKgH2O = CBMGenerics::methaneSolubility::compute(
      parameters->salinity(), getTemperature(), getPressure());
   assert(methaneSolubilityPerKgH2O >= 0.0);

   if (methaneSolubilityPerKgH2O == 0.0)
      return;

   // The number of concentrationConsts should be the same as the number of diffusionConsts:
   assert((int)parameters->diffusionConsts().size() >= size);

   double methaneSolubilityPerM3 = methaneSolubilityPerKgH2O * m_diffusionOverburdenProps->
      sealFluidDensity();

   vector<double> solubilities( parameters->concentrationConsts() );

   for (c = 0; c < size; ++c)
      solubilities[c] *= methaneSolubilityPerM3;

   vector<DiffusionLeak*> diffusionLeaks;
   diffusionLeaks.reserve(size);

   for (c = 0; c < size; ++c)
   {
      DiffusionCoefficient coefficient( parameters->diffusionConsts()[c], parameters->activationEnergy() );

      double penetrationDistance = parameters->transientModel() == Interface::Transient ? 
	 getCrestColumn ()->getPenetrationDistance ((ComponentId) c) : parameters->maximumSealThickness();

      DiffusionLeak*  diffusionLeak = new DiffusionLeak( m_diffusionOverburdenProps->properties(), 
         m_diffusionOverburdenProps->sealFluidDensity(), penetrationDistance, 
         parameters->maximumSealThickness(), coefficient, maxTimeStep, maxFluxError);

      /// debugging!!!
      if (c == 0 && penetrationDistance > diffusionLeak->penetrationDistance ())
      {
	 cerr << this << ":limiting " << penetrationDistance << " => " << diffusionLeak->penetrationDistance () << endl;
      }

      diffusionLeaks.push_back( diffusionLeak );
   }
   assert( diffusionLeaks.size() <= solubilities.size());

   m_distributed[GAS].computeDiffusionLeakages(diffusionStartTime, intervalStartTime, intervalEndTime, solubilities, getSurface(GAS), diffusionLeaks, 
      computeGorm(m_distributed[GAS], m_distributed[OIL]), &m_distributed[GAS], &m_diffusionLeaked[GAS]);

   if (parameters->transientModel() == Interface::Transient)
   {
      for (c = 0; c < size; ++c)
      {
	 setPenetrationDistance ((ComponentId) c, diffusionLeaks[c]->penetrationDistance ());
	 delete diffusionLeaks[c];
      }
   }

   m_reservoir->reportDiffusionLoss (this, m_diffusionLeaked[GAS]);

#ifdef DETAILED_MASS_BALANCE
   m_massBalance->subtractFromBalance("diffusion leaked", m_diffusionLeaked[GAS].getWeight());
#endif

#ifdef DETAILED_MASS_BALANCE
   m_volumeBalance->subtractFromBalance("diffusion leaked", m_diffusionLeaked[GAS].getVolume());
#endif
}

bool Trap::computeDistributionParameters(const Interface::FracturePressureFunctionParameters* 
   parameters, const SurfaceGridMapContainer& fullOverburden, const Snapshot* snapshot)
{
   delete m_distributor;
   m_distributor = 0;

   // Calculate the following overburden parameters:
   bool sealPresent;
   double fracPressure; 
   double sealFluidDensity;
   vector<translateProps::CreateCapillaryLithoProp::output> lithProps;
   vector<double> lithFracs;
   CBMGenerics::capillarySealStrength::MixModel mixModel;
   double permeability;

   if (!computeSealPressureLeakParametersImpl(parameters, fullOverburden, snapshot, sealPresent, 
      fracPressure, sealFluidDensity, lithProps, lithFracs, mixModel, permeability))
      return false;

   setFracturePressure (fracPressure);
   setSealPermeability (permeability);

   if (sealPresent)
   {
      if (!isUndersized ())
      {
         // The fracture seal strength is provided by the fracturePressure minus the pressure:
	 double porePressure = getPressure();
         double fracSealStrength = fracPressure - porePressure / Pa2MPa;
#if 0
	 cerr << "trap = " << this << endl;
	 cerr << "porePressure = " << porePressure / Pa2MPa << endl;
	 cerr << "fracPressure = " << fracPressure << endl;
	 cerr << "fracSealStrength = " << fracSealStrength << endl;
#endif
	 if (fracSealStrength < 0) fracSealStrength = 0;

#ifdef DEBUG
	 if (GetRank () == 0)
	 {
	    cerr << "fracPressure = " << fracPressure <<
	       " Pa, pressure = " << getPressure () * MPa2Pa <<
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
	 m_distributor = new LeakWasteAndSpillDistributor(sealFluidDensity, fracSealStrength, 
            CapillarySealStrength(lithProps, lithFracs, mixModel, permeability, sealFluidDensity), 
	    m_levelToVolume);
      }
      else {

#ifdef DETAILED_MASS_BALANCE
         m_massBalance->addComment("Trap undersized!");
#endif

#ifdef DETAILED_VOLUME_BALANCE
         m_volumeBalance->addComment("Trap undersized!");
#endif

	 m_distributor = new SpillAllGasAndOilDistributor(m_levelToVolume);
      }
   }
   else {

#ifdef DETAILED_MASS_BALANCE
         m_massBalance->addComment("No seal present!");
#endif

#ifdef DETAILED_VOLUME_BALANCE
         m_volumeBalance->addComment("No seal present!");
#endif

      m_distributor = new LeakAllGasAndOilDistributor(m_levelToVolume);
   }

   return true;
}

bool Trap::computeSealPressureLeakParametersImpl(const Interface::FracturePressureFunctionParameters* 
   fracturePressureParameters, const SurfaceGridMapContainer& fullOverburden, const Snapshot* snapshot,
   bool& sealPresent, double& fracPressure, double& sealFluidDensity, vector<translateProps::
   CreateCapillaryLithoProp::output>& lithProps, vector<double>& lithFracs, CBMGenerics::capillarySealStrength::
   MixModel& mixModel, double& permeability) const
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
   if (!overburden_MPI::getRelevantOverburdenFormations (begin, depths.end (), snapshot,
	    i, j, numeric_limits < double >::max (), 1, true, formations))
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
   while (d != depths.end ())
   {
      assert (d != depths.end ());
      assert (p != permeabilities.end ());
      assert (l0 != lithoType1Percents.end ());
      // l1 and l2 may be empty, so no asserts for l1 and l2.

      // The formations of d, p and l0 should match:
      assert ((*d).formation () == (*p).formation ());
      assert ((*d).formation () == (*l0).first);

      // Check whether the formation of d, p and l0 is the seal formation:
      if ((*d).formation () == *f)
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
	 assert (!(*d).top ().valid () || getTopDepth () - (*d).top ()[functions::tuple (i, j)] > 0.0);

	 if (!(*d).base ().valid ())
	 {
	    cerr << "Trap.C:1650: Exiting as no valid depth property found for base of formation: '" <<
	       (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
	    cerr.flush ();

	    return false;
	 }
	 assert ((*d).base ()[functions::tuple (i, j)] - (*d).top ()[functions::tuple (i, j)] > 0.0);

	 // The right formation is found:
	 break;
      }

      // If the formation of l1 and l2 matches that of d, go to the next l1 and l2:
      if (l1 != lithoType2Percents.end () && (*l1).first == (*d).formation ())
	 ++l1;
      if (l2 != lithoType3Percents.end () && (*l2).first == (*d).formation ())
	 ++l2;
      ++d;
      ++p;
      ++l0;
   }
   assert (d != depths.end ());

   assert ((*f)->getLithoType1 ());
   assert (l0 != lithoType1Percents.end ());
   assert (l0->first == *f);

   // So we have found the right d, p, l0, and possibly l1 and l2 iterators if they 
   // do exist.  Get the fractions of the LithoTypes:
   lithFracs.push_back (0.01 * (*l0).second[functions::tuple (i, j)]);
   if ((*f)->getLithoType2 () && l1 != lithoType2Percents.end () && (*l1).first == (*f))
      lithFracs.push_back (0.01 * (*l1).second[functions::tuple (i, j)]);
   if ((*f)->getLithoType3 () && l2 != lithoType3Percents.end () && (*l2).first == (*f))
      lithFracs.push_back (0.01 * (*l2).second[functions::tuple (i, j)]);

   // For the capillary entry pressure, we need to know what mixing model to apply when calculating 
   // the effective capillary parameters:
   mixModel = (*f)->getMixModel ();

   // If the trap lies inside the formation the relevant permeability is the permeability 
   // of the trap, else the permeability is the permeability of the base of the overburden 
   // formation:
   if (getCrestColumn ()->getTopDepthOffset () != 0.0)
      permeability = getCrestColumn ()->getPermeability ();
   else
   {
      if (!(*p).base ().valid ())
      {
	 cerr << "Trap.C:1694: Exiting as no valid permeability property found for base of formation: '" <<
	    (*f)->getName () << "' at time: " << snapshot->getTime () << "." << endl;
	 cerr.flush ();

	 return false;
      }
      else
	 permeability = (*p).base ()[functions::tuple (i, j)];
   }

   // But as depth and temperatureC are of course continuous properties, we may get them from 
   // the Trap:
   double depth = getTopDepth ();
   double depthWrtSedimentSurface = getCrestColumn ()->getOverburden ();
   double temperatureC = getTemperature ();

   // Get the water density:
   const Interface::FluidType * fluidType = (*f)->getFluidType ();

   sealFluidDensity = CBMGenerics::waterDensity::compute (fluidType->fluidDensityModel (),
	 fluidType->density (), fluidType->salinity (),
	 temperatureC, getPressure ());

   // Finally use the gathered information to calculate the capillary seal strength of gas and oil:
   translateProps::translate < translateProps::CreateCapillaryLithoProp > (*f,
	 translateProps::CreateCapillaryLithoProp (),
	 lithProps);

   // Compute the fracture pressure:
   fracPressure = numeric_limits < double >::max ();
   bool result = true;

   switch (fracturePressureParameters->type ())
   {
      case Interface::None:
	 break;
      case Interface::FunctionOfDepthWrtSeaLevelSurface:
	 fracPressure =
	    CBMGenerics::fracturePressure::computeForFunctionOfDepthWrtSeaLevelSurface (fracturePressureParameters->
		  coefficients (), depth);
	 break;
      case Interface::FunctionOfDepthWrtSedimentSurface:
	 {
	    fracPressure =
	       CBMGenerics::fracturePressure::computeForFunctionOfDepthWrtSedimentSurface (fracturePressureParameters->coefficients (),
		     depthWrtSedimentSurface,
		     getCrestColumn ()->getSeaBottomPressure ());
	    break;
	 }
      case Interface::FunctionOfLithostaticPressure:
	 result = computeForFunctionOfLithostaticPressure (fullOverburden, *f, lithFracs, fracPressure);
	 break;
      default:
	 assert (0);
   }
   return result;
}

bool Trap::computeForFunctionOfLithostaticPressure(const SurfaceGridMapContainer& fullOverburden,
      const Formation* formation, const vector<double>& lithFracs, double& fracPressure) const
{
   vector<double> lithHydraulicFracturingFracs;
   translateProps::translate<translateProps::CreateLithHydraulicFracturingFrac>(formation, 
      translateProps::CreateLithHydraulicFracturingFrac(), lithHydraulicFracturingFracs);
   double hydraulicFracture = CBMGenerics::fracturePressure::hydraulicFracturingFrac(
      lithHydraulicFracturingFracs, lithFracs);

   fracPressure = CBMGenerics::fracturePressure::computeForFunctionOfLithostaticPressure(hydraulicFracture, 
      getLithostaticPressure (), getHydrostaticPressure ());
   return true;
}

bool Trap::distributeCharges (void)
{
#ifdef MOVEDTODISTRIBUTECHARGES
   collectAndSplitCharges ();
#endif

   // requiresDistribution () won't work as it also looks at the crest column,
   // which is not required/advisable as it is not used further on. 
   // Also, Anton's distributor does not handle the case where there is nothing toBeDistributed very well.

   bool distributionNeeded = false;
   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      if (!m_toBeDistributed[phase].isEmpty ()) distributionNeeded = true;
   }

   if (!distributionNeeded) return true;

   incrementChargeDistributionCount ();

   assert(m_distributor);

#if 0
   cerrstrstr << GetRankString () << ": " << this << "::distributeCharges ("
      << m_toBeDistributed[GAS].getWeight () << ", " << m_toBeDistributed[OIL].getWeight () << ")" << endl;
#endif

   // Check if wasting is possible:
   if (getWasteDepth(GAS) != WasteDepth) {
      double wasteLevel = getWasteDepth(GAS) - getTopDepth();
      if (wasteLevel < m_levelToVolume->invert(numeric_limits<double>::max())) {
	 m_distributor->setWasteLevel(wasteLevel);
	 m_distributor->setWasting(true);
      }
   }

   // Check is leaking is necessary:
   m_distributor->setLeaking(!isUndersized());

   Composition gasLeaked;
   Composition gasWasted;
   Composition gasSpilled;
   Composition oilLeaked;
   Composition oilSpilledOrWasted;

   double finalGasLevel;
   double finalHCLevel;

   m_distributor->distribute(m_toBeDistributed[GAS], m_toBeDistributed[OIL], 
      getTemperature() + C2K, m_distributed[GAS], m_distributed[OIL], gasLeaked, gasWasted, 
      gasSpilled, oilLeaked, oilSpilledOrWasted, finalGasLevel, finalHCLevel);

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

    
   if (!gasLeaked.isEmpty() || !oilLeaked.isEmpty()) {
      gasLeaked.add(oilLeaked);
      getCrestColumn()->addLeakComposition (gasLeaked);
   }
      
   if (!gasWasted.isEmpty())
   {
      getWasteColumn(GAS)->addWasteComposition(gasWasted);
      m_reservoir->reportWaste (this, getWasteColumn (GAS), gasWasted);
   }

   if (!oilSpilledOrWasted.isEmpty()) {
      if (getWasteColumn (OIL))
      {
	 getWasteColumn (OIL)->addWasteComposition (oilSpilledOrWasted);
	 m_reservoir->reportWaste (this, getWasteColumn (OIL), oilSpilledOrWasted);
      }
      else
      {
         Column* targetColumn = getFinalSpillTarget (OIL);

         targetColumn->addSpillComposition(oilSpilledOrWasted);
         m_reservoir->reportSpill (this, targetColumn, oilSpilledOrWasted);
         getSpillTarget ()->addMigrated (OIL, oilSpilledOrWasted.getWeight ());
         setSpilling();
      }
   }

   if (!gasSpilled.isEmpty()) {
      Column* targetColumn = getFinalSpillTarget (GAS);

      targetColumn->addSpillComposition(gasSpilled);
      m_reservoir->reportSpill (this, targetColumn, gasSpilled);
      getSpillTarget ()->addMigrated (GAS, gasSpilled.getWeight ());
      setSpilling();
   }

   // can be removed???
   // negotiateDensity(GAS);
   // negotiateDensity(OIL);

   m_toBeDistributed[GAS].reset();
   m_toBeDistributed[OIL].reset();

   if (finalGasLevel + getTopDepth() > getSpillDepth () ||
         finalHCLevel + getTopDepth() > getSpillDepth ())
   {
      cerr << GetRankString () << ": " << this << ": finalGasDepth = " << getTopDepth () + finalGasLevel
         << ", finalHCDepth = " << getTopDepth () + finalHCLevel
         << ", spill depth = " << getSpillDepth () << endl;
   }

   setFillDepth(GAS, finalGasLevel + getTopDepth());
   setFillDepth(OIL, finalHCLevel + getTopDepth());

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
      if (getCrestColumn ()->isSpillingBack ((PhaseId) phase))
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
      ((getCrestColumn()->getCompositionState() & (INITIAL + SPILLED)) != 0))
      result = true;

   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      if (!m_toBeDistributed[phase].isEmpty ()) result = true;
   }

   return result;
}

void Trap::checkDistributedCharges (PhaseId phase)
{
   const double MinimumVolume = 5000;
   const double MaximumCapacityError = 0.001;

   double chargeVolume = m_distributed[phase].getVolume ();
   double usedTrapVolume = getVolumeBetweenDepths (getTopDepth (), getFillDepth (phase));
   if (phase != FIRST_PHASE)
   {
      usedTrapVolume -= getVolumeBetweenDepths (getTopDepth (), getFillDepth (PhaseId (int (phase) - 1)));
   }

   if (chargeVolume > MinimumVolume)
   {
      double capacityError = Abs (chargeVolume - usedTrapVolume) / getCapacity ();

      if (capacityError > MaximumCapacityError)
      {
         cerr << GetRankString () << ": ERROR in phase " << PhaseNames[phase] << " of " << this << ":" << endl
            << "\t" << "charge volume = " << chargeVolume
            << ", used volume = " << usedTrapVolume << ", capacity = " << getCapacity () << endl;
      }
   }
}

double Trap::computeHydrocarbonWaterContactDepth (void) const
{
   double volumeOil = m_toBeDistributed[OIL].getVolume();
   double volumeGas = m_toBeDistributed[GAS].getVolume();

   double maximumCapacityOfTrap = getVolumeBetweenDepths2(getTopDepth(), getBottomDepth());
   double hydrocarbonWaterContactDepth = -199999;

   if ((volumeGas + volumeOil) < maximumCapacityOfTrap)
   {
      hydrocarbonWaterContactDepth = (m_levelToVolume->invert(volumeGas + volumeOil));
      hydrocarbonWaterContactDepth += getTopDepth();
   }
   else
   {
      hydrocarbonWaterContactDepth = getBottomDepth();
   }
      
   assert((hydrocarbonWaterContactDepth >= getTopDepth()) && (hydrocarbonWaterContactDepth <= getBottomDepth()));

   return hydrocarbonWaterContactDepth;
}

double Trap::computeFractionVolumeBiodegraded (const double& timeInterval)
{
   double volumeOil = m_toBeDistributed[OIL].getVolume();
   double volumeGas = m_toBeDistributed[GAS].getVolume();

   // Arbitrary value which states the thickness of the biodegradation impact above the OWC
   double inputThicknessAffectedByBiodegradationAboveOWC = 3;   // Original value entered by the user or by default: 3m/10Myr
   double thicknessAffectedByBiodegradationAboveOWC = inputThicknessAffectedByBiodegradationAboveOWC * timeInterval / 10;  // ~3m/10Myr, but depend on timeInterval

   double maximumCapacityOfTrap = getVolumeBetweenDepths2(getTopDepth(), getBottomDepth());
   double volumeBiodegraded = min(getVolumeBetweenDepths2(getFillDepth(OIL) - thicknessAffectedByBiodegradationAboveOWC, getFillDepth(OIL)), (volumeGas + volumeOil));

   assert(volumeBiodegraded >= 0.0);
   assert(volumeBiodegraded <= maximumCapacityOfTrap && volumeBiodegraded <= (volumeGas + volumeOil));

   double fractionVolumeBiodegraded = volumeBiodegraded / (volumeGas + volumeOil);
   assert(fractionVolumeBiodegraded >= 0.0 && fractionVolumeBiodegraded <= 1.0);

#ifdef DEBUG_BIODEGRADATION
   std::cerr << endl << "==== Compute the fraction of volume which is biodegraded ====" << endl;
   std::cerr << "Volume of OIL in the Trap: " << volumeOil << " ; Volume of GAS in the Trap: " << volumeGas << endl;
   std::cerr << "Volume total of HC in the Trap: " << volumeOil + volumeGas << endl;
   std::cerr << "Top depth of the trap (crest column): " << getTopDepth() << endl;
   std::cerr << "Hydrocarbon - water contact depth: " << getFillDepth(OIL) << endl;
   std::cerr << "Bottom depth of the trap (spilling Point): " << getBottomDepth() << endl;
   std::cerr << "Maximum capacity of the trap: " << getVolumeBetweenDepths2(getTopDepth(), getBottomDepth()) << endl;
   std::cerr << "Volume impacted by biodegradation in the trap: " << volumeBiodegraded << endl;
   std::cerr << "Fraction of the volume impacted by biodegradation: " << fractionVolumeBiodegraded * 100 << "% " << endl;
#endif

   assert(getTopDepth() < getBottomDepth());
   return fractionVolumeBiodegraded;
}

double Trap::computeHydrocarbonWaterContactTemperature()
{
   // Obtain the depths at the top and bottom of the crest column
   double topCrestColumnDepth = getCrestColumn()->getTopDepth();
   double bottomCrestColumnDepth = getCrestColumn()->getBottomDepth();

   // Obtain a gridMap of temperature thanks to the DerivedProperties
   const DataAccess::Interface::Property* property = getReservoir()->getProjectHandle()->findProperty("Temperature");
   Migrator* mig = dynamic_cast<migration::Migrator*>(getReservoir()->getProjectHandle());
   DerivedProperties::FormationPropertyPtr gridMap = mig->getPropertyManager().getFormationProperty(property, getReservoir()->getEnd(), getReservoir()->getFormation());

   // Initialisation of the hydrocarbon - water contact temperature at the temperature at the top of crest column of the trap
   double hydrocarbonWaterContactTemperature = getCrestColumn()->getTemperature();

   if (gridMap == 0) // No gridMap, then we use the temperature of the crest column for biodegradation
   {
      std::cerr << "The temperature at the hydrocarbon - water contact can't be computed for the trapID " << getGlobalId() 
                << " of the reservoir " << getReservoir()->getName()
                << ". The temperature at the crest of the trap has been selected instead." << endl;
      return hydrocarbonWaterContactTemperature;
   }

   unsigned int depth = gridMap->lengthK();
   assert(depth > 1);
   gridMap->retrieveData();
   
   // If the hydrocarbon - water contact is included between the top and the bottom of the crest column
   // The interpolation of temperature will be done only on the crest column
   if (getFillDepth(OIL) <= bottomCrestColumnDepth)
   {
      // Transform the depth of the hydrocarbon - water contact in a node position of the crest column
      double percentageHeightHydrocarbonWaterContact = (bottomCrestColumnDepth - getFillDepth(OIL)) / (bottomCrestColumnDepth - topCrestColumnDepth);
      double nodeHydrocarbonWaterContact = depth * percentageHeightHydrocarbonWaterContact;
      
      LocalColumn * column = getReservoir()->getLocalColumn(getCrestColumn()->getI(), getCrestColumn()->getJ());
      double index = (nodeHydrocarbonWaterContact - 1) - column->getTopDepthOffset() * (nodeHydrocarbonWaterContact - 1);
      index = Max((double)0, index);
      index = Min((double)depth - 1, index);
      hydrocarbonWaterContactTemperature = (gridMap->interpolate(getCrestColumn()->getI(), getCrestColumn()->getJ(), index));

   }
   // Else we need to find another column which has a top and a bottom depth which surrounded the OWC 
   else
   {
      // Find a column of the trap with a bottom depth deeper than the hydrocarbon - water contact
      // and a top depth shallower than the hydrocarbon - water contact
      ConstColumnIterator iter;
      for (iter = m_interior.begin(); iter != m_interior.end(); ++iter)
      {
         const LocalColumn * column = dynamic_cast<LocalColumn *> (*iter);

         if (getFillDepth(OIL) <= column->getBottomDepth() && getFillDepth(OIL) >= column->getTopDepth())
         {
            // Compute the temperature at the top of the crest column
            double topColumnDepth = column->getTopDepth();
            double bottomColumnDepth = column->getBottomDepth();           
           
            // Transform the depth of the hydrocarbon - water contact in a node position of the crest column
            double percentageHeightHydrocarbonWaterContact = (bottomColumnDepth - getFillDepth(OIL)) / (bottomColumnDepth - topColumnDepth);
            double nodeHydrocarbonWaterContact = depth * percentageHeightHydrocarbonWaterContact;

            double index = (nodeHydrocarbonWaterContact - 1) - column->getTopDepthOffset() * (nodeHydrocarbonWaterContact - 1);
            index = Max((double)0, index);
            index = Min((double)depth - 1, index);
            hydrocarbonWaterContactTemperature = (gridMap->interpolate(column->getI(), column->getJ(), index));
            
            break;
         }
      }

   }

#ifdef DEBUG_BIODEGRADATION
   std::cerr << endl << "==== Compute the temperature at the hydrocarbon - water contact ====" << endl;
   std::cerr << "Temperature of the crest column: " << getCrestColumn()->getTemperature() << endl;
   std::cerr << "Temperature at the hydrocarbon - Water contact: " << hydrocarbonWaterContactTemperature << endl;
#endif
   
   return hydrocarbonWaterContactTemperature;
}

double Trap::biodegradeCharges (const double& timeInterval, const Biodegrade& biodegrade, PhaseId phase)
{
   Composition biodegraded;

   assert(!m_toBeDistributed[phase].isEmpty ());
   
   double hydrocarbonWaterContactDepth = computeHydrocarbonWaterContactDepth();
   setFillDepth(OIL, hydrocarbonWaterContactDepth);
   double fractionVolumeBiodegraded = computeFractionVolumeBiodegraded(timeInterval);

   double hydrocarbonWaterContactTemperature = computeHydrocarbonWaterContactTemperature();
      
    m_toBeDistributed[phase].computeBiodegradation(timeInterval, hydrocarbonWaterContactTemperature, biodegrade,
      biodegraded, fractionVolumeBiodegraded);
   
   m_toBeDistributed[phase].subtract(biodegraded);

   m_reservoir->reportBiodegradationLoss (this, biodegraded);

   return biodegraded.getWeight ();
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
      Column * column = * iter;

      total += column->getVolumeBetweenDepths (column->getFillDepth (phase), column->getFillDepth (phase) + column->getFillHeight (phase));
   }

   return total;
}

void Trap::printSpillTrajectory (PhaseId phase)
{
   Column * targetColumn = getFinalSpillTarget (phase);

   cerr << GetRankString () << ": " << m_reservoir->getName () << "->" << this 
      << "->spills to " << targetColumn;

   Column * tmpColumn = getSpillTarget ();
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
      (* iter)->setDiffusionStartTime (diffusionStartTime);
   }
}

void Trap::broadcastPenetrationDistances (void)
{
   const double * trapPenetrationDistances = getPenetrationDistances ();
   const double * columnPenetrationDistances = getCrestColumn ()->getPenetrationDistances ();

   if (std::equal (columnPenetrationDistances, columnPenetrationDistances + DiffusionComponentSize, trapPenetrationDistances))
   {
      return; // no reason for broadcasting
   }

   for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
   {
      (* iter)->setPenetrationDistances (trapPenetrationDistances);
   }
}

void Trap::broadcastFillDepthProperties (void)
{
   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      double fillDepth = getFillDepth ((PhaseId) phase);

      if (fillDepth > getSpillDepth ())
      {
         cerr << "Error in " << this << ": fillDepth (" << fillDepth << ") for phase " << PhaseNames[phase] << " is larger than spillDepth (" << getSpillDepth () << ")" << endl;
      }

      for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = * iter;

         column->setFillDepth ((PhaseId) phase, fillDepth);
      }
   }
}

void Trap::broadcastChargeProperties (void)
{
   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      for (ConstColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = * iter;

         column->setChargesToBeMigrated ((PhaseId) phase, m_distributed[phase]);
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
      setFillDepth (PhaseId (phase), getFillDepth (PhaseId (phase - 1)));
   }
}

/// move all the stuff already in the trap back to the trap's crest column.
/// To be executed if new stuff got spilled to this trap.
void Trap::moveBackToCrestColumn (void)
{
   for (int phase = FIRST_PHASE; phase < NUM_PHASES; ++phase)
   {
      moveBackToCrestColumn (PhaseId (phase));
   }
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
      moveBackToBeMigrated (PhaseId (phase));
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
      cerr << * iter << ", ";
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
      cerr << * iter << ", ";
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
      return stream << & trap;
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
   tpRequest.spillPointI = getSpillColumn()->getI();
   tpRequest.spillPointJ = getSpillColumn()->getJ();
   tpRequest.wcSurface = getSurface (LAST_PHASE);
   tpRequest.temperature = getTemperature ();
   tpRequest.pressure = getPressure ();
   tpRequest.permeability = getPermeability ();
   tpRequest.sealPermeability = getSealPermeability ();
   tpRequest.fracturePressure = getFracturePressure () * Pa2MPa;
   tpRequest.netToGross = getNetToGross () * Fraction2Percentage;
   tpRequest.fractureSealStrength = -1;

   // funny code by nlamol
   LeakWasteAndSpillDistributor* leakDistributor = dynamic_cast<LeakWasteAndSpillDistributor*>(m_distributor);

   tpRequest.fractureSealStrength = -1;
   if (leakDistributor && leakDistributor->fractureSealStrength() != numeric_limits<double>::max())
      tpRequest.fractureSealStrength = Pa2MPa * leakDistributor->fractureSealStrength();

   tpRequest.goc = getFillDepth (GAS);
   tpRequest.owc = getFillDepth (OIL);

   double gorm = computeGorm(m_distributed[GAS], m_distributed[OIL]);

   for (int phase = (int) FIRST_PHASE; phase < (int) NUM_PHASES; ++phase)
   {
      tpRequest.cep[phase] = -1;
      tpRequest.criticalTemperature[phase] = -1;
      tpRequest.interfacialTension[phase] = -1;

      if (leakDistributor && m_distributed[phase].getWeight() > 0.0) {
         double capillarySealStrength = leakDistributor->capillarySealStrength().compute(m_distributed[phase], 
            gorm, getTemperature() + C2K);
         if (capillarySealStrength != numeric_limits<double>::max())
            tpRequest.cep[phase] = Pa2MPa * capillarySealStrength;

         double criticalTemperature = 
            leakDistributor->capillarySealStrength().criticalTemperature(m_distributed[phase], gorm) - C2K;
            tpRequest.criticalTemperature[phase] = criticalTemperature;

         double interfacialTension = leakDistributor->capillarySealStrength().interfacialTension(m_distributed[phase], 
            gorm, getTemperature() + C2K);
            tpRequest.interfacialTension[phase] = interfacialTension;
      }
      tpRequest.volume[phase] = getVolume ((PhaseId) phase);
   }

   for (int component = FIRST_COMPONENT; component < NUM_COMPONENTS; ++component)
   {
      tpRequest.composition.set ((ComponentId) component, getWeight ((ComponentId) component));
   }
}

/// not used
bool Trap::saveProperties (void)
{
   if (isUndersized ())
   {

      for (ColumnIterator iter = m_interior.begin (); iter != m_interior.end (); ++iter)
      {
         Column * column = * iter;
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
