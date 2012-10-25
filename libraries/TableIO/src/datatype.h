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
