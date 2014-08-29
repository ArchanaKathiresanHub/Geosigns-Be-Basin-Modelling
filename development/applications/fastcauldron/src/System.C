#include "System.h"

#ifndef _MSC_VER  //TODO_SK: this does not compile on Windows

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>

#include "petscconf.h"

double GetResidentSetSize()
{
  static struct rusage temp;

  getrusage(RUSAGE_SELF,&temp);

  return ((double)temp.ru_maxrss) / 1024;
  
}

int GetCurrentLimit()
{
#if defined(PARCH_linux) || defined(PARCH_solaris)
  static struct rlimit rlp;
  getrlimit(RLIMIT_DATA,&rlp);
#else
  static struct rlimit64 rlp;
  getrlimit64(RLIMIT_DATA,&rlp);
#endif


  return (int) rlp.rlim_cur;

}

long GetPageSize()
{

  return sysconf (_SC_PAGESIZE);

}

int GetProcPID()
{

  return (int)getpid();

}
#else

double GetResidentSetSize() { return 0; }
int GetCurrentLimit() { return 0; }
long GetPageSize() { return 0; }
int GetProcPID() { return 0; }

#endif
