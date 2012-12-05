#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

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
   } else {
      m_snapshot = 0;
      m_kind = BASEMENT_SURFACE;
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

const string & Surface::getTopFormationName (void)
{
   return m_top->getName ();
}

const string & Surface::getBottomFormationName (void)
{
   return m_bottom->getName ();
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
   if ((gridMap = (GridMap *) getChild (DepthMap)) != 0) return gridMap;
   if ((gridMap = loadDepthMap ()) != 0) return gridMap;
   else if ((gridMap = computeDepthMap ()) != 0) return gridMap;
   else return 0;
}

GridMap * Surface::loadDepthMap (void) const
{
   double depth;
   GridMap * gridMap = 0;

   if ((depth = getDepth (m_record)) != RecordValueUndefined)
   {
      //const Grid * grid = m_projectHandle->getInputGrid ();
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, DepthMap, grid, depth);
      assert (gridMap == (GridMap *) getChild (DepthMap));
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

   GridMap * myDepthMap = m_projectHandle->getFactory ()->produceGridMap (this, DepthMap, depthMap, thicknessMap, ::minus);

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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Surface:";
   buf << " name = " << getName ();
   buf << ", deposition age = " << getSnapshot ()->getTime () << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}


bool SurfaceLessThan::operator ()( const Surface* s1,
                                   const Surface* s2 ) const {
   return s1->getSnapshot ()->getTime () < s2->getSnapshot ()->getTime ();
}
