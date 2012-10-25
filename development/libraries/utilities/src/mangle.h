#ifndef _UTILITIES_MANGLE_H_
#define _UTILITIES_MANGLE_H_

#include "DllExport.h"
#include <string>

namespace utilities {

#ifdef __cplusplus 
extern "C" {
#endif
   /// remove characters from name that may create difficulties when used in files.
   UTILITIES_DLL_EXPORT std::string mangle (const std::string & name);
#ifdef __cplusplus
}
#endif

} // namespace utilities

#endif // _UTILITIES_MANGLE_H_
