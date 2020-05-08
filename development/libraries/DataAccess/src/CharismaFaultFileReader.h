// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#pragma once

#include "FaultFileReader3D.h"

#include "auxiliaryfaulttypes.h"

#include <string>
#include <vector>

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

            CharismaFaultFileReader();

            void preParseFaults ();

            MutableFaultCollectionList parseFaults ( ProjectHandle* projectHandle, const std::string& mapName ) const final;

         private:
            double m_splitDistance;
            std::vector<PointSequence> m_faultSticks;
      };
   }
}
