//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLECTC_H
#define INTERFACE_TABLECTC_H

// std library
#include <string>
#include <memory>

// DataAccess library
#include "database.h"
using database::Database;

namespace DataAccess
{

   namespace Interface
   {

      // forward declaration is needed to avoid circular includes
      class CrustalThicknessData;
      class ProjectHandle;

      /// @class TableCTC Contains all accessors to the [CTCIoTbl] data
      class TableCTC{

      public:

         /// @brief Loads the [CTCIoTbl]
         /// @details Calls the associated factory
         TableCTC( ProjectHandle& projectHandle );
         const std::shared_ptr<const CrustalThicknessData> data() const { return m_data; }

      private:

         const std::string m_name;                           ///< The table name (CTCIoTBl)
         std::shared_ptr<const CrustalThicknessData> m_data; ///< The data accessors (the unique line from CTCIoTBl)

      };
   }
}
#endif