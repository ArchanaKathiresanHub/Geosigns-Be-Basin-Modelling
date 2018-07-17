#include "../src/cmbAPI.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

#include "FilePath.h"

//#include <cmath>

#include <gtest/gtest.h>

static const double eps = 1.e-5;

class mbapiModelTest : public ::testing::Test
{
public:
   mbapiModelTest( )  { ; }
   ~mbapiModelTest( ) { ; }

   bool compareFiles( const char * projFile1, const char * projFile2 );
   bool compareModels( mbapi::Model & model1, mbapi::Model & model2 );

   static const char * m_sourceRockTestProject;
   static const char * m_testProject;
   static const char * m_lithologyTestProject;
   static const char * m_fluidTestProject;
   static const char * m_dupLithologyTestProject;
   static const char * m_mapsTestProject;
   static const char * m_nnTestProject;
};

const char * mbapiModelTest::m_sourceRockTestProject   = "SourceRockTesting.project3d";
const char * mbapiModelTest::m_lithologyTestProject    = "LithologyTesting.project3d";
const char * mbapiModelTest::m_fluidTestProject        = "FluidTesting.project3d";
const char * mbapiModelTest::m_testProject             = "Project.project3d";
const char * mbapiModelTest::m_mapsTestProject         = "MapsTesting.project3d";
const char * mbapiModelTest::m_dupLithologyTestProject = "DupLithologyTesting.project3d";
const char * mbapiModelTest::m_nnTestProject           = "NNTesting.project3d";

bool mbapiModelTest::compareFiles( const char * projFile1, const char * projFile2 )
{
   struct stat filestatus;
   stat( projFile1, &filestatus );
   off_t case1Size = filestatus.st_size;
   stat( projFile2, &filestatus );

   bool ret = case1Size == filestatus.st_size;

   if ( ret )
   {
      std::ifstream p1f( projFile1 );
      std::ifstream p2f( projFile2 );

      size_t lineNum = 0;
      while ( ret && p1f.good() && p2f.good() )
      {
         char buf1[ 2048 ];
         char buf2[ 2048 ];

         p1f.getline( buf1, sizeof( buf1 ) / sizeof( char ) );
         p2f.getline( buf2, sizeof( buf2 ) / sizeof( char ) );

         if ( lineNum > 1 && strcmp( buf1, buf2 ) ) // ignore 2 first lines because of filename and dates there
         {
            ret = false;
         }
         lineNum++;
      }
      // check that both files with the same state
      ret = ret && ( p1f.good() == p2f.good() );
   }
   return ret;
}

bool mbapiModelTest::compareModels( mbapi::Model & model1, mbapi::Model & model2 )
{
   const char * projFile1 = "project_model_1.proj3d";
   const char * projFile2 = "project_model_2.proj3d";
   
   model1.saveModelToProjectFile( projFile1 );
   model2.saveModelToProjectFile( projFile2 );

   bool isTheSame = compareFiles( projFile1, projFile2 );

   // clean files copy
   remove( projFile1 );
   remove( projFile2 );

   return isTheSame;
}

// load model, save it to another file. Then load this file and save again.
// compare second and third project file to be the same
TEST_F( mbapiModelTest, ModelLoadSaveProjectRoundTrip )
{
   std::unique_ptr<mbapi::Model> modelBase;
   modelBase.reset( new mbapi::Model() );

   // load original project file
   ASSERT_EQ( ErrorHandler::NoError, modelBase->loadModelFromProjectFile( m_testProject ) );

   // Save a first copy 
   ASSERT_EQ( ErrorHandler::NoError, modelBase->saveModelToProjectFile( "Project_case1.project3d" ) );

   std::unique_ptr<mbapi::Model> modelCase2;
   modelCase2.reset( new mbapi::Model() );
   // load first copy
   ASSERT_EQ( ErrorHandler::NoError, modelCase2->loadModelFromProjectFile( "Project_case1.project3d" ) );
   // save secon copy
   ASSERT_EQ( ErrorHandler::NoError, modelCase2->saveModelToProjectFile(   "Project_case2.project3d" ) );
   
   // compare first and second copies of project files to be the same
   ASSERT_TRUE( compareFiles( "Project_case1.project3d", "Project_case2.project3d" ) );

   // clean files copy
   remove( "Project_case1.project3d" );
   remove( "Project_case2.project3d" );
}

// Test top crust heat production coefficient set/get methods
TEST_F( mbapiModelTest, BasementPropertySetting )
{
   mbapi::Model testModel;
   // load project file
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_testProject ) );

   // check that BasementIoTable has only 1 record
   ASSERT_EQ( testModel.tableSize( "BasementIoTbl" ), 1 );

   // Get bottom boundary condition model name. Must be "Fixed Temperature"
   std::string modelName = testModel.tableValueAsString( "BasementIoTbl", 0, "BottomBoundaryModel" );
   ASSERT_EQ( testModel.errorCode(), ErrorHandler::NoError );
   ASSERT_TRUE( modelName == "Fixed Temperature" );

   // Get value for top crust heat production rate
   double topCrustHeatProd = testModel.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" );
   ASSERT_EQ( ErrorHandler::NoError, testModel.errorCode() );
   // must be 2.5
   ASSERT_NEAR( topCrustHeatProd, 2.5, eps );

   // change it value to 4.0
   ASSERT_EQ( ErrorHandler::NoError, testModel.setTableValue( "BasementIoTbl", 0, "TopCrustHeatProd", 4.0 ) );
   // and save as a new project
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "Project_basem_prop.project3d" ) );

   // reaload project to another model
   mbapi::Model modifModel;
   ASSERT_EQ( ErrorHandler::NoError, modifModel.loadModelFromProjectFile( "Project_basem_prop.project3d" ) );

   // get value of top crust production rate
   topCrustHeatProd = modifModel.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" );
   ASSERT_EQ( ErrorHandler::NoError, testModel.errorCode() );
   
   // must be 4.0 (as it was set before)
   ASSERT_NEAR( topCrustHeatProd, 4.0, eps );

   // delete copy of the project
   remove( "Project_basem_prop.project3d" );
}

// Test set/get TOC in source rock lithology
TEST_F( mbapiModelTest, SourceRockTOCSettings )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_sourceRockTestProject ) );

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   // change TOC values to some others - 19, 70
   double ltocOld[] = { 70.2, 10.0 };
   double ltocNew[] = { 19.0, 70.0 };
   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for ( size_t i = 0; i < sizeof( ltocOld ) / sizeof( double ); ++i )
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( lNames[i] );
      ASSERT_EQ( IsValueUndefined( lid ), false );
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName( lid );
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2U : 1U) );

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( lNames[i], layerSourceRocks.front() );
      ASSERT_EQ( IsValueUndefined( sid ), false );
      
      // check what was set before
      double tocInFile = srMgr.tocIni( sid );
      ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
      ASSERT_NEAR( tocInFile, ltocOld[i], eps );

      // set the new value
      ASSERT_EQ( ErrorHandler::NoError, srMgr.setTOCIni( sid, ltocNew[i] ) );
   }

   // save as a new temporary project file
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "ChangedTOC.project3d" ) );

   // reload this new project file into other model
   mbapi::Model modifModel;
   ASSERT_EQ( ErrorHandler::NoError, modifModel.loadModelFromProjectFile( "ChangedTOC.project3d" ) );
   
   mbapi::SourceRockManager   & srModMgr = modifModel.sourceRockManager();
   mbapi::StratigraphyManager & stModMgr = modifModel.stratigraphyManager();

   // check values for the TOC
   for ( size_t i = 0; i < sizeof( ltocNew ) / sizeof( double ); ++i )
   {
      // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stModMgr.layerID( lNames[i] );
      ASSERT_EQ( IsValueUndefined( lid ), false );

      const std::vector<std::string> & layerSourceRocks = stModMgr.sourceRockTypeName( lid );
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2U : 1U) );

      mbapi::SourceRockManager::SourceRockID sid = srModMgr.findID( lNames[i], layerSourceRocks.front() );
      ASSERT_EQ( IsValueUndefined( sid ), false );

      // check if the new values are set
      double tocInFile = srModMgr.tocIni( sid );
      ASSERT_EQ( ErrorHandler::NoError, srModMgr.errorCode() );
      ASSERT_NEAR( tocInFile, ltocNew[i], eps );
   }

   // delete temporary project file
   remove( "ChangedTOC.project3d" );
}

// Test set/get TOC in source rock lithology
TEST_F( mbapiModelTest, SourceRockHI_HCSettings )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_sourceRockTestProject ) );

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for ( size_t i = 0; i < 2; ++i )
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( lNames[i] );
      ASSERT_EQ( IsValueUndefined( lid ), false );

      // source rock must be active
      ASSERT_EQ( stMgr.isSourceRockActive( lid ), true );

      // get source rock type
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName( lid );

      // and find it id
      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( lNames[i], layerSourceRocks.front() );
      ASSERT_EQ( IsValueUndefined( sid ), false );
 
      double hiSR = srMgr.hiIni( sid );
      double hcSR = srMgr.hcIni( sid );
 
      // check mixing settings
      switch ( i )
      {
         case 0:  // Wetphalian
            {
               ASSERT_EQ( stMgr.isSourceRockMixingEnabled( lid ), true );
               ASSERT_EQ( layerSourceRocks.size(), 2U );
               ASSERT_NEAR( stMgr.sourceRockMixHI( lid ), 700.0, eps );
   
               mbapi::SourceRockManager::SourceRockID sid2 = srMgr.findID( lNames[i], layerSourceRocks.back() );
               ASSERT_EQ( IsValueUndefined( sid2 ), false );
               
               double hiSR2 = srMgr.hiIni( sid2 );
               double hcSR2 = srMgr.hcIni( sid2 );
            
               ASSERT_NEAR( hiSR,   94.364368, eps );
               ASSERT_NEAR( hiSR2, 472.068687, eps );

               ASSERT_NEAR( hcSR,  0.801, eps );
               ASSERT_NEAR( hcSR2, 1.25, eps );

               srMgr.setHCIni( sid2, 1.0 );
               ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
               ASSERT_NEAR( srMgr.hiIni( sid2 ), 193.17523, eps );
            }
            break;

         case 1:  // Lower Jurassic
            ASSERT_EQ( stMgr.isSourceRockMixingEnabled( lid ), false );
            ASSERT_EQ( layerSourceRocks.size(), 1U );
            ASSERT_NEAR( stMgr.sourceRockMixHI( lid ), 0, eps );

            ASSERT_NEAR( hiSR,   472.068687, eps );
            ASSERT_NEAR( hcSR,  1.25, eps );

            srMgr.setHIIni( sid, 500.0 );
            ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
            ASSERT_NEAR( srMgr.hcIni( sid ), 1.261, eps );
          break;
      }
   }
}

TEST_F (mbapiModelTest, SnapshotManager )
{
   mbapi::Model testModel;
   // load project file
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_testProject ) );

   // check that SnapshotIoTable has only 5 records
   ASSERT_EQ( testModel.snapshotManager().snapshotsNumber(), 5U );
   
   // add snapshot
   ASSERT_EQ( ErrorHandler::NoError, testModel.snapshotManager().requestMajorSnapshot( 10.0 ) );
   ASSERT_EQ( testModel.snapshotManager().snapshotsNumber(), 6U );

   ASSERT_NEAR( testModel.snapshotManager().time( 1 ), 10.0, eps );
   ASSERT_EQ( testModel.snapshotManager().isMinor( 1 ), false );
}


// Test set/get TOC in source rock lithology
TEST_F( mbapiModelTest, SourceRockPreAsphActEnergySettings )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_sourceRockTestProject ) );

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   // change preasphalt activation energy values to some others - 207, 211
   double lvalOld[] = { 206.0, 210.0 };
   double lvalNew[] = { 207.0, 211.0 };
   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for ( size_t i = 0; i < sizeof( lvalOld ) / sizeof( double ); ++i )
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( lNames[i] );
      ASSERT_EQ( IsValueUndefined( lid ), false );
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName( lid );
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2U : 1U) );

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( lNames[i], layerSourceRocks.front() );
      ASSERT_EQ( IsValueUndefined( sid ), false );

      // check what was set before
      double valInFile = srMgr.preAsphActEnergy( sid );
      ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
      ASSERT_NEAR( valInFile, lvalOld[i], eps );

      // set the new value
      ASSERT_EQ( ErrorHandler::NoError, srMgr.setPreAsphActEnergy( sid, lvalNew[i] ) );
   }

   // save as a new temporary project file
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "ChangedPreAsphEnergy.project3d" ) );

   // reload this new project file into other model
   mbapi::Model modifModel;
   ASSERT_EQ( ErrorHandler::NoError, modifModel.loadModelFromProjectFile( "ChangedPreAsphEnergy.project3d" ) );

   mbapi::SourceRockManager   & srModMgr = modifModel.sourceRockManager();
   mbapi::StratigraphyManager & stModMgr = modifModel.stratigraphyManager();

   // check values for the TOC
   for ( size_t i = 0; i < sizeof( lvalNew ) / sizeof( double ); ++i )
   {
      // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stModMgr.layerID( lNames[i] );
      ASSERT_EQ( IsValueUndefined( lid ), false );

      const std::vector<std::string> & layerSourceRocks = stModMgr.sourceRockTypeName( lid );
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2U : 1U) );

      mbapi::SourceRockManager::SourceRockID sid = srModMgr.findID( lNames[i], layerSourceRocks.front() );
      ASSERT_EQ( IsValueUndefined( sid ), false );

      // check if the new values are set
      double valInFile = srModMgr.preAsphActEnergy( sid );
      ASSERT_EQ( ErrorHandler::NoError, srModMgr.errorCode() );
      ASSERT_NEAR( valInFile, lvalNew[i], eps );
   }

   // delete temporary project file
   remove( "ChangedPreAsphEnergy.project3d" );
}

TEST_F( mbapiModelTest, CopyLithologyTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_testProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();
   
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );

   // first try to copy to existent lithology
   ASSERT_TRUE( IsValueUndefined( lthMgr.copyLithology( lid, "Crust" ) ) );
   ASSERT_EQ( lthMgr.errorCode(), ErrorHandler::AlreadyDefined );
   lthMgr.resetError();

   size_t lithNum = lthMgr.lithologiesIDs().size();
   
   mbapi::LithologyManager::LithologyID newLID = lthMgr.copyLithology( lid, "Std. Sandstone COPY" );
   
   ASSERT_FALSE( IsValueUndefined( newLID ) );
   const std::string & newName = lthMgr.lithologyName( newLID );
   ASSERT_TRUE( newName == "Std. Sandstone COPY" );
   ASSERT_EQ( lithNum+1, lthMgr.lithologiesIDs().size() );
}


TEST_F( mbapiModelTest, DeleteLithologyTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_testProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();
   
   // First create a copy of lithology
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
   mbapi::LithologyManager::LithologyID newLID = lthMgr.copyLithology( lid, "Std. Sandstone COPY" );
   ASSERT_FALSE( IsValueUndefined( newLID ) );

   // then try to delete lithology which has references. Expecting ValidationError on this.
   ASSERT_EQ( ErrorHandler::ValidationError, lthMgr.deleteLithology( lid ) );

   // and now delete copy of lithology which has no any reference
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.deleteLithology( newLID ) );

   newLID = lthMgr.findID( "Std. Sandstone COPY" );
   ASSERT_TRUE( IsValueUndefined( newLID ) );
}

TEST_F( mbapiModelTest, DeleteDuplicatedLithologyTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_dupLithologyTestProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   size_t lithNum = lthMgr.lithologiesIDs().size();

   // project file already has duplicated lithologies. Clean them
   ASSERT_EQ( lithNum, 14U );
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.cleanDuplicatedLithologies() );

   // check that 5 lithologies were deleted
   lithNum = lthMgr.lithologiesIDs().size();
   ASSERT_EQ( lithNum, 9U );
}


TEST_F( mbapiModelTest, GetPermeabilityModelParametersTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_lithologyTestProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();
   
   // Check Sands permeability
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );

   mbapi::LithologyManager::PermeabilityModel permModel;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermSandstone );
   ASSERT_EQ( modelPrms.size(), 3U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 6000.0, eps );
   ASSERT_NEAR( modelPrms[2], 1.5, eps );

   // Check Shales permeability
   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermMudstone );
   ASSERT_EQ( modelPrms.size(), 4U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 0.01, eps );
   ASSERT_NEAR( modelPrms[2], 1.5, eps );
   ASSERT_NEAR( modelPrms[3], 0.01, eps );

   // Check Nones permeability
   lid = lthMgr.findID( "Crust" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermNone );
   ASSERT_EQ( modelPrms.size(), 0U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );

   // Check Impermeable permeability
   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermImpermeable );
   ASSERT_EQ( modelPrms.size(), 0U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );

   // Check Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ(   permModel, mbapi::LithologyManager::PermMultipoint );
   ASSERT_EQ(   modelPrms.size(), 1U );
   ASSERT_NEAR( modelPrms[0],  1.0, eps );
   ASSERT_EQ(   mpPor.size(),  2U );
   ASSERT_EQ(   mpPerm.size(), 2U );
   ASSERT_NEAR( mpPor[0],  5.0, eps );
   ASSERT_NEAR( mpPor[1],  60.0, eps );
   ASSERT_NEAR( mpPerm[0], 0.3, eps );
   ASSERT_NEAR( mpPerm[1], 8.6, eps );
}



TEST_F( mbapiModelTest, SetPermeabilityModelParametersTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_lithologyTestProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();
   
   // Check Sands permeability
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID( "Crust" );
   ASSERT_FALSE( IsValueUndefined( lid ) );

   // change model from None to Impermeable
   mbapi::LithologyManager::PermeabilityModel permModel = mbapi::LithologyManager::PermImpermeable;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change from Impermeable to None
   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
 
   permModel = mbapi::LithologyManager::PermNone;
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change a bit coeff for Mudstone
   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   permModel = mbapi::LithologyManager::PermMudstone;
   modelPrms.push_back( 1.0 );  // anisothropic coeff
   modelPrms.push_back( 0.03 ); // depo por
   modelPrms.push_back( 1.55 ); // incr coeff
   modelPrms.push_back( 0.02 ); // decr coeff

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change Sandstone permeability
   lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );

   permModel = mbapi::LithologyManager::PermSandstone;
   modelPrms.resize( 1 );
   modelPrms.push_back( 7000.0 ); // depo por
   modelPrms.push_back( 1.45 ); // incr coeff

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
   permModel = mbapi::LithologyManager::PermMultipoint;

   modelPrms.resize( 1 );
   mpPor.push_back( 10.0 ); 
   mpPor.push_back( 20.0 ); 
   mpPor.push_back( 30.0 ); 

   mpPerm.push_back( 2.0 ); 
   mpPerm.push_back( 3.0 ); 
   mpPerm.push_back( 4.0 ); 

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // do round trip - save/load project file and compare parameters
   std::string newProjName ( "mod_" ); 
   newProjName += m_lithologyTestProject;

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( newProjName.c_str() ) );

   mpPor.clear();
   mpPerm.clear();

   mbapi::Model checkTestModel;
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( newProjName.c_str() ) );

   lid = lthMgr.findID( "Crust" );
   ASSERT_FALSE( IsValueUndefined( lid ) );

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermImpermeable );
   ASSERT_EQ( modelPrms.size(), 0U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );

   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermNone );
   ASSERT_EQ( modelPrms.size(), 0U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );

   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermMudstone );
   ASSERT_EQ( modelPrms.size(), 4U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 0.03, eps );
   ASSERT_NEAR( modelPrms[2], 1.55, eps );
   ASSERT_NEAR( modelPrms[3], 0.02, eps );

   lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermSandstone );
   ASSERT_EQ( modelPrms.size(), 3U );
   ASSERT_EQ( mpPor.size(), 0U );
   ASSERT_EQ( mpPerm.size(), 0U );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 7000.0, eps );
   ASSERT_NEAR( modelPrms[2], 1.45, eps );

   // Check Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_FALSE( IsValueUndefined( lid ) );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ(   permModel, mbapi::LithologyManager::PermMultipoint );
   ASSERT_EQ(   modelPrms.size(), 1U );
   ASSERT_NEAR( modelPrms[0],  1.0, eps );
   ASSERT_EQ(   mpPor.size(),  3U );
   ASSERT_EQ(   mpPerm.size(), 3U );
   ASSERT_NEAR( mpPor[0],  10.0, eps );
   ASSERT_NEAR( mpPor[1],  20.0, eps );
   ASSERT_NEAR( mpPor[2],  30.0, eps );
   ASSERT_NEAR( mpPerm[0], 2.0, eps );
   ASSERT_NEAR( mpPerm[1], 3.0, eps );
   ASSERT_NEAR( mpPerm[2], 4.0, eps );
}


TEST_F( mbapiModelTest, MapsManagerCopyMapTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_mapsTestProject ) );
   std::string mapName = "MAP-1076770443-4";
   
   // clean any previous runs result
   if ( ibs::FilePath( "MAP-1076770443-4_copy.HDF" ).exists() ) { ibs::FilePath( "MAP-1076770443-4_copy.HDF" ).remove(); }
   if ( ibs::FilePath( "Test.HDF"                  ).exists() ) { ibs::FilePath( "Test.HDF" ).remove(); }
   if ( ibs::FilePath( "MapsTest1.project3d"       ).exists() ) { ibs::FilePath( "MapsTest1.project3d" ).remove(); }
   if ( ibs::FilePath( "MapsTest2.project3d"       ).exists() ) { ibs::FilePath( "MapsTest2.project3d" ).remove(); }

   mbapi::MapsManager & mm = testModel.mapsManager();

   const std::vector<mbapi::MapsManager::MapID> & ids = mm.mapsIDs();
   ASSERT_EQ( ids.size(), 8U ); // number of maps in GridMapIoTbl

   mbapi::MapsManager::MapID id = mm.findID( mapName );
   ASSERT_EQ( id, 6U ); // given map is 7th in the list

   double minV, maxV;
   ASSERT_EQ( ErrorHandler::NoError, mm.mapValuesRange( id, minV, maxV ) );

   ASSERT_NEAR( minV, 2288.0, eps );
   ASSERT_NEAR( maxV, 6434.0, eps );

   mbapi::MapsManager::MapID nid = mm.copyMap( id, mapName+"_copy" );

   ASSERT_EQ( nid, 8U ); // the new map is at the end of the list
   ASSERT_EQ( mm.mapsIDs().size(), 9U ); // now maps in GridMapIoTbl are 9

   // min/max values in the copy are the same
   mm.mapValuesRange( nid, minV, maxV );

   ASSERT_NEAR( minV, 2288.0, eps );
   ASSERT_NEAR( maxV, 6434.0, eps );

   double coeff = 3000 / maxV;
   ASSERT_EQ( ErrorHandler::NoError, mm.scaleMap( nid, coeff ) );

   // min/max values in the copy are as given for rescale
   mm.mapValuesRange( nid, minV, maxV );

   ASSERT_NEAR( minV, 2288.0 * coeff, eps );
   ASSERT_NEAR( maxV, 3000.0, eps );


   // If a file already exist the maps will be appended to that file
   ASSERT_EQ( ErrorHandler::OutOfRangeValue, mm.saveMapToHDF( nid, "/tmp/Inputs.HDF", 0 ) ); // attempt to save in a place different from the project location should fail

   ASSERT_EQ( ErrorHandler::NoError, mm.saveMapToHDF( nid, "" , 0) ); // file name will be generated from map name
   ASSERT_EQ( true, ibs::FilePath( mapName + "_copy.HDF" ).exists() ); // file was written
   
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "MapsTest1.project3d" ) );
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile( "MapsTest1.project3d" );

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager().mapsIDs();
      ASSERT_EQ( tids.size(), 9U );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapName" ), std::string( "MAP-1076770443-4_copy" ) );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapFileName" ), std::string( "MAP-1076770443-4_copy.HDF" ) );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapType" ), std::string( "HDF5" ) );
   }
   ibs::FilePath( "MapsTest1.project3d" ).remove();

   ASSERT_EQ( ErrorHandler::NoError, mm.saveMapToHDF( nid, "Test.HDF", 0 ) ); // file name will be generated from map name
   ASSERT_EQ( true, ibs::FilePath( "Test.HDF" ).exists() ); // file was written
   
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "MapsTest2.project3d" ) );
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile( "MapsTest2.project3d" );

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager().mapsIDs();
      ASSERT_EQ( tids.size(), 9U );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapName" ), std::string( "MAP-1076770443-4_copy" ) );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapFileName" ), std::string( "Test.HDF" ) );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 8, "MapType" ), std::string( "HDF5" ) );
   }
   ibs::FilePath( "MapsTest2.project3d" ).remove();
}

TEST_F( mbapiModelTest, MapsManagerNNInterpolation )
{ 
   // set the folder where to save the files
   ibs::FilePath    masterResults( "." );
   std::string      casaResultsFile( "CasaModel_Results.HDF" );
   masterResults << casaResultsFile;

   // clean any previous runs result
   if ( masterResults.exists( ) ) { masterResults.remove( ); }
   if ( ibs::FilePath( "NNTesting2.project3d" ).exists( ) ) { ibs::FilePath( "NNTesting2.project3d" ).remove( ); }
   
   // load test project
   mbapi::Model testModel;
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_nnTestProject ) );

   // read lithofractions from file
   std::ifstream NNInputLithofractions( "NNInputLithofractions" );
   double xTemp;
   double yTemp;
   double lf1Temp;
   double lf2Temp;
   double lf3Temp;
   std::vector<double> xin;
   std::vector<double> yin;
   std::vector<double> lf1;
   std::vector<double> lf2;
   std::vector<double> lf3;

   while ( NNInputLithofractions.good( ) )
   {
      NNInputLithofractions >> xTemp;
      NNInputLithofractions >> yTemp;
      NNInputLithofractions >> lf1Temp;
      NNInputLithofractions >> lf2Temp;
      NNInputLithofractions >> lf3Temp;
      xin.push_back( xTemp );
      yin.push_back( yTemp );
      lf1.push_back( lf1Temp );
      lf2.push_back( lf2Temp );
      lf3.push_back( lf3Temp );
   }
   NNInputLithofractions.close( );

   std::vector<double> xout;
   std::vector<double> yout;
   std::vector<double> rpInt;
   std::vector<double> r13Int;

   testModel.interpolateLithoFractions( xin, yin, lf1, lf2, lf3, xout, yout, rpInt, r13Int );

   // check the number of points is correct
   ASSERT_EQ( xout.size(),   6527U );
   ASSERT_EQ( yout.size(),   6527U );
   ASSERT_EQ( rpInt.size(),  6527U );
   ASSERT_EQ( r13Int.size(), 6527U );

   // check the interpolated values are equal to those generated by the original prototype
   std::ifstream NNInt( "NNInt" );
   double rpTemp;
   double r13Temp;
   for ( size_t i = 0; i < xout.size(); ++i )
   {
      NNInt >> xTemp;
      NNInt >> yTemp;
      NNInt >> rpTemp;
      NNInt >> r13Temp;
      ASSERT_NEAR( xTemp, xout[i], eps );
      ASSERT_NEAR( yTemp, yout[i], eps );
      ASSERT_NEAR( rpTemp, rpInt[i], eps );
      ASSERT_NEAR( r13Temp, r13Int[i], eps );
   }
   NNInt.close( );

   // back transform the lithofractions
   std::vector<double> lf1CorrInt;
   std::vector<double> lf2CorrInt;
   std::vector<double> lf3CorrInt;
   testModel.backTransformLithoFractions( rpInt, r13Int, lf1CorrInt, lf2CorrInt, lf3CorrInt );

   // check the back transformation is correct, as in the original prototype
   std::ifstream NNbt( "NNbt" );
   double lf1CorrTemp;
   double lf2CorrTemp;
   for ( size_t i = 0; i < lf1CorrInt.size(); ++i )
   {
      NNbt >> lf1CorrTemp;
      NNbt >> lf2CorrTemp;
      ASSERT_NEAR( lf1CorrTemp, lf1CorrInt[i], 1e-4 );
      ASSERT_NEAR( lf2CorrTemp, lf2CorrInt[i], 1e-4 );
   }
   NNbt.close( );
   
   // save the backtransformed lithofractions in a temporary model
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "NNTesting2.project3d" ) );
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile( "NNTesting2.project3d" );

      // get the maps manager
      mbapi::MapsManager & mapsMgr = tmpModel.mapsManager( );

      // get the stratigraphy manager
      mbapi::StratigraphyManager & strMgr = tmpModel.stratigraphyManager( );

      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = strMgr.layerID( "Rotliegend" );

      // generate the maps
      std::string  correctFirstLithoFractionMap("5_percent_1");
      std::string  correctSecondLithoFractionMap("5_percent_2");
      size_t mapSeqNbr = Utilities::Numerical::NoDataIDValue;

      // first map, produce the map and update GridmapIoTbl
      mbapi::MapsManager::MapID id = mapsMgr.generateMap( "StratIoTbl", correctFirstLithoFractionMap, lf1CorrInt, mapSeqNbr, masterResults.path( ) );
      ASSERT_FALSE( IsValueUndefined( id ) );

      // second map, produce the map and update GridmapIoTbl
      id = mapsMgr.generateMap( "StratIoTbl", correctSecondLithoFractionMap, lf2CorrInt, mapSeqNbr, masterResults.path( ) );
      ASSERT_FALSE( IsValueUndefined( id ) );

      ASSERT_EQ( ErrorHandler::NoError, strMgr.setLayerLithologiesPercentageMaps( lid, correctFirstLithoFractionMap, correctSecondLithoFractionMap ) );

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager( ).mapsIDs( );
      ASSERT_EQ( tids.size( ), 28U );
      
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 26, "MapName" ), std::string( correctFirstLithoFractionMap ) );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 27, "MapName" ), std::string( correctSecondLithoFractionMap ) );

      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 26, "MapFileName" ), casaResultsFile );
      ASSERT_EQ( tmpModel.tableValueAsString( "GridMapIoTbl", 27, "MapFileName" ), casaResultsFile );

      ASSERT_EQ( tmpModel.tableValueAsString( "StratIoTbl", 5, "Percent1Grid" ), correctFirstLithoFractionMap  );
      ASSERT_EQ( tmpModel.tableValueAsString( "StratIoTbl", 5, "Percent2Grid" ), correctSecondLithoFractionMap );
   }
   
   ibs::FilePath( "NNTesting2.project3d" ).remove( );
   masterResults.remove( );
}

TEST_F(mbapiModelTest, FluidManager)
{
	mbapi::Model testModel;

	// load test project
	ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_fluidTestProject));

	mbapi::FluidManager & flMgr = testModel.fluidManager();

	//get fluid ids
	auto fluids = flMgr.getFluidsID();

	size_t actualTableSize = 5;
	//check whether all entries in FluidTypeIOTbl were read
	ASSERT_EQ(actualTableSize,fluids.size());

	std::vector<std::string> actualFluidNames = {"Std. Water","Std. Marine Water","Std. Hyper Saline Water","Std. Sea Water","NVG_Water"};

	std::string fluidName;

	//Check whether all names are being read correctlty
	for (auto flId : fluids)
	{
		flMgr.getFluidName(flId, fluidName);
		ASSERT_EQ(actualFluidNames[flId], fluidName);
	}

	//check whether density model and value can be read and modified
	{
		mbapi::FluidManager::FluidID id;
		mbapi::FluidManager::FluidDensityModel model1, model2;
		double density1, density2;

		id = 0;
		flMgr.densityModel(id, model1, density1);
		ASSERT_EQ(mbapi::FluidManager::FluidDensityModel::Calculated, model1);
		ASSERT_EQ(double(1000), density1);

		id = 4;
		flMgr.densityModel(id, model1, density1);
		ASSERT_EQ(mbapi::FluidManager::FluidDensityModel::Constant, model1);
		ASSERT_EQ(double(1000), density1);
		density1 = density1 * 10.0;
		flMgr.setDensityModel(id, mbapi::FluidManager::FluidDensityModel::Calculated, density1);

		flMgr.densityModel(id, model2, density2);
		ASSERT_EQ(mbapi::FluidManager::FluidDensityModel::Calculated, model2);
		ASSERT_EQ(density1, density2);

		density1 = density1 / 10.0;
		flMgr.setDensityModel(id, mbapi::FluidManager::FluidDensityModel::Constant, density1);
	}

	//check whether seismic velocity model and value can be read and modified
	{
		mbapi::FluidManager::FluidID id;
		mbapi::FluidManager::CalculationModel model1, model2;
		double seismicVelocity1, seismicVelocity2;

		id = 0;
		flMgr.seismicVelocityModel(id, model1, seismicVelocity1);
		ASSERT_EQ(mbapi::FluidManager::CalculationModel::CalculatedModel, model1);
		ASSERT_EQ(double(1500), seismicVelocity1);

		id = 4;
		flMgr.seismicVelocityModel(id, model1, seismicVelocity1);
		ASSERT_EQ(mbapi::FluidManager::CalculationModel::ConstantModel, model1);
		ASSERT_EQ(double(1500), seismicVelocity1);
		seismicVelocity1 = seismicVelocity1 * 10.0;
		flMgr.setSeismicVelocityModel(id, mbapi::FluidManager::CalculationModel::CalculatedModel, seismicVelocity1);

		flMgr.seismicVelocityModel(id, model2, seismicVelocity2);
		ASSERT_EQ(mbapi::FluidManager::CalculationModel::CalculatedModel, model2);
		ASSERT_EQ(seismicVelocity1, seismicVelocity2);

		seismicVelocity1 = seismicVelocity1 / 10.0;
		flMgr.setSeismicVelocityModel(id, mbapi::FluidManager::CalculationModel::ConstantModel, seismicVelocity1);
	}
}

TEST_F(mbapiModelTest, BiodegradeManager)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_fluidTestProject));

   mbapi::BiodegradeManager & bioDegMgr = testModel.biodegradeManager();

   std::map<std::string, double> mapBioDegConstants = {
      { "MaxBioTemp", 65.0 },
      { "TempConstant", 70.0 },
      { "TimeConstant", 0.2 },
      { "BioRate", 0.3 },
      { "C1_BioFactor", 0.0008 },
      { "C2_BioFactor", 0.0005 },
      { "C3_BioFactor", 0.0003 },
      { "C4_BioFactor", 0.0008 },
      { "C5_BioFactor", 0.001 },
      { "N2_BioFactor", 0.001 },
      { "COx_BioFactor", 0.0 },
      { "C6_14Aro_BioFactor", 0.0015 },
      { "C6_14Sat_BioFactor", 0.001 },
      { "C15Aro_BioFactor", 0.03 },
      { "C15Sat_BioFactor", 0.0009 },
      { "resins_BioFactor", 0.05 },
      { "asphaltenes_BioFactor", 0.1 }
   };

   for (auto& itr : mapBioDegConstants) {
      double valueFromP3dFile;

      // get biodegradation constant from BioDegradIoTbl in project3d file
      bioDegMgr.getBioConstant(itr.first, valueFromP3dFile);
      ASSERT_NEAR(valueFromP3dFile, itr.second, 1e-6);

      valueFromP3dFile = 0.5555;
      // set biodegradation constant 0.5555
      bioDegMgr.setBioConstant(itr.first, valueFromP3dFile);
      // get biodegradation constant 0.5555
      bioDegMgr.getBioConstant(itr.first, valueFromP3dFile);
      ASSERT_NEAR(valueFromP3dFile, 0.5555, 1e-6);
   }
}