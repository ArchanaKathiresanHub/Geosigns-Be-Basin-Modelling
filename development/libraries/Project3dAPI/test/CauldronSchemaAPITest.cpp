#include "Project3dAPI.h"

#include <gtest/gtest.h>

TEST ( TableIoDataBaseTest, CauldronSchemaAPIRoundTripTest )
{
   project3d::ProjectIoAPI projFile( "Reference.project3d" );
   ASSERT_EQ( projFile.saveToProjectFile( "Reference_cmbapi.project3d" ), true );
} 


TEST ( TableIoDataBaseTest, CauldronSchemaAPIComparisonOperator )
{
   project3d::ProjectIoAPI proj1( "Reference.project3d" );
   project3d::ProjectIoAPI proj2( "Reference.project3d" );

   ASSERT_EQ( proj1, proj2 );
   proj1.m_CrustIoTbl->setAge( 0, 101 );
   ASSERT_EQ( proj1 == proj2, false );
} 

TEST ( TableIoDataBaseTest, CauldronSchemaAPICopyConstructor )
{
   project3d::ProjectIoAPI proj1( "Reference.project3d" );
   project3d::ProjectIoAPI proj2( proj1 );

   ASSERT_EQ( proj1, proj2 );
} 

TEST ( TableIoDataBaseTest, CauldronSchemaAPIAssignOperator )
{
   project3d::ProjectIoAPI proj1( "Reference.project3d" );
   project3d::ProjectIoAPI proj2;

   proj2 = proj1;

   ASSERT_EQ( proj1, proj2 );
} 



#include "CauldronSchemaAPITest.h"

TEST ( TableIoDataBaseTest, CauldronSchemaAPIFromScratchTest )
{
   project3d::ProjectIoAPI ph;
   
   FillAllTables( ph );

   ASSERT_EQ( ph.saveToProjectFile( "Reference_cmbapiFromScratch.project3d" ), true );
}

