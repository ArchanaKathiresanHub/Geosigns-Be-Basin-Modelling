//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file InterfaceInput.C This file tests the CTC interface input by checking:
///    # The construction of the interface object and its possible exceptions
///    # The loading of the configuration file and its possible exceptions
///    # The loading of the CTCIoTbl and its possible exceptions
///    # The loading and interpreation of the CTCRiftingHistoryIoTbl and its possible exceptions
///      The details of the CTCRiftingHistoryIoTbl unit tests are documented in an excel file
///      located @ https://sps.sede-coe.pds.nl/COE-II/Basin%20Modeling/_layouts/xlviewer.aspx?id=/COE-II/Basin%20Modeling/Development/CTC%20upgrade/CTCHistoryIoTbl_UniTests.xlsx&Source=https%3A%2F%2Fsps%2Esede%2Dcoe%2Epds%2Enl%2FCOE%2DII%2FBasin%2520Modeling%2FDevelopment%2FForms%2FAllItems%2Easpx%3FRootFolder%3D%252FCOE%252DII%252FBasin%2520Modeling%252FDevelopment%252FCTC%2520upgrade%26FolderCTID%3D0x01200075835670C4A4424D851CD5CC38F7B3E5%26View%3D%7B611C9535%2DA717%2D4886%2DAEF3%2D7E8F9A1B43AB%7D%26InitialTabId%3DRibbon%252EDocument%26VisibilityContext%3DWSSTabPersistence&DefaultItemOpen=1&DefaultItemOpen=1

#include "InterfaceInputTester.h"

// utilites library
#include "../../utilities/test/GoogleTestMacros.h"
#include "../../utilities/src/LogHandler.h"

// std library
#include <exception>
#include <string>

using CrustalThickness::UnitTests::InterfaceInputTester;

TEST_F( InterfaceInputTester, start_log ) {
   //starts the log handler in order to debug unit tests more easily
   LogHandler( "InterfaceInputUnitTests", LogHandler::DETAILED_LEVEL, s_myRank );
}

TEST_F( InterfaceInputTester, constructor ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Interface Input Constructor Tests";

   //1. Test that exception is thrown when the ctc data is a null pointer
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that exception is thrown when the ctc data is a null pointer";
   std::invalid_argument exception1( "No crustal thickness data provided to the CTC");
   EXPECT_EXCEPTION_EQ( InterfaceInput( nullptr, m_ctcRiftingDataVec ), exception1 )

   //2. Test that exception is thrown when the ctc rifting history data is empty
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that exception is thrown when the ctc rifting history data is empty";
   std::invalid_argument exception2( "No crustal thickness rifting history data provided to the CTC");
   EXPECT_EXCEPTION_EQ( InterfaceInput( m_ctcGlobalData, {} ), exception2 )

   //3. Test that exception is thrown when the ctc rifting history data has a null pointer
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that exception is thrown when the ctc rifting history data has a null pointer";
   m_ctcRiftingDataVec.push_back( nullptr );
   std::invalid_argument exception3( "The crustal thickness rifting event data number " +
      std::to_string( m_ctcRiftingDataVec.size()-1 ) + " provided to the CTC is corrupted" );
   EXPECT_EXCEPTION_EQ( InterfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec ), exception3 )

   //4. Check default values
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Check default values of the constructor";
   m_ctcRiftingDataVec.pop_back();
   InterfaceInput interfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec );

   //a. from interface input object
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "check default values from interface input object";
   EXPECT_EQ( 0      , interfaceInput.getSmoothRadius()               );
   EXPECT_EQ( 0      , interfaceInput.getFlexuralAge()                );
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
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "check default values from configuration file";
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
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "check debug parameters";
   EXPECT_EQ( "", interfaceInput.getBaseRiftSurfaceName() );
}

TEST_F( InterfaceInputTester, loadConfigurationFile ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load Configuration File Tests";
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();
   EXPECT_EQ( 4055   , interfaceInput->getInitialSubsidence()                          );
   EXPECT_EQ( 3205   , interfaceInput->getBackstrippingMantleDensity()                 );
   EXPECT_EQ( 1035   , interfaceInput->getWaterDensity()                               );
   EXPECT_EQ( 3.45E-5, interfaceInput->getConstants().getCoeffThermExpansion()         );
   EXPECT_EQ( 4055,    interfaceInput->getConstants().getInitialSubsidenceMax()        );
   EXPECT_EQ( 2656,    interfaceInput->getConstants().getE0()                          );
   EXPECT_EQ( 42.9,    interfaceInput->getConstants().getTau()                         );
   EXPECT_EQ( 92500,   interfaceInput->getConstants().getModelTotalLithoThickness()    );
   EXPECT_EQ( 3205,    interfaceInput->getConstants().getBackstrippingMantleDensity()  );
   EXPECT_EQ( 3360,    interfaceInput->getConstants().getLithoMantleDensity()          );
   EXPECT_EQ( 1330,    interfaceInput->getConstants().getBaseLithosphericTemperature() );
   EXPECT_EQ( 35000,   interfaceInput->getConstants().getReferenceCrustThickness()     );
   EXPECT_EQ( 2875,    interfaceInput->getConstants().getReferenceCrustDensity()       );
   EXPECT_EQ( 1035,    interfaceInput->getConstants().getWaterDensity()                );
   EXPECT_EQ( 2.9881,  interfaceInput->getConstants().getA()                           );
   EXPECT_EQ( 1050,    interfaceInput->getConstants().getB()                           );
   EXPECT_EQ( 280,     interfaceInput->getConstants().getC()                           );
   EXPECT_EQ( -294000, interfaceInput->getConstants().getD()                           );
   EXPECT_EQ( 2830,    interfaceInput->getConstants().getE()                           );
   EXPECT_EQ( 2830,    interfaceInput->getConstants().getF()                           );
   EXPECT_EQ( 10000,   interfaceInput->getConstants().getDecayConstant()               );
}

TEST_F( InterfaceInputTester, loadCTCIoTbl ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCIoTbl Tests";
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();
   EXPECT_EQ( 40000 , interfaceInput->getHCuMap().getConstantValue()  );
   EXPECT_EQ( 110000, interfaceInput->getHLMuMap().getConstantValue() );
   EXPECT_EQ( 10    , interfaceInput->getSmoothRadius()               );
   EXPECT_EQ( 0.5   , interfaceInput->getContinentalCrustRatio()      );
   EXPECT_EQ( 0.7   , interfaceInput->getOceanicCrustRatio()          );
}

TEST_F( InterfaceInputTester, loadCTCIoTbl_exceptions ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCIoTbl exception Tests";

   //1. Test that the interface throws an exception when the continental crust ratio is negative
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the continental crust ratio is negative";
   m_upperLowerContinentalCrustRatio = -1;
   std::shared_ptr<InterfaceInput> interfaceInput1 = createInterfaceInput();
   std::invalid_argument exception1( "The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is negative");
   EXPECT_EXCEPTION_EQ( interfaceInput1->loadInputData(), exception1 )
   m_upperLowerContinentalCrustRatio = 0.5;

   //2. Test that the interface throws an exception when the oceanic crust ratio is negative
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the oceanic crust ratio is negative";
   m_upperLowerOceanicCrustRatio     = -1;
   std::shared_ptr<InterfaceInput> interfaceInput2 = createInterfaceInput();
   std::invalid_argument exception2( "The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is negative");
   EXPECT_EXCEPTION_EQ( interfaceInput2->loadInputData(), exception2 )
   m_upperLowerOceanicCrustRatio = 0.7;

   //3. Test that the interface throws an exception when the HCu map is a null pointer
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the HCu map is a null pointer";
   auto tempHCuIni = m_HCuIni;
   m_HCuIni = nullptr;
   std::shared_ptr<InterfaceInput> interfaceInput3 = createInterfaceInput();
   std::invalid_argument exception3( "The initial crustal thickness map maps cannot be retreived by the interface input" );
   EXPECT_EXCEPTION_EQ( interfaceInput3->loadInputData(), exception3 )
   m_HCuIni = tempHCuIni;

   //4. Test that the interface throws an exception when the HLMu map is a null pointer
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the HLMu map is a null pointer";
   auto tempHLMuIni = m_HLMuIni;
   m_HLMuIni = nullptr;
   std::shared_ptr<InterfaceInput> interfaceInput4 = createInterfaceInput();
   std::invalid_argument exception4( "The initial lithospheric mantle thickness map maps cannot be retreived by the interface input");
   EXPECT_EXCEPTION_EQ( interfaceInput4->loadInputData(), exception4 )
   m_HLMuIni = tempHLMuIni;

   //5. Test that the interface throws an exception when smoothing radius is negative
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when smoothing radius is negative";
   m_filterHalfWidth = -10;
   std::shared_ptr<InterfaceInput> interfaceInput5 = createInterfaceInput();
   std::invalid_argument exception5( "The smoothing radius is set to a negative value");
   EXPECT_EXCEPTION_EQ( interfaceInput5->loadInputData(), exception5 )
   m_filterHalfWidth = 5;
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_singleRift_1 ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Single Rift 1";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 1; i < 9; i++) {
      m_hasSurfaceDepthHistory[ m_snapshots[i] ] = false;
   }
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge( m_snapshots.size(), 220    );
   const std::vector<double      > expectedEndAge  ( m_snapshots.size(), 60     );
   const std::vector<unsigned int> expectedriftID  ( m_snapshots.size(), 1      );
   const std::vector<double      > expectedHBu     ( m_snapshots.size(), 7000   );
   const std::vector<bool        > expectedCalculationMask = { false, false, false,  false, false, false,  false,   false, false, true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,     0, 324.45,    20,     0,    235, -564.45,   300,  235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_singleRift_2 ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Single Rift 2";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 1; i < 8; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge( m_snapshots.size(), 220    );
   const std::vector<double      > expectedEndAge  ( m_snapshots.size(), 60     );
   const std::vector<unsigned int> expectedriftID  ( m_snapshots.size(), 1      );
   const std::vector<double      > expectedHBu     ( m_snapshots.size(), 7000   );
   const std::vector<bool        > expectedCalculationMask = { false, false, false,  false, false, false,  false,   false, false, true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,     0, 324.45,    20,     0,    235, -564.45,   300,  235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_singleRift_3 ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Single Rift 3";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 1; i < 6; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   m_hasSurfaceDepthHistory[m_snapshots[7]] = false;
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge( m_snapshots.size(), 220    );
   const std::vector<double      > expectedEndAge  ( m_snapshots.size(), 60     );
   const std::vector<unsigned int> expectedriftID  ( m_snapshots.size(), 1      );
   const std::vector<double      > expectedHBu     ( m_snapshots.size(), 7000   );
   const std::vector<bool        > expectedCalculationMask = { false, false, false,  false, false, false,  false,   false, false, true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,     0, 324.45,    20,     0,    235, -564.45,   300,  235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_singleRift_4 ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Single Rift 4";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 0; i < 9; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge( m_snapshots.size(), 220    );
   const std::vector<double      > expectedEndAge  ( m_snapshots.size(), 0      );
   const std::vector<unsigned int> expectedriftID  ( m_snapshots.size(), 1      );
   const std::vector<double      > expectedHBu     ( m_snapshots.size(), 7000   );
   const std::vector<bool        > expectedCalculationMask = { false, false, false,  false, false, false,  false,   false, false, true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,     0, 324.45,    20,     0,    235, -564.45,   300,  235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_MultiRift_1 ) {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Multi Rift 1";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 1; i < 5; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   m_hasSurfaceDepthHistory[m_snapshots[6]] = false;
   m_hasSurfaceDepthHistory[m_snapshots[7]] = false;
   //from present day to oldest age (since inputs are ordered this way)
   for (unsigned int i = 0; i < 4; i++) {
      m_HBu[i].setValues(25000);
   }
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge        = {  220,    220,   220,    220,   220,  220,    70,      70,    70,    70 };
   const std::vector<double      > expectedEndAge          = {   90,     90,    90,     90,    90,   90,    60,      60,    60,    60 };
   const std::vector<unsigned int> expectedriftID          = {    1,      1,     1,      1,     1,    1,     2,       2,     2,     2 };
   const std::vector<double      > expectedHBu             = { 7000,   7000,  7000,   7000,  7000, 7000, 25000,   25000, 25000, 25000 };
   const std::vector<bool        > expectedCalculationMask = { false, false, false,  false, false, true, false,   false, false,  true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,     0, 324.45,    20,    0,   235, -564.45,   300,   235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_MultiRift_2 ) {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Multi Rift 2";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, ACTIVE_RIFTING, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 2; i < 5; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   m_hasSurfaceDepthHistory[m_snapshots[6]] = false;
   m_hasSurfaceDepthHistory[m_snapshots[7]] = false;
   //from present day to oldest age (since inputs are ordered this way)
   m_HBu[8].setValues( 6000 );
   m_HBu[9].setValues( 6000 );
   for (unsigned int i = 1; i < 4; i++) {
      m_HBu[i].setValues(15000);
   }
   m_HBu[0].setValues( 25000 );
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge        = {  220,   220,     200,    200,   200,  200,    70,     70,    70,    10 };
   const std::vector<double      > expectedEndAge          = {  200,   200,      90,     90,    90,   90,    60,     60,    60,     0 };
   const std::vector<unsigned int> expectedriftID          = {    1,     1,       2,      2,     2,    2,     3,      3,     3,     4 };
   const std::vector<double      > expectedHBu             = { 6000,  6000,    7000,   7000,  7000, 7000, 15000,  15000, 15000, 25000 };
   const std::vector<bool        > expectedCalculationMask = { false, true,   false,  false, false, true, false,  false,  true,  true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,    0,       0, 324.45,    20,    0,   235, -564.45,  300,   235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_MultiRift_3 ) {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Multi Rift 3";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, FLEXURAL_BASIN, ACTIVE_RIFTING,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 2; i < 5; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   m_hasSurfaceDepthHistory[m_snapshots[8]] = false;
   //from present day to oldest age (since inputs are ordered this way)
   m_HBu[8].setValues( 6000 );
   m_HBu[9].setValues( 6000 );
   for (unsigned int i = 1; i < 4; i++) {
      m_HBu[i].setValues(15000);
   }
   m_HBu[0].setValues( 25000 );
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge        = {  220,   220,  200,     200,   200,  200,    70,  m_nDVd, m_nDVd, m_nDVd };
   const std::vector<double      > expectedEndAge          = {  200,   200,   90,      90,    90,   90,    60,  m_nDVd, m_nDVd, m_nDVd };
   const std::vector<unsigned int> expectedriftID          = {    1,     1,    2,       2,     2,    2,     3,  m_nDVi, m_nDVi, m_nDVi };
   const std::vector<double      > expectedHBu             = { 6000,  6000, 7000,    7000,  7000, 7000, 15000,   15000,  15000,  25000 };
   const std::vector<bool        > expectedCalculationMask = { false, true, false,  false, false, true,  true,   false,  false,  false };
   const std::vector<double      > expectedDeltaSL         = { 18.78,    0,     0, 324.45,    20,    0,   235, -564.45,    300,    235 };

   EXPECT_RIFT_EQ( 60, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_MultiRift_4 ) {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl Tests: Multi Rift 4";
   //from present day to oldest age (since inputs are ordered this way)
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   m_hasSurfaceDepthHistory[m_snapshots[1]] = false;
   //from oldest age to present day (since snapshots are reordered)
   for (unsigned int i = 4; i < 8; i++) {
      m_hasSurfaceDepthHistory[m_snapshots[i]] = false;
   }
   //from present day to oldest age (since inputs are ordered this way)
   for (unsigned int i = 0; i < 6; i++) {
      m_HBu[i].setValues( 15000 );
   }
   for (unsigned int i = 7; i < m_snapshots.size(); i++) {
      m_HBu[i].setValues(6000);
   }
   std::shared_ptr<InterfaceInput> interfaceInput = createInterfaceInput();
   interfaceInput->loadInputData();

   //from oldest age to present day (since outputs are ordered this way)
   const std::vector<double      > expectedStartAge        = {  220,    220,  220,    150,    90,    90,    90,      90,    90,    90 };
   const std::vector<double      > expectedEndAge          = {  150,    150,  150,     90,    60,    60,    60,      60,    60,    60 };
   const std::vector<unsigned int> expectedriftID          = {    1,      1,    1,      2,     3,     3,     3,       3,     3,     3 };
   const std::vector<double      > expectedHBu             = { 6000,   6000, 6000,   7000, 15000, 15000, 15000,   15000, 15000, 15000 };
   const std::vector<bool        > expectedCalculationMask = { false, false, true,   true, false, false, false,   false, false,  true };
   const std::vector<double      > expectedDeltaSL         = { 18.78,     0,    0, 324.45,    20,     0,   235, -564.45,   300,   235 };

   EXPECT_RIFT_EQ( 0, expectedCalculationMask, expectedStartAge, expectedEndAge, expectedriftID, expectedDeltaSL, expectedHBu, interfaceInput );
}

TEST_F( InterfaceInputTester, loadCTCRiftingHistoryIoTbl_exceptions ){
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "Load CTCRiftingHistoryIoTbl exceptions Tests";

   //1.  Test that the interface throws an exception when the size if the CTCRiftingHistoryIoTbl is
   //      different form the number of major snapshots
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the size if the CTCRiftingHistoryIoTbl is" <<
      "different form the number of major snapshots";
   auto saveSnapshots = m_snapshots;
   m_snapshots.push_back( 300.0 );
   std::shared_ptr<InterfaceInput> interfaceInput1 = createInterfaceInput();
   std::runtime_error expectedException1( "The number of snpashots (" + std::to_string( m_snapshots.size() )
      + ") differ from the number of rifting events (" + std::to_string( m_ctcRiftingDataVec.size() ) + ")" );
   EXPECT_EXCEPTION_EQ( interfaceInput1->loadInputData(), expectedException1 );
   m_snapshots = saveSnapshots;
 
   //2.  Test that the interface throws an exception when there is no rifting event for the requested age
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when there is no rifting event for the requested age";
   const double age = 2500;
   std::shared_ptr<InterfaceInput> interfaceInput2 = createInterfaceInput();
   std::runtime_error exception2( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   interfaceInput2->loadInputData();
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftingStartAge( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftingEndAge  ( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getRiftId         ( age )                   , exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getDeltaSLMap     ( age ).getConstantValue(), exception2 );
   EXPECT_EXCEPTION_EQ( interfaceInput2->getHBuMap         ( age ).getConstantValue(), exception2 );

   //3.  Test that the interface throws an exception when the first event is not active (RULE_ID #13)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the first event is not active";
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::invalid_argument exception3( "The first rifting event is not an active rifting" );
   std::shared_ptr<InterfaceInput> interfaceInput3 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput3->loadInputData(), exception3);

   //4  Test that the interface throws an exception when there is no active rifting event defined (RULE_ID #16)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when there is no active rifting event defined";
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN };
   std::shared_ptr<InterfaceInput> interfaceInput4 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput4->loadInputData(), exception3 );

   //5.  Test that the interface throws an exception when there are active events after the flexural age (RULE_ID #14)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when there are active events after the flexural age";
   m_tectonicFalgs = { FLEXURAL_BASIN, ACTIVE_RIFTING, ACTIVE_RIFTING, FLEXURAL_BASIN, PASSIVE_MARGIN,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::invalid_argument exception5( "An active rifting event is defined after a flexural event" );
   std::shared_ptr<InterfaceInput> interfaceInput5 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput5->loadInputData(), exception5 );
   
   //6.  Test that the interface throws an exception when there are passive events after the flexural age (RULE_ID #14)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when there are passive events after the flexural age";
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, PASSIVE_MARGIN, FLEXURAL_BASIN, PASSIVE_MARGIN,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::invalid_argument exception6( "A passive margin event is defined after a flexural event" );
   std::shared_ptr<InterfaceInput> interfaceInput6 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput6->loadInputData(), exception6 );

   //7.  Test that the interface throws an exception when there is no flexural event (RULE_ID #17)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "There is no flexural event defined in the rifting history, at least the present day event has to be set to flexural";
   m_tectonicFalgs = { PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   std::invalid_argument exception7( "There is no flexural event defined in the rifting history, at least the present day event has to be set to flexural" );
   std::shared_ptr<InterfaceInput> interfaceInput7 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput7->loadInputData(), exception7 );

   //8.  Test that the interface throws an exception when there are no SDH defined at the beginning of a rifting event (RULE_ID #15)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when there are no SDH defined at the beginning of a rifting event";
   m_tectonicFalgs = { FLEXURAL_BASIN, PASSIVE_MARGIN, ACTIVE_RIFTING, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   m_hasSurfaceDepthHistory[m_snapshots[7]] = false;
   std::invalid_argument exception8( "The begining of rift ID " + std::to_string( 6 ) +
      " at age " + std::to_string( m_snapshots[7] ) + " does not have any surface depth history associated" );
   std::shared_ptr<InterfaceInput> interfaceInput8 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput8->loadInputData(), exception8 );
   m_hasSurfaceDepthHistory[m_snapshots[7]] = true;

   //9.  Test that the interface throws an exception when the first flexural event doesn't have an SDH (RULE_ID #11)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when the first flexural event doesn't have an SDH";
   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, ACTIVE_RIFTING, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING, ACTIVE_RIFTING };
   m_hasSurfaceDepthHistory[m_snapshots[8]] = false;
   std::invalid_argument exception9( "There is no surface depth history defined for the first flexural event" );
   std::shared_ptr<InterfaceInput> interfaceInput9 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput9->loadInputData(), exception9 );
   m_hasSurfaceDepthHistory[m_snapshots[8]] = true;

   //10.  Test that the interface throws an exception when more than one maximum basalt thickness value is allowed in one rift (RULE_ID #10)
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Test that the interface throws an exception when more than one maximum basalt thickness value is allowed in one rift";
   m_tectonicFalgs = { FLEXURAL_BASIN, FLEXURAL_BASIN, ACTIVE_RIFTING, PASSIVE_MARGIN, ACTIVE_RIFTING,
                       PASSIVE_MARGIN, PASSIVE_MARGIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING };
   m_HBu[5].setValues( 1000 );
   std::invalid_argument exception10( "Only one Maximum Oceanic Thickness value can be allowed within a rift" );
   std::shared_ptr<InterfaceInput> interfaceInput10 = createInterfaceInput();
   EXPECT_EXCEPTION_EQ( interfaceInput10->loadInputData(), exception10 );
   m_HBu[5].setValues( 0 );
}

