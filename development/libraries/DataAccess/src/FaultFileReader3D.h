//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <string>
#include <vector>

#include "FaultFileReader.h"

namespace DataAccess
{
   namespace Interface
   {
      class FaultFileReader3D : public ASCIIFaultFileReader
      {
         public :

            ///
            /// Return an iterator pointing to the start of the fault sequence
            ///
            FaultDataSetIterator begin () const final;

            ///
            /// Return an iterator pointing to the end of the fault sequence
            ///
            FaultDataSetIterator end () const final;

      protected:
         void addFault ( const std::string& newFaultName, const std::vector<PointSequence>& newFaultLine ) final;

      private:
         FaultDataSet m_faultData;

      };
   }
}
