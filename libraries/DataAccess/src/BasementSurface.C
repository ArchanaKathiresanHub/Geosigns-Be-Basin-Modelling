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

#include "Interface/BasementSurface.h"
#include "Interface/CrustFormation.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Parent.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"

using namespace DataAccess;
using namespace Interface;


BasementSurface::BasementSurface (ProjectHandle * projectHandle, const std::string& surfaceName) :
   Interface::Surface ( projectHandle )
{
   m_surfaceName = surfaceName;
   m_mangledName = utilities::mangle ( m_surfaceName );
}


BasementSurface::~BasementSurface (void) {
}

const std::string & BasementSurface::getName (void) const {
   return m_surfaceName;
}

GridMap * BasementSurface::loadDepthMap (void) const {
   return 0;
}

GridMap * BasementSurface::computeDepthMap (void) const
{

   AdditionFunctor add;

   bool isALC = (( m_mangledName == "Bottom_of_Crust" ) && m_projectHandle->getBottomBoundaryConditions () == ADVANCED_LITHOSPHERE_CALCULATOR);
   
   const Formation * upperFormation = (Formation *) Surface::getTopFormation ();
   if (!upperFormation) return false;

   GridMap * upperFormationGridMap = ( isALC ? (GridMap *) (dynamic_cast<const CrustFormation *>(upperFormation))->getInitialThicknessMap () :
                                       (GridMap *) upperFormation->getInputThicknessMap ());
   
   const Surface * upperSurface = (Surface *) upperFormation->getTopSurface ();
   if (!upperSurface) return false;
   
   const GridMap * thicknessMap = upperFormationGridMap;
   const GridMap * depthMap = (GridMap *) upperSurface->getInputDepthMap ();

   if (!thicknessMap) return false;
   if (!depthMap) return false;

   GridMap * myDepthMap = m_projectHandle->getFactory ()->produceGridMap (this, DepthMap, depthMap, thicknessMap, add );

   return myDepthMap;
}


GridMap * BasementSurface::computeDepthMap ( const GridMap* operand1, UnaryFunctor& op ) const {
   GridMap * myDepthMap = m_projectHandle->getFactory ()->produceGridMap (this, DepthMap, operand1, op);
   return myDepthMap;
}

void BasementSurface::asString (string & str) const {

#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "BasementSurface:";
   buf << " name = " << getName () << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}

