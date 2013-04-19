#include <math.h>
#include "Interface/IBSFaultFileReader.h"

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include<sstream>
using std::stringstream;
#else                           // !_STANDARD_C_PLUS_PLUS
#include<strstream.h>
typedef strstream stringstream;
#endif                          // _STANDARD_C_PLUS_PLUS
#else                           // !sgi
#include <sstream>
using std::stringstream;
#endif                          // sgi

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

const double IBSFaultFileReader::IBSFaultNullPoint = 999.999;

//------------------------------------------------------------//

FaultFileReader *DataAccess::Interface::allocateIBSFaultFileReader ()
{
   return new IBSFaultFileReader;
}

//------------------------------------------------------------//

IBSFaultFileReader::IBSFaultFileReader ()
{
   isOpen = false;
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
      addFault (newFaultName, newFaultLine);
   }
}

//------------------------------------------------------------//

void IBSFaultFileReader::readFault (std::string & newFaultName, PointSequence & newFaultLine, bool &done)
{
   if (!isOpen)
   {
#ifndef _FAULTUNITTEST_
      fprintf (stderr,
                   "****************    ERROR IBSFaultFileReader::readFault   fault file is not open   ****************\n");
      //PetscPrintf (PETSC_COMM_WORLD,
      //             "****************    ERROR IBSFaultFileReader::readFault   fault file is not open   ****************\n");
#endif
      return;
   }

   const double epsilon = 0.00001;

   bool endOfFaultLine = false;
   char buffer[BufferSize];
   Point faultPoint;

   faultCount = faultCount + 1;
   done = false;

   while (faultFile.good () && !endOfFaultLine)
   {
      stringstream pointBuffer;

      faultFile.getline (buffer, BufferSize);

      if (faultFile.good () && !isCommentLine (buffer))
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

   done = !faultFile.good ();
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
