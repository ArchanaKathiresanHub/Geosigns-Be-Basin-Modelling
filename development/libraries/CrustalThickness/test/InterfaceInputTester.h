//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_INTERFACEINPUTTESTER_H
#define CRUSTALTHICKNESS_INTERFACEINPUTTESTER_H

// CrustalThickness library
#include "MockInterfaceInput.h"

// DataAccess library
#include "../../DataAccess/src/Interface/Interface.h"
#include "../../DataAccess/src/Interface/Snapshot.h"
#include "../../DataAccess/test/MockCrustalThicknessData.h"
#include "../../DataAccess/test/MockCrustalThicknessRiftingHistoryData.h"

// SerialDataAccess library
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

// Google test library
#include <gtest/gtest.h>

// std library
#include <vector>
#include <algorithm>
#include <memory>

using DataAccess::UnitTests::MockCrustalThicknessData;
using DataAccess::UnitTests::MockCrustalThicknessRiftingHistoryData;

using DataAccess::Interface::Snapshot;
using DataAccess::Interface::SnapshotList;

using DataAccess::Interface::FLEXURAL_BASIN;
using DataAccess::Interface::PASSIVE_MARGIN;
using DataAccess::Interface::ACTIVE_RIFTING;


namespace CrustalThickness
{
   namespace UnitTests
   {

      /// @class InterfaceInputTester Creates some data for the tests and define some functions to easily create the interface input
      class InterfaceInputTester : public ::testing::Test
      {
      public:
         /// @brief Initialise all the required by the McKenzieCalculator
         InterfaceInputTester();
         ~InterfaceInputTester();

         /// @brief Test that the provided interface inputs contains the expected data
         /// @details The values tested are the start ages, end ages, Ids, sea level adjustment
         ///    and maximum basalt thickness of the rift events.
         ///    Also the first rift age and flexural age are both tested
         /// @warning The expected vectors should be in the same order than the snapshot list
         ///    and have the same size than the snapshot list
         /// @param[in] flexuralAge             The age of the first flexural event
         /// @param[in] expectedCalculationMask The expected calculation masks of the rift events
         /// @param[in] expectedStartAge        The expected starting ages of the rift events
         /// @param[in] expectedEndAge          The expected ending ages of the rift events
         /// @param[in] expectedriftID          The expected rift ids of the rift events
         /// @param[in] expectedDeltaSL         The expected sea level adjustement of the rift events
         /// @param[in] expectedHBu             The expected maximum basalt thickness of the rift events
         /// @param[in] interfaceInput          The interface to test
         void EXPECT_RIFT_EQ( const double flexuralAge,
                              const std::vector< bool         >& expectedCalculationMask,
                              const std::vector< double       >& expectedStartAge,
                              const std::vector< double       >& expectedEndAge,
                              const std::vector< unsigned int >& expectedriftID,
                              const std::vector< double       >& expectedDeltaSL,
                              const std::vector< double       >& expectedHBu,
                              std::shared_ptr<InterfaceInput>    interfaceInput );

      protected:

         /// @brief Create a CTC interface input
         /// @details Set some default data to the produced CTC interface input
         /// @return A smart pointer a the new MockInterfaceInput object
         std::shared_ptr<MockInterfaceInput> createInterfaceInput();

         /// @defgroup UniTestData
         ///    Some data to use in the unit tests
         ///    Can be modified in the test fixtures (TEST_F)
         /// @{
         std::vector< DataAccess::Interface::TectonicFlag  > m_tectonicFalgs; ///< The list of tectonic flags

         int m_filterHalfWidth;                    ///< The smoothing radius
         double m_upperLowerContinentalCrustRatio; ///< The upper/lower continental crust ration
         double m_upperLowerOceanicCrustRatio;     ///< The upper/lower oceanic crust ration
         const double m_nDVd;                      ///< The no data value in double
         const unsigned int m_nDVi;                ///< The no data value in integer

         DataAccess::Interface::SerialGrid m_grid;                      ///< The main grid used in map construction
         std::vector< DataAccess::Interface::SerialGridMap > m_HBu;     ///< The list of maximum oceanic crustal thicknesses [m]
         std::vector< DataAccess::Interface::SerialGridMap > m_DeltaSL; ///< The list of sea level adjustment                [m]
         DataAccess::Interface::SerialGridMap* m_HLMuIni;               ///< The initial lithospheric mantle thickness       [m]
         DataAccess::Interface::SerialGridMap* m_HCuIni;                ///< The initial continental crust thickness         [m]

         std::vector<double> m_snapshots;                       ///< The list of snapshots [Ma]
         std::map< const double, bool> m_asSurfaceDepthHistory; ///< Maping between the snapshots age and the existence of a SDH at this age
         /// @}

         /// @defgroup MockDataAccess
         ///    Mocks the DataAccess library and stores the UniTestData
         /// @{
         std::shared_ptr< MockCrustalThicknessData > m_ctcGlobalData;                                   ///< The mock CTCIoTbl data
         std::vector< std::shared_ptr< const CrustalThicknessRiftingHistoryData> > m_ctcRiftingDataVec; ///< The list of mock CTCRiftingHistoryIoTbl data
         /// @}

         static constexpr int s_myRank = 0; ///< The unit tests are using serial data access, so we set the rank to 0

      private:
         /// @brief Updates the data stored in m_ctcGlobalData and in m_ctcRiftingDataVec
         ///    according to the current UniTestData
         void updateData();

      };
   }
}

#endif
