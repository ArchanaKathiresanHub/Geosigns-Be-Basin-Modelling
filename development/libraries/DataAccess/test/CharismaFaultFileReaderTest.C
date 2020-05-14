// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#include "CharismaFaultFileReader.h"
#include "FaultCollection.h"
#include "FaultFileReader.h"
#include "Faulting.h"

#include "FilePath.h"

#include <memory>
#include <gtest/gtest.h>

using namespace DataAccess;
using namespace Interface;
using namespace std;

class FaultExpected {

public :

  FaultExpected ();

  Fault createFaultFaultExpected () const;

};

TEST(DataAccess, ReadCharismaFileTest)
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::ObjectFactory* factoryptr = &factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
  try
  {
    ph.reset( DataAccess::Interface::OpenCauldronProject("Project.project3d", factoryptr) );
  }
  catch (const std::exception& e)
  {
    std::cout << e.what();
  }

  if( nullptr == ph ) FAIL();

  DataAccess::Interface::CharismaFaultFileReader reader;

  const char* FaultFile = "FaultIntersect.charisma";

  bool fileIsOpen;

  ibs::FilePath fullFileName( "." );
  fullFileName << FaultFile;
  reader.open( fullFileName.path(), fileIsOpen );

  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  MutableFaultCollectionList faultCollectionList = reader.parseFaults(ph.get(), "FaultIntersect");

  EXPECT_FALSE(faultCollectionList.empty());

  FaultExpected fault;
  Fault faultExpected = fault.createFaultFaultExpected();

  MutableFaultCollectionList::const_iterator fcIter;

  for ( fcIter = faultCollectionList.begin(); fcIter != faultCollectionList.end(); ++fcIter )
  {
    FaultCollection * fc = *fcIter;

    FaultList::const_iterator faultIter;

    std::unique_ptr<FaultList> faults(fc->getFaults());
    for (faultIter = faults->begin (); faultIter != faults->end (); ++faultIter)
    {
      EXPECT_TRUE( faultExpected.getFaultLine() == (*faultIter)->getFaultLine());
      EXPECT_TRUE( faultExpected.getName() == (*faultIter)->getName());
    }
  }

  reader.close();
}

TEST(DataAccess, ReadCharismaNoIntersectionTest)
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::ObjectFactory* factoryptr = &factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
  try
  {
    ph.reset( DataAccess::Interface::OpenCauldronProject("Project.project3d", factoryptr) );
  }
  catch (const std::exception& e)
  {
    std::cout << e.what();
  }

  if( nullptr == ph ) FAIL();

  DataAccess::Interface::CharismaFaultFileReader reader;

  const char* FaultFile = "FaultNoIntersect.charisma";

  bool fileIsOpen;

  ibs::FilePath fullFileName( "." );
  fullFileName << FaultFile;
  reader.open( fullFileName.path(), fileIsOpen );

  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  MutableFaultCollectionList faultCollectionList = reader.parseFaults(ph.get(), "FaultIntersect");

  EXPECT_TRUE(faultCollectionList.empty());

  reader.close();
}

TEST(CharismaFaultFileReaderTest, readEmptyFaultFile)
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::ObjectFactory* factoryptr = &factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
  try
  {
    ph.reset( DataAccess::Interface::OpenCauldronProject("Project.project3d", factoryptr) );
  }
  catch (const std::exception& e)
  {
    std::cout << e.what();
  }

  if( nullptr == ph ) FAIL();

  DataAccess::Interface::CharismaFaultFileReader reader;

  const char* FaultFile = "FaultEmptyFileTest.charisma";

  bool fileIsOpen;

  ibs::FilePath fullFileName( "." );
  fullFileName << FaultFile;
  reader.open( fullFileName.path(), fileIsOpen );

  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  MutableFaultCollectionList faultCollectionList = reader.parseFaults(ph.get(), "FaultIntersect");

  EXPECT_TRUE(faultCollectionList.empty());

  reader.close();
}

TEST(CharismaFaultFileReaderTest, readFileNotFound)
{
  DataAccess::Interface::CharismaFaultFileReader reader;

  bool fileIsOpen;

  reader.open( "ThisDoesNotExist.charisma", fileIsOpen );

  EXPECT_FALSE(fileIsOpen);
}

FaultExpected::FaultExpected ()
{
}

Fault FaultExpected::createFaultFaultExpected () const
{
  PointSequence line;
  Point faultPoint;

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.277108434;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.594059406;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.721311475;

  line.push_back(faultPoint);

  return Fault ("faultPlane0", line);
}
