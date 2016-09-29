//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_CRUSTALTHICKNESS_DATA_H
#define INTERFACE_CRUSTALTHICKNESS_DATA_H

// std library
#include <vector>
#include <memory>

// TableIO library
#include "database.h"

// DataAccess library
#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"

namespace DataAccess
{
   namespace Interface
   {

      /// @class CrustalThicknessData The CTC interface which reads input data from the CTCIoTbl
      class CrustalThicknessData : public DAObject
      {
         public:
            CrustalThicknessData (ProjectHandle * projectHandle, database::Record * record);
            virtual ~CrustalThicknessData (void);

            /// @defgroup GeneralInputs
            /// @brief Are defined in the project file via the UI
            /// @{
            /// @brief Return the Initial Crustal Thickness
            double getHCuIni() const;
            /// @brief Return the Initial mantle thickness
            double getHLMuIni() const;
            /// @brief Return the Filter width (half), this is the smoothing radius
            int getFilterHalfWidth() const;
            /// @}

            /// @defgroup DebugInputs
            /// @brief Are defined in the project file manually (R&D only)
            /// @{
            /// @brief Return the ratio used to define the lower and upper part of the continental crust (r=upper/low)
            double getUpperLowerContinentalCrustRatio() const;
            /// @brief Return the ratio used to define the lower and upper part of the oceanic crust (r=upper/low)
            double getUpperLowerOceanicCrustRatio() const;
            /// @}

            /// @brief Create a map for the corresponding attribute if it doesn't exist yet
            GridMap const * getMap( const Interface::CTCMapAttributeId attributeId ) const;

         private:
            static const std::vector<std::string> s_MapAttributeNames;   ///< The names of the CTC maps from the CTCIoTbl
      };
   }
}


#endif // INTERFACE_CRUSTALTHICKNESS_DATA_H
