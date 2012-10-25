/// Class EnvironmentSettings handles queries
/// to the environment by accessing environment variables

#ifndef __environmentsettings__
#define __environmentsettings__

struct EnvironmentSettings
{
   static bool debugOn ();
   static bool isLinux ();
   static void systemCommand (const char* command);
};

#endif
