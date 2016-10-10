//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MOCK_CRUSTALTHICKNESSRIFTINGHISTORY_DATA_H
#define MOCK_CRUSTALTHICKNESSRIFTINGHISTORY_DATA_H

// std library
#include <map>

// DataAccess library
#include "Interface/CrustalThicknessRiftingHistoryData.h"

namespace DataAccess
{
   namespace UnitTests
   {

      /// @class CrustalThicknessRiftingHistoryData The CTC interface which reads input data from the CTCRiftingHistoryIoTbl
      class MockCrustalThicknessRiftingHistoryData : public CrustalThicknessRiftingHistoryData
      {
         public:

            MockCrustalThicknessRiftingHistoryData() : CrustalThicknessRiftingHistoryData( nullptr, nullptr ),
               m_flagName( "placeholder" ){};
            virtual ~MockCrustalThicknessRiftingHistoryData() {};

            /// @defgroup Accessors
            /// @brief Use it to get CTCIoTbl data in your unit tests
            /// @ Return the tectonic flag as an enumeration
            virtual Interface::TectonicFlag getTectonicFlag() const final
            {
               return m_tectonicFlag;
            }
            /// @ Return an "placeholder" string so the test do not fail because of the CTC loging
            virtual const std::string& getTectonicFlagName() const final {
               return m_flagName;
            };
            /// @brief Create a map for the corresponding attribute if it doesn't exist yet
            virtual Interface::GridMap const * getMap( const Interface::CTCRiftingHistoryMapAttributeId attributeId ) const final
            {
               return m_ctcRiftingHistoryIoTblMaps.find(attributeId)->second;
            }
            /// @}

            /// @defgroup Mutators
            /// @brief Use it to set CTCIoTbl data in your unit tests
            /// @{
            void setTectonicFlag( const Interface::TectonicFlag flag )
            {
               m_tectonicFlag = flag;
            }
            void setMap( const Interface::CTCRiftingHistoryMapAttributeId attributeId, Interface::GridMap const * map )
            {
               m_ctcRiftingHistoryIoTblMaps[attributeId] = map;
            }
            ///@}

         private:
            Interface::TectonicFlag m_tectonicFlag; ///< The tectonic flag
            const std::string m_flagName;           ///< The tectonic flag name which is used just for loging purposes
            /// Stores the input maps (maximum oceanic crust thickness and sea level adjustment)
            std::map< const Interface::CTCRiftingHistoryMapAttributeId, Interface::GridMap const * > m_ctcRiftingHistoryIoTblMaps;


      };
   }
}

#endif
