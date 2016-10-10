//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "InterfaceInputTester.h"

// utilites library
#include "../../utilities/test/GoogleTestMacros.h"

// std library
#include <exception>
#include <string>

using CrustalThickness::UnitTests::InterfaceInputTester;

TEST_F( InterfaceInputTester, constructor ){
   //1. Test that exception is thrown when the ctc data is a null pointer
   std::invalid_argument exception1( "No crustal thickness data provided to the CTC");
   EXPECT_EXCEPTION_EQ( InterfaceInput( nullptr, m_ctcRiftingDataVec ), exception1 )

   //2. Test that exception is thrown when the ctc rifting history data is empty
   std::invalid_argument exception2( "No crustal thickness rifting history data provided to the CTC");
   EXPECT_EXCEPTION_EQ( InterfaceInput( m_ctcGlobalData, {} ), exception2 )

   //3. Test that exception is thrown when the ctc rifting history data has a null pointer
   m_ctcRiftingDataVec.push_back( nullptr );
   std::invalid_argument exception3( "The crustal thickness rifting event data number " +
      std::to_string( m_ctcRiftingDataVec.size()-1 ) + " provided to the CTC is corrupted" );
   EXPECT_EXCEPTION_EQ( InterfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec ), exception3 )

   //4. Check default values
   m_ctcRiftingDataVec.pop_back();
   InterfaceInput interfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec );

   //a. from interface input object
   EXPECT_EQ( 0      , interfaceInput.getSmoothRadius()               );
   EXPECT_EQ( 0      , interfaceInput.getFlexuralAge()                );
   EXPECT_EQ( 0      , interfaceInput.getFirstRiftAge()               );
   EXPECT_EQ( 0      , interfaceInput.getContinentalCrustRatio()      );
   EXPECT_EQ( 0      , interfaceInput.getOceanicCrustRatio()          );
   EXPECT_EQ( nullptr, interfaceInput.getPressureBasement()           );
   EXPECT_EQ( nullptr, interfaceInput.getPressureWaterBottom()        );
   EXPECT_EQ( nullptr, interfaceInput.getPressureMantle()             );
   EXPECT_EQ( nullptr, interfaceInput.getPressureMantleAtPresentDay() );
   EXPECT_EQ( nullptr, interfaceInput.getDepthBasement()              );
   EXPECT_EQ( nullptr, interfaceInput.getDepthWaterBottom()           );
   EXPECT_EQ( nullptr, interfaceInput.getTopOfSedimentSurface()       );
   EXPECT_EQ( nullptr, interfaceInput.getBotOfSedimentSurface()       );
   // check that HCuMap is a null pointer
   //test if the exception is thrown
   std::runtime_error exception4( "Undefined initial crust thickness map");
   EXPECT_EXCEPTION_EQ( interfaceInput.getHCuMap(), exception4 )
   // check that HLMuMap is a null pointer
   //test if the exception is thrown
   std::runtime_error exception5("Undefined initial lithospheric mantle thickness map");
   EXPECT_EXCEPTION_EQ( interfaceInput.getHLMuMap(), exception5 )

   //b. from configuration file
   EXPECT_EQ( 0, interfaceInput.getInitialSubsidence()                          );
   EXPECT_EQ( 0, interfaceInput.getBackstrippingMantleDensity()                 );
   EXPECT_EQ( 0, interfaceInput.getWaterDensity()                               );
   EXPECT_EQ( 0, interfaceInput.getConstants().getCoeffThermExpansion()         );
   EXPECT_EQ( 0, interfaceInput.getConstants().getInitialSubsidenceMax()        );
   EXPECT_EQ( 0, interfaceInput.getConstants().getE0()                          );
   EXPECT_EQ( 0, interfaceInput.getConstants().getTau()                         );
   EXPECT_EQ( 0, interfaceInput.getConstants().getModelTotalLithoThickness()    );
   EXPECT_EQ( 0, interfaceInput.getConstants().getBackstrippingMantleDensity()  );
   EXPECT_EQ( 0, interfaceInput.getConstants().getLithoMantleDensity()          );
   EXPECT_EQ( 0, interfaceInput.getConstants().getBaseLithosphericTemperature() );
   EXPECT_EQ( 0, interfaceInput.getConstants().getReferenceCrustThickness()     );
   EXPECT_EQ( 0, interfaceInput.getConstants().getReferenceCrustDensity()       );
   EXPECT_EQ( 0, interfaceInput.getConstants().getWaterDensity()                );
   EXPECT_EQ( 0, interfaceInput.getConstants().getA()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getB()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getC()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getD()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getE()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getF()                           );
   EXPECT_EQ( 0, interfaceInput.getConstants().getDecayConstant()               );

   //c. debug parameter
   EXPECT_EQ( "", interfaceInput.getBaseRiftSurfaceName() );

}

TEST_F( InterfaceInputTester, loadConfigurationFile ){

   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );
   EXPECT_EQ( 4055   , interfaceInput->getInitialSubsidence()                          );
   EXPECT_EQ( 3330   , interfaceInput->getBackstrippingMantleDensity()                 );
   EXPECT_EQ( 1030   , interfaceInput->getWaterDensity()                               );
   EXPECT_EQ( 3.45E-5, interfaceInput->getConstants().getCoeffThermExpansion()         );
   EXPECT_EQ( 4055,    interfaceInput->getConstants().getInitialSubsidenceMax()        );
   EXPECT_EQ( 2656,    interfaceInput->getConstants().getE0()                          );
   EXPECT_EQ( 42.9,    interfaceInput->getConstants().getTau()                         );
   EXPECT_EQ( 92500,   interfaceInput->getConstants().getModelTotalLithoThickness()    );
   EXPECT_EQ( 3330,    interfaceInput->getConstants().getBackstrippingMantleDensity()  );
   EXPECT_EQ( 3360,    interfaceInput->getConstants().getLithoMantleDensity()          );
   EXPECT_EQ( 1330,    interfaceInput->getConstants().getBaseLithosphericTemperature() );
   EXPECT_EQ( 35000,   interfaceInput->getConstants().getReferenceCrustThickness()     );
   EXPECT_EQ( 2875,    interfaceInput->getConstants().getReferenceCrustDensity()       );
   EXPECT_EQ( 1030,    interfaceInput->getConstants().getWaterDensity()                );
   EXPECT_EQ( 2.9881,  interfaceInput->getConstants().getA()                           );
   EXPECT_EQ( 1050,    interfaceInput->getConstants().getB()                           );
   EXPECT_EQ( 280,     interfaceInput->getConstants().getC()                           );
   EXPECT_EQ( -294000, interfaceInput->getConstants().getD()                           );
   EXPECT_EQ( 2830,    interfaceInput->getConstants().getE()                           );
   EXPECT_EQ( 2830,    interfaceInput->getConstants().getF()                           );
   EXPECT_EQ( 10000,   interfaceInput->getConstants().getDecayConstant()               );
}

TEST_F( InterfaceInputTester, loadCTCIoTbl ){

   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );
   EXPECT_EQ( 40000 , interfaceInput->getHCuMap().getConstantValue()  );
   EXPECT_EQ( 110000, interfaceInput->getHLMuMap().getConstantValue() );
   EXPECT_EQ( 10    , interfaceInput->getSmoothRadius()               );
   EXPECT_EQ( 0.5   , interfaceInput->getContinentalCrustRatio()      );
   EXPECT_EQ( 0.7   , interfaceInput->getOceanicCrustRatio()          );

}

TEST_F( InterfaceInputTester, loadCTCIoTbl_exceptions ){

   //1. Test that the interface throws an exception when the continental crust ratio is negative
   m_upperLowerContinentalCrustRatio = -1;
   std::shared_ptr<InterfaceInput> interfaceInput1 = createInterfaceInput();
   std::invalid_argument exception1( "The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is negative");
   EXPECT_EXCEPTION_EQ( interfaceInput1->loadInputData( "InterfaceData.cfg" ), exception1 )
   m_upperLowerContinentalCrustRatio = 0.5;

   //2. Test that the interface throws an exception when the oceanic crust ratio is negative
   m_upperLowerOceanicCrustRatio     = -1;
   std::shared_ptr<InterfaceInput> interfaceInput2 = createInterfaceInput();
   std::invalid_argument exception2( "The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is negative");
   EXPECT_EXCEPTION_EQ( interfaceInput2->loadInputData( "InterfaceData.cfg" ), exception2 )
   m_upperLowerOceanicCrustRatio = 0.7;

   //3. Test that the interface throws an exception when the HCu map is a null pointer
   auto tempHCuIni = m_HCuIni;
   m_HCuIni = nullptr;
   std::shared_ptr<InterfaceInput> interfaceInput3 = createInterfaceInput();
   std::invalid_argument exception3( "The initial crustal thickness map maps cannot be retreived by the interface input" );
   EXPECT_EXCEPTION_EQ( interfaceInput3->loadInputData( "InterfaceData.cfg" ), exception3 )
   m_HCuIni = tempHCuIni;

   //4. Test that the interface throws an exception when the HLMu map is a null pointer
   auto tempHLMuIni = m_HLMuIni;
   m_HLMuIni = nullptr;
   std::shared_ptr<InterfaceInput> interfaceInput4 = createInterfaceInput();
   std::invalid_argument exception4( "The initial lithospheric mantle thickness map maps cannot be retreived by the interface input");
   EXPECT_EXCEPTION_EQ( interfaceInput4->loadInputData( "InterfaceData.cfg" ), exception4 )
   m_HLMuIni = tempHLMuIni;

   //5. Test that the interface throws an exception when smoothing radius is negative
   m_filterHalfWidth = -10;
   std::shared_ptr<InterfaceInput> interfaceInput5 = createInterfaceInput();
   std::invalid_argument exception5( "The smoothing radius is set to a negative value");
   EXPECT_EXCEPTION_EQ( interfaceInput5->loadInputData( "InterfaceData.cfg" ), exception5 )
   m_filterHalfWidth = 5;
}


TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_simpleRift_startPassive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge = {   50,  50,      50,  50, m_nDVd,  m_nDVd };
   const std::vector<const double      > expectedEndAge   = {   10,  10,      10,  10, m_nDVd,  m_nDVd };
   const std::vector<const unsigned int> expectedriftID   = {    1,   1,       1,   1,      0,       0 };
   const std::vector<const double      > expectedDeltaSL  = { 235,  300, -564.45, 235,      0, -562.45 };
   const std::vector<const double      > expectedHBu      = { 578,    0,    7468, 5120,  3540,    7468 };

   EXPECT_RIFT_EQ( 50, 0, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}


TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_simpleRift_startActive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge( 6, 253.12 );
   const std::vector<const double      > expectedEndAge  ( 6, 20     );
   const std::vector<const unsigned int> expectedriftID  ( 6, 1      );
   const std::vector<const double      > expectedDeltaSL  = { 235,  300, -564.45, 235,      0, -562.45 };
   const std::vector<const double      > expectedHBu      = { 578,    0,    7468, 5120,  3540,    7468 };


   EXPECT_RIFT_EQ( 253.12, 0, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_simpleRift_uniqueActive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, ACTIVE_RIFTING };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge = { m_nDVd, m_nDVd,  m_nDVd, m_nDVd, 253.12,  253.12 };
   const std::vector<const double      > expectedEndAge   = { m_nDVd, m_nDVd,  m_nDVd, m_nDVd,    100,     100 };
   const std::vector<const unsigned int> expectedriftID   = { m_nDVi, m_nDVi,  m_nDVi, m_nDVi,      1,       1 };
   const std::vector<const double      > expectedDeltaSL  = {    235,    300, -564.45,    235,      0, -562.45 };
   const std::vector<const double      > expectedHBu      = {    578,      0,    7468,   5120,   3540,    7468 };

   EXPECT_RIFT_EQ( 253.12, 100, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_simpleRift_onlyActive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge( 6, 253.12 );
   const std::vector<const double      > expectedEndAge  ( 6, 0      );
   const std::vector<const unsigned int> expectedriftID  ( 6, 1      );
   const std::vector<const double      > expectedDeltaSL  = {    235, 300, -564.45,    235,      0, -562.45 };
   const std::vector<const double      > expectedHBu      = {    578,   0,    7468,    5120,  3540,    7468 };

   EXPECT_RIFT_EQ( 253.12, 0, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_multiRift_startActive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, ACTIVE_RIFTING, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge = { m_nDVd,  20,      20, 253.12, 253.12,  253.12 };
   const std::vector<const double      > expectedEndAge   = { m_nDVd,  10,      10,     50,     50,      50 };
   const std::vector<const unsigned int> expectedriftID   = { m_nDVi,   2,       2,      1,      1,       1 };
   const std::vector<const double      > expectedDeltaSL  = {    235, 300, -564.45,    235,      0, -562.45 };
   const std::vector<const double      > expectedHBu      = {    578,   0,    7468,    5120,  3540,    7468 };

   EXPECT_RIFT_EQ( 253.12, 10, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_multiRift_startPassive ){

   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, ACTIVE_RIFTING, PASSIVE_MARGIN, ACTIVE_RIFTING, PASSIVE_MARGIN };
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData( "InterfaceData.cfg" );

   const std::vector<const double      > expectedStartAge = { 20,    20,      20,  100,   100,  m_nDVd };
   const std::vector<const double      > expectedEndAge   = { 10,    10,      10,   50,    50,  m_nDVd };
   const std::vector<const unsigned int> expectedriftID   = {   2,    2,       2,    1,     1,       0 };
   const std::vector<const double      > expectedDeltaSL  = { 235,  300, -564.45,  235,     0, -562.45 };
   const std::vector<const double      > expectedHBu      = { 578,    0,    7468, 5120,  3540,    7468 };

   EXPECT_RIFT_EQ( 100, 0, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );

}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_exceptions ){
   //1. Test that the interface throws an exception when the size if the CTCRiftingHistoryIoTbl is
   //      different form the number of major snapshots
   m_snapshots.push_back( 300.0 );
   std::shared_ptr<InterfaceInput> interfaceInput1 = createInterfaceInput();
   std::runtime_error expectedException1( "The number of snpashots (" + std::to_string( m_snapshots.size() )
      + ") differ from the number of rifting events (" + std::to_string( m_ctcRiftingDataVec.size() ) + ")" );
   EXPECT_EXCEPTION_EQ( interfaceInput1->loadInputData( "InterfaceData.cfg" ), expectedException1 );
   m_snapshots.pop_back();
 
   //2. Test that the interface throws an exception when there is no rifting event for the requested age
   const double age = 2500;
   std::shared_ptr<InterfaceInput> interfaceInput2 = createInterfaceInput();
   std::runtime_error exception2( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   interfaceInput2->loadInputData( "InterfaceData.cfg" );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftingStartAge( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftingEndAge  ( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftId         ( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getDeltaSLMap     ( age ).getConstantValue(), exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getHBuMap         ( age ).getConstantValue(), exception2 );

   //3.  Test that the interface throws an exception when there is no active rifting event defined
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::invalid_argument exception3( "There is no active rifting event defined in the rifting history" );
   std::shared_ptr<InterfaceInput> interfaceInput3 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput3->loadInputData( "InterfaceData.cfg" ), exception3);
  
   //4.  Test that the interface throws an exception when there are active events after the flexural age
   m_tectonicFalgs = { FLEXURAL_BASIN, ACTIVE_RIFTING, ACTIVE_RIFTING, FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::invalid_argument exception4( "An active rifting event is defined after a flexural event" );
   std::shared_ptr<InterfaceInput> interfaceInput4 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput4->loadInputData( "InterfaceData.cfg" ), exception4 );
   
   //5.  Test that the interface throws an exception when there are passive events after the flexural age
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::invalid_argument exception5( "A passive margin event is defined after a flexural event" );
   std::shared_ptr<InterfaceInput> interfaceInput5 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput5->loadInputData( "InterfaceData.cfg" ), exception5 );
   
   //6.  Test that the interface throws an exception when the first event is flexural
   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN };
   std::invalid_argument exception6( "There is no active rifting event defined in the rifting history" );
   std::shared_ptr<InterfaceInput> interfaceInput6 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput6->loadInputData( "InterfaceData.cfg" ), exception6 );

   //7.  Test that the interface throws an exception when there is no flexural event
   m_tectonicFalgs = { PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::invalid_argument exception7( "There is no flexural event defined in the rifting history" );
   std::shared_ptr<InterfaceInput> interfaceInput7 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput7->loadInputData( "InterfaceData.cfg" ), exception7 );

}
