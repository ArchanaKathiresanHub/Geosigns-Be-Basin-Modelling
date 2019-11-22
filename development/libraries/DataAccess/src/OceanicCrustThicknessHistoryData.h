//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_OCEANICCRUSTTHICKNESS_DATA_H
#define INTERFACE_OCEANICCRUSTTHICKNESS_DATA_H

// std library
#include <vector>
#include <string>

// DataAccess library
#include "DAObject.h"
#include "Interface.h"

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
      class GridMap;

      /// @class OceanicCrustThicknessHistoryData The Advanced Lithospheric Calculator (ALC) interface
      ///    which reads input data from the OceaCrustalThicknessIoTbl
      class OceanicCrustThicknessHistoryData : public DAObject
      {
         public:

            enum OceanicCrustThicknessMaps {
               BASALT_THICKNESS
            };

            OceanicCrustThicknessHistoryData( ProjectHandle& projectHandle, database::Record * record );
            virtual ~OceanicCrustThicknessHistoryData();

            /// @brief Create a map for the corresponding attribute if it doesn't exist yet
            virtual GridMap const * getMap( const OceanicCrustThicknessMaps attributeId = BASALT_THICKNESS ) const final;

            double getAge() const;

         private:
            static const std::vector<std::string> s_MapAttributeNames; ///< The names of the oceanic crustal thickness maps from the OceaCrustalThicknessIoTbl

      };
   }
}

#endif
