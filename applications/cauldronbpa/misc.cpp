#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

static char * tmpdirptr = 0;
static char tmpdirarray[256];

const char * createTmpDir ()
{
   if (!tmpdirptr)
   {
      tmpdirptr = tmpdirarray;

      strcpy (tmpdirarray, "");
      char *envptr = getenv ("TMPDIR");

      if (envptr)
      {
         strcat (tmpdirarray, envptr);
         strcat (tmpdirarray, "/");
      }
      else
      {
         strcat (tmpdirarray, "/tmp/");
      }

      char *userName = getlogin ();

      if (!userName)
      {
	 cerr << "createTmpDir (): could not retrieve user name" << endl;
	 userName = "nouser";
      }


      strcat (tmpdirarray, userName);
      strcat (tmpdirarray, ".");

      char pid[16];

      sprintf (pid, "%ld", getpid ());
      strcat (tmpdirarray, pid);
   }

   if (mkdir (tmpdirarray, S_IRWXU | S_IRGRP | S_IXGRP) < 0 && errno != EEXIST)
   {
      perror (tmpdirarray);
      return 0;
   }
   else
   {
      return tmpdirptr;
   }
}

