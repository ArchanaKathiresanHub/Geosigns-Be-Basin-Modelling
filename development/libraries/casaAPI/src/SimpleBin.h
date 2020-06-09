//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#ifndef SIMPLE_BIN_SD
#define SIMPLE_BIN_SD

#ifndef SIMPLE_BIN_SD_DATA_TYPES
#error( "SimpleBin keeps details of implementation of SimpleBin Serializer/Deserializer and is not inteded to be used as CASA API"
#endif

namespace casa
{
   // Set of axillary function to be used in loading 

   // to reduce binary file size, change simple data types to IDs
   // this enum is duplicated in SimpleSerializer.cpp
   typedef enum
   {
      BoolID,
      IntID,
      UintID,
      LlongID,
      RefID,
      FloatID,
      DoubleID,
      StringID,
      UnknownID
   } SimpleDataTypeBinID;
}

#endif // SIMPLE_BIN_SD
