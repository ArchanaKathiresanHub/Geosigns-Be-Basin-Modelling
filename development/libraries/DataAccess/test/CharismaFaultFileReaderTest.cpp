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

void readCharismaFile(const std::string& fileName, MutableFaultCollectionList& faultCollectionList)
{
  DataAccess::Interface::ObjectFactory factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> projectHandle;
  try
  {
    projectHandle.reset( DataAccess::Interface::OpenCauldronProject("Project.project3d", &factory) );
  }
  catch (const std::exception& e)
  {
    std::cout << e.what();
  }

  if( nullptr == projectHandle ) FAIL();

  DataAccess::Interface::CharismaFaultFileReader reader;

  bool fileIsOpen;

  ibs::FilePath fullFileName( "." );
  fullFileName << fileName;
  reader.open( fullFileName.path(), fileIsOpen );

  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  faultCollectionList = reader.parseFaults(projectHandle.get(), "TestFault");
  reader.close();
}

TEST(DataAccess, ReadCharismaFileTest)
{
  MutableFaultCollectionList faultCollectionList;
  readCharismaFile("FaultIntersect.charisma", faultCollectionList);

  EXPECT_FALSE(faultCollectionList.empty());

  FaultExpected fault;
  Fault faultExpected = fault.createFaultFaultExpected();

  for (FaultCollection* fc : faultCollectionList)
  {
    std::unique_ptr<FaultList> faults(fc->getFaults());
    for (const Fault* fault : *faults)
    {
      std::cout << std::setprecision(17);
      std::cout <<fault->getFaultLine() << std::endl;

      EXPECT_TRUE( faultExpected.getFaultLine() == fault->getFaultLine());
      EXPECT_TRUE( faultExpected.getName() == fault->getName());
    }
  }
}

TEST(DataAccess, ReadCharismaNoIntersectionTest)
{
  MutableFaultCollectionList faultCollectionList;
  readCharismaFile("FaultNoIntersect.charisma", faultCollectionList);

  EXPECT_TRUE(faultCollectionList.empty());
}

TEST(CharismaFaultFileReaderTest, readEmptyFaultFile)
{
  MutableFaultCollectionList faultCollectionList;
  readCharismaFile("FaultEmptyFileTest.charisma", faultCollectionList);

  EXPECT_TRUE(faultCollectionList.empty());
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
  faultPoint [Interface::Y_COORD] = 6750002.7792207794;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7792207794;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7696476961;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7536231885;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7352024922;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7213114752;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.662251655;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.5940594058;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.2816901412;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.7752808984;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.4082840234;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.3432835825;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.2771084337;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001.1895604394;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750001;

  line.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 460002;
  faultPoint [Interface::Y_COORD] = 6750002.7792207794;

  line.push_back(faultPoint);

  return Fault ("faultPlane0", line);
}
