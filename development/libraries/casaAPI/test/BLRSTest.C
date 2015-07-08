#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/VarSpaceImpl.h"

#include "../src/PrmSourceRockTOC.h"
#include "../src/PrmSourceRockHI.h"
#include "../src/PrmSourceRockHC.h"
#include "../src/PrmSourceRockType.h"
#include "../src/PrmSourceRockPreAsphaltStartAct.h"
#include "../src/PrmTopCrustHeatProduction.h"
#include "../src/PrmOneCrustThinningEvent.h"
#include "../src/PrmPorosityModel.h"
#include "../src/PrmLithoSTPThermalCond.h"

#include "../src/VarPrmSourceRockTOC.h"
#include "../src/VarPrmSourceRockHI.h"
#include "../src/VarPrmSourceRockHC.h"
#include "../src/VarPrmSourceRockType.h"
#include "../src/VarPrmSourceRockPreAsphaltStartAct.h"
#include "../src/VarPrmTopCrustHeatProduction.h"
#include "../src/VarPrmOneCrustThinningEvent.h"
#include "../src/VarPrmPorosityModel.h"
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
   const static char * m_testProject;
   const static char * m_testProjectCatPrms;
};

const char * BLRSTest::m_testProject        = "Ottoland.project3d";
const char * BLRSTest::m_testProjectCatPrms = "OttolandCatPrms.project3d";

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



