// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.


#include "CharismaFaultFileReader.h"

#include "FaultCollection.h"
#include "Formation.h"
#include "ProjectHandle.h"
#include "Surface.h"
#include "FaultPlane.h"

#include <memory>
#include <sstream>
#include <string>
#include <algorithm>

using namespace DataAccess;
using namespace Interface;


CharismaFaultFileReader::CharismaFaultFileReader() :
  FaultFileReader3D(),
  m_splitDistance( 5000 ),
  m_faultSticks()
{
}

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
    ss >> dummy;
    ss >> faultNumber;

    faultPoint [Interface::X_COORD] = xCoord;
    faultPoint [Interface::Y_COORD] = yCoord;
    faultPoint [Interface::Z_COORD] = zCoord;

    if ( currentFaultNumber == -1 )
    {
      currentFaultNumber = faultNumber;
    }
    else if ( faultNumber != currentFaultNumber )
    {
      currentFaultNumber = faultNumber;
      m_faultSticks.push_back( faultStick );
      faultStick.clear();
    }

    faultStick.push_back( faultPoint );
  }

  m_faultSticks.push_back( faultStick );
}

MutableFaultCollectionList CharismaFaultFileReader::parseFaults(ProjectHandle* projectHandle, const string& mapName) const
{
  // Do conversion from m_faultSticks to a FaultCollectionList with correct Formations and names
  // All individual Faults have to be called "faultPlane" so the events are copied to all faults in a fault plane
  //"faultPlane"

  MutableFaultCollectionList faultCollectionList;

  FaultPlane plane ( m_faultSticks );

  std::unique_ptr<SurfaceList> surfaceList ( projectHandle->getSurfaces() );
  for ( const Surface* surface : *surfaceList )
  {
    std::cout << "Surface " << surface->getName() << std::endl;
    std::vector<PointSequence> faultCuts;
    if (plane.intersect(surface->getInputDepthMap(), m_splitDistance, faultCuts))
    {
      const Formation* formation = surface->getBottomFormation();
      const std::string faultCollectionName = mapName + "_" + formation->getMangledName();
      FaultCollection* faultCollection = projectHandle->getFactory()->produceFaultCollection( *projectHandle, faultCollectionName );
      int counter = 0;
      for (PointSequence faultCut : faultCuts)
      {
        faultCollection->addFault( "faultPlane" + std::to_string(counter), faultCut );
        counter++;
      }

      faultCollection->addFormation( formation );
      faultCollectionList.push_back(faultCollection);
    }
  }


  return faultCollectionList;
}
