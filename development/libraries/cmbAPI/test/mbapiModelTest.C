#include "../src/cmbAPI.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

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
};

const char * mbapiModelTest::m_sourceRockTestProject = "SourceRockTesting.project3d";
const char * mbapiModelTest::m_lithologyTestProject  = "LithologyTesting.project3d";
const char * mbapiModelTest::m_testProject           = "Project.project3d";

bool mbapiModelTest::compareFiles( const char * projFile1, const char * projFile2 )
{
   struct stat filestatus;
   stat( projFile1, &filestatus );
   size_t case1Size = filestatus.st_size;
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
   std::auto_ptr<mbapi::Model> modelBase;
   modelBase.reset( new mbapi::Model() );

   // load original project file
   ASSERT_EQ( ErrorHandler::NoError, modelBase->loadModelFromProjectFile( m_testProject ) );

   // Save a first copy 
   ASSERT_EQ( ErrorHandler::NoError, modelBase->saveModelToProjectFile( "Project_case1.project3d" ) );

   std::auto_ptr<mbapi::Model> modelCase2;
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
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2: 1) );

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
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2 : 1) );

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
               ASSERT_EQ( layerSourceRocks.size(), 2 );
               ASSERT_NEAR( stMgr.sourceRockMixHI( lid ), 700.0, eps );
   
               mbapi::SourceRockManager::SourceRockID sid2 = srMgr.findID( lNames[i], layerSourceRocks.back() );
               ASSERT_EQ( IsValueUndefined( sid2 ), false );
               
               double hiSR2 = srMgr.hiIni( sid2 );
               double hcSR2 = srMgr.hcIni( sid2 );
            
               ASSERT_NEAR( hiSR,   94.364368, eps );
               ASSERT_NEAR( hiSR2, 472.068687, eps );

               ASSERT_NEAR( hcSR,  0.801, eps );
               ASSERT_NEAR( hcSR2, 1.25, eps );

               srMgr.setHCIni( 1.0, sid2 );
               ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
               ASSERT_NEAR( srMgr.hiIni( sid2 ), 193.17523, eps );
            }
            break;

         case 1:  // Lower Jurassic
            ASSERT_EQ( stMgr.isSourceRockMixingEnabled( lid ), false );
            ASSERT_EQ( layerSourceRocks.size(), 1 );
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
   ASSERT_EQ( testModel.snapshotManager().snapshotsNumber(), 5 );
   
   // add snapshot
   ASSERT_EQ( ErrorHandler::NoError, testModel.snapshotManager().requestMajorSnapshot( 10.0 ) );
   ASSERT_EQ( testModel.snapshotManager().snapshotsNumber(), 6 );

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
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2 : 1) );

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
      ASSERT_EQ( layerSourceRocks.size(), (i == 0 ? 2 : 1) );

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
   ASSERT_NE( lid, UndefinedIDValue );

   // first try to copy to existent lithology
   ASSERT_EQ( UndefinedIDValue, lthMgr.copyLithology( lid, "Crust" ) );
   ASSERT_EQ( lthMgr.errorCode(), ErrorHandler::AlreadyDefined );
   lthMgr.resetError();

   size_t lithNum = lthMgr.lithologiesIDs().size();
   
   mbapi::LithologyManager::LithologyID newLID = lthMgr.copyLithology( lid, "Std. Sandstone COPY" );
   
   ASSERT_NE( newLID, UndefinedIDValue );
   const std::string & newName = lthMgr.lithologyName( newLID );
   ASSERT_TRUE( newName == "Std. Sandstone COPY" );
   ASSERT_EQ( lithNum+1, lthMgr.lithologiesIDs().size() );
}

TEST_F( mbapiModelTest, GetPermeabilityModelParametersTest )
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( m_lithologyTestProject ) );

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();
   
   // Check Sands permeability
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );

   mbapi::LithologyManager::PermeabilityModel permModel;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermSandstone );
   ASSERT_EQ( modelPrms.size(), 3 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 6000.0, eps );
   ASSERT_NEAR( modelPrms[2], 1.5, eps );

   // Check Shales permeability
   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermMudstone );
   ASSERT_EQ( modelPrms.size(), 4 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 0.01, eps );
   ASSERT_NEAR( modelPrms[2], 1.5, eps );
   ASSERT_NEAR( modelPrms[3], 0.01, eps );

   // Check Nones permeability
   lid = lthMgr.findID( "Crust" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermNone );
   ASSERT_EQ( modelPrms.size(), 0 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );

   // Check Impermeable permeability
   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_NE( lid, UndefinedIDValue );
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermImpermeable );
   ASSERT_EQ( modelPrms.size(), 0 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );

   // Check Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ(   permModel, mbapi::LithologyManager::PermMultipoint );
   ASSERT_EQ(   modelPrms.size(), 1 );
   ASSERT_NEAR( modelPrms[0],  1.0, eps );
   ASSERT_EQ(   mpPor.size(),  2 );
   ASSERT_EQ(   mpPerm.size(), 2 );
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
   ASSERT_NE( lid, UndefinedIDValue );

   // change model from None to Impermeable
   mbapi::LithologyManager::PermeabilityModel permModel = mbapi::LithologyManager::PermImpermeable;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change from Impermeable to None
   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_NE( lid, UndefinedIDValue );
 
   permModel = mbapi::LithologyManager::PermNone;
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change a bit coeff for Mudstone
   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   permModel = mbapi::LithologyManager::PermMudstone;
   modelPrms.push_back( 1.0 );  // anisothropic coeff
   modelPrms.push_back( 0.03 ); // depo por
   modelPrms.push_back( 1.55 ); // incr coeff
   modelPrms.push_back( 0.02 ); // decr coeff

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change Sandstone permeability
   lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );

   permModel = mbapi::LithologyManager::PermSandstone;
   modelPrms.resize( 1 );
   modelPrms.push_back( 7000.0 ); // depo por
   modelPrms.push_back( 1.45 ); // incr coeff

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.setPermeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );

   // change Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );
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
   ASSERT_NE( lid, UndefinedIDValue );

   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermImpermeable );
   ASSERT_EQ( modelPrms.size(), 0 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );

   lid = lthMgr.findID( "Standard Ice" );
   ASSERT_NE( lid, UndefinedIDValue );
   
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermNone );
   ASSERT_EQ( modelPrms.size(), 0 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );

   lid = lthMgr.findID( "Std. Shale" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermMudstone );
   ASSERT_EQ( modelPrms.size(), 4 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 0.03, eps );
   ASSERT_NEAR( modelPrms[2], 1.55, eps );
   ASSERT_NEAR( modelPrms[3], 0.02, eps );

   lid = lthMgr.findID( "Std. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ( permModel, mbapi::LithologyManager::PermSandstone );
   ASSERT_EQ( modelPrms.size(), 3 );
   ASSERT_EQ( mpPor.size(), 0 );
   ASSERT_EQ( mpPerm.size(), 0 );
   ASSERT_NEAR( modelPrms[0], 1.0, eps );
   ASSERT_NEAR( modelPrms[1], 7000.0, eps );
   ASSERT_NEAR( modelPrms[2], 1.45, eps );

   // Check Multipoint permeability
   lid = lthMgr.findID( "SM. Sandstone" );
   ASSERT_NE( lid, UndefinedIDValue );
  
   ASSERT_EQ( ErrorHandler::NoError, lthMgr.permeabilityModel( lid, permModel, modelPrms, mpPor, mpPerm ) );
   ASSERT_EQ(   permModel, mbapi::LithologyManager::PermMultipoint );
   ASSERT_EQ(   modelPrms.size(), 1 );
   ASSERT_NEAR( modelPrms[0],  1.0, eps );
   ASSERT_EQ(   mpPor.size(),  3 );
   ASSERT_EQ(   mpPerm.size(), 3 );
   ASSERT_NEAR( mpPor[0],  10.0, eps );
   ASSERT_NEAR( mpPor[1],  20.0, eps );
   ASSERT_NEAR( mpPor[2],  30.0, eps );
   ASSERT_NEAR( mpPerm[0], 2.0, eps );
   ASSERT_NEAR( mpPerm[1], 3.0, eps );
   ASSERT_NEAR( mpPerm[2], 4.0, eps );
}
