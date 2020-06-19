//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLECTCRIFTINGHISTORY_H
#define INTERFACE_TABLECTCRIFTINGHISTORY_H

// std library
#include <string>
#include <vector>
#include <memory>

namespace DataAccess
{

   namespace Interface
   {

      // forward declaration is needed to avoid circular include
      class CrustalThicknessRiftingHistoryData;
      class ProjectHandle;

      /// @class TableCTCRiftingHistory Contains all accessors to the [CTCRiftingHistoryIoTbl] data
      class TableCTCRiftingHistory{

      public:

         /// @brief Loads the [CTCRiftingHistoryIoTbl]
         /// @details Calls the associated factory
         explicit TableCTCRiftingHistory( ProjectHandle& projectHandle );
         const std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>>& data() const { return m_data; }

      private:

         const std::string m_name;                                                      ///< The table name (CTCRiftingHistoryIoTbl)
         std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>> m_data; ///< The data accessors (vector of lines from CTCRiftingHistoryIoTbl)

      };
   }
}
#endif
