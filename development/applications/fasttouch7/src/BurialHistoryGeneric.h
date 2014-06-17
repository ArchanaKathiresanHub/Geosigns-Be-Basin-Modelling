#ifndef __burialhistorygeneric__
#define __burialhistorygeneric__

#include <fstream>

#include <string>
#include <map>
using namespace std;

/// The function of the Burial History class is to gather all 
/// burial history information relating to a specific surface from
/// the project file needed by ResQ for the ResQ calculations and hold it in 
/// a dynamic array which can be easily accessed by any class that
/// needs it 

#include "FastTouch.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"

#include <assert.h>
#include "array.h"


#define PA_TO_MPA 1e-6

namespace DataAccess
{
   namespace Interface
   {
      class Formation;
      class Surface;
      class Property;
   }
}

using namespace DataAccess;
using namespace Interface;

namespace fasttouch
{
    class FastTouch;

/** The function of the Burial History class is to gather all 
 *  burial history information relating to a specific surface from
 *  the project file needed by ResQ for the ResQ calculations and hold it in 
 *  a dynamic array which can be easily accessed by any class that
 *  needs it 
 */
    template <class BurHistTimeStep>
    class BurialHistory
    {
        private:
            // these are used to match the reservoir
            // name to a layer

            const DataAccess::Interface::Formation * m_formation;
            const DataAccess::Interface::Surface * m_surface;
            const DataAccess::Interface::Property * m_depthProperty;
            const DataAccess::Interface::Property * m_temperatureProperty;
            const DataAccess::Interface::Property * m_vesProperty;
            
            int m_firstI, m_lastI, m_firstJ, m_lastJ, m_numI, m_numJ;

            // user the project class to extract 
            // TimeIoTbl
            FastTouch * m_fastTouch;

            // structure to hold IoTbl data   
            struct Properties
            {
                Properties ():depth (0), temp (0), ves (0) { };
                double **depth;
                double **temp;
                double **ves;
            };
            // map to hold Properties in order of timesteps
            typedef map < double, Properties > BurialHistoryMap;
            
            BurialHistoryMap m_burialHistoryMap;
      
            // extract the relevant paleo data into a stl map 
            bool loadPaleoData (void);
            //void setToNull (void);
            void clearSnapshotMapMemory (void);

            // dissallowing copy ctor and assignment op
            BurialHistory (const BurialHistory & rhs);
            BurialHistory & operator= (const BurialHistory & rhs);

            /* TEMP TEST CODE */
            //fstream burialOutput;
        
        public:
            BurialHistory ( const DataAccess::Interface::Surface * surface,
                            const DataAccess::Interface::Formation * formation, FastTouch * ft );

            // return the filled map
            const BurialHistoryMap *returnAsMap (void);
            
            // convert the filled map into a struct array
            BurHistTimeStep *returnAsArray (const int &i, const int &j, int &validTimesteps, bool reverse);
            
            int maxTimeCount (void)
            {
                return m_burialHistoryMap.size ();
            }
            
            ~BurialHistory (void);

    private:
            BurHistTimeStep * m_burialHistoryTimestep;
            BurHistTimeStep * m_burialHistoryTsteptmp;
   };

//
// PUBLIC METHODS
//

/// The class constructor validates the input parameters and
/// instructs the relevant I/O operations to be carried out
/// so that all necessary I/O is done one time. 
template < class BurHistTimeStep >
BurialHistory< BurHistTimeStep >::BurialHistory(const Surface * surface, const Formation * formation,  
                              FastTouch * ft)
: m_formation        (formation), 
  m_surface          (surface), 
  m_fastTouch        (ft)
{
   m_depthProperty = m_fastTouch->findProperty ("Depth");
   m_temperatureProperty = m_fastTouch->findProperty ("Temperature");
   m_vesProperty = m_fastTouch->findProperty ("Ves");

   const Grid * activeGrid = m_fastTouch->getActivityOutputGrid ();

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

template < class BurHistTimeStep >
BurialHistory< BurHistTimeStep >::~BurialHistory(void)
{
   clearSnapshotMapMemory ();
   if ( m_burialHistoryTimestep )
   {
      delete [] m_burialHistoryTimestep;
   }
   if ( m_burialHistoryTsteptmp )
   {
      delete [] m_burialHistoryTsteptmp;
   }
   
   // CLOSE TEMP TEST FILE
   // burialOutput.close ();
}

template < class BurHistTimeStep >
const typename BurialHistory< BurHistTimeStep >::BurialHistoryMap * BurialHistory< BurHistTimeStep >::returnAsMap (void)
{
    return &m_burialHistoryMap;
}

/** This function converts the stl::map holding the burial history into the
 *  array needed by ResQ::calculate. An stl::map is used initially for
 *  gathering the data from the project file as it has necessary build in
 *  functionality. COnverting this data to the array at a later stage
 *  when it is needed externally can be done safely.
 */
template < class BurHistTimeStep >
BurHistTimeStep* BurialHistory< BurHistTimeStep >::returnAsArray (const int &i, const int &j, int &validTimesteps, bool reverse)
{
   Properties *p;
   validTimesteps = 0;
   
   typename BurialHistoryMap::reverse_iterator mapIt;
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
	 validTimesteps = 0;
         return 0;
      }
      // if one property present for timestep, then all should be as should be switched on 
      // before run.
      m_burialHistoryTimestep[validTimesteps].time        = mapIt->first;
      m_burialHistoryTimestep[validTimesteps].temperature = p->temp  [iArray][jArray];
      m_burialHistoryTimestep[validTimesteps].depth       = p->depth [iArray][jArray];
      m_burialHistoryTimestep[validTimesteps].effStress   = p->ves   [iArray][jArray] * PA_TO_MPA;
      m_burialHistoryTimestep[validTimesteps].waterSat    = 100;

      ++validTimesteps;
   }

   if (reverse)
   {
       for (int i = 0; i < validTimesteps; ++i)
       {
           m_burialHistoryTsteptmp[validTimesteps - i - 1].time        = m_burialHistoryTimestep[i].time;
           m_burialHistoryTsteptmp[validTimesteps - i - 1].temperature = m_burialHistoryTimestep[i].temperature;
           m_burialHistoryTsteptmp[validTimesteps - i - 1].depth       = m_burialHistoryTimestep[i].depth;
           m_burialHistoryTsteptmp[validTimesteps - i - 1].effStress   = m_burialHistoryTimestep[i].effStress;
           m_burialHistoryTsteptmp[validTimesteps - i - 1].waterSat    = m_burialHistoryTimestep[i].waterSat;
       }
       return validTimesteps > 0 ? m_burialHistoryTsteptmp : 0;
   }
   else
   {
       return validTimesteps > 0 ? m_burialHistoryTimestep : 0;
   }
}
/// This function collects 3d burial history.
/// The burial history is saved in an stl::map
/// which stores the data, sorting it by timestep. When all data is 
/// collected, the array needed to hold the data at as later stage, for
/// external use is dynamically allocated here.
template < class BurHistTimeStep >
bool BurialHistory < BurHistTimeStep >::loadPaleoData (void)
{
    PropertyValueList * propertyValues = m_fastTouch->getPropertyValues (SURFACE, 0, 0, 0, 0, m_surface, MAP);
 
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

    // initialise return array 
    m_burialHistoryTimestep = new BurHistTimeStep [m_burialHistoryMap.size()];
    m_burialHistoryTsteptmp = new BurHistTimeStep [m_burialHistoryMap.size()];
 
    return true;
}
template < class BurHistTimeStep >
void BurialHistory < BurHistTimeStep >::clearSnapshotMapMemory (void)
{
   Properties *p;

   for (typename BurialHistoryMap::iterator it = m_burialHistoryMap.begin (); it != m_burialHistoryMap.end (); ++it)
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
#endif
