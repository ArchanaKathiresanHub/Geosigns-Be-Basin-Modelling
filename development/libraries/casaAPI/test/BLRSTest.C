#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/VarSpaceImpl.h"

#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmSourceRockHI.h"
#include "../src/PrmSourceRockHC.h"
#include "../src/PrmSourceRockType.h"
#include "../src/PrmSourceRockPreAsphaltStartAct.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/PrmCrustThinning.h"
#include "../src/PrmOneCrustThinningEvent.h"
#include "../src/PrmPorosityModel.h"
#include "../src/PrmPermeabilityModel.h"
#include "../src/PrmLithoSTPThermalCond.h"

#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmSourceRockHI.h"
#include "../src/VarPrmSourceRockHC.h"
#include "../src/VarPrmSourceRockType.h"
#include "../src/VarPrmSourceRockPreAsphaltStartAct.h"
#include "../src/VarPrmTopCrustHeatProduction.h"
#include "../src/VarPrmCrustThinning.h"
#include "../src/VarPrmOneCrustThinningEvent.h"
#include "../src/VarPrmPorosityModel.h"
#include "../src/VarPrmPermeabilityModel.h"
#include "../src/VarPrmLithoSTPThermalCond.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;
using namespace casa::BusinessLogicRulesSet;

static const double eps = 1.e-5;

// Test Business logic rules set (BLRS) methods in  CASA API
class BLRSTest : public ::testing::Test
{
public:
   BLRSTest()  { ; }
   ~BLRSTest() { ; }
   static const char * m_testProject;
   static const char * m_testProjectCatPrms;
   static const char * m_testProjectLithology;
};

const char * BLRSTest::m_testProject          = "Ottoland.project3d";
const char * BLRSTest::m_testProjectCatPrms   = "OttolandCatPrms.project3d";
const char * BLRSTest::m_testProjectLithology = "LithologyTesting.project3d";

/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable parameter TopCrustHeatProduction to scenario analysis
TEST_F( BLRSTest, VaryTopCrustHeatProductionTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // add the new variable parameter TopCrustHeatProduction to the scenario analysis by using one of the BLRS API function
   ASSERT_EQ( ErrorHandler::NoError, VaryTopCrustHeatProduction( sc,  0.2, 4.0, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmTopCrustHeatProduction * p1c = dynamic_cast<const VarPrmTopCrustHeatProduction*>( varPrms.continuousParameter( 0 ) );

   ASSERT_TRUE( p1c != NULL ); // do we have the required parameter in the list?
   
   const std::vector<double> & minV = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_NEAR( minV[0],  0.2, eps ); // does it range have given min value
   ASSERT_NEAR( maxV[0],  4.0, eps ); // does it range have given max value
   ASSERT_NEAR( baseV[0], 2.5, eps );  // does it range have base value from the project?
}


/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable parameter source rock TOC to scenario analysis
TEST_F( BLRSTest, VarySourceRockTOCTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockTOC( sc, "Lower Jurassic", 10.0, 30.0, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockTOC * p1c = dynamic_cast<const VarPrmSourceRockTOC*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_NEAR( minV[0],  10.0, eps );  // does it range have given min value?
   ASSERT_NEAR( maxV[0],  30.0, eps );  // does it range have given max value?
   ASSERT_NEAR( baseV[0], 10.0, eps );  // does it range have base value from the project?
}


/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable parameter source rock HI to scenario analysis
TEST_F( BLRSTest, VarySourceRockHITest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockHI( sc, "Lower Jurassic", 371.0, 571.0, VarPrmContinuous::Block ) );
   // expect failure so HI and H/C can't be variated both in the same time
   ASSERT_EQ( ErrorHandler::AlreadyDefined, VarySourceRockHC( sc, "Lower Jurassic", 0.5,   1.5,   VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockHI * p1c = dynamic_cast<const VarPrmSourceRockHI*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_NEAR( minV[0],  371.0,      eps );  // does it range have given min value?
   ASSERT_NEAR( maxV[0],  571.0,      eps );  // does it range have given max value?
   ASSERT_NEAR( baseV[0], 472.068687, eps );  // does it range have base value from the project?
}


/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable parameter source rock H/C to scenario analysis
TEST_F( BLRSTest, VarySourceRockHCTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockHC( sc, "Lower Jurassic", 0.5, 1.75,   VarPrmContinuous::Block ) );
   // expect failure so HI and H/C can't be variated both in the same time
   ASSERT_EQ( ErrorHandler::AlreadyDefined, VarySourceRockHI( sc, "Lower Jurassic", 371.0, 571.0, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>(sc.varSpace());

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockHC * p1c = dynamic_cast<const VarPrmSourceRockHC*>(varPrms.continuousParameter( 0 ));
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_NEAR( minV[0], 0.5, eps );  // does it range have given min value?
   ASSERT_NEAR( maxV[0], 1.75, eps );  // does it range have given max value?
   ASSERT_NEAR( baseV[0], 1.25, eps );  // does it range have base value from the project?
}


/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable parameter source rock preasphalten activation energy to scenario analysis
TEST_F( BLRSTest, VarySourceRockPreasphaltActEnergyTest )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockPreAsphaltActEnergy( sc, "Lower Jurassic", 208, 212, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>(sc.varSpace());

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );
   const VarPrmSourceRockPreAsphaltStartAct * p1c = dynamic_cast<const VarPrmSourceRockPreAsphaltStartAct*>(varPrms.continuousParameter( 0 ));
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_NEAR( minV[0], 208, eps );  // does it range have given min value?
   ASSERT_NEAR( maxV[0], 212, eps );  // does it range have given max value?
   ASSERT_NEAR( baseV[0], 210, eps );  // does it range have base value from the project?
}

/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variable categorical parameter source rock type to scenario analysis
TEST_F( BLRSTest, VarySourceRockTypeTest )
{
   std::vector<std::string> srList;

   srList.push_back( "Type_I_CenoMesozoic_Lacustrine_kin" );
   srList.push_back( "Type_II_Mesozoic_MarineShale_kin"   );
   srList.push_back( "Type_III_II_Mesozoic_HumicCoal_lit" );
   
   std::vector<double> srWeights( 3, 0.33 );

   // check order of variable parameters for Source Rock Type. User can't add any source rock variable parameter before 
   // source rock type categorical parameter
   for ( size_t i = 0; i < 4; ++i )
   {
      // create new scenario analysis
      ScenarioAnalysis sc;

      // load base case to scenario
      ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProjectCatPrms ) );

      // set one of the Source Rock parameter
      switch( i )
      {
         case 0: ASSERT_EQ( ErrorHandler::NoError, VarySourceRockTOC(                 sc, "Lower Jurassic", 10.0,  30.0,  VarPrmContinuous::Block ) ); break;
         case 1: ASSERT_EQ( ErrorHandler::NoError, VarySourceRockHI(                  sc, "Lower Jurassic", 371.0, 571.0, VarPrmContinuous::Block ) ); break;
         case 2: ASSERT_EQ( ErrorHandler::NoError, VarySourceRockHC(                  sc, "Lower Jurassic", 0.5,   1.75,  VarPrmContinuous::Block ) ); break;
         case 3: ASSERT_EQ( ErrorHandler::NoError, VarySourceRockPreAsphaltActEnergy( sc, "Lower Jurassic", 208.0, 212.0, VarPrmContinuous::Block ) ); break;
         default: break;
      }
      // expect a failure if any of source rock variable parameters are defined before source rock type variation
      ASSERT_EQ( ErrorHandler::AlreadyDefined, VarySourceRockType( sc, "Lower Jurassic", srList, srWeights ) );
   }

   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProjectCatPrms ) );

   // define new variable parameter
   ASSERT_EQ( ErrorHandler::NoError, VarySourceRockType( sc, "Lower Jurassic", srList, srWeights ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl &>( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmSourceRockType * prm = dynamic_cast<const VarPrmSourceRockType *>(varPrms.categoricalParameter( 0 ) );
   ASSERT_TRUE( prm != NULL ); // do we have required the parameter in the list?

   ASSERT_NEAR( prm->minValue()->asInteger(),  0, eps );  // does it range have given min value?
   ASSERT_NEAR( prm->maxValue()->asInteger(),  2, eps );  // does it range have given max value?
   ASSERT_NEAR( prm->baseValue()->asInteger(), 1, eps );  // does it range have base value from the project?
}


/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variate one crust thinning event parameters
TEST_F( BLRSTest, VaryOneCrustThinningEvent )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, 
              casa::BusinessLogicRulesSet::VaryOneCrustThinningEvent( sc, 15000.0, 40000.0, 120.0, 230.0, 30.0, 45.0, 0.5, 0.8, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmOneCrustThinningEvent * p1c = dynamic_cast<const VarPrmOneCrustThinningEvent*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   // does it range have given min value
   ASSERT_NEAR( minV[0], 15000.0, eps );
   ASSERT_NEAR( minV[1], 120.0,   eps );
   ASSERT_NEAR( minV[2], 30.0,    eps );
   ASSERT_NEAR( minV[3], 0.5,     eps );

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 40000.0, eps );  
   ASSERT_NEAR( maxV[1], 230.0,   eps );  
   ASSERT_NEAR( maxV[2], 45.0,    eps );  
   ASSERT_NEAR( maxV[3], 0.8,     eps );  

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 25000.0, eps );  
   ASSERT_NEAR( baseV[1], 220.0,   eps );  
   ASSERT_NEAR( baseV[2], 35.0,    eps );  
   ASSERT_NEAR( baseV[3], 0.55,    eps );  
}

/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variate one crust thinning event parameters
TEST_F( BLRSTest, VaryCrustThinningNoMaps )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   std::vector<double> minT0(3), maxT0(3), minDeltaT(3), maxDeltaT(3), minThinningFct(3), maxThinningFct(3);
   std::vector<std::string> mapsList( 3 );

   minT0[0] = 240.0; minT0[1] = 190.0; minT0[2] = 140.0;
   maxT0[0] = 260.0; maxT0[1] = 210.0; maxT0[2] = 160.0;

   minDeltaT[0] = 5.0;  minDeltaT[1] =  5.0; minDeltaT[2] =  5.0;
   maxDeltaT[0] = 10.0; maxDeltaT[1] = 10.0; maxDeltaT[2] = 10.0;

   minThinningFct[0] = 0.5; minThinningFct[1] = 0.6; minThinningFct[2] = 0.7;
   maxThinningFct[0] = 0.7; maxThinningFct[1] = 0.8; maxThinningFct[2] = 0.9; 

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, 
              casa::BusinessLogicRulesSet::VaryCrustThinning( sc, 15000.0,        40000.0
                                                                , minT0,          maxT0
                                                                , minDeltaT,      maxDeltaT
                                                                , minThinningFct, maxThinningFct
                                                                , mapsList, VarPrmContinuous::Block ) );
   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmCrustThinning * p1c = dynamic_cast<const VarPrmCrustThinning*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_EQ( minV.size(),  10 );
   ASSERT_EQ( baseV.size(), 10 );
   ASSERT_EQ( maxV.size(),  10 );

   // does it range have given min/base/max values
   ASSERT_NEAR( minV[0], 15000.0, eps ); ASSERT_NEAR( baseV[0], 27500.0, eps ); ASSERT_NEAR( maxV[0], 40000.0, eps ); 
   
   // event 1
   ASSERT_NEAR( minV[1], 240.0, eps ); ASSERT_NEAR( baseV[1], 250.0, eps ); ASSERT_NEAR( maxV[1], 260.0, eps ); 
   ASSERT_NEAR( minV[2], 5.0,   eps ); ASSERT_NEAR( baseV[2], 7.5,   eps ); ASSERT_NEAR( maxV[2], 10.0,  eps ); 
   ASSERT_NEAR( minV[3], 0.5,   eps ); ASSERT_NEAR( baseV[3], 0.6,   eps ); ASSERT_NEAR( maxV[3], 0.7,   eps ); 
   // event 2
   ASSERT_NEAR( minV[4], 190.0, eps ); ASSERT_NEAR( baseV[4], 200.0, eps ); ASSERT_NEAR( maxV[4], 210.0,  eps ); 
   ASSERT_NEAR( minV[5], 5.0,   eps ); ASSERT_NEAR( baseV[5], 7.5,   eps ); ASSERT_NEAR( maxV[5], 10.0,   eps ); 
   ASSERT_NEAR( minV[6], 0.6,   eps ); ASSERT_NEAR( baseV[6], 0.7,   eps ); ASSERT_NEAR( maxV[6], 0.8,    eps ); 
   // event3                                                                                   
   ASSERT_NEAR( minV[7], 140.0, eps ); ASSERT_NEAR( baseV[7], 150.0, eps ); ASSERT_NEAR( maxV[7], 160.0,  eps );
   ASSERT_NEAR( minV[8], 5.0,   eps ); ASSERT_NEAR( baseV[8], 7.5,   eps ); ASSERT_NEAR( maxV[8], 10.0,   eps );
   ASSERT_NEAR( minV[9], 0.7,   eps ); ASSERT_NEAR( baseV[9], 0.8,   eps ); ASSERT_NEAR( maxV[9], 0.9,    eps );
}


/////////////////////////////////////////////////////////////////////////////////////////
// Exponential model test
TEST_F( BLRSTest, VaryPorosityExponentialModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // the first one - try to give wrong porosity model name
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, casa::BusinessLogicRulesSet::VaryPorosityModelParameters( sc, "Std. Sandstone"
               , "Eponential"
               , 30.0, 60.0, 2.0, 4.0, UndefinedDoubleValue, UndefinedDoubleValue, UndefinedDoubleValue, UndefinedDoubleValue, VarPrmContinuous::Block 
               ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPorosityModelParameters( 
              sc
            , "Std. Sandstone"
            , "Exponential"
            , 30.0, 60.0
            , 2.0, 4.0
            , UndefinedDoubleValue, UndefinedDoubleValue
            , UndefinedDoubleValue, UndefinedDoubleValue
            , VarPrmContinuous::Block )
            );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPorosityModel * p1c = dynamic_cast<const VarPrmPorosityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?
  
   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_EQ( minV.size(),  2 );
   ASSERT_EQ( maxV.size(),  2 );
   ASSERT_EQ( baseV.size(), 2 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 30.0, eps );
   ASSERT_NEAR( minV[1], 2.0,   eps );

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 60.0, eps );
   ASSERT_NEAR( maxV[1], 4.0,  eps );

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 48.0, eps );  
   ASSERT_NEAR( baseV[1], 3.22, eps );  
}

/////////////////////////////////////////////////////////////////////////////////////////
// Soil mechanics model parameters variation test
TEST_F( BLRSTest, VaryPorositySoilMechanicsModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // the first one - try to define both parameters in inconsistent way
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, casa::BusinessLogicRulesSet::VaryPorosityModelParameters( sc, "Std. Sandstone"
               , "Soil_Mechanics"
               , 30.0, 60.0, 2.0, 4.0, UndefinedDoubleValue, UndefinedDoubleValue, UndefinedDoubleValue, UndefinedDoubleValue, VarPrmContinuous::Block 
               ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPorosityModelParameters( 
              sc
            , "Std. Sandstone"
            , "Soil_Mechanics"
            , 30.0, 60.0
            , UndefinedDoubleValue, UndefinedDoubleValue
            , UndefinedDoubleValue, UndefinedDoubleValue
            , UndefinedDoubleValue, UndefinedDoubleValue
            , VarPrmContinuous::Block )
            );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPorosityModel * p1c = dynamic_cast<const VarPrmPorosityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?
  
   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_EQ( minV.size(),  1 );
   ASSERT_EQ( maxV.size(),  1 );
   ASSERT_EQ( baseV.size(), 1 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 6.92570e-2, eps );

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 4.625794e-1, eps );

   // does it have base values as middle for porosity and calculated for comp. coeff. ?
   ASSERT_NEAR( baseV[0], 2.3947558e-1, eps );  
}


/////////////////////////////////////////////////////////////////////////////////////////
// Double exponential model test
TEST_F( BLRSTest, VaryPorosityDoubleExponentialModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPorosityModelParameters( 
              sc
            , "Std. Sandstone"
            , "Double_Exponential"
            , 30.0, 60.0  // surface porosity
            , 2.0,  4.0   // compaction coeff. A
            , 10,   20    // minimal porosity
            , 3.0,  5.0   // compaction coeff. B
            , VarPrmContinuous::Block )
            );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPorosityModel * p1c = dynamic_cast<const VarPrmPorosityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?
  
   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   
   ASSERT_EQ( minV.size(),  4 );
   ASSERT_EQ( maxV.size(),  4 );
   ASSERT_EQ( baseV.size(), 4 );

    // does it range have given min value
   ASSERT_NEAR( minV[0], 30.0,  eps );
   ASSERT_NEAR( minV[1], 2.0,   eps );
   ASSERT_NEAR( minV[2], 10.0,  eps );
   ASSERT_NEAR( minV[3], 3.0,   eps );
                     
   // does it range have given max value
   ASSERT_NEAR( maxV[0], 60.0,  eps );
   ASSERT_NEAR( maxV[1], 4.0,   eps );
   ASSERT_NEAR( maxV[2], 20.0,  eps );
   ASSERT_NEAR( maxV[3], 5.0,   eps );
                     
   // does it have base values as middle values?
   ASSERT_NEAR( baseV[0], 45.0, eps );  
   ASSERT_NEAR( baseV[1], 3.0,  eps );  
   ASSERT_NEAR( baseV[2], 15.0, eps );  
   ASSERT_NEAR( baseV[3], 4.0,  eps );  
}                     

/////////////////////////////////////////////////////////////////////////////////////////
// Test how ones can add variate one crust thinning event parameters
TEST_F( BLRSTest, VaryLithoSTPThermalCondCoeff )
{
   // create new scenario analysis
   ScenarioAnalysis sc;

   // load base case to scenario
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   // set the parameter
   ASSERT_EQ( ErrorHandler::NoError, 
              casa::BusinessLogicRulesSet::VaryLithoSTPThermalCondCoeffParameter( sc, "Std. Shale", 1, 2, VarPrmContinuous::Block ) );

   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace( ) );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmLithoSTPThermalCond * p1c = dynamic_cast<const VarPrmLithoSTPThermalCond*>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   // does it range have given min value
   ASSERT_NEAR( minV[0], 1.0, eps );

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 2.0, eps );  

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 1.4, eps );  
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Lithology type permeability model parameters variation tests
TEST_F( BLRSTest, VaryPermeabilityNoneModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   std::vector<double> minMdlPrms( 1, 1.0 );
   std::vector<double> maxMdlPrms( 1, 1.0 );

   // Try to set variation of None model - it has no any parameters and can't be varied
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "Upper Jurassic"
                                                                                                         , "Std. Shale"
                                                                                                         , "None"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
   // Try to set variation of None model - it has no any parameters and can't be varied
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "Upper Jurassic"
                                                                                                         , "Std. Shale"
                                                                                                         , "Impermeable"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sandstone model test
TEST_F( BLRSTest, VaryPermeabilitySandstoneModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   std::vector<double> minMdlPrms( PrmPermeabilityModel::ClayPercentage + 1 );
   std::vector<double> maxMdlPrms( PrmPermeabilityModel::ClayPercentage );

   // Anisotropic coeff
   minMdlPrms[ PrmPermeabilityModel::AnisotropicCoeff ] = 1.0;
   maxMdlPrms[ PrmPermeabilityModel::AnisotropicCoeff ] = 1.0;

   // Depositional permeability
   minMdlPrms[ PrmPermeabilityModel::DepositionalPerm ] = 5000;
   maxMdlPrms[ PrmPermeabilityModel::DepositionalPerm ] = 7000;

   // Sands clay percentage
   minMdlPrms[ PrmPermeabilityModel::ClayPercentage] = 1.0;
   
   // Try to set variation of Sandstone model with different number of input parameters
   ASSERT_EQ( ErrorHandler::UndefinedValue, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "Upper Jurassic"
                                                                                                         , "Std. Sandstone"
                                                                                                         , "Sandstone"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
   maxMdlPrms.push_back( 2.0 ); // add clay percentage prm

   // try to give wrong porosity model name
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "Upper Jurassic"
                                                                                                         , "Std. Sandstone"
                                                                                                         , "Std. Sandstone"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
   // try to give wrong layer name
   ASSERT_EQ( ErrorHandler::NonexistingID, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "Wrong layer name"
                                                                                                         , "Std. Sandstone"
                                                                                                         , "Sands"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
   // try to give wrong lithology name
   ASSERT_EQ( ErrorHandler::NonexistingID, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                         , "LowerUpper Jurassic"
                                                                                                         , "Wrong lithology name"
                                                                                                         , "Sands"
                                                                                                         , minMdlPrms
                                                                                                         , maxMdlPrms
                                                                                                         , VarPrmContinuous::Block
                                                                                                         ) );
   // define different model with one undefined parameter
   minMdlPrms.resize( PrmPermeabilityModel::RecoverCoeff + 1 );
   maxMdlPrms.resize( PrmPermeabilityModel::RecoverCoeff + 1 );
   
   minMdlPrms[ PrmPermeabilityModel::SensitivityCoeff ] = 1.0; 
   maxMdlPrms[ PrmPermeabilityModel::SensitivityCoeff ] = 2.0; 

   minMdlPrms[ PrmPermeabilityModel::RecoverCoeff ] = UndefinedDoubleValue; 
   maxMdlPrms[ PrmPermeabilityModel::RecoverCoeff ] = UndefinedDoubleValue; 

   ASSERT_EQ( ErrorHandler::UndefinedValue, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                        , "Upper Jurassic"
                                                                                                        , "Std. Sandstone"
                                                                                                        , "Shales"
                                                                                                        , minMdlPrms
                                                                                                        , maxMdlPrms
                                                                                                        , VarPrmContinuous::Block
                                                                                                        ) );
   minMdlPrms.resize( PrmPermeabilityModel::ClayPercentage + 1 );
   maxMdlPrms.resize( PrmPermeabilityModel::ClayPercentage + 1 );

   minMdlPrms[ PrmPermeabilityModel::ClayPercentage] = 1.0;
   maxMdlPrms[ PrmPermeabilityModel::ClayPercentage] = 2.0;

   // set the parameters variation
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                 , "Upper Jurassic"
                                                                                                 , "Std. Sandstone"
                                                                                                 , "Sands"
                                                                                                 , minMdlPrms
                                                                                                 , maxMdlPrms
                                                                                                 , VarPrmContinuous::Block
                                                                                                 ) );

  // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPermeabilityModel * p1c = dynamic_cast<const VarPrmPermeabilityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?
  
   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_EQ( minV.size(),  3 );
   ASSERT_EQ( maxV.size(),  3 );
   ASSERT_EQ( baseV.size(), 3 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 1.0, eps );
   ASSERT_NEAR( minV[1], 5000.0,   eps );
   ASSERT_NEAR( minV[2], 1.0,   eps );

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 1.0, eps );
   ASSERT_NEAR( maxV[1], 7000.0,  eps );
   ASSERT_NEAR( maxV[2], 2.0,  eps );

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 1.0, eps );  
   ASSERT_NEAR( baseV[1], 6000.0, eps );  
   ASSERT_NEAR( baseV[2], 1.5, eps );  
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sandstone model test
TEST_F( BLRSTest, VaryPermeabilityMudstoneModelParameters )
{
   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProject ) );

   std::vector<double> minMdlPrms( PrmPermeabilityModel::RecoverCoeff + 1 );
   std::vector<double> maxMdlPrms( PrmPermeabilityModel::RecoverCoeff + 1 );

   // Anisotropic coeff
   minMdlPrms[ PrmPermeabilityModel::AnisotropicCoeff ] = 1.0;
   maxMdlPrms[ PrmPermeabilityModel::AnisotropicCoeff ] = 1.0;

   // Depositional permeability
   minMdlPrms[ PrmPermeabilityModel::DepositionalPerm ] = 0.05;
   maxMdlPrms[ PrmPermeabilityModel::DepositionalPerm ] = 0.15;

   // Sensitivity coeff
   minMdlPrms[ PrmPermeabilityModel::SensitivityCoeff] = 1.0;
   maxMdlPrms[ PrmPermeabilityModel::SensitivityCoeff] = 2.0;

   // Recover coeff
   minMdlPrms[ PrmPermeabilityModel::RecoverCoeff] = 0.005;
   maxMdlPrms[ PrmPermeabilityModel::RecoverCoeff] = 0.015;

   // set the parameters variation
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                 , "Mid Cretaceous"
                                                                                                 , "Std. Chalk"
                                                                                                 , "Shales"
                                                                                                 , minMdlPrms
                                                                                                 , maxMdlPrms
                                                                                                 , VarPrmContinuous::Block
                                                                                                 ) );
   // get varspace 
   casa::VarSpaceImpl & varPrms = dynamic_cast<casa::VarSpaceImpl&>( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPermeabilityModel * p1c = dynamic_cast<const VarPrmPermeabilityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const std::vector<double> & minV  = p1c->minValue()->asDoubleArray();
   const std::vector<double> & maxV  = p1c->maxValue()->asDoubleArray();
   const std::vector<double> & baseV = p1c->baseValue()->asDoubleArray();

   ASSERT_EQ( minV.size(),  4 );
   ASSERT_EQ( maxV.size(),  4 );
   ASSERT_EQ( baseV.size(), 4 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 1.0,   eps );
   ASSERT_NEAR( minV[1], 0.05,  eps );
   ASSERT_NEAR( minV[2], 1.0,   eps );
   ASSERT_NEAR( minV[3], 0.005, eps );
   
   // does it range have given max value
   ASSERT_NEAR( maxV[0], 1.0,   eps );
   ASSERT_NEAR( maxV[1], 0.15,  eps );
   ASSERT_NEAR( maxV[2], 2.0,   eps );
   ASSERT_NEAR( maxV[3], 0.015, eps );

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 1.0,  eps );  
   ASSERT_NEAR( baseV[1], 0.1,  eps );  
   ASSERT_NEAR( baseV[2], 1.5,  eps );  
   ASSERT_NEAR( baseV[3], 0.01, eps );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sandstone model test
TEST_F( BLRSTest, VaryPermeabilityMultipointModelParameters )
{
   double prof1Por[]    = { 5.0, 60.0 };
   double prof1PrmMin[] = { 0.1,  7.0  };
   double prof1PrmMax[] = { 0.6,  9.0  };
   double prof1PrmBas[] = { 0.35, 8.0  };

   ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( m_testProjectLithology ) );

   std::vector<double> minMdlPrms( PrmPermeabilityModel::MPProfileNumPoints + 1 );
   std::vector<double> maxMdlPrms( PrmPermeabilityModel::MPProfileNumPoints + 1 );

   minMdlPrms[PrmPermeabilityModel::AnisotropicCoeff] = 1.0; 
   maxMdlPrms[PrmPermeabilityModel::AnisotropicCoeff] = 1.0;

   minMdlPrms[PrmPermeabilityModel::MPProfileNumPoints] = (sizeof( prof1Por ) / sizeof( double ) );
   maxMdlPrms[PrmPermeabilityModel::MPProfileNumPoints] = (sizeof( prof1Por ) / sizeof( double ) );
   for ( size_t i = 0; i < sizeof( prof1Por ) / sizeof( double ); ++i )
   {
      minMdlPrms.push_back( prof1Por[i] ); minMdlPrms.push_back( prof1PrmMin[i] );
      maxMdlPrms.push_back( prof1Por[i] ); maxMdlPrms.push_back( prof1PrmMax[i] );
   }
   
   // set the parameters variation, expecting an error here because Permian layer has Std. Sandstone lithology
   ASSERT_EQ( ErrorHandler::NonexistingID, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                 , "Permian"
                                                                                                 , "SM. Sandstone"
                                                                                                 , "Multipoint"
                                                                                                 , minMdlPrms
                                                                                                 , maxMdlPrms
                                                                                                 , VarPrmContinuous::Block ) );
   // no it should be smooth
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                       , "Permian"
                                                                                                       , "Std. Sandstone"
                                                                                                       , "Multipoint"
                                                                                                       , minMdlPrms
                                                                                                       , maxMdlPrms
                                                                                                       , VarPrmContinuous::Block ) );
   // get varspace 
   VarSpaceImpl & varPrms = dynamic_cast< VarSpaceImpl & >( sc.varSpace() );

   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 1 );

   const VarPrmPermeabilityModel * p1c = dynamic_cast<const VarPrmPermeabilityModel *>( varPrms.continuousParameter( 0 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   const PrmPermeabilityModel * prmMin = dynamic_cast<const PrmPermeabilityModel *>( p1c->minValue().get()  );
   const PrmPermeabilityModel * prmMax = dynamic_cast<const PrmPermeabilityModel *>( p1c->maxValue().get()  );
   const PrmPermeabilityModel * prmBas = dynamic_cast<const PrmPermeabilityModel *>( p1c->baseValue().get() );

   std::vector<double> minV  = prmMin->asDoubleArray();
   std::vector<double> maxV  = prmMax->asDoubleArray();
   std::vector<double> baseV = prmBas->asDoubleArray();

   ASSERT_EQ( minV.size(),  2 );
   ASSERT_EQ( maxV.size(),  2 );
   ASSERT_EQ( baseV.size(), 2 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 1.0, eps ); // Anisotropy coeff
   ASSERT_NEAR( minV[1], 0.0, eps ); // profile scale value

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 1.0, eps );
   ASSERT_NEAR( maxV[1], 1.0, eps );

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 1.0, eps );  
   ASSERT_NEAR( baseV[1], 0.5, eps );  

   // check profiles values
   for ( size_t i = 0; i < sizeof( prof1Por ) / sizeof( double ); ++i )
   {
      ASSERT_NEAR( prmMin->multipointPorosity()[i], prof1Por[i], eps );
      ASSERT_NEAR( prmMax->multipointPorosity()[i], prof1Por[i], eps );
      ASSERT_NEAR( prmBas->multipointPorosity()[i], prof1Por[i], eps );

      ASSERT_NEAR( prmMin->multipointPermeability()[i], prof1PrmMin[i], eps );
      ASSERT_NEAR( prmMax->multipointPermeability()[i], prof1PrmMax[i], eps );
      ASSERT_NEAR( prmBas->multipointPermeability()[i], prof1PrmBas[i], eps );
   }
  
   // Define another Multipoint permeability model for the layer which has before Mudstone model
   // with different profiles size
   minMdlPrms.resize( PrmPermeabilityModel::MPProfileNumPoints + 1 );
   maxMdlPrms.resize( PrmPermeabilityModel::MPProfileNumPoints + 1 );

   double prof2PorMin[]    = {  2.0,                              20.0,        60.0 };
   double prof2PrmMin[]    = { -6.0,                              -4.0,        -0.5 };
   double prof2PrmMinInt[] = { -6.0,      -5.5555556,  -4.555556, -4.0, -2.25, -0.5 };

   double prof2PorMax[]    = {             6.0,        15.0,            40.0,  60.0 };
   double prof2PrmMax[]    = {            -5.0,        -3.0,             0.0,   0.5 };
   double prof2PrmMaxInt[] = { -5.888889, -5.0,        -3.0,      -2.4,  0.0,   0.5 };

   double prof2PorBas[]    = {  2.0,       6.0,        15.0,      20.0, 40.0,  60.0 };
   double prof2PrmBas[]    = { -5.944444, -5.277778,   -3.777778, -3.2, -1.125, 0.0 };


   minMdlPrms[PrmPermeabilityModel::MPProfileNumPoints] = sizeof( prof2PorMin ) / sizeof( double );
   maxMdlPrms[PrmPermeabilityModel::MPProfileNumPoints] = sizeof( prof2PorMax ) / sizeof( double );

   for ( size_t i = 0; i < sizeof( prof2PorMin ) / sizeof( double ); ++i ) { minMdlPrms.push_back( prof2PorMin[i] ); minMdlPrms.push_back( prof2PrmMin[i] ); }
   for ( size_t i = 0; i < sizeof( prof2PorMax ) / sizeof( double ); ++i ) { maxMdlPrms.push_back( prof2PorMax[i] ); maxMdlPrms.push_back( prof2PrmMax[i] ); }

   // set the parameters variation
   ASSERT_EQ( ErrorHandler::NoError, casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( sc
                                                                                                 , "Triassic"
                                                                                                 , "Std. Shale"
                                                                                                 , "Multipoint"
                                                                                                 , minMdlPrms
                                                                                                 , maxMdlPrms
                                                                                                 , VarPrmContinuous::Block
                                                                                                 ) );
   // check how the parameter was set
   ASSERT_EQ( varPrms.size(), 2 );

   p1c = dynamic_cast<const VarPrmPermeabilityModel *>( varPrms.continuousParameter( 1 ) );
   ASSERT_TRUE( p1c != NULL ); // do we have required the parameter in the list?

   prmMin = dynamic_cast<const PrmPermeabilityModel *>( p1c->minValue().get()  );
   prmMax = dynamic_cast<const PrmPermeabilityModel *>( p1c->maxValue().get()  );
   prmBas = dynamic_cast<const PrmPermeabilityModel *>( p1c->baseValue().get() );

   minV  = prmMin->asDoubleArray();
   maxV  = prmMax->asDoubleArray();
   baseV = prmBas->asDoubleArray();

   ASSERT_EQ( minV.size(),  2 );
   ASSERT_EQ( maxV.size(),  2 );
   ASSERT_EQ( baseV.size(), 2 );

   // does it range have given min value
   ASSERT_NEAR( minV[0], 1.0, eps ); // Anisotropy coeff
   ASSERT_NEAR( minV[1], 0.0, eps ); // profile scale value

   // does it range have given max value
   ASSERT_NEAR( maxV[0], 1.0, eps );
   ASSERT_NEAR( maxV[1], 1.0, eps );

   // does it have base values from project?
   ASSERT_NEAR( baseV[0], 1.0, eps );  
   ASSERT_NEAR( baseV[1], 0.5, eps );  

   // check profiles values
   for ( size_t i = 0; i < sizeof( prof2PorBas ) / sizeof( double ); ++i )
   {
      ASSERT_NEAR( prmMin->multipointPorosity()[i], prof2PorBas[i], eps );
      ASSERT_NEAR( prmMax->multipointPorosity()[i], prof2PorBas[i], eps );
      ASSERT_NEAR( prmBas->multipointPorosity()[i], prof2PorBas[i], eps );

      ASSERT_NEAR( prmMin->multipointPermeability()[i], prof2PrmMinInt[i], eps );
      ASSERT_NEAR( prmMax->multipointPermeability()[i], prof2PrmMaxInt[i], eps );
      ASSERT_NEAR( prmBas->multipointPermeability()[i], prof2PrmBas[i], eps );
   } 
} 
