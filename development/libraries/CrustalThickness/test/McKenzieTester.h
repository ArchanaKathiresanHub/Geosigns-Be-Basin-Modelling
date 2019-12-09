//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_MCKENZIETESTER_H
#define CRUSTALTHICKNESS_MCKENZIETESTER_H

#include "../src/McKenzieCrustCalculator.h"

// CrustalThickness library test utilities
#include "MockConfigFileParameterCtc.h"
#include "MockInterfaceInput.h"
#include "MockInterfaceOutput.h"

// DataModel library test utilities
#include "../../DataModel/test/MockValidator.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/SerialGrid.h"
#include "../../SerialDataAccess/src/SerialGridMap.h"

// Derived Properties library test utilities
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// Google test library
#include <gtest/gtest.h>

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

typedef std::shared_ptr<McKenzieCrustCalculator> McKenzieCrustCalculatorPtr;

namespace CrustalThickness
{
   namespace UnitTests
   {
      /// @class McKenzieTester Creates some data for the tests and define some functions to easily create the calculators
      ///    This class should be used as a test feature for google tests
      class McKenzieTester : public ::testing::Test
      {
      public:

         /// @brief Creates a tester object which derives from google test framework
         /// @details Initialize object members to their default values
         McKenzieTester();
         ~McKenzieTester();

      protected:

         /// @brief Initialise all the data required by the McKenzieCalculator
         void initTestData();

         /// @brief Initialise the constants (to mimic configuration file)
         void initConstants( MockConfigFileParameterCtc& constants ) const;

         /// @brief Create a McKenzie calculator
         /// @details The outputData is filled with zeros
         ///   The constants are defined by the initialisation function but can still be modified afterward if needed
         /// @return A smart pointer to a new McKenzieCrustCalculator object
         const McKenzieCrustCalculatorPtr createMcKenzieCalculator();

         // Global grid size variables (no gosth nodes)
         const unsigned int m_firstI;
         const unsigned int m_firstJ;
         const unsigned int m_lastI;
         const unsigned int m_lastJ;
         const double m_minI;
         const double m_minJ;
         const double m_maxI;
         const double m_maxJ;
         const unsigned int m_numI;
         const unsigned int m_numJ;

         double m_age;      ///< Age of computation for the McKenzie calculator       [Ma]
         double m_startAge; ///< Starting age of the rift for the current computation [Ma]
         double m_endAge;   ///< Ending age of the rift for the current computation   [Ma]

         double m_contRatio; ///< Continental crust upper/lower crust ratio
         double m_oceaRatio; ///< Oceanic crust upper/lower crust ratio

         riftingEvents m_riftingEvents; ///< The list of rifting events mapped to the snapshot ages

         const DataAccess::Interface::SerialGrid* m_grid;
         DataAccess::Interface::SerialGridMap* m_seaLevelAdjustment;                ///< The map used to define the sea level adjustement                  (0m  by default   ) [m]
         DataAccess::Interface::SerialGridMap* m_HCuMap;                            ///< The map used to define the initial continental crustal thickness  (40Km by default  ) [m]
         DataAccess::Interface::SerialGridMap* m_HBuMap;                            ///< The map used to define the maximum oceanic crustal thickness      (4Km by default   ) [m]
         DataAccess::Interface::SerialGridMap* m_HLMuMap;                           ///< The map used to define initial lithospheric mantle thickness      (115Km by default ) [m]
         DataAccess::Interface::SerialGridMap* m_previousRiftITS;                   ///< The map used to define the stacked ITS for the rift               (1000m by default ) [m]
         DataAccess::Interface::SerialGridMap* m_previousContinentalCrustThickness; ///< The map used to define the previous continental crustal thickness (40000m by default) [m]
         DataAccess::Interface::SerialGridMap* m_previousOceanicCrustThickness;     ///< The map used to define the previous oceanic crustal thickness     (200m by default  ) [m]

         DataModel::MockDerivedSurfaceProperty* m_depthBasement; ///< Depth of the basement at 10Ma (5Km by default) [m]

         MockConfigFileParameterCtc m_constants; ///< Configuration file constants

         DataAccess::Interface::ProjectHandle m_projectHandle;
         MockInterfaceInput  m_inputData;  ///< Input interface
         MockInterfaceOutput m_outputData; ///< Outuput interface (data are stored in a 3d boost array)

         MockValidator m_validator; ///< Global validator (does not depends on project handle)

      };
   }
}

#endif
