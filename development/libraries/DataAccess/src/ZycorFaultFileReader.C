//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ZycorFaultFileReader.h"
#include <sstream>
using std::ostringstream;

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

void ZyCorFaultFileReader::preParseFaults ()
{
   if (!m_isOpen)
   {
      fprintf ( stderr,
                "****************    ERROR ZyCorFaultFileReader::readFault   fault file is not open   ****************\n");
      return;
   }

   int faultNumber;
   int currentFaultNumber;
   bool newFault;

   double xCoord;
   double yCoord;

   Point faultPoint;
   PointSequence newFaultLine;

   readLine (xCoord, yCoord, faultNumber);
   currentFaultNumber = faultNumber;

   while (m_faultFile.good ())
   {
      ostringstream newFaultName;

      newFaultLine.clear ();

      newFaultName << "faultcut " << currentFaultNumber;

      newFault = false;

      while (m_faultFile.good () && !newFault)
      {
         faultPoint (Interface::X_COORD) = xCoord;
         faultPoint (Interface::Y_COORD) = yCoord;
         faultPoint (Interface::Z_COORD) = 0.0;

         newFaultLine.push_back (faultPoint);

         readLine (xCoord, yCoord, faultNumber);

         if (faultNumber != currentFaultNumber)
         {
            currentFaultNumber = faultNumber;
            newFault = true;
         }
      }

      addFault (newFaultName.str (), { newFaultLine } );
   }
}

//------------------------------------------------------------//

void ZyCorFaultFileReader::readLine (double &xCoord, double &yCoord, int &faultNumber)
{
   m_faultFile >> xCoord;
   m_faultFile >> yCoord;
   m_faultFile >> faultNumber;
}

//------------------------------------------------------------//
