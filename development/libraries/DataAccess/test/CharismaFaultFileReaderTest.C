// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.


#include "CharismaFaultFileReader.h"
#include "FaultFileReader.h"

#include "FilePath.h"

#include <memory>
#include <gtest/gtest.h>

using namespace DataAccess;
using namespace Interface;
using namespace std;

class PlanesExpected {

public :

   PlanesExpected ();

   void createFaultPlanesExpected (std::vector<FaultFileReader::FaultDataItem>& faultPlanesExpected) const;

};

TEST(CharismaFaultFileReaderTest, readFaultFile)
{
  DataAccess::Interface::CharismaFaultFileReader reader;

  const char* FaultFile = "FaultFileTest.charisma";

  bool fileIsOpen;

  ibs::FilePath fullFileName( "." );
  fullFileName << FaultFile;
  reader.open( fullFileName.path(), fileIsOpen );


  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  std::vector<FaultFileReader::FaultDataItem> faultPlanesExpected;

  PlanesExpected planes;
  planes.createFaultPlanesExpected( faultPlanesExpected );

  FaultFileReader::FaultDataSetIterator faultIter;

  int faultElement = 0;
  for ( faultIter = reader.begin(); faultIter != reader.end(); ++faultIter )
  {
     ASSERT_EQ( reader.faultName( faultIter ), faultPlanesExpected[faultElement].faultName );

     int pntSeqItem = 0;
     for (PointSequence pntSeq : reader.fault( ( faultIter ) ) )
     {
       int pntItem = 0;
       for (Point pnt : pntSeq)
       {
         ASSERT_EQ( pnt , faultPlanesExpected[faultElement].fault[pntSeqItem][pntItem] );
         pntItem++;
       }
       pntSeqItem++;
     }
     faultElement++;
  }

  reader.close();

}

TEST(CharismaFaultFileReaderTest, readEmptyFaultFile)
{
  DataAccess::Interface::CharismaFaultFileReader reader;

  bool fileIsOpen;

  reader.open( "FaultEmptyFileTest.charisma", fileIsOpen );

  EXPECT_TRUE(fileIsOpen);

  reader.preParseFaults();

  FaultFileReader::FaultDataSetIterator faultIter;

  int faultElement = 0;
  for ( faultIter = reader.begin(); faultIter != reader.end(); ++faultIter )
  {
     faultElement++;
  }

  reader.close();

  ASSERT_EQ( 0 , faultElement );
}

TEST(CharismaFaultFileReaderTest, readFileNotFound)
{
  DataAccess::Interface::CharismaFaultFileReader reader;

  bool fileIsOpen;

  reader.open( "ThisDoesNotExist.charisma", fileIsOpen );

  EXPECT_FALSE(fileIsOpen);
}

PlanesExpected::PlanesExpected () {
}

void PlanesExpected::createFaultPlanesExpected (std::vector<FaultFileReader::FaultDataItem>& faultPlanesExpected) const
{

  FaultFileReader::FaultDataItem faultPlane;
  std::vector<PointSequence> faultSticks;

  PointSequence faultStick;
  Point faultPoint;

  faultPoint (Interface::X_COORD) = 532819.00000;
  faultPoint (Interface::Y_COORD) = 54877.00000;
  faultPoint (Interface::Z_COORD) = 1148.43750;

  faultStick.push_back(faultPoint);

  faultPoint (Interface::X_COORD) = 532819.00000;
  faultPoint (Interface::Y_COORD) = 54402.00000;
  faultPoint (Interface::Z_COORD) = 1656.25000;

  faultStick.push_back(faultPoint);
  faultSticks.push_back(faultStick);
  faultPlane.faultName = "Fault3_0";
  faultPlane.fault = faultSticks;

  faultPlanesExpected.push_back(faultPlane);

  faultStick.clear();
  faultSticks.clear();

  faultPoint (Interface::X_COORD) = 531819.00000;
  faultPoint (Interface::Y_COORD) = 55402.00000;
  faultPoint (Interface::Z_COORD) = 31.25000;

  faultStick.push_back(faultPoint);
  faultPoint (Interface::X_COORD) = 531819.00000;
  faultPoint (Interface::Y_COORD) = 55052.00000;
  faultPoint (Interface::Z_COORD) = 1164.06250;

  faultStick.push_back(faultPoint);

  faultPoint (Interface::X_COORD) = 531819.00000;
  faultPoint (Interface::Y_COORD) = 54852.00000;
  faultPoint (Interface::Z_COORD) = 1632.81250;

  faultStick.push_back(faultPoint);
  faultSticks.push_back(faultStick);
  faultPlane.faultName = "Fault3_1";
  faultPlane.fault = faultSticks;

  faultPlanesExpected.push_back(faultPlane);

  faultStick.clear();
  faultSticks.clear();

  faultPoint (Interface::X_COORD) = 531319.00000;
  faultPoint (Interface::Y_COORD) = 55302.00000;
  faultPoint (Interface::Z_COORD) = 39.06250;

  faultStick.push_back(faultPoint);
  faultPoint (Interface::X_COORD) = 531319.00000;
  faultPoint (Interface::Y_COORD) = 55152.00000;
  faultPoint (Interface::Z_COORD) = 1054.68750;

  faultStick.push_back(faultPoint);

  faultPoint (Interface::X_COORD) = 531319.00000;
  faultPoint (Interface::Y_COORD) = 54877.00000;
  faultPoint (Interface::Z_COORD) = 1460.93750;

  faultStick.push_back(faultPoint);

  faultPoint (Interface::X_COORD) = 531319.00000;
  faultPoint (Interface::Y_COORD) = 54502.00000;
  faultPoint (Interface::Z_COORD) = 1937.50000;

  faultStick.push_back(faultPoint);
  faultSticks.push_back(faultStick);
  faultPlane.faultName = "Fault3_2";
  faultPlane.fault = faultSticks;

  faultPlanesExpected.push_back(faultPlane);
}
