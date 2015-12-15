//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#ifndef _DATATYPE_
#define _DATATYPE_

namespace datatype
{
   /// Enumeration datatype used in the definition of FieldTypes
   enum DataType
   {
      NoDataType = -1,
      Bool = 0, ///< For use with bool values.
      Int, ///< For use with int values.
      Long, ///< For use with long values.
      Float, ///< For use with float values.
      Double, ///< For use with double values.
      String ///< For use with string values.
   };

}
#endif //  _DATATYPE_
