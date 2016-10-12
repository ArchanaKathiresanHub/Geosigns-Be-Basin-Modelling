#include "Interface/ZycorFaultFileReader.h"
#include <sstream>
using std::ostringstream;

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

FaultFileReader *DataAccess::Interface::allocateZyCorFaultFileReader ()
{
   return new ZyCorFaultFileReader;
}

//------------------------------------------------------------//

void ZyCorFaultFileReader::preParseFaults ()
{
   if (!isOpen)
   {
#ifndef _FAULTUNITTEST_
      fprintf (stderr,
                   "****************    ERROR ZyCorFaultFileReader::readFault   fault file is not open   ****************\n");
      //PetscPrintf (PETSC_COMM_WORLD,
      //             "****************    ERROR ZyCorFaultFileReader::readFault   fault file is not open   ****************\n");
#endif
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

   while (faultFile.good ())
   {
      ostringstream newFaultName;

      newFaultLine.clear ();

      newFaultName << "faultcut " << currentFaultNumber;

      newFault = false;

      while (faultFile.good () && !newFault)
      {
         faultPoint (Interface::X_COORD) = xCoord;
         faultPoint (Interface::Y_COORD) = yCoord;

         newFaultLine.push_back (faultPoint);

         readLine (xCoord, yCoord, faultNumber);

         if (faultNumber != currentFaultNumber)
         {
            currentFaultNumber = faultNumber;
            newFault = true;
         }
      }

      addFault (newFaultName.str (), newFaultLine);
   }
}

//------------------------------------------------------------//

void ZyCorFaultFileReader::readLine (double &xCoord, double &yCoord, int &faultNumber)
{
   faultFile >> xCoord;
   faultFile >> yCoord;
   faultFile >> faultNumber;
}

//------------------------------------------------------------//
