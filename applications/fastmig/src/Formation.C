#include "RequestHandling.h"

#include "Formation.h"
#include "Surface.h"
#include "rankings.h"
#include "migration.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"

#include <unistd.h>
#include <values.h>
#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
using namespace std;

using namespace CBMGenerics;

using namespace DataAccess;
using Interface::Property;
using Interface::Snapshot;
using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FormationList;

namespace migration
{

Formation::Formation (ProjectHandle * projectHandle, database::Record * record)
   : Interface::Formation (projectHandle, record)
{
}

Formation::~Formation (void)
{
}

FormationSurfaceGridMaps Formation::getFormationSurfaceGridMaps(const Property* prop, const Snapshot* snapshot) const
{
   const ProjectHandle* projectHandle = getProjectHandle();

   const GridMap* top = 0;
   const GridMap* base = 0;
   unsigned int index = 0;
   {
      const PropertyValueList* allProps = projectHandle->getPropertyValues(Interface::FORMATION, 
	 prop, snapshot, 0, this, 0, Interface::FORMATION);
      if (allProps && allProps->size() > 0) {
	 top = (*allProps)[0]->getGridMap();
	 if (top)
	    index = top->getDepth()-1;
	 base = top;
      }
      delete allProps;
   }

   if (!top)
      top = getTopGridMap(prop, snapshot);

   if (!base) {
      base = getBaseGridMap(prop, snapshot);
      index = 0;
   }

#ifdef DEBUG_FORMATION
   string formName = this->getName();
   double snapTime = snapshot->getTime();
   string propName = prop->getName();

   if (!top && !base) {
      cerr << "Formation.C:139: No formation and surface property grid maps found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
   else if (!top) {
      cerr << "Formation.C:143: No formation and top surface property grid map found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
   else if (!base) {
      cerr << "Formation.C:147: No formation and base surface property grid map found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
#endif

   return FormationSurfaceGridMaps(SurfaceGridMap(top,index), SurfaceGridMap(base, 
      (unsigned int)0), this);
}

SurfaceGridMap Formation::getTopSurfaceGridMap(const Property* prop, 
   const Snapshot* snapshot) const
{
   const ProjectHandle* projectHandle = getProjectHandle();

   const GridMap* top = 0;
   unsigned int index = 0;
   { 
      const PropertyValueList* topProps = projectHandle->getPropertyValues(Interface::FORMATION, 
	 prop, snapshot, 0, this, 0, Interface::FORMATION);
	 if (topProps && topProps->size() > 0) {
	 top = (*topProps)[0]->getGridMap();
	 if (top)
	    index = top->getDepth()-1;
      }
      delete topProps;
   }
   
   if (!top) 
      top = getTopGridMap(prop, snapshot);

   return SurfaceGridMap(top, index);
}

SurfaceGridMap Formation::getBaseSurfaceGridMap(const Property* prop, 
   const Snapshot* snapshot) const
{
   const ProjectHandle* projectHandle = getProjectHandle();

   const GridMap* base = 0;
   { 
      const PropertyValueList* baseProps = projectHandle->getPropertyValues(Interface::FORMATION, 
	 prop, snapshot, 0, this, 0, Interface::FORMATION);
      if (baseProps && baseProps->size() > 0)
	 base = (*baseProps)[0]->getGridMap();
      delete baseProps;
   }

   if (!base)
      base = getBaseGridMap( prop, snapshot);

   return SurfaceGridMap(base, (unsigned int)0);
}

const GridMap* Formation::getTopGridMap(const Property* prop, 
   const Snapshot* snapshot) const
{
   const GridMap* top = 0;

   const ProjectHandle* projectHandle = getProjectHandle();
   const PropertyValueList* topProps = projectHandle->getPropertyValues(Interface::SURFACE, 
      prop, snapshot, 0, 0, getTopSurface(), Interface::SURFACE);
   if (topProps && topProps->size() > 0)
      top = (*topProps)[0]->getGridMap();
   delete topProps;

   return top;
}

const GridMap* Formation::getBaseGridMap(const Property* prop, 
   const Snapshot* snapshot) const
{
   const GridMap* base = 0;

   const ProjectHandle* projectHandle = getProjectHandle();
   const PropertyValueList* baseProps = projectHandle->getPropertyValues(Interface::SURFACE, 
      prop, snapshot, 0, 0, getBottomSurface(), Interface::SURFACE);
   if (baseProps && baseProps->size() > 0)
      base = (*baseProps)[0]->getGridMap();
   delete baseProps;

   return base;
}

const Formation * Formation::getTopFormation () const
{
   const Interface::Surface * surface = getTopSurface ();
      
   return (const Formation *) (surface ? surface->getTopFormation () : 0);
}

const Formation * Formation::getBottomFormation () const
{
   const Interface::Surface * surface = getBottomSurface ();
      
   return (const Formation *) (surface ? surface->getBottomFormation () : 0);
}

}

