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

TEST_F( mbapiModelTest, ModelLoadSaveProjectRoundTrip )
{
   std::auto_ptr<mbapi::Model> modelBase;
   modelBase.reset( new mbapi::Model() );

   ASSERT_EQ( ErrorHandler::NoError, modelBase->loadModelFromProjectFile( "Project.project3d" ) );

   // Create project file with current version of DataAccess library
   ASSERT_EQ( ErrorHandler::NoError, modelBase->saveModelToProjectFile( "Project_case1.project3d" ) );

   std::auto_ptr<mbapi::Model> modelCase2;
   modelCase2.reset( new mbapi::Model() );
   ASSERT_EQ( ErrorHandler::NoError, modelCase2->loadModelFromProjectFile( "Project_case1.project3d" ) );
   ASSERT_EQ( ErrorHandler::NoError, modelCase2->saveModelToProjectFile(   "Project_case2.project3d" ) );

   ASSERT_TRUE( compareFiles( "Project_case1.project3d", "Project_case2.project3d" ) );

   // clean files copy
   remove( "Project_case1.project3d" );
   remove( "Project_case2.project3d" );
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



