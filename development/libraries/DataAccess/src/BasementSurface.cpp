#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;


#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "BasementSurface.h"
#include "CrustFormation.h"
#include "Surface.h"
#include "Formation.h"
#include "GridMap.h"
#include "ObjectFactory.h"
#include "Parent.h"
#include "ProjectHandle.h"
#include "Snapshot.h"

using namespace DataAccess;
using namespace Interface;


BasementSurface::BasementSurface (ProjectHandle& projectHandle, const std::string& surfaceName) :
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
   return nullptr;
}

GridMap * BasementSurface::computeDepthMap (void) const
{
   AdditionFunctor add;

   bool isALC = (( m_mangledName == "Bottom_of_Crust" ) && getProjectHandle().isALC() );

   const Formation * upperFormation = (Formation *) Surface::getTopFormation ();
   if (!upperFormation) return nullptr;

   GridMap * upperFormationGridMap = ( isALC ? (GridMap *) (dynamic_cast<const CrustFormation *>(upperFormation))->getInitialThicknessMap () :
                                       (GridMap *) upperFormation->getInputThicknessMap ());

   const Surface * upperSurface = (Surface *) upperFormation->getTopSurface ();
   if (!upperSurface) return nullptr;

   const GridMap * thicknessMap = upperFormationGridMap;
   const GridMap * depthMap = (GridMap *) upperSurface->getInputDepthMap ();

   if (!thicknessMap) return nullptr;
   if (!depthMap) return nullptr;

   GridMap * myDepthMap = getProjectHandle().getFactory ()->produceGridMap (this, DEPTH, depthMap, thicknessMap, add );

   return myDepthMap;
}


GridMap * BasementSurface::computeDepthMap ( const GridMap* operand1, UnaryFunctor& op ) const {
   GridMap * myDepthMap = getProjectHandle().getFactory ()->produceGridMap (this, DEPTH, operand1, op);
   return myDepthMap;
}

void BasementSurface::asString (string & str) const {


   std::ostringstream buf;

   buf << "BasementSurface:";
   buf << " name = " << getName () << endl;

   str = buf.str ();
}

