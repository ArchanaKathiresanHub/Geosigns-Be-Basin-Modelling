// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#pragma once

#include "FaultFileReader3D.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The identifier and object allocator for the fault file reader factory.
      ///
      const std::string CharismaFaultFileReaderID = "CHARISMA";

      class CharismaFaultFileReader : public FaultFileReader3D
      {
         public :

            void preParseFaults ();
      };
   }
}
