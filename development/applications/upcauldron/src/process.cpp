#include "process.h"

#ifdef sgi
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif


#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <cstdio>
#include <cerrno>

#include <cstdlib>
#include <cstring>

#include <unistd.h>

static int NumChildren = 0;
static struct sigaction SignalActions;
static sigset_t SignalSet, EmptySignalSet;
static int MaxNumChildren = 1;

static void ChildHandler(int sig);
static void installChildHandling (void);

// Wait until the number of children is not higher than numChildren
void waitForChildren (int numChildren)
{
   while (NumChildren > numChildren)
   {
      sigsuspend(&EmptySignalSet);
   }
}

// compute the number of processors
int getNumProcessors (void)
{
   long numProcessors;
#ifdef sgi
   numProcessors = sysconf( _SC_NPROC_ONLN );
#else
   numProcessors = sysconf( _SC_NPROCESSORS_ONLN );
#endif
   return (int) numProcessors;
}

// Calculate the maximum number of children
void calculateMaxNumChildren (void)
{
   int numProcessors = getNumProcessors ();

   numProcessors /= 2;

   if (numProcessors < 1) numProcessors = 1;

   setMaxNumChildren ((int) numProcessors);

}

// Set the maximum number of children that can run in parallel
void setMaxNumChildren (int numChildren)
{
   if (numChildren > 0)
      MaxNumChildren = numChildren;
}

// run the command specified by path and argv,
// wait specifies whether the calling process should wait for the command to finish
int processCmd (char * path, char ** argv, bool wait)
{
   static bool firstTime = true;
   if (firstTime)
   {
      firstTime = false;
      installChildHandling ();
   }

   // wait until the number of running children is below the maximum allowed.
   waitForChildren (MaxNumChildren-1);

   pid_t pid = fork ();

   if (pid == -1)
   {
      return -1;
   }
   else if (pid == 0)
   {
      // we are in the child
      if (setpgid (0, 0) < 0)
      {
   perror ("setpgid (0, 0) failed: ");
      }

      close (0);
      int fd = open ("/dev/null", O_RDONLY);

      if (fd < 0)
      {
         char errorstr[128];

         snprintf (errorstr, sizeof (errorstr), "Could not open /dev/null:");
         perror (errorstr);
      }

      if (execvp (path, argv) == -1)
      {
         char errorstr[128];

         snprintf (errorstr, sizeof (errorstr), "Exec of '%s' failed: ", path);
         perror (errorstr);
         exit (-errno);
      }

      return 0;
   }
   else
   {
      // we are in the parent
#if 0
      cerr << "started process " << pid << endl;
#endif
      NumChildren++;
      if (wait)
      {
   // and wait for the child to finish
         int status;

         (void) waitpid (pid, &status, 0);
         NumChildren--;

         if (WIFEXITED (status))
         {
            return WEXITSTATUS (status);
         }
         else
         {
            fprintf (stderr, "Process '%s' terminated abnormally\n", path);
            return -2;
         }
      }
      else
      {
   // and do not wait for the child to finish
   return pid;
      }
   }
}

bool childHasFinished (int pid, int & exitValue)
{
   int status;

   if (waitpid (pid, &status, WNOHANG) > 0)
   {
      NumChildren--;
      if (WIFEXITED (status))
      {
         exitValue = WEXITSTATUS (status);
      }
      else
      {
         exitValue = -2;
      }
      return true;
   }
   else
   {
      return false;
   }
}

void killChild (int pid)
{
   cerr << "killing process " << -pid << endl;
   if (kill (-pid, SIGTERM) < 0)
      perror ("killChild failed: ");
#if 0
   sleep (1);
   kill (pid, SIGTERM);
   sleep (1);
   kill (pid, SIGQUIT);
   sleep (1);
   kill (pid, SIGKILL);
#endif
}

// Find an executable in '$PATH' with file name 'name'.
// Returns the complete path to a found executable.
// This path will be overwritten during the next call to this function
char * findExecutable (char * name)
{
   struct stat statusbuffer;

   char * ptrStart;
   char * ptrEnd;
   static char execPath[1024];
   static char * env = NULL;

   // Get the directories in which to look
   if (!env) env = getenv ("PATH");

   // Iterate through them
   for (ptrStart = env; ptrStart != NULL; ptrStart = (ptrEnd ? ptrEnd + 1 : NULL))
   {
      ptrEnd = strchr (ptrStart, ':');
      if (ptrEnd) * ptrEnd = '\0';

      snprintf (execPath, sizeof (execPath), "%s/%s", ptrStart, name);

      // Restore $PATH
      if (ptrEnd) * ptrEnd = ':';

			if (stat (execPath, &statusbuffer) == 0 &&
		access (execPath, X_OK) == 0)
			{
	 // found it!!
	 return execPath;
			}
	 }

   // Didn't find it.
   return NULL;
}


static void ChildHandler(int sig)
{
   int status;

   while (waitpid (-1, &status, WNOHANG) > 0)
   {
      NumChildren--;
   }
}

static void installChildHandling (void)
{
   SignalActions.sa_handler = ChildHandler;
   sigemptyset (&SignalActions.sa_mask);
   sigaddset (&SignalActions.sa_mask, SIGCHLD);
   SignalActions.sa_flags = 0;

   sigaction (SIGCHLD, &SignalActions, NULL);
   sigemptyset (&SignalSet);
   sigemptyset (&EmptySignalSet);
   sigaddset (&SignalSet, SIGCHLD);
   sigprocmask (SIG_BLOCK, &SignalSet, NULL);
   setbuf (stdout, NULL);
}





