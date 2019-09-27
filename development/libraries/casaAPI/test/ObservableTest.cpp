#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/Observable.h"
#include "../src/ObsGridPropertyWell.h"
#include "../src/ObsGridPropertyXYZ.h"
#include "../src/ObsGridXYLayerTopSurfaceProp.h"
#include "../src/ObsSourceRockMapProp.h"
#include "../src/ObsSpaceImpl.h"
#include "../src/ObsValueDoubleScalar.h"
#include "../src/PrmWindow.h"
#include "../src/RunCaseImpl.h"
#include "../src/RunCaseSetImpl.h"

#include <memory>

#include <gtest/gtest.h>

// Auxiliary functions and variables

ibs::FolderPath copyFileFormBaseToCase( const ibs::FolderPath & casePath
                                      , const std::string     & baseProj3dFilePathStr
                                      , const std::string     & fileName
                                      , const bool              cutLast
                                      )
{
  ibs::FilePath baseFilePath( baseProj3dFilePathStr );

  if ( cutLast )
  {
    baseFilePath.cutLast();
    baseFilePath << fileName;
  }

  ibs::FilePath caseFilePath( casePath );
  caseFilePath << fileName;
  baseFilePath.copyFile( caseFilePath );

  return caseFilePath;
}

// Well target data for Two Way Time.

std::vector<double> xWell0 = { 186980.8, 186980.8, 186980.8 };
std::vector<double> xWell1 = { 186900.2, 186900.2, 186900.2, 186900.2 };
std::vector<double> xWell2 = { 191772.5, 191772.5 };

std::vector<double> yWell0 = { 610967.1, 610967.1, 610967.1 };
std::vector<double> yWell1 = { 610869.9, 610869.9, 610869.9, 610869.9 };
std::vector<double> yWell2 = { 615200.3, 615200.3 };

std::vector<double> zWell0 = { 0.0, 529.8, 1084.8 };
std::vector<double> zWell1 = { 0.0, 656, 843, 1111.8 };
std::vector<double> zWell2 = { 0.0, 249.6 };

std::vector<double> TWTTWell0 = { 0.0, 557.8, 1107.8 };
std::vector<double> TWTTWell1 = { 0.0, 662, 840.6, 1104.6 };
std::vector<double> TWTTWell2 = { 0.0, 268 };

TEST( ObservableTest, ObsMatchesWindowObs )
{
   casa::ScenarioAnalysis scenario;
   const std::string proj3dFileName = "Ameland.project3d";
   const std::string baseProj3dFilePathStr = "./Ameland/" + proj3dFileName;
   ibs::FolderPath baseProj3dFilePath( baseProj3dFilePathStr );
   ASSERT_EQ( ErrorHandler::NoError, scenario.defineBaseCase( baseProj3dFilePath.cpath() ) );

   casa::ObsSpace& obs = scenario.obsSpace();
   casa::RunCaseSet & rcs = scenario.doeCaseSet();
   std::vector<std::shared_ptr<casa::RunCase>> rcv;

   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell0, yWell0, zWell0, "TwoWayTime", 0.0 ) );
   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell1, yWell1, zWell1, "TwoWayTime", 0.0 ) );
   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell2, yWell2, zWell2, "TwoWayTime", 0.0 ) );

   std::vector<std::vector<double>> obsMatchingWindowObsExpected =
   { { true,  false, false }
   , { false, true,  false }
   , { false, false, true  } };

   int    minI, maxI, minJ, maxJ;
   double centreX, centreY;
   for ( int i = 0; i < obs.size(); ++i )
   {
     mbapi::Model & baseCaseCurrent = scenario.baseCase();
     ASSERT_EQ( ErrorHandler::NoError, baseCaseCurrent.windowSize( obs.observable(i)->xCoords().front(), obs.observable(i)->yCoords().front(), minI, maxI, minJ, maxJ, centreX, centreY ) );
     casa::PrmWindow prmWindow( minI, maxI, minJ, maxJ, obs.observable(i)->xCoords().front(), obs.observable(i)->yCoords().front() );

     std::shared_ptr<casa::RunCaseImpl>  rc( new casa::RunCaseImpl( ) );
     rcv.push_back( rc );

     // Create folder for current runCase, copy projct3d file and Set path
     const std::string casePathName = "ObsMatchesWindowObsTestObservable_case_" + std::to_string( i );
     ibs::FolderPath casePath( baseProj3dFilePathStr );
     casePath.cutLast();
     casePath.cutLast();
     casePath << casePathName;
     if ( casePath.exists() )
     {
       casePath.remove();
     }
     casePath.create();

     ibs::FilePath caseProj3dFilePath = copyFileFormBaseToCase( casePath, baseProj3dFilePathStr, proj3dFileName, false );

     rc->setProjectPath( caseProj3dFilePath.path().c_str() );
     rc->mutateCaseTo( baseCaseCurrent, caseProj3dFilePath.fullPath().cpath() );

     // Write observable coordinates to windowObservable file
     ibs::FilePath windowObsFile( casePath );
     windowObsFile << "windowObservable.txt";
     std::ofstream ofsWindowObs( windowObsFile.path().c_str(), std::ios_base::out | std::ios_base::trunc );
     ASSERT_FALSE ( ofsWindowObs.fail() );
     const std::vector<double> & prmWindowObsOrigin = prmWindow.observableOrigin();
     std::ostringstream ossWindowObsStream;
     ossWindowObsStream << std::fixed;
     ossWindowObsStream << prmWindowObsOrigin[0] << " " << prmWindowObsOrigin[1];
     std::string ossWindowObs = ossWindowObsStream.str();
     ofsWindowObs << ossWindowObs;
     ofsWindowObs.close();

     rc->loadProject();
   }
   rcs.addNewCases( rcv, "ObsMatchesWindowObsTest" );

   for ( int i = 0; i < rcs.size(); ++i )
   {
     mbapi::Model * baseCaseCurrent = rcs.runCase( i )->caseModel();

     for ( int j = 0; j < obs.size(); ++j )
     {
       const bool obsMatchingWindowObsActual = obs.observable(j)->checkObservableOriginForProject( (*baseCaseCurrent) );
       ASSERT_EQ( obsMatchingWindowObsExpected[i][j], obsMatchingWindowObsActual );
     }
   }
}

TEST( ObservableTest, ObsGridXYLayerTopSurfacePropRequestInModel )
{
   casa::ScenarioAnalysis scenario;
   const std::string project3dFileName = "OttolandWithGenex.project3d";
   ASSERT_EQ( ErrorHandler::NoError, scenario.defineBaseCase( project3dFileName.c_str() ) );

   casa::ObsSpace & obs = scenario.obsSpace();

   obs.addObservable( casa::ObsGridXYLayerTopSurfaceProp::createNewInstance( 460001.0, 6750001.0, "Lower Jurassic", "Temperature", 65.0 ) );
   obs.addObservable( casa::ObsGridXYLayerTopSurfaceProp::createNewInstance( 460001.0, 6750001.0, "Lower Jurassic", "Vr", 100.0 ) );

   std::shared_ptr<casa::RunCaseImpl> rc( new casa::RunCaseImpl( ) );
   rc->setProjectPath( project3dFileName.c_str() );
   rc->loadProject();

   mbapi::Model * mdl = rc->caseModel();

   ASSERT_TRUE( mdl );

   for ( int i = 0; i < obs.size(); ++i  )
   {
     casa::Observable * myObs = obs[i];
     EXPECT_EQ( ErrorHandler::NoError, myObs->requestObservableInModel( *mdl ) );
   }
}

TEST( ObservableTest, ObsGetFromModel )
{
   const std::vector<double> obsVals = {76.057103, 76.536606, 63.617744, 0.414689};

   casa::ScenarioAnalysis scenario;
   const std::string project3dFileName = "OttolandWithGenexObservables.project3d";
   ASSERT_EQ( ErrorHandler::NoError, scenario.defineBaseCase( project3dFileName.c_str() ) );

   casa::ObsSpace & obsVars = scenario.obsSpace();
   ASSERT_EQ( ErrorHandler::NoError, obsVars.addObservable( casa::ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 1293.0, "Temperature", 0.0 ) ) );
   ASSERT_EQ( ErrorHandler::NoError, obsVars.addObservable( casa::ObsSourceRockMapProp::createNewInstance( 460001.0, 6750001.0, "Lower Jurassic", "Temperature", 65.0 ) ) );
   ASSERT_EQ( ErrorHandler::NoError, obsVars.addObservable( casa::ObsGridXYLayerTopSurfaceProp::createNewInstance( 460001.0, 6750001.0, "Lower Jurassic", "Temperature", 65.0 ) ) );
   ASSERT_EQ( ErrorHandler::NoError, obsVars.addObservable( casa::ObsGridXYLayerTopSurfaceProp::createNewInstance( 460001.0, 6750001.0, "Lower Jurassic", "Vr", 100.0 ) ) );

   mbapi::Model & baseCase = scenario.baseCase();

   scenario.baseCase().clearTable("DataMiningIoTbl");

   std::shared_ptr<casa::RunCaseImpl> rc( new casa::RunCaseImpl( ) );
   rc->setProjectPath( project3dFileName.c_str() );
   mbapi::Model & mdl = baseCase;

   rc->mutateCaseTo(mdl, project3dFileName.c_str());
   rc->loadProject();

   for ( int i = 0; i < obsVars.size(); ++i )
   {
     EXPECT_EQ( ErrorHandler::NoError, obsVars[i]->requestObservableInModel( mdl ) );
     EXPECT_EQ( ErrorHandler::NoError, mdl.setTableValue( "DataMiningIoTbl", i, "Value", obsVals[i] ) );
     EXPECT_TRUE( ErrorHandler::NoError == mdl.saveModelToProjectFile( project3dFileName.c_str() ) );

     const casa::ObsValue * obsVal = obsVars[i]->getFromModel( mdl );
     EXPECT_TRUE( obsVal );
     const std::vector<double> vals = obsVal->asDoubleArray();
     EXPECT_EQ( vals[0], obsVals[i] );
   }
}
