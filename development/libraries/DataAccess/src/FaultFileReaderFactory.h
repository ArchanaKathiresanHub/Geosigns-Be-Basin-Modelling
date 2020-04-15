//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FAULT_FILE_READER_FACTORY_H
#define _FAULT_FILE_READER_FACTORY_H

#include <string>

#include "CharismaFaultFileReader.h"
#include "IBSFaultFileReader.h"
#include "LandmarkFaultFileReader.h"
#include "ZycorFaultFileReader.h"

namespace DataAccess
{
   namespace Interface
   {
      class FaultFileReaderFactory
      {
         public :

           ~FaultFileReaderFactory();

           static FaultFileReaderFactory& getInstance ();

           FaultFileReader* createReader ( const std::string& ID ) const;

      private :

           FaultFileReaderFactory ();

           static FaultFileReaderFactory* s_FaultFileReaderFactoryInstance;

      };
   }
}



#endif // _FAULT_FILE_READER_FACTORY_H
