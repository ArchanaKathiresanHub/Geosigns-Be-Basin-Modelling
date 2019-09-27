#include "../src/cmbAPI.h"
#include "../src/casaAPI.h"
#include "../src/ObsGridPropertyWell.h"
#include "../src/ObsGridPropertyXYZ.h"
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

TEST( DataDiggerTest, GetIJKObservable )
{
   casa::ScenarioAnalysis sc;
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( "OttolandWithGenex.project3d" ) );

   casa::ObsSpace & obs = sc.obsSpace();
   casa::DataDigger & dd = sc.dataDigger();
   casa::RunCaseSet & rcs = sc.doeCaseSet();

   obs.addObservable( casa::ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2751.0, "Temperature", 0.01 ) );
   obs.addObservable( casa::ObsGridPropertyXYZ::createNewInstance( 460001.0, 6750001.0, 2730.0, "Vr", 0.002 ) );

   // add new case and set case project path
   std::vector<std::shared_ptr<casa::RunCase>> rcv;
   std::shared_ptr<casa::RunCaseImpl>  rc( new casa::RunCaseImpl( ) );
   rcv.push_back( rc );
   rc->setProjectPath( "./OttolandWithGenex.project3d" );
   rc->loadProject();

   rcs.addNewCases( rcv, "DataDiggerTest" );

   // test itself
   ASSERT_EQ( ErrorHandler::NoError, dd.requestObservables( obs, rcs ) ); // update SnapshotIoTable and Property table

   ASSERT_EQ( ErrorHandler::NoError, rc->caseModel()->saveModelToProjectFile( "./OttolandWithGenextDD.project3d" ) );

   ASSERT_EQ( ErrorHandler::NoError, dd.collectRunResults( obs, rcs ) );
}

TEST( DataDiggerTest, GetWellWindowObservable )
{
   casa::ScenarioAnalysis sc;

   const std::string proj3dFileName = "Ameland.project3d";
   const std::string baseProj3dFilePathStr = "./Ameland/" + proj3dFileName;
   ibs::FolderPath baseProj3dFilePath( baseProj3dFilePathStr );
   ASSERT_EQ( ErrorHandler::NoError, sc.defineBaseCase( baseProj3dFilePath.cpath() ) );

   casa::ObsSpace & obs = sc.obsSpace();
   casa::DataDigger & dd = sc.dataDigger();
   casa::RunCaseSet & rcs = sc.doeCaseSet();

   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell0, yWell0, zWell0, "TwoWayTime", 0.0 ) );
   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell1, yWell1, zWell1, "TwoWayTime", 0.0 ) );
   obs.addObservable( casa::ObsGridPropertyWell::createNewInstance( xWell2, yWell2, zWell2, "TwoWayTime", 0.0 ) );

   std::vector<std::shared_ptr<casa::RunCase>> rcv;

   int    minI, maxI, minJ, maxJ;
   double centreX, centreY;
   for ( int i = 0; i < obs.size(); ++i )
   {
     mbapi::Model & baseCaseCurrent = sc.baseCase();
     ASSERT_EQ( ErrorHandler::NoError, baseCaseCurrent.windowSize( obs.observable(i)->xCoords().front(), obs.observable(i)->yCoords().front(), minI, maxI, minJ, maxJ, centreX, centreY ) );
     casa::PrmWindow prmWindow( minI, maxI, minJ, maxJ, obs.observable(i)->xCoords().front(), obs.observable(i)->yCoords().front() );

     std::shared_ptr<casa::RunCaseImpl>  rc( new casa::RunCaseImpl( ) );
     rcv.push_back( rc );

     // Create folder for current runCase, copy projct3d file and Set path
     const std::string casePathName = "GetWellWindowObsTestDataDigger_case_" + std::to_string( i );
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
     rc->setProjectPath( caseProj3dFilePath.fullPath().cpath() );
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
   rcs.addNewCases( rcv, "DataDiggerTestWellWindowObs" );

   for ( int i = 0; i < rcs.size(); ++i )
   {
     ASSERT_EQ( ErrorHandler::NoError, dd.requestObservablesInWindow( obs, rcs.runCase(i) ) ); // update SnapshotIoTable and Property table
   }
}
