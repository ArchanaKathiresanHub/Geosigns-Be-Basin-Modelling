#include "MassBalance.C"

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <fstream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<fstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <fstream>
#endif // sgi

namespace migration {

template class MassBalance<ofstream>;

}
