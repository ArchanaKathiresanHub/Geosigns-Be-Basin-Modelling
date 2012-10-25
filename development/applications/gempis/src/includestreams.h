#ifndef __includestreams__
#define __includestreams__
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      #include <fstream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <fstream>
   #include <iomanip>
   using namespace std;
#endif // sgi

#endif
