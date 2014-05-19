#ifndef __burialhistory__
#define __burialhistory__

#ifdef __INTEL_COMPILER // disable warning "type qualifier on return type is meaningless" in tslibI.h
#pragma warning push
#pragma warning(disable:858)
#endif

#include "tslibI.h"

#ifdef __INTEL_COMPILER
#pragma warning pop
#endif

#include <fstream>

#include <string>
#include <map>
using namespace std;


namespace DataAccess
{
   namespace Interface
   {
      class Formation;
      class Surface;
      class Property;
   }
}

namespace fasttouch
{
    class FastTouch;

/** The function of the Burial History class is to gather all 
 *  burial history information relating to a specific surface from
 *  the project file needed by ResQ for the ResQ calculations and hold it in 
 *  a dynamic array which can be easily accessed by any class that
 *  needs it 
 */
    
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
            Geocosm::TsLib::burHistTimestep *returnAsArray (const int &i, const int &j, int &validTimesteps);
            
            int maxTimeCount (void)
            {
                return m_burialHistoryMap.size ();
            }
            
            ~BurialHistory (void);

    private:
            Geocosm::TsLib::burHistTimestep * m_burialHistoryTimestep;
   };
}
#endif
