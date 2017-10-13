//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_TABLEOCEANICCRUSTTHICKNESSHISTORY_H
#define INTERFACE_TABLEOCEANICCRUSTTHICKNESSHISTORY_H

// std library
#include <string>
#include <vector>
#include <memory>

namespace DataAccess
{

   namespace Interface
   {

      // forward declaration is needed to avoid circular include
      class OceanicCrustThicknessHistoryData;
      class ProjectHandle;

      /// @class TableOceanicCrustThicknessHistory Contains all accessors to the [OceaCrustalThicknessIoTbl] data
      class TableOceanicCrustThicknessHistory {

      public:

         /// @brief Loads the [OceaCrustalThicknessIoTbl]
         /// @details Calls the associated factory
         TableOceanicCrustThicknessHistory( ProjectHandle& projectHandle );
         const std::vector<std::shared_ptr<const OceanicCrustThicknessHistoryData>>& data() const { return m_data; }

      private:

         const std::string m_name;                                                    ///< The table name (OceaCrustalThicknessIoTbl)
         std::vector<std::shared_ptr<const OceanicCrustThicknessHistoryData>> m_data; ///< The data accessors (vector of lines from OceaCrustalThicknessIoTbl)

      };
   }
}
#endif