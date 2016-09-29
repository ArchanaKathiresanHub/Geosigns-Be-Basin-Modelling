//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
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
#include "MockValidator.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

// Derived Properties library test utilities
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// Google test library
#include <gtest/gtest.h>

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

typedef std::shared_ptr<McKenzieCrustCalculator> McKenzieCrustCalculatorPtr;

/// @class McKenzieTester Creates some data for the tests and define some fuctions to easily create the calculators
class McKenzieTester : public ::testing::Test
{
public:

   McKenzieTester();
   ~McKenzieTester();

protected:

   /// @brief Initialise all the required by the McKenzieCalculator
   void initTestData();

   /// @brief Initialise the constants (to mimic configuration file)
   void initConstants( MockConfigFileParameterCtc& constants ) const;

   /// @brief Create a McKenzie calculator
   /// @details The outputData is filled with zeros
   ///   The constants are defined by the initialisation function but can still be modified afterward if needed
   /// @return A smart pointer to a McKenzieCrustCalculator object
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

   //// grids
   const DataAccess::Interface::SerialGrid* m_grid;
   DataAccess::Interface::SerialGridMap* m_HCuMap;                            ///< The map used to define the initial continental crustal thickness  (40Km by default  )
   DataAccess::Interface::SerialGridMap* m_HBuMap;                            ///< The map used to define the maximum oceanic crustal thickness      (4Km by default   )
   DataAccess::Interface::SerialGridMap* m_HLMuMap;                           ///< The map used to define initial lithospheric mantle thickness      (115Km by default )
   DataAccess::Interface::SerialGridMap* m_T0Map;                             ///< The map used to define the end of rifting                         (100Ma by default )
   DataAccess::Interface::SerialGridMap* m_TRMap;                             ///< The map used to define the end of rifting                         (60Ma by default  )
   DataAccess::Interface::SerialGridMap* m_previousContinentalCrustThickness; ///< The map used to define the previous continental crustal thickness (40000m by default)
   DataAccess::Interface::SerialGridMap* m_previousThinningFactor;            ///< The map used to define the previousthinning factor                (0 by default     )
   DataAccess::Interface::SerialGridMap* m_previousOceanicCrustThickness;     ///< The map used to define the previous oceanic crustal thickness     (200m by default  )

   // derived properties
   DataModel::MockDerivedSurfaceProperty* m_depthBasement; ///< Depth of the basement at 10Ma (5Km by default)

   // configuration file constants
   MockConfigFileParameterCtc m_constants;

   // input data
   MockInterfaceInput m_inputData;

   // ouput data array
   MockInterfaceOutput m_outputData;

   // global validator
   MockValidator m_validator;

};

#endif
