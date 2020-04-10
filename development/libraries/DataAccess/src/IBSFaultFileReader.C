//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <math.h>
#include "IBSFaultFileReader.h"
#include <sstream>
using std::stringstream;

using namespace DataAccess;
using namespace Interface;

//class FaultFileReader;
//------------------------------------------------------------//

const double IBSFaultFileReader::IBSFaultNullPoint = 999.999;


//------------------------------------------------------------//

IBSFaultFileReader::IBSFaultFileReader ()
{
   m_isOpen = false;
   faultCount = 0;
}

//------------------------------------------------------------//

void IBSFaultFileReader::open (const std::string & fileName, bool &fileIsOpen)
{
   ASCIIFaultFileReader::open (fileName, fileIsOpen);
   faultCount = 0;
}

//------------------------------------------------------------//

void IBSFaultFileReader::preParseFaults ()
{
   std::string newFaultName;
   PointSequence newFaultLine;
   bool done = false;

   while (true)
   {
      newFaultLine.clear ();
      readFault (newFaultName, newFaultLine, done);
      if (done) break;
      addFault (newFaultName, { newFaultLine });
   }
}

//------------------------------------------------------------//

void IBSFaultFileReader::readFault (std::string & newFaultName, PointSequence & newFaultLine, bool &done)
{
   if (!m_isOpen)
   {
      fprintf ( stderr,
                "****************    ERROR IBSFaultFileReader::readFault   fault file is not open   ****************\n");
      return;
   }

   const double epsilon = 0.00001;

   bool endOfFaultLine = false;
   char buffer[BufferSize];
   Point faultPoint;
   faultPoint (Interface::Z_COORD) = 0.0;

   faultCount = faultCount + 1;
   done = false;

   while (m_faultFile.good () && !endOfFaultLine)
   {
      stringstream pointBuffer;

      m_faultFile.getline (buffer, BufferSize);

      if (m_faultFile.good () && !isCommentLine (buffer))
      {
         pointBuffer << buffer;
         pointBuffer >> faultPoint (X_COORD);
         pointBuffer >> faultPoint (Y_COORD);

         ///
         /// Check to see of the point that was read is not the end-of-fault marker (a pair of 999.999's)
         /// or that there was no error in extracting the point values.
         ///
         endOfFaultLine = ((fabs (faultPoint (X_COORD) - IBSFaultNullPoint) < epsilon &&
                            fabs (faultPoint (Y_COORD) - IBSFaultNullPoint) < epsilon) || pointBuffer.fail ());

         if (!endOfFaultLine)
         {
            ///
            /// If the point that was read is not the end-of-fault marker and there was no error in
            /// reading the point, then it is safe to add the point to the fault line sequence.
            ///
            ///

            newFaultLine.push_back (faultPoint);
         }

      }

   }

   ///
   /// Make the fault name (faultcut+number of fault).
   ///
   stringstream buf;

   buf << "faultcut ";
   buf << faultCount;

   newFaultName = buf.str ();

   done = !m_faultFile.good ();
}

//------------------------------------------------------------//

bool IBSFaultFileReader::isCommentLine (const char *buffer) const
{
   int I;

   for (I = 0; I < BufferSize && buffer[I] != 0; I++)
   {

      if (buffer[I] == '!')
      {
         return true;
      }

   }

   return false;
}
