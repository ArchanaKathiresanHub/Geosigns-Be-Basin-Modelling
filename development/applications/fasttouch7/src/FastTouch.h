//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef _FASTTOUCH_FASTTOUCH_H_
#define _FASTTOUCH_FASTTOUCH_H_

#include "ProjectHandle.h"
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
