#ifndef _FASTTOUCH_FASTTOUCH_H_
#define _FASTTOUCH_FASTTOUCH_H_

#include "Interface/ProjectHandle.h"
#include "MasterTouch.h"

#include <string>
#include <vector>
#include <memory>

namespace database
{
   class Database;
   class Table;
   class Record;
}


namespace DataAccess
{
   namespace Interface
   {
      class TouchstoneMap;
      class ObjectFactory;
   }
}

namespace fasttouch
{

   class MasterTouch;

   const std::string FastTouchActivityName = "FastTouch";

   /** Top class of the FastTouch class hierarchy.
    *  Inherits from the DataAccess::Migrator class to provide easy access to Formations, Reservoirs, etc.
    */
   class FastTouch 
   {
      public:
         /**  Constructor. */
         FastTouch (const std::string & inputFileName, DataAccess::Interface::ObjectFactory* factory);
         virtual ~FastTouch (void);

         bool saveTo (const std::string & outputFileName);

         bool removeResqPropertyValues (void);

         /// The top migration routine
         bool compute (void);

         bool mergeOutputFiles ( );

      private:

         bool addToComputationList (const DataAccess::Interface::TouchstoneMap * touchstoneMap);

         std::unique_ptr<DataAccess::Interface::ProjectHandle> m_projectHandle;
         MasterTouch m_masterTouch;
   };
}

#endif // _FASTTOUCH_FASTTOUCH_H
