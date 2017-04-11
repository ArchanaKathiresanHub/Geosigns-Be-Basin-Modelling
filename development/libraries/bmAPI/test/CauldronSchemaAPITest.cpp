#include "bmAPI.h"

#include <gtest/gtest.h>

TEST ( TableIoDataBaseTest, CauldronSchemaAPIRoundTripTest )
{
   bmapi::ProjectIoAPI projFile( "Reference.project3d" );
   ASSERT_EQ( projFile.saveToProjectFile( "Reference_cmbapi.project3d" ), true );
} 


TEST ( TableIoDataBaseTest, CauldronSchemaAPIComparisonOperator )
{
   bmapi::ProjectIoAPI proj1( "Reference.project3d" );
   bmapi::ProjectIoAPI proj2( "Reference.project3d" );

   ASSERT_EQ( proj1, proj2 );
   proj1.m_CrustIoTbl->setAge( 0, 101 );
   ASSERT_EQ( proj1 == proj2, false );
} 

TEST ( TableIoDataBaseTest, CauldronSchemaAPICopyConstructor )
{
   bmapi::ProjectIoAPI proj1( "Reference.project3d" );
   bmapi::ProjectIoAPI proj2( proj1 );

   ASSERT_EQ( proj1, proj2 );
} 

TEST ( TableIoDataBaseTest, CauldronSchemaAPIAssignOperator )
{
   bmapi::ProjectIoAPI proj1( "Reference.project3d" );
   bmapi::ProjectIoAPI proj2;

   proj2 = proj1;

   ASSERT_EQ( proj1, proj2 );
} 



#include "CauldronSchemaAPITest.h"

TEST ( TableIoDataBaseTest, CauldronSchemaAPIFromScratchTest )
{
   bmapi::ProjectIoAPI ph;
   size_t id;
   
   FillAllTables( ph );

   ASSERT_EQ( ph.saveToProjectFile( "Reference_cmbapiFromScratch.project3d" ), true );
}

