#include "BurialHistory.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"

#include "FastTouch.h"
#include "BurialHistoryTimeStep.h"

#include <cassert>
#include "array.h"

using namespace DataAccess;
using namespace Interface;


namespace fasttouch
{

//
// PUBLIC METHODS
//

/// The class constructor validates the input parameters and
/// instructs the relevant I/O operations to be carried out
/// so that all necessary I/O is done one time. 
BurialHistory::BurialHistory(const Surface * surface,  ProjectHandle & projectHandle)
: m_surface          (surface), 
  m_projectHandle(projectHandle)
{
   m_depthProperty = m_projectHandle.findProperty ("Depth");
   m_temperatureProperty = m_projectHandle.findProperty ("Temperature");
   m_vesProperty = m_projectHandle.findProperty ("Ves");

   const Grid * activeGrid = m_projectHandle.getActivityOutputGrid ();

   m_firstI = activeGrid->firstI ();
   m_firstJ = activeGrid->firstJ ();
   m_lastI = activeGrid->lastI ();
   m_lastJ = activeGrid->lastJ ();
   m_numI = activeGrid->numI ();
   m_numJ = activeGrid->numJ ();

   assert (m_lastI + 1 - m_firstI == m_numI);
   assert (m_lastJ + 1 - m_firstJ == m_numJ);


   loadPaleoData ();
}

BurialHistory::~BurialHistory(void)
{
   clearSnapshotMapMemory ();
}

/** This function converts the stl::map holding the burial history into the
 *  array needed by ResQ::calculate. An stl::map is used initially for
 *  gathering the data from the project file as it has necessary build in
 *  functionality. COnverting this data to the array at a later stage
 *  when it is needed externally can be done safely.
 */
const std::vector<BurialHistoryTimeStep> & BurialHistory::returnAsArray (int i, int j, bool reverse)
{
   Properties *p;
   m_burialHistoryTimestep.clear();
   
   BurialHistoryMap::reverse_iterator mapIt;
   for ( mapIt = m_burialHistoryMap.rbegin(); mapIt != m_burialHistoryMap.rend (); ++mapIt )
   {
      p = &(mapIt->second);
      
      // check if all properties present at timestep
      if ( ! p->temp || ! p->depth || ! p->ves )
      {
         continue;
      }

      unsigned int iArray = i - m_firstI;
      unsigned int jArray = j - m_firstJ;

      // check for null value
      if ( p->temp  [iArray][jArray] == Interface::DefaultUndefinedMapValue ||
           p->depth [iArray][jArray] == Interface::DefaultUndefinedMapValue ||
           p->ves   [iArray][jArray] == Interface::DefaultUndefinedMapValue )
      {
         return m_burialHistoryTimestep;
      }
      // if one property present for timestep, then all should be as should be switched on 
      // before run.
      BurialHistoryTimeStep timeStep;
      
      timeStep.time        = mapIt->first;
      timeStep.temperature = p->temp  [iArray][jArray];
      timeStep.depth       = p->depth [iArray][jArray];
      timeStep.effStress   = p->ves   [iArray][jArray] * PA_TO_MPA;
      timeStep.waterSat    = 100;

      m_burialHistoryTimestep.push_back( timeStep );
   }

   if (reverse)
      std::reverse( m_burialHistoryTimestep.begin(), m_burialHistoryTimestep.end() );

   return m_burialHistoryTimestep;
}
/// This function collects 3d burial history.
/// The burial history is saved in an stl::map
/// which stores the data, sorting it by timestep. When all data is 
/// collected, the array needed to hold the data at as later stage, for
/// external use is dynamically allocated here.
bool BurialHistory::loadPaleoData (void)
{
    PropertyValueList * propertyValues = m_projectHandle.getPropertyValues (SURFACE, 0, 0, 0, 0, m_surface, MAP);
 
    PropertyValueList::iterator iter;
 
    for (iter = propertyValues->begin (); iter != propertyValues->end (); ++iter)
    {
        const PropertyValue * propertyValue = * iter;
        const Snapshot * snapshot = propertyValue->getSnapshot ();
        
        double **propertyArray;
        
        if (propertyValue->getProperty () == m_depthProperty) 
        {
            propertyArray = m_burialHistoryMap[snapshot->getTime ()].depth = Array<double>::create2d (m_numI, m_numJ);
        }
        else if (propertyValue->getProperty () == m_temperatureProperty) 
        {
            propertyArray = m_burialHistoryMap[snapshot->getTime ()].temp = Array<double>::create2d (m_numI, m_numJ);
        }
        else if (propertyValue->getProperty () == m_vesProperty) 
        {
            propertyArray = m_burialHistoryMap[snapshot->getTime ()].ves = Array<double>::create2d (m_numI, m_numJ);
        }
        else
        {
            continue;
        }
  
        const GridMap * gridMap = propertyValue->getGridMap ();
  
        string propValueString;
        propertyValue->asString (propValueString);
        // cerr << "retrieving data for " << propValueString << endl;

        gridMap->retrieveData ();

        int i, j;
        for (i = m_firstI; i <= m_lastI; ++i)
        {
            for (j = m_firstJ; j <= m_lastJ; ++j)
            {
                propertyArray [i - m_firstI][j - m_firstJ] = gridMap->getValue ((unsigned int) i, (unsigned int) j);
            }
        }
        gridMap->restoreData ();
    }
    delete propertyValues;
 
    return true;
}
void BurialHistory::clearSnapshotMapMemory (void)
{
   Properties *p;

   for (BurialHistoryMap::iterator it = m_burialHistoryMap.begin (); it != m_burialHistoryMap.end (); ++it)
   {
      p = &(it->second);

      if (p->depth)
         Array < double >::delete2d (p->depth);

      if (p->temp)
         Array < double >::delete2d (p->temp);

      if (p->ves)
         Array < double >::delete2d (p->ves);
   }
}


}
