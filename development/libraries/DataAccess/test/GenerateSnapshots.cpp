//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "Snapshot.h"

#include <memory>
#include <gtest/gtest.h>

TEST(DataAccess, GenerateSnapshots)
{
   DataAccess::Interface::ObjectFactory factory;   
   std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
   try
   {
      ph.reset( DataAccess::Interface::OpenCauldronProject("GenerateSnapshotsTest.project3d", &factory) );
   }
   catch(...)
   {
      FAIL() << "Unexpected exception caught";
   }

   if( nullptr == ph ) FAIL(); 

   // Check snapshots
   std::unique_ptr<DataAccess::Interface::SnapshotList> majorSnap(ph->getSnapshots(DataAccess::Interface::MAJOR));

   std::vector<double> ages;
   for ( const DataAccess::Interface::Snapshot* snap : *majorSnap )
   {
     ages.push_back(snap->getTime());
   }
   const std::vector<double> expectedAges({0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 87.993625, 87.998425, 87.999625, 87.999975, 88.0, 90.0, 100.0});

   EXPECT_EQ(ages, expectedAges);
}
