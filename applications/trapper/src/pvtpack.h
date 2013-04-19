///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rybygeologymodel_pvtpack__
#define __rybygeologymodel_pvtpack__
///////////////////////////////////////////////////////////////////////////////
//                            PVTCALC definitions                            //
///////////////////////////////////////////////////////////////////////////////
#include "includestreams.h"
#include "xmig.h"

class PvtPack
{
public:
   static int codeToFortranStrArg (int *intTab, const char *inputStr);

   static bool compute( double dTemperature, double dPressure,
        int iNumberComponentsMax,
        int iNumberComponentsPresent, int* ComponentsPresent,
        double* ComponentsMass, int iNumberOfPhasesMax,
        int iNumberOfPhases, int* phaseIsPresent,
        double phasesComponents[][XMig::NUM_COMPONENT], double* phasesDensity,
        double* phasesViscosity );

private:
   static bool initialize (void);
};

// C to Fortran mappings

#ifdef __cplusplus
extern "C" {
#endif
#ifdef RS6000
  void pvtinit(
#else
  void pvtinit_(
#endif
    int* pvtModel,   /* indicator for pvt model to be used */
    int* ierror);    /* error indicator should be 0 on exit if no errors */

/* init the input file path for the pvt pack ... temporary fix */
#ifdef RS6000
  void extfil(
#else
  void extfil_(
#endif
    int* hitchp,
    int* pvtpck,
    int* noutm,
    int* shale,
    int* trcfil,
    int* ierror);    /* error indicator should be 0 on exit if no errors */

#ifdef RS6000
  void pvtflash(
#else
  void pvtflash_(
#endif
    double*  dTemperature,      /* temperature (Centigrade) */
    double*  dPressure,         /* fluid pressure (MPa) */
    int*     iNumberComponentsMax,
 /* maximum of number of components and size of component arrays */
    int*     iNumberComponentsPresent, /* number of components */
    int*     ComponentsPresent, /* array of component numbers of all components present */
    double*  componentsMass,    /* array of component masses in Kg */
    int*     iNumberOfPhasesMax,
	/* maximum of number of phases and size of phase arrays */
    int*     iNumberOfPhases,   /* number of phases (should always be 2, Gas and Fluid) */
    int*     phaseIsPresent,    /* tells if the phase (output) is present or not */
    double   * phasesComponents,
    	/* array of phases and for each phases an array of component masses in kg */
    double*  phasesDensity,     /* array of phase density (kg/m3) */
    double*  phasesViscosity,   /* array of phase viscosity (mPoise) */
    int*     errorCode);        /*  error indicator should be 0 on exit if no errors */
  
#ifdef __cplusplus
}
#endif
#endif // __rybygeologymodel_pvtpack__
