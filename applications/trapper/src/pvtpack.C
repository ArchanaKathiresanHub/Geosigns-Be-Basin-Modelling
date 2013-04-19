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
#include   <stdio.h>
#include   <string.h>
#include   "IBSinclude.h"
#include   <stdlib.h>
#include   "pvtpack.h"


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
       double phasesComponents[][XMig::NUM_COMPONENT],
       double *phasesDensity, double *phasesViscosity)
{
   int errorCode;               // 0 means no error
   double *ptr = ComponentsMass;

   // Done only once...
   initialize ();

#ifdef PVTPACK_DEBUG
   // This code will print out the input values
   cout << dTemperature << ", " << dPressure  << ", ";
   {
       int j;
       //     cout << "ComponentsMass:" << endl;
       for ( j=0; j<iNumberComponentsPresent; j++, ptr++ )
       {
         cout << *ptr << ", ";
       }
   }
   cout << endl;
#endif

#ifdef RS6000
   pvtflash
#else
   pvtflash_
#endif
         (&dTemperature, &dPressure, &iNumberComponentsMax, &iNumberComponentsPresent,
            ComponentsPresent, ComponentsMass, &iNumberOfPhasesMax, &iNumberOfPhases,
            phaseIsPresent, &(phasesComponents[0][0]), phasesDensity, phasesViscosity,
            &errorCode);

#ifdef PVTPACK_DEBUG
   cout << " errorCode = " << errorCode << endl;
   {
     int i, j;
     //     cout << "phaseComponentsPresent:" << endl;
     for ( i=0; i<iNumberOfPhases; i++ )
     {
       for ( j=0; j<iNumberComponentsPresent; j++ )
       {
         cout << phasesComponents[i][j] << " ";
       }
       cout << endl;
     }
   }
   cout << endl;
#endif

   if (!errorCode)
   {
      // computation seems OK.
      return true;
   }
   else
   {
      // there is a problem ...
      cerr << "PvtPack Failure, Temperature = "
           << dTemperature
           << " C, Pressure = "
           << dPressure
           << " MPa, Error code = "
           << errorCode
           << endl;

      for (int j = 0; j < iNumberOfPhases; j++)
      {
          for (int i = 0; i < XMig::NUM_COMPONENT; i++)
          {
              phasesComponents[j][i] = 0;
          }

          phasesDensity[j] = 0;
          phasesViscosity[j] = 0;
      }
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

   char abspath[IBSLINESIZE];
   char pathroot[IBSLINESIZE];

   if (getenv ("IBS_NAMES_PLATFORM") != NULL)
   {
      strcpy (pathroot, getenv ("IBS_NAMES_PLATFORM"));
   }
   else
   {
      cout << endl 
            << "PvtPack: 'IBS_NAMES_PLATFORM' run time environment variable not set" 
            << endl << "Please check your installation or contact the IBS+ Helpdesk" 
            << endl << "When IBS_NAMES_PLATFORM is reset, you may restart your program"
            << endl;
      return false;

   }

   sprintf (abspath, "%s%s%s", pathroot, pvtpacksubdir, "hitchpep.out");
   codeToFortranStrArg (hitchpep, abspath);

   sprintf (abspath, "./%s", "pvtpack.out");
   codeToFortranStrArg (pvtpack, abspath);

   sprintf (abspath, "./%s", "noutm.out");
   codeToFortranStrArg (noutm, abspath);

   sprintf (abspath, "%s%s%s", pathroot, pvtpacksubdir, "");
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
