#ifndef _FASTTOUCH_FASTTOUCH_H_
#define _FASTTOUCH_FASTTOUCH_H_

namespace database
{
   class Database;
   class Table;
   class Record;
}

#include "Interface/ProjectHandle.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

namespace DataAccess
{
    namespace Interface
    {
        class TouchstoneMap;
    }
}

using namespace DataAccess;

namespace fasttouch
{
    
    class MasterTouch;
    
    /** Top class of the FastTouch class hierarchy.
     *  Inherits from the DataAccess::Migrator class to provide easy access to Formations, Reservoirs, etc.
     */
    class FastTouch : public Interface::ProjectHandle
    {
        public:
            /**  Constructor. */
            FastTouch (database::Database * database, const string & name, const string & accessMode);
            static FastTouch *CreateFrom (const string & inputFileName);
            virtual ~FastTouch (void);
          
            bool saveTo (const string & outputFileName);
          
            bool removeResqPropertyValues (void);
          
            /// The top migration routine
            bool compute (void);
            
        private:
          
            bool addToComputationList (const Interface::TouchstoneMap * touchstoneMap);
            
            MasterTouch * m_masterTouch;
   };
}

#endif // _FASTTOUCH_FASTTOUCH_H
