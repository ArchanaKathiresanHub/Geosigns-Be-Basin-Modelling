#ifndef FASTTOUCH7_BURIALHISTORY_H
#define FASTTOUCH7_BURIALHISTORY_H

#include <map>
#include <vector>

#include "BurialHistoryTimeStep.h"

/// The function of the Burial History class is to gather all 
/// burial history information relating to a specific surface from
/// the project file needed by ResQ for the ResQ calculations and hold it in 
/// a dynamic array which can be easily accessed by any class that
/// needs it 


static const double  PA_TO_MPA  = 1e-6;

namespace DataAccess
{
   namespace Interface
   {
      class Formation;
      class Surface;
      class Property;
      class ProjectHandle;
   }
}

namespace fasttouch
{
/** The function of the Burial History class is to gather all 
 *  burial history information relating to a specific surface from
 *  the project file needed by ResQ for the ResQ calculations and hold it in 
 *  a dynamic array which can be easily accessed by any class that
 *  needs it 
 */
    class BurialHistory
    {
       private:
          // structure to hold IoTbl data   
          struct Properties
          {
             Properties ():depth (0), temp (0), ves (0) { };
             double **depth;
             double **temp;
             double **ves;
          };
          // map to hold Properties in order of timesteps
          typedef std::map < double, Properties > BurialHistoryMap;

       public:
          BurialHistory ( const DataAccess::Interface::Surface * surface,
                DataAccess::Interface::ProjectHandle & projectHandle );

          // convert the filled map into a struct array
          const std::vector<BurialHistoryTimeStep> & returnAsArray(int i, int j, bool reverse);

          int maxTimeCount (void)
          {
             return m_burialHistoryMap.size ();
          }

          ~BurialHistory (void);

       private:
          std::vector<BurialHistoryTimeStep> m_burialHistoryTimestep;

          // these are used to match the reservoir
          // name to a layer

          const DataAccess::Interface::Surface * m_surface;
          const DataAccess::Interface::Property * m_depthProperty;
          const DataAccess::Interface::Property * m_temperatureProperty;
          const DataAccess::Interface::Property * m_vesProperty;

          int m_firstI, m_lastI, m_firstJ, m_lastJ, m_numI, m_numJ;

          // user the project class to extract 
          // TimeIoTbl
          DataAccess::Interface::ProjectHandle & m_projectHandle;


          BurialHistoryMap m_burialHistoryMap;

          // extract the relevant paleo data into a stl map 
          bool loadPaleoData (void);
          //void setToNull (void);
          void clearSnapshotMapMemory (void);

          // dissallowing copy ctor and assignment op
          BurialHistory (const BurialHistory & rhs);
          BurialHistory & operator= (const BurialHistory & rhs);
    };

}
#endif
