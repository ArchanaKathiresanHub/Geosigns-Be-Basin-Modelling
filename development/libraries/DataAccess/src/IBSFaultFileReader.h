//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _IBS_FAULT_FILE_READER_H
#define _IBS_FAULT_FILE_READER_H

#include "FaultFileReader2D.h"

namespace DataAccess
{
   namespace Interface
   {
      ///
      /// The identifier and object allocator for the fault file reader factory.
      ///
      const std::string IBSFaultFileReaderID = "FLT";

      ///
      /// Reads faults from the IBS fault file format.
      /// This format is basically a sequence of points in 2D space
      /// ended by a special marker point (999.999 999.999), unless
      /// the point is the last point in the file when it may be omitted.
      /// Must each point be on its own line?
      /// Comments begin with an '!'.
      ///
      class IBSFaultFileReader : public FaultFileReader2D {


	 public :

			IBSFaultFileReader ();

			void open ( const std::string& fileName, bool& fileIsOpen ) final;

			void preParseFaults ();


	 private :

			static const double IBSFaultNullPoint;

			static const int    BufferSize = 4096;


			void readFault ( std::string&   faultName,
			PointSequence& faultLine,
			bool&          done );

			void readFault2 ( std::string&   faultName,
			PointSequence& faultLine,
			bool&          done );

			bool isCommentLine ( const char* buffer ) const;

			///
			/// Used to create the fault name
			///
			int faultCount;

      };
   }
}

#endif // _IBS_FAULT_FILE_READER_H
