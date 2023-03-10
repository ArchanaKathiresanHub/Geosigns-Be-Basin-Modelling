#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Formation.h"
#include "GridMap.h"
#include "MapWriter.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "Property.h"
#include "PropertyValue.h"
#include "Reservoir.h"
#include "Snapshot.h"
#include "Surface.h"

using namespace DataAccess;
using namespace Interface;

PropertyValue::PropertyValue (ProjectHandle& projectHandle, Record * record, const string & name, const Property * property, const Snapshot * snapshot,
                              const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage, const std::string & fileName) :
   DAObject (projectHandle, record),
   m_name (name),
   m_property (property), m_snapshot (snapshot),
   m_reservoir (reservoir), m_formation (formation), m_surface (surface),
   m_storage (storage), m_fileName (fileName)
{
}


PropertyValue::~PropertyValue (void)
{
}

bool PropertyValue::matchesConditions (int selectionFlags, const Property * property, const Snapshot * snapshot,
                                       const Reservoir * reservoir, const Formation * formation, const Surface * surface, int propertyType) const
{
   bool selected = false;

   if (property != 0 && getProperty () != property)
      return false;

   if (snapshot != 0 && getSnapshot () != snapshot)
      return false;

   if (getStorage () == SNAPSHOTIOTBL && (propertyType & VOLUME) == 0)
      return false;

   if (getStorage () == THREEDTIMEIOTBL && (propertyType & VOLUME) == 0)
      return false;

   if (getStorage () == TIMEIOTBL && (propertyType & SURFACE) == 0)
      return false;

   if (getProperty ()->getType () == RESERVOIRPROPERTY)
   {
      if ((selectionFlags & RESERVOIR) && (reservoir == 0 || getReservoir () == reservoir))
      {
         selected = true;
      }
   }

   if (getProperty ()->getType () == FORMATIONPROPERTY)
   {
      if ((selectionFlags & SURFACE) && getFormation () == 0 && (surface == 0 || getSurface () == surface))
      {
         selected = true;
      }

      if ((selectionFlags & FORMATION) && getSurface () == 0 &&
          (formation == 0 || getFormation () == formation))
      {
         selected = true;
      }

      if ((selectionFlags & FORMATIONSURFACE) && getSurface () != 0 &&
          (surface == 0 || getSurface () == surface) && (formation == 0 || getFormation () == formation))
      {
         selected = true;
      }
   }

   return selected;
}

const std::string & PropertyValue::getName (void) const
{
   return m_name;
}

const std::string & PropertyValue::getFileName (void) const
{
   return m_fileName;
}

const Snapshot * PropertyValue::getSnapshot (void) const
{
   return m_snapshot;
}

const Property * PropertyValue::getProperty (void) const
{
   return m_property;
}

const Reservoir * PropertyValue::getReservoir (void) const
{
   return m_reservoir;
}

const Formation * PropertyValue::getFormation (void) const
{
   return m_formation;
}

const Surface * PropertyValue::getSurface (void) const
{
   return m_surface;
}

PropertyStorage PropertyValue::getStorage (void) const
{
  return m_storage;
}

bool PropertyValue::toBeSaved () const {
   return true;
}

bool PropertyValue::hasRecord() const {


   if (! getRecord() ) return false;

   return true;

}

GridMap * PropertyValue::createGridMap (const Grid * grid, unsigned int depth)
{
   assert (grid);
   if (getStorage () == TIMEIOTBL && depth != 1) return 0;

   if (getChild (ValueMap) != 0) return 0;

   (void) getFactory ()->produceGridMap (this, ValueMap, grid, DefaultUndefinedMapValue, depth);

   return (GridMap *) getChild (ValueMap);
}

/// Return the GridMap if already there
GridMap * PropertyValue::hasGridMap (void) const
{
   return (GridMap *) getChild (ValueMap);
}

void PropertyValue::getHDFinfo(string& fileName, string& dataSetName, string& outputDir) const
{
   Record * record = getRecord();
   if (!record) return;

   outputDir = getProjectHandle().getFullOutputDir();

   if (getStorage() == TIMEIOTBL)
   {
      // The record to refer to is a TimeIoTbl record

      const string & mapFileName = getMapFileName(record);
      const string & propertyId = getPropertyGrid(record);

      if (mapFileName != "")
      {
         fileName = mapFileName;
         dataSetName = "/Layer=" + propertyId;
      }
      else
      {
         fileName = propertyId + ".HDF";
         dataSetName = "/Layer=0";
      }
   }
   else if (getStorage() == THREEDTIMEIOTBL)
   {
      // The record to refer to is SnapshotIoTbl record
      fileName = getMapFileName(record);

      dataSetName += "/" + getGroupName(record) + "/" + getDataSetName(record);
   }
   else if (getStorage() == SNAPSHOTIOTBL)
   {
      // The record to refer to is SnapshotIoTbl record
      fileName = getSnapshotFileName(record);
      dataSetName = "/" + getName() + "/" + (dynamic_cast<const Formation *>(getFormation())->getMangledName());
   }
}

/// Read in the GridMap if not there yet and return it
GridMap * PropertyValue::getGridMap (void) const
{
   if (getChild (ValueMap) != 0)
   {
      return (GridMap *) getChild (ValueMap);
   }

   Record * record = getRecord();
   if (!record) return 0;

   // The GridMap is to be retrieved from file
   string fileName, dataSetName, outputDir;
   getHDFinfo(fileName, dataSetName, outputDir);

   const bool oldPrimaryDoubleFlag = getProjectHandle().isPrimaryDouble();

   if( not isPrimary() ) {
      getProjectHandle().setPrimaryDouble( false );
   }
   (void) getProjectHandle().loadOutputMap (this, ValueMap, fileName, dataSetName);

   if( not isPrimary() ) {
      getProjectHandle().setPrimaryDouble( oldPrimaryDoubleFlag );
   }

   return (GridMap *) getChild (ValueMap);
}

database::Record* PropertyValue::createTimeIoRecord (database::Table * timeIoTbl)
{
   assert (timeIoTbl);
   database::Record * timeIoRecord = timeIoTbl->createRecord ();

   database::setPropertyName (timeIoRecord, getName ());
   database::setTime (timeIoRecord, m_snapshot->getTime ());

   database::setAverage (timeIoRecord, DefaultUndefinedScalarValue);
   database::setMinimum (timeIoRecord, DefaultUndefinedScalarValue);
   database::setMaximum (timeIoRecord, DefaultUndefinedScalarValue);
   database::setSum (timeIoRecord, DefaultUndefinedScalarValue);
   string propertyGrid;

   propertyGrid += getName ();

   propertyGrid += "_";
   propertyGrid += m_snapshot->asString ();

   if (getReservoir () && !m_surface && !getFormation ())
   {
      propertyGrid += "_";

      // Supress printing of this information as
      // a) It is not avialable (Seep formation is no reservoir), and
      // b) It is not needed.
      if (getName () != "SeepageBasinTop_Gas" and getName () != "SeepageBasinTop_Oil")
      {
         propertyGrid += ((Reservoir *) getReservoir ())->getMangledName ();
         database::setFormationName (timeIoRecord, getReservoir ()->getName ());
         propertyGrid += "_";
      }
   }
   else if (m_surface || m_formation)
   {
      propertyGrid += "_";
      if (m_surface)
      {
         propertyGrid += m_surface->getMangledName ();
         database::setSurfaceName (timeIoRecord, m_surface ->getName ());
      }

      propertyGrid += "_";
      if (m_formation)
      {
         propertyGrid += m_formation->getMangledName ();
         database::setFormationName (timeIoRecord, m_formation->getName ());
      }
   }
   else
   {
      timeIoTbl->deleteRecord (timeIoRecord);
      return 0;
   }

   database::setPropertyGrid (timeIoRecord, propertyGrid);

   int depoSequence = 0;
   if (m_surface)
   {
      const Formation * formation = m_surface->getBottomFormation ();
      if (!formation)
      {
         --depoSequence;
         formation = m_surface->getTopFormation ();
      }

      depoSequence += formation->getDepositionSequence ();
   }

   // Supress printing of depo sequence for seep formations
   if (m_reservoir and getName () != "SeepageBasinTop_Gas" and getName () != "SeepageBasinTop_Oil")
      depoSequence += m_reservoir->getFormation ()->getDepositionSequence () * 1000;

   if (m_formation)
      depoSequence += m_formation->getDepositionSequence () * 1000;

   database::setSum2 (timeIoRecord, DefaultUndefinedScalarValue);
   database::setNP (timeIoRecord, DefaultUndefinedScalarIntValue );

   setRecord (timeIoRecord);

   return timeIoRecord;
}

database::Record* PropertyValue::create3DTimeIoRecord (database::Table * timeIoTbl)
{
   assert (timeIoTbl);

   database::Record * timeIoRecord = timeIoTbl->createRecord ();

   database::setPropertyName (timeIoRecord, getName ());
   database::setTime (timeIoRecord, m_snapshot->getTime ());

   database::setFormationName (timeIoRecord, m_formation->getName ());

   database::setMapFileName (timeIoRecord, "");

   database::setNumberX (timeIoRecord, DefaultUndefinedScalarIntValue );
   database::setNumberY (timeIoRecord, DefaultUndefinedScalarIntValue );
   database::setNumberZ (timeIoRecord, DefaultUndefinedScalarIntValue );

   database::setAverage (timeIoRecord, DefaultUndefinedScalarValue);
   database::setMinimum (timeIoRecord, DefaultUndefinedScalarValue);
   database::setMaximum (timeIoRecord, DefaultUndefinedScalarValue);
   database::setSum (timeIoRecord, DefaultUndefinedScalarValue);
   database::setSum2 (timeIoRecord, DefaultUndefinedScalarValue);
   database::setNP (timeIoRecord, DefaultUndefinedScalarIntValue );

   setRecord (timeIoRecord);

   return timeIoRecord;
}

bool PropertyValue::linkToSnapshotIoRecord (void)
{

//    cout << " PropertyValue::linkToSnapshotIoRecord " << (unsigned long)(getSnapshot ()) << "  "
//         << database::getTime ( (((Snapshot *) getSnapshot ())->getRecord ())) << endl;

   setRecord (((Snapshot *) getSnapshot ())->getRecord ());
   return true;
}

bool PropertyValue::saveMapToFile (MapWriter & mapWriter, const bool saveAsPrimary)
{
   float time = (float) getSnapshot ()->getTime ();

   GridMap * gridMapCalculated = (GridMap *) getGridMap ();
   GridMap * gridMapToOutput = gridMapCalculated;

   if (getProjectHandle().saveAsInputGrid ())
   {
      gridMapToOutput = getFactory ()->produceGridMap (0, 0, getProjectHandle().getInputGrid(), DefaultUndefinedMapValue, 1);
      gridMapCalculated->convertToGridMap (gridMapToOutput);

   }
   database::setMapFileName (m_record, mapWriter.getFileName ());

   gridMapToOutput->retrieveData();

   double min, max;
   gridMapToOutput->getMinMaxValue (min, max);

   database::setMinimum (m_record, min);
   database::setMaximum (m_record, max);
   database::setAverage (m_record, gridMapToOutput->getAverageValue ());
   database::setSum (m_record, gridMapToOutput->getSumOfValues ());
   database::setSum2 (m_record, gridMapToOutput->getSumOfSquaredValues ());
   database::setNP (m_record, gridMapToOutput->getNumberOfDefinedValues ());

   gridMapToOutput->restoreData();

   mapWriter.writeMapToHDF (gridMapToOutput, time, time, database::getPropertyGrid (m_record),
                            (m_surface == 0 ? "" : m_surface->getName ()), saveAsPrimary);

   if (getProjectHandle().saveAsInputGrid ()) delete gridMapToOutput;

   return true;
}

bool PropertyValue::saveVolumeToFile (MapWriter & mapWriter, const bool primaryFlag )
{
   database::setMapFileName (m_record, mapWriter.getFileName ());
   database::setGroupName (m_record, getName ());
   database::setDataSetName (m_record, getFormation ()->getMangledName ());

   GridMap * gridMap = getGridMap ();

   gridMap->retrieveData();

   database::setNumberX (m_record, gridMap->numI ());
   database::setNumberY (m_record, gridMap->numJ ());
   database::setNumberZ (m_record, gridMap->getDepth ());

   double min, max;
   gridMap->getMinMaxValue (min, max);

   database::setMinimum (m_record, min);
   database::setMaximum (m_record, max);
   database::setAverage (m_record, gridMap->getAverageValue ());
   database::setSum (m_record, gridMap->getSumOfValues ());
   database::setSum2 (m_record, gridMap->getSumOfSquaredValues ());
   database::setNP (m_record, gridMap->getNumberOfDefinedValues ());

   gridMap->restoreData();

   mapWriter.writeVolumeToHDF (gridMap, getName (), getFormation ()->getMangledName (), primaryFlag );
   return true;
}

/// First sorts on the age of the PropertyValue objects,
/// then on the deposition age of
/// the Surface objects that are, directly or indirectly,
/// associated with the PropertyValue objects.
bool PropertyValue::SortByAgeAndDepoAge (const PropertyValue * lhs, const PropertyValue * rhs)
{
   return lhs->compareByAgeAndDepoAge (rhs) < 0;
}

/// Sorts on the deposition age of
/// the Surface objects that are, directly or indirectly,
/// associated with the PropertyValue objects.
bool PropertyValue::SortByDepoAge (const PropertyValue * lhs, const PropertyValue * rhs)
{
   return lhs->compareByDepoAge (rhs) < 0;
}

/// compare on the age of the PropertyValue objects
int PropertyValue::compareByAge (const PropertyValue * rhs) const
{
   const PropertyValue * lhs = this;

   const Snapshot *lhsSnapshot = (const Snapshot *) lhs->getSnapshot ();
   assert (lhsSnapshot);

   const Snapshot *rhsSnapshot = (const Snapshot *) rhs->getSnapshot ();
   assert (rhsSnapshot);

   if (lhsSnapshot != rhsSnapshot)
   {
      if (lhsSnapshot->getTime () < rhsSnapshot->getTime ())
         return -1;
      else if (lhsSnapshot->getTime () > rhsSnapshot->getTime ())
         return 1;
   }
   return 0;
}

/// First compares on the age of the PropertyValue objects,
/// then on the deposition age of
/// the Surface objects that are, directly or indirectly,
/// associated with the PropertyValue objects.
int PropertyValue::compareByAgeAndDepoAge (const PropertyValue * rhs) const
{
   if (compareByAge (rhs) == 0)
   {
      return compareByDepoAge (rhs);
   }
   else
   {
      return 0;
   }
}

/// Compares on the deposition age of
/// the Surface objects that are, directly or indirectly,
/// associated with the PropertyValue objects.
int PropertyValue::compareByDepoAge (const PropertyValue * rhs) const
{

   const PropertyValue * lhs = this;

   const Snapshot *lhsSnapshot;

   const Snapshot *rhsSnapshot;

   const Surface *lhsSurface;

   if ((lhsSurface = lhs->getSurface ()) == 0)
   {
      const Formation *lhsFormation;

      if ((lhsFormation = lhs->getFormation ()) == 0)
      {
         const Reservoir *lhsReservoir;

         if ((lhsReservoir = (const Reservoir *) lhs->getReservoir ()) == 0)
         {
            assert (0);
         }

         lhsFormation = lhsReservoir->getFormation ();
         assert (lhsFormation);
      }

      lhsSurface = lhsFormation->getTopSurface ();
      assert (lhsSurface);
   }

   const Surface *rhsSurface;

   if ((rhsSurface =  rhs->getSurface ()) == 0)
   {
      const Formation *rhsFormation;

      if ((rhsFormation = rhs->getFormation ()) == 0)
      {
         const Reservoir *rhsReservoir;

         if ((rhsReservoir = (const Reservoir *) rhs->getReservoir ()) == 0)
         {
            assert (0);
         }
         rhsFormation = rhsReservoir->getFormation ();
         assert (rhsFormation);
      }
      rhsSurface = rhsFormation->getTopSurface ();
      assert (rhsSurface);
   }


   if ( lhsSurface->kind () == BASEMENT_SURFACE and rhsSurface->kind () == BASEMENT_SURFACE )
   {
      // Since both surfaces are in the basement [and basement surfaces do not have associated snapshots],
      // age order is determined by which surface is the deeper.

      // Saved as Boolean here to save on string comparisons.
      bool lhsIsCrust = lhsSurface->getName () == Interface::CrustBottomSurfaceName;
      bool rhsIsCrust = rhsSurface->getName () == Interface::CrustBottomSurfaceName;

      if ( lhsIsCrust and not rhsIsCrust )
      {
         return -1;
      }
      else if ( not lhsIsCrust and rhsIsCrust )
      {
         return 1;
      }
      else
      {
         return 0;
      }

   }
   else if ( lhsSurface->kind () == BASEMENT_SURFACE )
   {
      // Return 1, since the lhs-surface is in basement and rhs is not, therefore the lhs must be the older surface.
      return 1;
   }
   else if ( rhsSurface->kind () == BASEMENT_SURFACE )
   {
      // Return -1, since the rhs-surface is in basement and lhs is not, therefore the rhs must be the older surface.
      return -1;
   }
   else
   {

      lhsSnapshot = (const Snapshot *) lhsSurface->getSnapshot ();
      assert (lhsSnapshot);

      rhsSnapshot = (const Snapshot *) rhsSurface->getSnapshot ();
      assert (rhsSnapshot);

      if (lhsSnapshot->getTime () < rhsSnapshot->getTime ())
         return -1;
      else if (lhsSnapshot->getTime () > rhsSnapshot->getTime ())
         return 1;
      else return 0;
   }

}

bool PropertyValue::isPrimary() const {

   return m_property->isPrimary();
}

void PropertyValue::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void PropertyValue::asString (string & str) const
{

   std::ostringstream buf;

   buf << "PropertyValue: ";
   buf << getName ();
   // buf << " (" << getProperty ()->getName () << ")";

   buf << ", Age = " << getSnapshot ()->getTime ();

   if (getReservoir ())
   {
      buf << ", Reservoir: " << getReservoir ()->getName ();
   }

   if (getFormation ())
   {
      buf << ", Formation: " << getFormation ()->getName ();
   }

   if (getSurface ())
   {
      buf << ", Surface: " << getSurface ()->getName ();
   }
   buf << endl;

   str = buf.str ();
}
