#ifndef _UTILITIES_MANGLE_H_
#define _UTILITIES_MANGLE_H_

#include "DllExport.h"
#include <string>

namespace utilities {
   /// remove characters from name that may create difficulties when used in files.
   UTILITIES_DLL_EXPORT std::string mangle (const std::string & name);
} // namespace utilities

#endif // _UTILITIES_MANGLE_H_
