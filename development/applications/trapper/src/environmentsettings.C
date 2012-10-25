/// Class EnvironmentSettings handles queries
/// to the environment by accessing environment variables

#include "environmentsettings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/sys/utsname.h"

bool EnvironmentSettings::debugOn ()
{
   const char *debugStr = getenv ("TT_DEBUG_ON");
   return debugStr ? atoi (debugStr) : false;
}

bool EnvironmentSettings::isLinux ()
{
   struct utsname thisMachine;
   uname (&thisMachine);
   return strcmp (thisMachine.sysname, "Linux") == 0;
}

void EnvironmentSettings::systemCommand (const char* command)
{
   system (command);
}


