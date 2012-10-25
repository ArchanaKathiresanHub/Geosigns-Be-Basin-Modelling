#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
using namespace std;

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

/// Check whether migration was successfully performed
bool migrationCompleted (DataAccess::Interface::ProjectHandle * projectHandle);

#endif // _UTILS_H_

