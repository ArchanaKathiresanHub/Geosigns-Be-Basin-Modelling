#include "cauldronschemaAPI.h"

#include <gtest/gtest.h>

TEST ( TableIoDataBaseTest, CauldronSchemaAPIRoundTripTest )
{
   bmapi::ProjectIoAPI projFile( "Reference.project3d" );
   ASSERT_EQ( projFile.saveToProjectFile( "Reference_cmbapi.project3d" ), true );
} 

TEST ( TableIoDataBaseTest, CauldronSchemaAPIFromScratchTest )
{
   bmapi::ProjectIoAPI ph;
   bmapi::RecNumber id;

#include "CauldronSchemaAPITest.h"

   ASSERT_EQ( ph.saveToProjectFile( "Reference_cmbapiFromScratch.project3d" ), true );
}

