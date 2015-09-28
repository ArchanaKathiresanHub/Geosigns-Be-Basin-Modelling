#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"
#include "../src/ObsGridPropertyXYZ.h"

#include <memory>
//#include <cmath>

#include <gtest/gtest.h>

using namespace casa;

static const double eps = 1.e-5;

class DataDiggerTest : public ::testing::Test
{
public:
   DataDiggerTest( ) { ; }
   ~DataDiggerTest( ) { ; }

};
  
TEST_F( DataDiggerTest, GetIJKObservable )
{
   casa::ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "OttolandWithGenex.project3d" ) );

   casa::ObsSpace       & obs = sc.obsSpace();
   casa::DataDigger     & dd = sc.dataDigger();
   casa::RunCaseSetImpl & rcs = dynamic_cast<casa::RunCaseSetImpl&>( sc.doeCaseSet() );

   obs.addObservable( ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2751.0, "Temperature", 0.01 ) );
   obs.addObservable( ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2730.0, "Vr", 0.002 ) );

   // add new case and set case project path
   std::vector<RunCase*> rcv;
   RunCaseImpl * rc = new RunCaseImpl();
   rcv.push_back( rc );
   rc->setProjectPath( "./OttolandWithGenex.project3d" );
   rc->loadProject();

   rcs.addNewCases( rcv, "DataDiggerTest" );

   // test itself
   dd.requestObservables( obs, rcs ); // update SnapshotIoTable and Property table

   rc->caseModel()->saveModelToProjectFile( "./OttolandWithGenextDD.project3d" );

   dd.collectRunResults( obs, rcs );
}

