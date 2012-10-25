///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef _PVTPACK_H_
#define _PVTPACK_H_
///////////////////////////////////////////////////////////////////////////////
//                            PVTCALC definitions                            //
///////////////////////////////////////////////////////////////////////////////
#include "pvtpack.h"

#include "migration.h"

namespace migration
{
   class PvtPack
   {
      public:
	 static int codeToFortranStrArg (int *intTab, const char *inputStr);

	 static bool compute (double dTemperature, double dPressure,
	       int iNumberComponentsMax,
	       int iNumberComponentsPresent, int *ComponentsPresent,
	       double *ComponentsMass, int iNumberOfPhasesMax,
	       int iNumberOfPhases, int *phaseIsPresent,
	       double phasesComponents[][NumComponents],
	       double *phasesDensity,
	       double *phasesViscosity);

	 //private:
	 static bool initialize (void);
   };
}


#endif // _PVTPACK_H_
