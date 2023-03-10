//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FluidType.h"
#include "Formation.h"
#include "LithoType.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "Snapshot.h"
#include "Surface.h"
#include "SerialGrid.h"

#include <memory>
#include <gtest/gtest.h>
#include <sstream> //std::stringstream

struct DisplayedMsgs
{
    const std::string mobLayMissMsg = "Basin_Warning:  Missing thickness history encountered for an active MobileLayer, neglecting use of mobile layer! check MobLayThicknIoTbl for further details!\n";

#ifdef _WIN32
    // dunno why the behaviour is different??
    const std::string match = "(database::ProjectFileHandler::loadOutputTables) Output tables file does not exist: Project_mobLay_CauldronOutputDir/Output.iotables3d\n";
#else
    const std::string match = "(loadOutputTables) Output tables file does not exist: Project_mobLay_CauldronOutputDir/Output.iotables3d\n";

#endif // _WIN32

};

TEST(DataAccess, OpenCauldronProjectFail)
{
   DataAccess::Interface::ObjectFactory factory;
   DataAccess::Interface::ObjectFactory* factoryptr = &factory;
   DataAccess::Interface::ProjectHandle* ph = nullptr;
   try
   {
      ph = DataAccess::Interface::OpenCauldronProject("THIS_FILE_DOES_NOT_EXIST", factoryptr);
      FAIL() << "Expected exception";
   }
   catch(...)
   {
      delete ph;
      SUCCEED();
   }
}

TEST(DataAccess, OpenCauldronProject)
{
   DataAccess::Interface::ObjectFactory factory;
   DataAccess::Interface::ObjectFactory* factoryptr = &factory;
   std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
   try
   {
      ph.reset( DataAccess::Interface::OpenCauldronProject("DataAccessTest.project3d", factoryptr) );
   }
   catch(...)
   {
      FAIL() << "Unexpected exception caught";
   }

   if( nullptr == ph ) FAIL();

   // Check name
   EXPECT_EQ( ph->getName(), "DataAccessTest.project3d" );
   EXPECT_EQ( ph->getProjectName(), "DataAccessTest" );

   // Check grid
   if( ph->getInputGrid() == nullptr ) FAIL();
   EXPECT_DOUBLE_EQ( ph->getInputGrid()->minI(), 50. );
   EXPECT_DOUBLE_EQ( ph->getInputGrid()->minJ(), 50. );
   EXPECT_EQ( ph->getInputGrid()->numI(), 1000 );
   EXPECT_EQ( ph->getInputGrid()->numJ(), 1000 );
   EXPECT_DOUBLE_EQ( ph->getInputGrid()->deltaI(), 100. );
   EXPECT_DOUBLE_EQ( ph->getInputGrid()->deltaJ(), 100. );

   // Check litho types
   std::unique_ptr<DataAccess::Interface::LithoTypeList> lithoList(ph->getLithoTypes());
   if( lithoList == nullptr ) FAIL();
   EXPECT_EQ( lithoList->size(), 11 );
   const DataAccess::Interface::LithoType * litho = (*lithoList)[0];
   EXPECT_EQ( litho->getName(), "SM. Sandstone" );
   EXPECT_DOUBLE_EQ( litho->getDensity(), 2650. );
   EXPECT_DOUBLE_EQ( litho->getSurfacePorosity(), 60. );

   // Check snapshots
   std::unique_ptr<DataAccess::Interface::SnapshotList> majorSnap(ph->getSnapshots(DataAccess::Interface::MAJOR));
   std::unique_ptr<DataAccess::Interface::SnapshotList> minorSnap(ph->getSnapshots(DataAccess::Interface::MINOR));
   if( majorSnap == nullptr ) FAIL();
   EXPECT_EQ( majorSnap->size(), 11 );
   if( minorSnap == nullptr ) FAIL();
   EXPECT_EQ( minorSnap->size(), 39 );
   const DataAccess::Interface::Snapshot * snap = (*majorSnap)[1];
   EXPECT_DOUBLE_EQ( snap->getTime(), 10. );
   EXPECT_EQ( snap->getFileName(), "Time_10.000000.h5" );

   // Check formations
   std::unique_ptr<DataAccess::Interface::FormationList> formationList(ph->getFormations());
   if( formationList == nullptr ) FAIL();
   EXPECT_EQ( formationList->size(), 10 );
   const DataAccess::Interface::Formation * formation = (*formationList)[0];
   EXPECT_EQ( formation->getTopSurfaceName(), "Surface 0" );
   EXPECT_EQ( formation->getName(), "Formation 1" );
   EXPECT_EQ( formation->getDepositionSequence(), 10 );
   EXPECT_EQ( formation->getLithoType1()->getName(), "Std. Siltstone" );
   EXPECT_EQ( formation->getLithoType2()->getName(), "Std. Sandstone" );
   EXPECT_EQ( formation->getLithoType3()->getName(), "Std. Shale" );
   EXPECT_EQ( formation->getFluidType()->getName(), "Std. Marine Water" );
   EXPECT_EQ( formation->getSourceRockType1Name(), "" );

   // Check surfaces
   std::unique_ptr<DataAccess::Interface::SurfaceList> surfList(ph->getSurfaces());
   if( surfList == nullptr ) FAIL();
   EXPECT_EQ( surfList->size(), 11 );
   const DataAccess::Interface::Surface * surf = (*surfList)[0];
   EXPECT_EQ( surf->getBottomFormation()->getName(), "Formation 1" );
   EXPECT_EQ( surf->getFormationDepoSequenceNumber(), 10 );
   EXPECT_EQ( surf->getName(), "Surface 0" );
   EXPECT_DOUBLE_EQ( surf->getSnapshot()->getTime(), 0. );

   // Check reservoirs
   std::unique_ptr<DataAccess::Interface::ReservoirList> resList(ph->getReservoirs());
   if( resList == nullptr ) FAIL();
   EXPECT_EQ( resList->size(), 0 );

   // Check fluid types
   std::unique_ptr<DataAccess::Interface::FluidTypeList> fluidList(ph->getFluids());
   if( fluidList == nullptr ) FAIL();
   EXPECT_EQ( fluidList->size(), 4 );
   const DataAccess::Interface::FluidType * fluid = (*fluidList)[1];
   EXPECT_EQ( fluid->getName(), "Std. Marine Water" );
   EXPECT_DOUBLE_EQ( fluid->getConstantSeismicVelocity(), 1500. );
   EXPECT_EQ( fluid->getSeismicVelocityCalculationModel(), DataAccess::Interface::CALCULATED_MODEL );
   EXPECT_DOUBLE_EQ( fluid->salinity(), 0.13 );

   // Check AllochthonousLithology
   EXPECT_TRUE( ph->getAllochthonousLithologyDistributions() == nullptr );
   EXPECT_TRUE( ph->getAllochthonousLithologyInterpolations() == nullptr );

   // Check LitThCondIoTbl
   std::unique_ptr<DataAccess::Interface::LithologyThermalConductivitySampleList> ltcList(ph->getLithologyThermalConductivitySampleList());
   if( ltcList == nullptr ) FAIL();
   EXPECT_EQ( ltcList->size(), 116 );

   // Check LitHeatCapIoTbl
   std::unique_ptr<DataAccess::Interface::LithologyHeatCapacitySampleList> lhcList(ph->getLithologyHeatCapacitySampleList());
   if( lhcList == nullptr ) FAIL();
   EXPECT_EQ( lhcList->size(), 105 );

   // Check FltThCondIoTbl
   std::unique_ptr<DataAccess::Interface::FluidThermalConductivitySampleList> ftcList(ph->getFluidThermalConductivitySampleList( ph->findFluid("Std. Water") ));
   if( ftcList == nullptr ) FAIL();
   EXPECT_EQ( ftcList->size(), 667 );

   // Check FltHeatCapIoTbl
   std::unique_ptr<DataAccess::Interface::FluidHeatCapacitySampleList> fhcList(ph->getFluidHeatCapacitySampleList( ph->findFluid("Std. Water") ));
   if( fhcList == nullptr ) FAIL();
   EXPECT_EQ( fhcList->size(), 156 );
}

#include "LogHandler.h"

TEST(DataAccess, MobileLayerMissingEntries)
{
    DataAccess::Interface::ObjectFactory factory;
    DataAccess::Interface::ObjectFactory* factoryptr = &factory;
    // create the actual log file
    LogHandler logUnitTestDiagnostic("logHandler", LogHandler::DIAGNOSTIC_LEVEL);
    // try and capture the console output also
    testing::internal::CaptureStdout();
    std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
    try
    {
        ph.reset(DataAccess::Interface::OpenCauldronProject("Project_mobLay.project3d", factoryptr));
        // for this scenario a log file with the active mobile layer history missing warning should be created
        bool fileExisits = false;
        DisplayedMsgs msg;
        std::ifstream myfile(logUnitTestDiagnostic.getName().c_str());
            if (myfile.is_open()) {

                // read the file content as a string and that should
                // contain the said ML missing msg
                std::stringstream strStream;
                strStream << myfile.rdbuf(); //read the file
                std::string str = strStream.str(); //str holds the content of the file

                EXPECT_TRUE((str.find(msg.mobLayMissMsg) != std::string::npos));

                fileExisits = true;
            }
            // the file should be created with the msg and...
            EXPECT_EQ(fileExisits, true);

            // the same warning message should apear in the console
            std::string output = testing::internal::GetCapturedStdout();
            EXPECT_EQ(output, msg.match + msg.mobLayMissMsg);
            
    }
    catch (std::exception& ex) {
        EXPECT_STREQ("My exception happened", ex.what());
    }
    catch (...)
    {
        FAIL() << "Unexpected exception caught";
    }
    
    if (nullptr == ph) FAIL();

    // Check name
    EXPECT_EQ(ph->getName(), "Project_mobLay.project3d");
    EXPECT_EQ(ph->getProjectName(), "Project_mobLay");
}