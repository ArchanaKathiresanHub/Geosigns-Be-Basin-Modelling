// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.


#include "FaultFileReader.h"
#include "FaultFileReaderFactory.h"

#include "CharismaFaultFileReader.h"
#include "IBSFaultFileReader.h"
#include "LandmarkFaultFileReader.h"
#include "ZycorFaultFileReader.h"

#include <gtest/gtest.h>

TEST(FaultFileReaderFactoryTest, createCharismaFaultFileReader)
{
  DataAccess::Interface::FaultFileReader *reader = DataAccess::Interface::FaultFileReaderFactory::getInstance().createReader( DataAccess::Interface::CharismaFaultFileReaderID );

  EXPECT_NE(nullptr, dynamic_cast<DataAccess::Interface::CharismaFaultFileReader*>(reader));
}

TEST(FaultFileReaderFactoryTest, createIBSFaultFileReader)
{
  DataAccess::Interface::FaultFileReader *reader = DataAccess::Interface::FaultFileReaderFactory::getInstance().createReader( DataAccess::Interface::IBSFaultFileReaderID );

  EXPECT_NE(nullptr, dynamic_cast<DataAccess::Interface::IBSFaultFileReader*>(reader));
}

TEST(FaultFileReaderFactoryTest, createLandmarkFaultFileReader)
{
  DataAccess::Interface::FaultFileReader *reader = DataAccess::Interface::FaultFileReaderFactory::getInstance().createReader( DataAccess::Interface::LandmarkFaultFileReaderID );

  EXPECT_NE(nullptr, dynamic_cast<DataAccess::Interface::LandmarkFaultFileReader*>(reader));
}

TEST(FaultFileReaderFactoryTest, createZycorFaultFileReader)
{
  DataAccess::Interface::FaultFileReader *reader = DataAccess::Interface::FaultFileReaderFactory::getInstance().createReader( DataAccess::Interface::ZyCorFaultFileReaderID );

  EXPECT_NE(nullptr, dynamic_cast<DataAccess::Interface::ZyCorFaultFileReader*>(reader));
}
