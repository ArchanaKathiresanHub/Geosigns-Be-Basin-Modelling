#ifndef FASTTOUCH_MISC_H
#define FASTTOUCH_MISC_H

#include <string>

namespace fasttouch
{
   extern int GetRank();
   extern int GetSize();

   extern void StartProgress ();
   extern void ReportProgress (const std::string & message);
}

#endif
