//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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

namespace migration
{

   template class MassBalance<ofstream>;

}
