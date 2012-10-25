#ifdef QT_THREAD_SUPPORT
#include <unistd.h>

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

#include "loaderthread.h"
#include "infobase.h"

extern bool cbTiming;
extern bool cbVerbose;

LoaderThread::LoaderThread (InfoBase * infoBase, unsigned int whatToLoad)
   :m_infoBase (infoBase), m_whatToLoad (whatToLoad)
{
}

void LoaderThread::run (void)
{
   if (cbVerbose)
      cerr << "Starting LoaderThread\n";
   m_infoBase->stayConnected ();
   if (cbVerbose)
      cerr << "Finishing LoaderThread\n";
}
#endif
