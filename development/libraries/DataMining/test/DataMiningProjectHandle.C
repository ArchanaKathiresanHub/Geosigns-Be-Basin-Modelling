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
#include "DataMiningObjectFactory.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

#include <memory>
#include <gtest/gtest.h>

TEST(DataMiningProjectHandle, OpenCauldronProject)
{
   std::unique_ptr<DataAccess::Mining::ObjectFactory> factory( new DataAccess::Mining::ObjectFactory);
   std::unique_ptr<DataAccess::Mining::ProjectHandle> ph(nullptr);
   try
   {
      ph.reset(dynamic_cast<DataAccess::Mining::ProjectHandle*>(DataAccess::Interface::OpenCauldronProject("DataMiningProjectHandleTest.project3d", "r", factory.get())));
      factory->initialiseObjectFactory(ph.get());
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
