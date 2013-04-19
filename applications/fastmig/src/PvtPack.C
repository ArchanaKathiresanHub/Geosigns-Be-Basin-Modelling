/*******************************************************************************
* Class:       PvtPack
*
* File:        pvtpack.C
*
* Copyright:   (c) 1997-1999 Shell International Exploration and Production
*
* Description: 
 
~ *****************************************************************************
~
~   File name:    pvtpack.C
~   Copyright:    KSEPL
~   Function:     interface to PVTPACK fortran library)
~   Dependencies: fortran PVT library + RubyUtilities
~   Usage:        
~   References:
~   Remarks:
~
~ *****************************************************************************
 
!==========================================================================

*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                              PVTCALC Methods                              //
///////////////////////////////////////////////////////////////////////////////
// This object is a member of the module RUBYPROJECT in project RBYPROJECT   //
///////////////////////////////////////////////////////////////////////////////
#include   <stdlib.h>
#include   <stdio.h>
#include   <string.h>
#include <iostream.h>
#include <fstream.h>

#include   "PvtPack.h"

using namespace migration;

int PvtPack::codeToFortranStrArg (int *intTab, const char *inputStr)
{
   int result = 0;              // no error
   int length = strlen (inputStr);

   if (length > 256)
   {
      result = 1;               // string to long for fortran
      length = -1;
   }
   for (int i = 0; i < 256; i++)
   {
      if (i <= length)
      {
         intTab[i] = inputStr[i];
      }
      else
      {
         intTab[i] = ' ';
      }
   }
   return result;
}


///////////////////////////////////////////////////////////////////////////////
//                      Member function Implementation                       //
///////////////////////////////////////////////////////////////////////////////


bool PvtPack::compute (double dTemperature, double dPressure,
			    int iNumberComponentsMax, int iNumberComponentsPresent,
			    int *ComponentsPresent, double *ComponentsMass,
			    int iNumberOfPhasesMax, int iNumberOfPhases,
			    int *phaseIsPresent,
			    double phasesComponents[][NumComponents],
			    double *phasesDensity, double *phasesViscosity)
{
   int errorCode;               // 0 means no error

   // Done only once...
   initialize ();

#ifdef RS6000
   pvtflash
#else
   pvtflash_
#endif
         (&dTemperature, &dPressure, &iNumberComponentsMax, &iNumberComponentsPresent,
          ComponentsPresent, ComponentsMass, &iNumberOfPhasesMax, &iNumberOfPhases,
          phaseIsPresent, &(phasesComponents[0][0]), phasesDensity, phasesViscosity, &errorCode);

   if (!errorCode)
   {
      // computation seems OK.
      return true;
   }
   else
   {
      // there is a problem ...
      cerr << "PvtPack Failure, Temperature = " << dTemperature << " C, Pressure = " << dPressure << " MPa" << endl;

      return false;
   }
}

bool PvtPack::initialize (void)
{
   static bool alreadyInitialized = false;

   if (alreadyInitialized)
   {
      // we do not need to init it again -> that causes problems for PVT pack!!!
      return true;
   }

   int ierror = 0;
   // default pvt model number 1. Presently the only one available.
   int pvtModel = 1;
   int hitchpep[256];
   int pvtpack[256];
   int noutm[256];
   int tracefile[256];
   int shale[256];
   const char pvtpacksubdir[] = "/pvtpack/";

   char abspath[256];
   char pathroot[256];

   if (getenv ("PvtPackDataDir") != NULL)
   {
      strcpy (pathroot, getenv ("PvtPackDataDir"));
   }
   if (getenv ("IBS_NAMES_PLATFORM") != NULL)
   {
      strcpy (pathroot, getenv ("IBS_NAMES_PLATFORM"));
      strcat (pathroot, pvtpacksubdir);
   }
   else
   {
      cerr << "Neither environment variable 'PvtPackDataDir' or 'IBS_NAMES_PLATFORM' was set."
	 << endl
	 << "Using current directory for PVT configuration files!!!"
	 << endl;
      strcpy(pathroot, ".");
   }

   sprintf (abspath, "%s%s", pathroot, "hitchpep.out");
   codeToFortranStrArg (hitchpep, abspath);

   sprintf (abspath, "./%s", "pvtpack.out");
   codeToFortranStrArg (pvtpack, abspath);

   sprintf (abspath, "./%s", "noutm.out");
   codeToFortranStrArg (noutm, abspath);

   sprintf (abspath, "%s%s", pathroot, "");
   codeToFortranStrArg (tracefile, abspath);

   sprintf (abspath, "./%s", "shale.INP");
   codeToFortranStrArg (shale, abspath);

#ifdef RS6000
   extfil
#else
   extfil_
#endif
         (hitchpep,
          pvtpack,
          noutm,
          shale,
          tracefile,
          &ierror);

#ifdef RS6000
   pvtinit
#else
   pvtinit_
#endif
         (&pvtModel,
          &ierror);

   if (!ierror)
   {
      // no error information means no error at all!!!
      alreadyInitialized = true;
      return true;
   }
   else
   {
      // TBD ...
      cout << "something wrong with PVT PACK cannot init" << endl;
      return false;
   }
}
