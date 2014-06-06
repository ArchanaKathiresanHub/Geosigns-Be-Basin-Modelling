#include "../src/cmbAPI.h"
#include "../src/PrmTopCrustHeatProduction.h"

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
};

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
   ASSERT_EQ( ErrorHandler::NoError, modelBase->loadModelFromProjectFile( "Project.project3d" ) );

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
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( "Project.project3d" ) );

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
   ASSERT_EQ( ErrorHandler::NoError, testModel.loadModelFromProjectFile( "Ottoland.project3d" ) );

   mbapi::SourceRockManager & srMgr = testModel.sourceRockManager();

   // get current values for TOC. Test project has 2 source rock lithologies with TOC 70.2 & 10.0
   double toc1st = srMgr.tocIni( 0 );
   ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
   ASSERT_NEAR( toc1st, 70.2, eps );
   toc1st = srMgr.tocIni( 1 );
   ASSERT_EQ( ErrorHandler::NoError, srMgr.errorCode() );
   ASSERT_NEAR( toc1st, 10.0, eps );

   // change TOC values to some others - 19 & 70
   ASSERT_EQ( ErrorHandler::NoError, srMgr.setTOCIni( "Lower Jurassic", 19.0 ) );
   ASSERT_EQ( ErrorHandler::NoError, srMgr.setTOCIni( "Westphalian", 70.0 ) );
   // save as a new temporary project file
   ASSERT_EQ( ErrorHandler::NoError, testModel.saveModelToProjectFile( "Ottoland_changedTOC.project3d" ) );

   // reload this new project file into other model
   mbapi::Model modifModel;
   ASSERT_EQ( ErrorHandler::NoError, modifModel.loadModelFromProjectFile( "Ottoland_changedTOC.project3d" ) );
   
   mbapi::SourceRockManager & srModMgr = modifModel.sourceRockManager();

   // check values for the TOC
   toc1st = srModMgr.tocIni( 0 );
   ASSERT_EQ( ErrorHandler::NoError, srModMgr.errorCode() );
   ASSERT_NEAR( toc1st, 70.0, eps );
   toc1st = srModMgr.tocIni( 1 );
   ASSERT_EQ( ErrorHandler::NoError, srModMgr.errorCode() );
   ASSERT_NEAR( toc1st, 19.0, eps );

   // delete temporary project file
   remove( "Ottoland_changedTOC.project3d" );
}

TEST_F( mbapiModelTest, ModelCopyOperator )
{
/*   std::auto_ptr<mbapi::Model> modelBase;
   modelBase.reset( new mbapi::Model() );

   ASSERT_EQ( ErrorHandler::NoError, modelBase->loadModelFromProjectFile( "Project.project3d" ) );

   mbapi::Model copyModel;
   copyModel = *( modelBase.get() );
   
   ASSERT_EQ( ErrorHandler::NoError, modelBase->saveModelToProjectFile( "Project_original.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, copyModel.saveModelToProjectFile( "Project_copied.project3d" ) );

   compareFiles( "Project_original.project3d", "Project_copied.project3d" );
   // clean files copy
   remove( "Project_case1.project3d" );
   remove( "Project_case2.project3d" );
  */ 
 }



