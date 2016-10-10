//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_CRUSTALTHICKNESSRIFTINGHISTORY_DATA_H
#define INTERFACE_CRUSTALTHICKNESSRIFTINGHISTORY_DATA_H

// std library
#include <vector>

// DataAccess library
#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

namespace DataAccess
{
   namespace Interface
   {

      /// @class CrustalThicknessRiftingHistoryData The CTC interface which reads input data from the CTCRiftingHistoryIoTbl
      class CrustalThicknessRiftingHistoryData : public DAObject
      {
         public:

            CrustalThicknessRiftingHistoryData( ProjectHandle * projectHandle, database::Record * record );
            virtual ~CrustalThicknessRiftingHistoryData();

            /// @return the tectonic flag as an enumeration
            virtual TectonicFlag getTectonicFlag() const;

            /// @return the tectonic flag as a string
            virtual const std::string& getTectonicFlagName() const;

            /// @brief Create a map for the corresponding attribute if it doesn't exist yet
            virtual GridMap const * getMap( const Interface::CTCRiftingHistoryMapAttributeId attributeId ) const;

         private:
            static const std::vector<std::string> s_MapAttributeNames; ///< The names of the CTC Rifting History maps from the CTCRiftingHistoryIoTbl

      };
   }
}

#endif
