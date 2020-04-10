//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _LANDMARK_FAULT_FILE_READER_H
#define _LANDMARK_FAULT_FILE_READER_H

#include "FaultFileReader2D.h"

namespace DataAccess
{
   namespace Interface
   {
      const std::string LandmarkFaultFileReaderID = "LANDMARKPOLYGON";

      class LandmarkFaultFileReader : public FaultFileReader2D {

         public :

            LandmarkFaultFileReader ();
            ~LandmarkFaultFileReader ();

            void preParseFaults ();

         private :


            double getDouble ( const char* buffer,
                  const int   numberStart,
                  const int   numberLength ) const;

            int getInteger ( const char* buffer,
                  const int   numberStart,
                  const int   numberLength ) const;

            std::string getString ( const char* buffer,
                  const int   stringStart,
                  const int   stringLength ) const;


            void readBuffer ( char* buffer );

            bool isComment ( const char* buffer ) const;

            double getXCoordinate ( const char* buffer ) const;

            double getYCoordinate ( const char* buffer ) const;

            double getZCoordinate ( const char* buffer ) const;

            int    getPointType  ( const char* buffer ) const;

            std::string getFaultName ( const char* buffer ) const;

            DistanceUnit getDistanceUnit ( const char* buffer ) const;



            static const int LandmarkFaultFileLineLength = 160;

            static const int StartOfFault  = 1;

            static const int MiddleOfFault = 2;

            static const int EndOfFault    = 3;

            ///
            /// Strings (char arrays) are zero based, so all constants here that designate the start of
            /// a particular field are 1 less than those specified in the landmark fault file format spec.
            /// E.g. In the spec. FAULT_NAME starts in column 44, and so it will start at position 43 in the string.
            ///
            static const int FAULT_X_START = 0;
            static const int FAULT_X_LENGTH = 12;

            static const int FAULT_Y_START = 12;
            static const int FAULT_Y_LENGTH = 12;

            static const int FAULT_Z_START = 24;
            static const int FAULT_Z_LENGTH = 12;

            static const int FAULT_NAME_START = 43;
            static const int FAULT_NAME_LENGTH = 50;

            static const int FAULT_POINT_TYPE_START = 41;
            static const int FAULT_POINT_TYPE_LENGTH = 2;

            static const int FAULT_DISTANCE_UNIT_START = 134;
            static const int FAULT_DISTANCE_UNIT_LENGTH = 20;


            static const int FaultFileLineLength = 256;

      };
   }
}


#endif // _LANDMARK_FAULT_FILE_READER_H
