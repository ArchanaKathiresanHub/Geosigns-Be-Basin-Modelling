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

// TableIO library
#include "database.h"

// DataAccess library
#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"

using DataAccess::Interface::SnapshotList;

namespace DataAccess
{
   namespace Interface
   {

      /// @class CrustalThicknessData The CTC interface which reads input data from the CTCIoTbl
      /// @details Also contains an utility to load the snapshots as a list of smart pointers from the project handle
      class CrustalThicknessData : public DAObject
      {
         public:
            CrustalThicknessData (ProjectHandle * projectHandle, database::Record * record);
            virtual ~CrustalThicknessData ();

            /// @defgroup GeneralInputs
            /// @brief Are defined in the project file via the UI
            /// @{
            /// @brief Return the filter half width, this is the smoothing radius
            virtual int getFilterHalfWidth() const;
            /// @brief Create a map for the corresponding attribute if it doesn't exist yet
            virtual GridMap const * getMap( const Interface::CTCMapAttributeId attributeId ) const;
            /// @brief Return the stratigraphic snapshots
            /// @details Detucted from the formations stored by the project handle
            virtual const std::vector<const double>& getSnapshots() const;
            /// @}

            /// @defgroup DebugInputs
            /// @brief Are defined in the project file manually (R&D only)
            /// @{
            /// @brief Return the ratio used to define the lower and upper part of the continental crust (r=upper/low)
            virtual double getUpperLowerContinentalCrustRatio() const;
            /// @brief Return the ratio used to define the lower and upper part of the oceanic crust (r=upper/low)
            virtual double getUpperLowerOceanicCrustRatio() const;
            /// @brief Return the name of a base of syn-rift 
            virtual const std::string& getSurfaceName() const;
            /// @}

         private:
            static const std::vector<std::string> s_MapAttributeNames; ///< The names of the CTC maps from the CTCIoTbl
      };
   }
}


#endif // INTERFACE_CRUSTALTHICKNESS_DATA_H
