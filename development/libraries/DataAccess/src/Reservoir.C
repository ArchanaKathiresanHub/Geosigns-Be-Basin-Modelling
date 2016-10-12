#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"


using namespace DataAccess;
using namespace Interface;

Reservoir::Reservoir (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_formation = 0;
   m_mangledName = utilities::mangle (getName ());
}

Reservoir::~Reservoir (void)
{
}

const string Reservoir::s_MapAttributeNames[] =
{
   "DepthOffset", "Thickness", "NetToGross", "LayerFrequency"
};

const string & Reservoir::getName (void) const
{
   return database::getReservoirName (m_record);
}

void Reservoir::setName (const string reservoirName)
{
   database::setReservoirName (m_record, reservoirName);
   return;
}

const string & Reservoir::getMangledName (void) const
{
   return m_mangledName;
}

const string & Reservoir::getActivityMode (void) const
{
   return database::getActivityMode (m_record);
}

double Reservoir::getActivityStart (void) const
{
   return database::getActivityStart (m_record);
}


bool Reservoir::isActive (const Interface::Snapshot * snapshot) const
{
   if (getActivityMode () == "NeverActive") return false; // not active anyway

   const Interface::Formation * formation = getFormation ();
   const Interface::Surface * topSurface = formation->getTopSurface();
   const Interface::Snapshot * depoSnapshot = topSurface->getSnapshot();

   if (depoSnapshot->getTime () <= snapshot->getTime ()) return false; // not active anyway

   if (getActivityMode () == "AlwaysActive") return true;

   if (getActivityMode () == "ActiveFrom")
   {
      const Interface::Snapshot * activeFromSnapshot = m_projectHandle->findSnapshot (getActivityStart ());
      return (activeFromSnapshot->getTime () >= snapshot->getTime ());
   }

   return true;
}


const string & Reservoir::getFormationName (void) const
{
   return database::getFormationName (m_record);
}

void Reservoir::setFormation (const Formation * formation)
{
   m_formation = formation;
}

const Formation * Reservoir::getFormation (void) const
{
   return m_formation;
}

double Reservoir::getTrapCapacity (void) const
{
   return database::getTrapCapacity (m_record);
}

bool Reservoir::isBlockingEnabled (void) const
{
   return database::getBlockingInd (m_record) == 1;
}

double Reservoir::getBlockingPermeability (void) const
{
   return database::getBlockingPermeability (m_record);
}

double Reservoir::getBlockingPorosity (void) const
{
   return database::getBlockingPorosity (m_record);
}

bool Reservoir::isDiffusionOn (void) const
{
   return database::getDiffusionInd (m_record) == 1;
}

double Reservoir::getMinOilColumnHeight (void) const
{
   return database::getMinOilColumnHeight (m_record);
}

double Reservoir::getMinGasColumnHeight (void) const
{
   return database::getMinGasColumnHeight (m_record);
}

bool Reservoir::isBioDegradationOn (void) const
{
   return database::getBioDegradInd (m_record) == 1;
}

bool Reservoir::isOilToGasCrackingOn (void) const
{
   return database::getOilToGasCrackingInd (m_record) == 1;
}

/// Return the (GridMap) value of one of this Reservoir's attributes
const GridMap * Reservoir::getMap (ReservoirMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (attributeIndex)) == 0)
   {
      gridMap = loadMap (attributeId);
   }
   return gridMap;
}
 
GridMap * Reservoir::loadMap (ReservoirMapAttributeId attributeId) const
{
   unsigned int attributeIndex = (unsigned int) attributeId;

   string attributeGridName = s_MapAttributeNames[attributeIndex] + "Grid";
   const string & valueGridMapId = m_record->getValue<std::string>(attributeGridName);

   GridMap * gridMap = 0;
   if (valueGridMapId.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("ReservoirIoTbl", valueGridMapId);
   }
   else
   {
      double value;
      if ((value = m_record->getValue<double>(s_MapAttributeNames[attributeIndex])) != RecordValueUndefined)
      {
         //const Grid *grid = m_projectHandle->getInputGrid ();
	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
	 if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();

         gridMap = m_projectHandle->getFactory ()->produceGridMap (this, attributeIndex, grid, value);

         assert (gridMap == getChild (attributeIndex));
      }
   }
   return gridMap;
}

void Reservoir::setTrappersUpAndDownstreamConnected (double time)
{
   m_trappersUpAndDownstreamConnected[time] = true;
}

bool Reservoir::trappersAreUpAndDownstreamConnected (double time) const
{
   return m_trappersUpAndDownstreamConnected.count (time) == 1;
}

void Reservoir::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Reservoir::asString (string & str) const
{
   ostringstream buf;

   buf << "Reservoir:";
   buf << " name = " << getName ();
   buf << ", formation name = " << getFormation ()->getName ();
   buf << endl;

   str = buf.str ();
}
