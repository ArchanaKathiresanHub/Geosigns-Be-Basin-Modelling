//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DataMiningProjectHandle.h"
#include "DomainPropertyFactory.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

#include <memory>
#include <gtest/gtest.h>

TEST(DataMiningProjectHandle, OpenCauldronProject)
{
   std::unique_ptr<DataAccess::Mining::DomainPropertyFactory> factory( new DataAccess::Mining::DomainPropertyFactory );
   std::unique_ptr<DataAccess::Mining::ProjectHandle> ph(nullptr);
   try
   {
      ph.reset( (DataAccess::Mining::ProjectHandle*)DataAccess::Interface::OpenCauldronProject("DataMiningProjectHandleTest.project3d", "r", factory.get()) );
   }
   catch(...)
   {
      FAIL() << "Unexpected exception caught";
   }

   if( nullptr == ph ) FAIL();

   EXPECT_TRUE( nullptr != ph->getDomainPropertyCollection() );

   EXPECT_EQ( ph->getPropertyNameFromAlternative("Depth"), "Depth");
   ph->addAlternativeName( "Depth", "DepthAlternative" );
   EXPECT_EQ( ph->getPropertyNameFromAlternative("DepthAlternative"), "Depth");
}
