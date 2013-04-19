#include "formation_MPI.h"
#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"
#include "limits.h"

using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FormationList;

namespace migration { namespace formation_MPI {

using namespace DataAccess;

SurfaceGridMap::SurfaceGridMap(const GridMap* gridMap, int depth):
   m_localGridMap(gridMap),
   m_depth(depth),
   m_status(CREATED)
{}

SurfaceGridMap::~SurfaceGridMap()
{
   if (m_status == RETRIEVED)
      restoreData();

   delete m_gridMap;
}

void SurfaceGridMap::retrieveData()
{
   assert(m_status == CREATED || m_status == RESTORED);
   m_gridMap->retrieveData();
   m_status = RETRIEVED;
}

void SurfaceGridMap::restoreData()
{
   assert(m_status == RETRIEVED);
   m_gridMap->restoreData();
   m_status = RESTORED;
}

double SurfaceGridMap::operator[](const Tuple2<unsigned int>& ij) const
{
   assert(m_status == RETRIEVED);
   double result = m_grid->getValue(ij[0], ij[1], m_depth);
   return result;
}


const GridMap* topPropertyMap(const Formation* formation, 
   const Snapshot* snapshot, const string& propName)
{
   const ProjectHandle* projectHandle = formation->getProjectHandle();
   const Property* prop = projectHandle->findProperty(propName);

   const GridMap* propertyValues = 0;
   const PropertyValueList* topProperties = projectHandle->getPropertyValues(Interface::SURFACE, 
      prop, snapshot, 0, 0, formation->getTopSurface(), Interface::SURFACE);
   if (topProperties && topProperties->size() > 0)
      propertyValues = (*topProperties)[0]->getGridMap();

   delete topProperties;
   return propertyValues;
}

const GridMap* basePropertyMap(const Formation* formation, 
   const Snapshot* snapshot, const string& propName)
{
   const ProjectHandle* projectHandle = formation->getProjectHandle();
   const Property* prop = projectHandle->findProperty(propName);

   const GridMap* propertyValues = 0;
   const PropertyValueList* baseProperties = projectHandle->getPropertyValues(Interface::SURFACE, 
      prop, snapshot, 0, 0, formation->getBottomSurface(), Interface::SURFACE);
   if (baseProperties && baseProperties->size() > 0)
      propertyValues = (*baseProperties)[0]->getGridMap();

   delete baseProperties;
   return propertyValues;
}

SurfaceGridMap surfaceTopPropertyMap(const Formation* formation, 
   const Snapshot* snapshot, const string& propName)
{
   const GridMap* gridMap = topPropertyMap(formation, snapshot, propName);
   return SurfaceGridMap(gridMap, formation, findBaseFormation(formation));
}

SurfaceGridMap surfaceBasePropertyMap(const Formation* formation, 
   const Snapshot* snapshot, const string& propName)
{
   const GridMap* gridMap = basePropertyMap(formation, snapshot, propName);
   return SurfaceGridMap(gridMap, findTopFormation(formation), formation);
}

FormationSurfaceGridMaps topAndBasePropertyMap(const Formation* formation, 
   const Snapshot* snapshot, const string& propName)
{
   const GridMap* top = topPropertyMap(formation, snapshot, propName);
   const GridMap* base = basePropertyMap(formation, snapshot, propName);
   return FormationSurfaceGridMaps(top, base, formation);
}

// const GridMap * Reservoir::getPropertyGridMap (const string & propertyName,
//       const Snapshot * snapshot) const
// {
//    int selectionFlags = Interface::FORMATION;

//    PropertyValueList * propertyValues =
//       m_projectHandle->getPropertyValues (selectionFlags,
// 	    m_projectHandle->findProperty (propertyName),
// 	    snapshot, 0, getFormation (), 0,
// 	    Interface::VOLUME);

//    if (propertyValues->size () != 1)
//    {
// #if 0
//       cerr << "Error: Wrong number ("
// 	 << propertyValues->size ()
// 	 << " != 1) of "
// 	 << propertyName
// 	 << " VolumePropertyValues returned"
// 	 << " for Formation "
// 	 << getFormation ()->getName ()
// 	 << " at Snapshot "
// 	 << snapshot->getTime ()
// 	 << "!"
// 	 << endl;
// #endif
//       return 0;
//    }

// #if 0
//    cerr << GetRankString () << ": " << getName () << "::getGridMap (" << propertyName << ") starting" << endl;
// #endif
//    const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();
// #if 0
//    cerr << GetRankString () << ": " << getName () << "::getGridMap (" << propertyName << ") finished" << endl;
// #endif

//    delete propertyValues;
//    return gridMap;
// }

bool getThickness(const Formation* formation, const Snapshot* snapshot, unsigned int i, 
   unsigned int j, double& thickness)
{
   const ProjectHandle* projectHandle = formation->getProjectHandle();
   const Property* depthProp = projectHandle->findProperty("Depth");

   const GridMap* top = 0;
   PropertyValueList* topProps = projectHandle->getPropertyValues(Interface::SURFACE, 
      depthProp, snapshot, 0, 0, formation->getTopSurface(), Interface::SURFACE);
   if (topProps && topProps->size() > 0)
      top = (*topProps)[0]->getGridMap();
   if (!top)
      return false;

   const GridMap* base = 0; 
   PropertyValueList* baseProps = projectHandle->getPropertyValues(Interface::SURFACE, 
      depthProp, snapshot, 0, 0, formation->getBottomSurface(), Interface::SURFACE);
   if (baseProps && baseProps->size() > 0)
      base = (*baseProps)[0]->getGridMap();
   if (!base)
      return false;

   thickness = base->getValue(i,j,(unsigned int)0);
   thickness -= top->getValue(i,j,(unsigned int)0);
   return true;
}

bool getThickness(const vector<Formation*>& formations, const Snapshot* snapshot, 
   unsigned int i, unsigned int j, double& thickness)
{
   bool result = true;
   thickness = 0.0;
   for (vector<Formation*>::const_iterator f = formations.begin(); f != formations.end(); ++f)
   {
      double formationThickness;
      if (getThickness(*f, snapshot, i,j, formationThickness))
         thickness += formationThickness;
      else
         result = false;
   }
   return result;
}

} } // namespace migration::formation
