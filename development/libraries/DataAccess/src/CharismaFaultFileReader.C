// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.


#include "CharismaFaultFileReader.h"

#include <sstream>
#include <string>

using namespace DataAccess;
using namespace Interface;


void CharismaFaultFileReader::preParseFaults ()
{
  if (!m_isOpen)
  {
    fprintf (stderr,
             "****************    ERROR CharismaFaultFileReader::readFault   fault file is not open   ****************\n");
    return;
  }

  if (isEmpty())
  {
    fprintf (stderr,
             "****************    ERROR CharismaFaultFileReader::readFault   fault file is empty   ****************\n");
    return;
  }

  int currentFaultNumber = -1;

  PointSequence faultStick;
  Point faultPoint;
  std::vector<PointSequence> faultSticks;
  std::string fName;

  std::string line;

  while ( getline( m_faultFile, line) )
  {
    std::stringstream ss (line);

    int faultNumber;
    double xCoord;
    double yCoord;
    double zCoord;

    std::string dummy;

    ss >> dummy;
    ss >> dummy;
    ss >> dummy;

    ss >> xCoord;
    ss >> yCoord;
    ss >> zCoord;
    ss >> fName;
    ss >> faultNumber;

    faultPoint (Interface::X_COORD) = xCoord;
    faultPoint (Interface::Y_COORD) = yCoord;
    faultPoint (Interface::Z_COORD) = zCoord;

    if ( currentFaultNumber == -1 )
    {
      currentFaultNumber = faultNumber;
    }
    else if ( faultNumber != currentFaultNumber )
    {
      currentFaultNumber = faultNumber;
      faultSticks.push_back( faultStick );
      faultStick.clear();
    }

    faultStick.push_back( faultPoint );
  }

  faultSticks.push_back( faultStick );

  addFault( fName, faultSticks );

}
