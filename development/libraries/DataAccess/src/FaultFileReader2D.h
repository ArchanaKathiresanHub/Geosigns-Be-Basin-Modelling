//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FAULT_FILE_READER_2D_H
#define _FAULT_FILE_READER_2D_H

#include <string>
#include <vector>

#include "FaultFileReader.h"

namespace DataAccess
{
   namespace Interface
   {
      class FaultFileReader2D : public ASCIIFaultFileReader
      {
         public:
            FaultFileReader2D ();

            virtual ~FaultFileReader2D ();

            MutableFaultCollectionList parseFaults ( ProjectHandle* projectHandle, const std::string& mapName ) const final;

        protected:
            void addFault ( const std::string& newFaultName, const PointSequence& newFault );

        private:
           FaultDataSet m_faultData;

      };
   }
}

#endif // _FAULT_FILE_READER_2D_H
