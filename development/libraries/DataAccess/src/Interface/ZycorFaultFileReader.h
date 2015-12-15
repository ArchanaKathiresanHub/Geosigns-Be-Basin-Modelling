#ifndef _ZYCOR_FAULT_FILE_READER_H
#define _ZYCOR_FAULT_FILE_READER_H

#include "FaultFileReader.h"
#include "auxiliaryfaulttypes.h"

namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The identifier and object allocator for the fault file reader factory.
      ///
      const std::string ZyCorFaultFileReaderID = "ZYCFCUT";

      FaultFileReader* allocateZyCorFaultFileReader ();

      class ZyCorFaultFileReader : public ASCIIFaultFileReader
      {
         public :

            void preParseFaults ();

         private :

            void readLine ( double& xCoord,
                  double& yCoord,
                  int&    faultNumber );
      };
   }
}

#endif // _ZYCOR_FAULT_FILE_READER_H
