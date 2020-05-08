//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FAULT_FILE_READER_H
#define _FAULT_FILE_READER_H

#include <fstream>
#include <string>
#include <vector>

#include "auxiliaryfaulttypes.h"


namespace DataAccess
{
   namespace Interface
   {
      class FaultCollection;
      class ProjectHandle;

      class FaultFileReader {
         public :
            struct FaultDataItem {
               std::string faultName;
               PointSequence fault;
            };

            typedef std::vector <FaultDataItem> FaultDataSet;
            typedef FaultDataSet::const_iterator FaultDataSetIterator;

            FaultFileReader ();

            virtual ~FaultFileReader ();

            virtual void open ( const std::string& fileName, bool& fileIsOpen ) = 0;

            virtual void close () = 0;

            ///
            /// Load fault from the fault file into the FaultDataSet
            ///
            virtual void preParseFaults () = 0;

            virtual MutableFaultCollectionList parseFaults ( ProjectHandle* projectHandle, const std::string& mapName ) const = 0;

      };


      class ASCIIFaultFileReader : public FaultFileReader {

         public :

            ASCIIFaultFileReader ();

            virtual ~ASCIIFaultFileReader ();

            void open ( const std::string& fileName,
                  bool&        fileIsOpen ) override;

            void close () final;

            bool isEmpty();

         protected :

            ifstream m_faultFile;

            ///
            /// Only reason for this is because, when using the IRIX compiler it doesn't seem to
            /// have the member function "is_open".
            ///
            bool     m_isOpen;

      };
   }
}

#endif // _FAULT_FILE_READER_H
