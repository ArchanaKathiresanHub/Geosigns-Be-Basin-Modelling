//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _ZYCOR_FAULT_FILE_READER_H
#define _ZYCOR_FAULT_FILE_READER_H

#include "FaultFileReader2D.h"

namespace DataAccess
{
   namespace Interface
   {
      const std::string ZyCorFaultFileReaderID = "ZYCFCUT";

      class ZyCorFaultFileReader : public FaultFileReader2D
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
