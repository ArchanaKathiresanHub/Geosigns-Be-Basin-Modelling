//                                                                      
// Copyright (C) 2018-2021 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include"gtest/gtest.h"
#include "../src/PersistentTrapFactory.h"
#include "../src/TrackProjectHandle.h"
#include "../../DataAccess/src/Formation.h"
#include "../src/TrackTrap.h"
using namespace PersistentTraps;
using namespace DataAccess::Interface;

const double relativeError = 1.0e-6;

class TrapperTest : public ::testing::Test {
public:
   void SetUp()
   {
      PersistentTrapFactory factory;
      projectHandle.reset(dynamic_cast<TrackProjectHandle*>(Interface::OpenCauldronProject(projectFileName, &factory)));

      ASSERT_NE(projectHandle, nullptr);
      
      auto snapshotList = projectHandle->getSnapshots();
      auto resFormation = projectHandle->findFormation("Top Chalk");
      
      auto resList = projectHandle->getReservoirs(resFormation);
      //get the trap @0Ma with trapId=1 for "Top Chalk" reservoir
      trapList = projectHandle->getTraps(resList->front(), snapshotList->front(), 1);


      projectHandle->extractRelevantTraps(snapshotList->front());
      projectHandle->computePersistentTraps(snapshotList->front(), 0);

      database::Table* trapperTable = projectHandle->getTable("TrapperIoTbl");

      size_t trapperTableSize = trapperTable->size();
      if (trapperTableSize != 0)
      {
         trapperTable->clear();
      }

      for (size_t index = 0; index < trapList->size(); ++index)
      {
         const Trap* trap = trapList->at(index);

         Trap* trap1 = const_cast<Trap*>(trapList->at(index));

         auto trackTrap = dynamic_cast<TrackTrap*>(trap1);
         trackTrap->save(trapperTable);
      }

   }
   void TearDown() {
      projectHandle.reset();
      delete trapList; 
   }

   const static std::string projectFileName;
   std::unique_ptr<TrackProjectHandle> projectHandle;
   Interface::TrapList* trapList;
};

const std::string TrapperTest::projectFileName = "Project.project3d";

TEST_F(TrapperTest, Buoyancy)
{
   database::Table* trapperTable = projectHandle->getTable("TrapperIoTbl");

   size_t trapperTableSize = trapperTable->size();
   EXPECT_EQ(trapperTableSize, 1);

   database::Record* record = trapperTable->getRecord(0);
   if (record)
   {
      double buoyancy = record->getValue<double>("Buoyancy");
      EXPECT_NEAR(0.278029091, buoyancy, relativeError);
   }
   
}

TEST_F(TrapperTest, trapCapacity)
{
   database::Table* trapperTable = projectHandle->getTable("TrapperIoTbl");
   size_t trapperTableSize = trapperTable->size();

   EXPECT_EQ(trapperTableSize, 1);

   database::Record* record = trapperTable->getRecord(0);
   if (record)
   {
      double trapCapacity = record->getValue<double>("TrapCapacity");
      EXPECT_NEAR(1597797.48864066, trapCapacity, relativeError);
   }

}


TEST_F(TrapperTest, freeGasProperties)
{
   database::Table* trapperTable = projectHandle->getTable("TrapperIoTbl");
   size_t trapperTableSize = trapperTable->size();

   EXPECT_EQ(trapperTableSize, 1);

   database::Record* record = trapperTable->getRecord(0);
   if (record)
   {
      double value = record->getValue<double>("FreeGasMass");
      EXPECT_NEAR(36066565.7543566, value, relativeError);

      value = record->getValue<double>("FreeGasVolume");
      EXPECT_NEAR(38071161.3339548, value, relativeError);

      value = record->getValue<double>("FreeGasDensity");
      EXPECT_NEAR(0.947346087974196, value, relativeError);

      value = record->getValue<double>("FreeGasViscosity");
      EXPECT_NEAR(1.00587543426243e-05, value, relativeError);

      value = record->getValue<double>("FreeGasMassC1");
      EXPECT_NEAR(18178887.6257595, value, relativeError);

      value = record->getValue<double>("FreeGasMassC2");
      EXPECT_NEAR(9149043.08830727, value, relativeError);

      value = record->getValue<double>("FreeGasMassC3");
      EXPECT_NEAR(5591613.21503083, value, relativeError);

      value = record->getValue<double>("FreeGasMassC4");
      EXPECT_NEAR(2556683.5498955, value, relativeError);

      value = record->getValue<double>("FreeGasMassC5");
      EXPECT_NEAR(530552.7092442, value, relativeError);

      value = record->getValue<double>("FreeGasMassN2");
      EXPECT_NEAR(59351.8180454352, value, relativeError);
   }

}
