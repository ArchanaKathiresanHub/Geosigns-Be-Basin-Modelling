//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLELOADER_H
#define INTERFACE_TABLELOADER_H

// std library
#include <string>
#include <functional>

// Forward declare
namespace database{
   class Record;
}

namespace DataAccess
{

   namespace Interface
   {
     
      // Forward declare
      class ProjectHandle;
      
      /// @class TableLoader Contains the static method to load an IoTbl
      class TableLoader{

         public:

            TableLoader() {};

            /// @brief Apply the given function to all the records (corresponding to the rows) of the specfied IoTbl
            /// @param[in] tableName The name of the IoTbl (i.e. CTCIoTbl)
            /// @param[in] functor The function which will be applied to the records (corresponding to the rows) of the table
            ///    Some basic functor classes are available to read simple tables: TableFunctorCompound and TableFunctorSimple
            static void load( const ProjectHandle& projectHandle, const std::string& tableName, const std::function<void( database::Record* )> functor );

      };
   }
}
#endif
