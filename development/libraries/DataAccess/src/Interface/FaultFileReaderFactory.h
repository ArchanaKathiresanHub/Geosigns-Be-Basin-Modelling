#ifndef _FAULT_FILE_READER_FACTORY_H
#define _FAULT_FILE_READER_FACTORY_H

#include <string>
#include <map>

#include "FaultFileReader.h"


namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The ID of the fault file reader, e.g. "FLT", "LANDMARKPOLYGON", ...
      ///
      typedef std::string FaultFileReaderID;

      ///
      /// A pointer to a function that allocates a concrete fault file reader object.
      ///
      typedef FaultFileReader* (*FaultFileReaderAllocator)();


      class FaultFileReaderFactory {

         typedef std::map<FaultFileReaderID, FaultFileReaderAllocator> ReaderAllocatorMap;

         public :

         ~FaultFileReaderFactory();

         static FaultFileReaderFactory& getInstance ();

         void registerReader ( const FaultFileReaderID&        ID,
               const FaultFileReaderAllocator& newAllocator );

         FaultFileReader* createReader ( const FaultFileReaderID& ID ) const;

         bool readerTypeIsRegistered ( const FaultFileReaderID& ID ) const;

         private :

         FaultFileReaderFactory ();

         static FaultFileReaderFactory* s_FaultFileReaderFactoryInstance;

         ReaderAllocatorMap readerAllocator;

      }; 
   }
}



#endif // _FAULT_FILE_READER_FACTORY_H
