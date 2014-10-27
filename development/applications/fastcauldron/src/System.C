#include "System.h"

#ifndef _MSC_VER  //TODO_SK: this does not compile on Windows

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

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


long getNumberOfCoresOnline () {
   return sysconf (_SC_NPROCESSORS_ONLN);
}

void getStatM ( StatM& statm ) {

   unsigned long dummy;
   const char* statm_path = "/proc/self/statm";

   FILE *f = fopen(statm_path,"r");

   if(!f){
      perror(statm_path);
   }

   if(7 != fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
                  &statm.size,&statm.resident,&statm.share,&statm.text,&statm.lib,&statm.data,&statm.dt))
   {
   }
   fclose(f);
}


#else

double GetResidentSetSize() { return 0; }
int GetCurrentLimit() { return 0; }
long GetPageSize() { return 0; }
int GetProcPID() { return 0; }

long getNumberOfCoresOnline () {
   return 0;
}

void getStatM ( StatM& statm ) {
   statm.size = 0;
   statm.resident = 0;
   statm.share = 0;
   statm.text = 0;
   statm.lib = 0;
   statm.data = 0;
   statm.dt = 0;

}


#endif
