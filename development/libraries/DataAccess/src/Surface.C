#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Parent.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Interface.h"

using namespace DataAccess;
using namespace Interface;


Surface::Surface (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record), m_top (0), m_bottom (0), m_snapshot (0)
{

   // It is up to the the derived classes to initialise correctly the member components of this class.
   const bool recordFromStratIOTbl = ( m_record != 0 and m_record->getTable ()->name () == "StratIoTbl" );

   if ( recordFromStratIOTbl ) {
      m_snapshot = (const Snapshot *) m_projectHandle->findSnapshot (getDepoAge (m_record));
      assert (m_snapshot);
      m_mangledName = utilities::mangle (getName ());
      m_kind = SEDIMENT_SURFACE;
      m_formationDepositionSequenceNumber = database::getDepoSequence ( m_record );
   } else {
      m_snapshot = 0;
      m_kind = BASEMENT_SURFACE;
      m_formationDepositionSequenceNumber = DefaultUndefinedScalarIntValue;
   }

}

Surface::Surface (ProjectHandle * projectHandle) : DAObject (projectHandle, 0), m_mangledName ( "" ), m_top (0), m_bottom (0), m_snapshot (0)
{
   m_kind = BASEMENT_SURFACE;
}

Surface::~Surface (void)
{
}

const string & Surface::getName (void) const
{
   return database::getSurfaceName (m_record);
}

const string & Surface::getMangledName (void) const
{
   return m_mangledName;
}


SurfaceKind Surface::kind () const {
   return m_kind;
}

const string & Surface::getTopFormationName (void) const
{

   if ( m_top != 0 ) {
      return m_top->getName ();
   } else {
      return NullString;
   }

}

const string & Surface::getBottomFormationName (void) const
{

   if ( m_bottom != 0 ) {
      return m_bottom->getName ();
   } else {
      return NullString;
   } 

}

void Surface::setTopFormation (Formation * formation)
{
   m_top = formation;
}

void Surface::setBottomFormation (Formation * formation)
{
   m_bottom = formation;
}

const Formation * Surface::getTopFormation (void) const
{
   return m_top;
}

const Formation * Surface::getBottomFormation (void) const
{
   return m_bottom;
}

const Snapshot * Surface::getSnapshot (void) const
{
   return m_snapshot;
}

const GridMap * Surface::getInputDepthMap (void) const
{
   const GridMap * gridMap;
   if ((gridMap = (GridMap *) getChild (DEPTH)) != 0) return gridMap;
   if ((gridMap = loadDepthMap ()) != 0) return gridMap;
   else if ((gridMap = computeDepthMap ()) != 0) return gridMap;
   else return 0;
}

const GridMap * Surface::getInputTwoWayTimeMap( void ) const
{
   const GridMap * gridMap = 0;
   // if the map is already loaded
   if ((gridMap = (GridMap *)getChild( TWOWAYTIME )) != 0) return gridMap;

   // else load it if possible
   else {
      database::Table* twoWayTimeTbl = m_projectHandle->getTable( "TwoWayTimeIoTbl" );
      database::Table::iterator tblIter;

      for (tblIter = twoWayTimeTbl->begin(); tblIter != twoWayTimeTbl->end(); ++tblIter)
      {
         Record * twoWayTimeRecord = *tblIter;
         // find the good line for the Surface in the table
         if (database::getSurfaceName( twoWayTimeRecord ) == getName())
         {
            const string &TwoWayTimeGridMapId = getTwoWayTimeGrid( twoWayTimeRecord );
            if (TwoWayTimeGridMapId.length() != 0)
            {
               gridMap = m_projectHandle->loadInputMap( "TwoWayTimeIoTbl", TwoWayTimeGridMapId );
               return gridMap;
            }
         }
      }
      return gridMap;
   }
}

float Surface::getInputTwoWayTimeScalar( void ) const
{

   database::Table* twoWayTimeTbl = m_projectHandle->getTable( "TwoWayTimeIoTbl" );
   database::Table::iterator tblIter;

   for (tblIter = twoWayTimeTbl->begin(); tblIter != twoWayTimeTbl->end(); ++tblIter)
   {
      Record * twoWayTimeRecord = *tblIter;
      // find the good line for the Surface in the table
      if (database::getSurfaceName( twoWayTimeRecord ) == getName())
      {
         float twoWayTimeScalar = static_cast<float>( database::getTwoWayTime( twoWayTimeRecord ) );
         // a two way time needs two be a positive number
         if (twoWayTimeScalar >= 0)
         {
            return twoWayTimeScalar;
         }
      }
   }
   return static_cast<float>( DefaultUndefinedScalarValue );
}

GridMap * Surface::loadDepthMap (void) const
{
   double depth;
   GridMap * gridMap = 0;

   if ((depth = getDepth (m_record)) != RecordValueUndefined)
   {
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, DEPTH, grid, depth);
      assert (gridMap == (GridMap *) getChild (DEPTH));
   }
   else
   {
      const string &depthGridMapId = getDepthGrid (m_record);
      if (depthGridMapId.length () != 0)
      {
	      gridMap = m_projectHandle->loadInputMap ("StratIoTbl", depthGridMapId);
      }
   }
   return gridMap;
}

static double minus (double a, double b)
{
   return a - b;
}

GridMap * Surface::computeDepthMap (void) const
{
   const Formation * lowerFormation = dynamic_cast<const Formation *>(getBottomFormation ());
   if (!lowerFormation) return false;
   const Surface * lowerSurface = dynamic_cast<const Surface *>(lowerFormation->getBottomSurface ());
   if (!lowerSurface) return false;

   const GridMap * thicknessMap = (GridMap *) lowerFormation->getInputThicknessMap ();
   const GridMap * depthMap = (GridMap *) lowerSurface->getInputDepthMap ();

   if (!thicknessMap) return false;
   if (!depthMap) return false;

   GridMap * myDepthMap = m_projectHandle->getFactory ()->produceGridMap (this, DEPTH, depthMap, thicknessMap, ::minus);

   return myDepthMap;
}

void Surface::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Surface::asString (string & str) const
{
   ostringstream buf;

   buf << "Surface:";
   buf << " name = " << getName ();
   buf << ", deposition age = " << getSnapshot ()->getTime () << endl;

   str = buf.str ();
}

int Surface::getFormationDepoSequenceNumber () const {
   return m_formationDepositionSequenceNumber;
}


bool SurfaceLessThan::operator ()( const Surface* s1,
                                   const Surface* s2 ) const {


   // Snapshots can be the same if the formation is an intrusion.
   // So the top surface of the intrusion formation will have the same time 
   // as the formation below.
   if ( s1->getSnapshot () == s2->getSnapshot ()) {

      if ( s1->getFormationDepoSequenceNumber () == DefaultUndefinedScalarValue ) {
         // The s1 surface is the top surface of a basement layer.
         return false;
      } else if ( s2->getFormationDepoSequenceNumber () == DefaultUndefinedScalarValue ) {
         // The s2 surface is the top surface of a basement layer.
         return true;
      } else {
         return s1->getFormationDepoSequenceNumber () > s2->getFormationDepoSequenceNumber ();
      }

   } else {
      return s1->getSnapshot ()->getTime () < s2->getSnapshot ()->getTime ();
   }

#if 0
   return s1->getSnapshot ()->getTime () < s2->getSnapshot ()->getTime ();
#endif

}
