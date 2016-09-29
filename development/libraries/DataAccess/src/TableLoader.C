//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface/TableLoader.h"

using namespace DataAccess::Interface;
using database::Table;
using database::Record;

void TableLoader::load( const ProjectHandle& projectHandle, const std::string& tableName, const std::function<void( Record* )> functor ){
   Table const * const table = projectHandle.getDataBase()->getTable( tableName );
   if (table == nullptr) {
      throw std::runtime_error( "Could not find the table [" + tableName + "]" );
   }
   else{
      database::Table::const_iterator tableIter;
      unsigned int index = 0;
      for (tableIter = table->begin(); tableIter != table->end(); ++tableIter)
      {
         Record* record = *tableIter;
         if (record == nullptr){
            throw std::runtime_error( "Could not find record " + std::to_string(index) + " for table [" + tableName + "]" );
         }
         functor( record );
         index++;
      }
   }
}
