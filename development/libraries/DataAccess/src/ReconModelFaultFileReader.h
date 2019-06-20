#ifndef _RECONMODEL_FAULT_FILE_READER_H
#define _RECONMODEL_FAULT_FILE_READER_H

#include "FaultFileReader.h"
#include "auxiliaryfaulttypes.h"

namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The identifier and object allocator for the fault file reader factory.
      ///
      const std::string ReconModelFaultFileReaderID = "RMFCUT";

      FaultFileReader* allocateReconModelFaultFileReader ();



      class ReconModelFaultFileReader : public ASCIIFaultFileReader {

	 public :

	    ReconModelFaultFileReader ();

	    void preParseFaults (); 

	 private :

      };
   }
}

#endif // _RECONMODEL_FAULT_FILE_READER_H
