/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsgxkernel.c,v 25.0 2005/07/05 08:03:57 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: ibsgxkernel.c,v $
 * Revision 25.0  2005/07/05 08:03:57  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:47  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:19  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:50  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:55  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:45:13  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 17.0  1998/12/15 14:26:56  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:43  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:29  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:43  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:07:04  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:29  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:43:07  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.5  1995/11/28  14:44:14  ibs
 * misprint
 *
 * Revision 9.4  1995/11/27  09:17:20  ibs
 * Debug statements out.
 *
 * Revision 9.3  1995/11/21  10:57:47  ibs
 * removed debug output.
 *
 * Revision 9.2  1995/11/21  09:02:22  ibs
 * Selection of gas species extended.
 * Temporary debug reports on species selected.
 *
 * Revision 9.1  1995/11/20  13:09:08  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		21-SEP-1993

Description:	Extensions to gxkernel to include GENEX3 functionality in
		IBS. These extensions might be included in a future release
		of the gxkernel. A number of things can be implemented is a
		better way if these extensions are part of the gxkernel.

		ibsgxkernel.c contains high level functions based on the
		functions available in gxkernel of Genex. These functions
		were developed to include the GENEX-3 functionality in IBS
		and are called from the IBS software.

		A list of the available functions and their function
		prottypes is present in the file ibsgxkernel.h.

Usage notes:	To be able to use the functions of this file ibsgxkernel.h
		must be included.
		All functions return an error code. The Genex error handler
		function is called.

History:
--------
21-SEP-1993	ErKo	initial version: GxGetGeologicalGroupExpulsions and
			GxSpeciesInSpeciesGroup
22-SEP-1993	ErKo	added: GxIsSpeciesOil, GxIsSpeciesHCGas,
			GxIsSpeciesDryGas, GxIsSpeciesWetGas and
			GxIsSpeciesCOx
27-SEP-1993	ErKo	added: GxIsInit, GxSetInit and GxClearInit
29-SEP_1993	ErKo	added: GxGetGeologicalGroupFluxes,
			GxGetGeologicalGroupGenRates,
			GxGetGeologicalGroupGenCumRates
 7-OCT-1993	ErKo	added GxCalcHCI and GxCalcOCI
17-DEC-1993     PJH     Changed GxIsSpeciesOil, GxIsSpeciesHCGas,
                        to GxIsSpeciesOilNam, GxIsSpeciesHCGasNam,
20-DEC-1993     ErKo    initialised GroupRates array
24-DEC-1993     ErKo    IsWetGas checks for WET_GAS constant
21-NOV-1995     PJH     
------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <string.h>
#include  "list.h"
#include  "gxgenerl.h"
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxhorizn.h"
#include  "gxlithol.h"
#include  "gxsbarea.h"
#include  "gxkernel.h"
#include  "ibsgxkernel.h"

/* Prototypes of local functions */
/* ----------------------------- */
static GxErrorCode GxSpeciesInSpeciesGroup
   (GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    GxBool *       SpeciesInGroup)
;

/* Local variables */
/* --------------- */
GxBool genexInitialised = GxFalse;

/*
------------------------------------------------------------------------------
GxCalcHCI -	Calculates the atomic H/C ratio
------------------------------------------------------------------------------

GxCalcHCI calculates the atomic G/C based on the formula
presented in the GENEX-3 User's Guide.
Note: The value for TOC must be used as a percentage (not a fraction) !
------------------------------------------------------------------------------
*/
double GxCalcHCI
   (double S2,
    double S3,
    double TOC)
{
   double value;

   value = (0.128 * S2 + 0.052 * S3) / TOC + 0.5;
   return (value);
}

/*
------------------------------------------------------------------------------
GxCalcOCI -	Calculates the atomic O/C ratio
------------------------------------------------------------------------------

GxCalcHCI calculates the atomic O/C ratio based on the formula
presented in the GENEX-3 User's Guide.
Note: The value for TOC must be used as a percentage (not a fraction) !
------------------------------------------------------------------------------
*/
double GxCalcOCI
   (double S3,
    double TOC)
{
   double value;

   value = 0.45 * S3 / TOC + 0.03;
   if (value < 0.05)
   {
      value = 0.05;
   }
   return (value);
}

/*
------------------------------------------------------------------------------
GxCalcS2 -	Calculates the initial S2
------------------------------------------------------------------------------

GxCalcS2 calculates the initial S2 based on the formula
presented in the GENEX-3 User's Guide.
Note: The value for TOC must be used as a percentage (not a fraction) !
------------------------------------------------------------------------------
*/
double GxCalcS2
   (double HCI,
    double OCI,
    double TOC)
{
   double value;

   value = ( -3.879166667 + 7.8125 * HCI - 0.902777777 * OCI ) * TOC;
   return (value);
}

/*
------------------------------------------------------------------------------
GxCalcS3 -	Calculates the initial S3
------------------------------------------------------------------------------

GxCalcS3 calculates the initial S3 based on the formula
presented in the GENEX-3 User's Guide.
Note: The value for TOC must be used as a percentage (not a fraction) !
------------------------------------------------------------------------------
*/
double GxCalcS3
   (double OCI,
    double TOC)
{
   double value;

   value = (OCI - 0.03) * TOC / 0.45;
   return (value);
}

/*
------------------------------------------------------------------------------
GxClearInit - Clears the boolean which indicated that GENEX is initialised 
------------------------------------------------------------------------------

GxClearInit clears the local boolean if this is correct.

GxClearInit sets genexInitialised to GxFalse if it had GxTrue as value.
------------------------------------------------------------------------------
*/
GxErrorCode GxClearInit
   (void)
{
   GxErrorCode rc;

   rc = GXE_NOERROR;
   if (GxIsInit ())
   {
      genexInitialised = GxFalse;
   }
   else
   {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxGetGeologicalGroupExpulsions - Get expulsions history for a group of species
------------------------------------------------------------------------------

GxGetGeologicalGroupExpulsions returns the expulsion data for the group of
species <group> in the submodel of the geological model identified by
<subarea>, <horizon> and <lithology>.

Which expulsion data is returned is specified by <updown> and <cumulative>.

The expulsion history array passed should be allocated by calling
GxCreateModelHistory.
------------------------------------------------------------------------------
*/
GxErrorCode GxGetGeologicalGroupExpulsions 
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    GxUpDown       UpAndOrDown,
    GxCumulative   NotOrCumulative,
    double         BeginTime,
    double         EndTime,
    double *       GroupExpulsions)
{
   GxErrorCode  rc;
   GxBool       addSpeciesHistory;
   double *     expulsions;
   List         speciesList;
   int          nTimes;
   char *       currentSpeciesName;
   int          i;

   rc = GXE_NOERROR;
   addSpeciesHistory = GxFalse;
   expulsions = NULL;
   speciesList = NULL;

   /* Check arguments */
   if    (!SubAreaName
      || (strlen (SubAreaName) == 0)
      || (strlen (SubAreaName) > GXL_SUBAREANAME)
      || !HorizonName
      || (strlen (HorizonName) == 0)
      || (strlen (HorizonName) > GXL_HORIZONNAME)
      || !LithologyName
      || (strlen (LithologyName) == 0)
      || (strlen (LithologyName) > GXL_LITHOLOGYNAME)
      || !GroupExpulsions)
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* Determine the length of the history array */
      rc = GxGetGeologicalTimesteps (BeginTime,
                                     EndTime,
                                     & nTimes);
      if (rc == GXE_NOERROR)
      {
         /* Create local array to store expulsions of one species */
         rc = GxCreateModelHistory (nTimes, & expulsions);
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Initialise the returned results */
      for (i = 0; i < nTimes; i++)
      {
         GroupExpulsions [i] = 0.0;
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Get all mobile species */
      rc = GxGetMobileSpeciesNameList (& speciesList);
   }

   if (rc == GXE_NOERROR)
   {
      /* Get first name of first species */
      currentSpeciesName = GetFirstFromList (speciesList);

      /* Check all mobile species */
      while   ((currentSpeciesName != NULL)
            && (rc == GXE_NOERROR))
      {
         /* Add the expulsion history of this species to the output array ? */
         rc = GxSpeciesInSpeciesGroup (SpeciesGroup,
                                       currentSpeciesName,
                                       & addSpeciesHistory);

         if   ((rc == GXE_NOERROR)
            && (addSpeciesHistory == GxTrue))
         {
            /* Get history for the current species */
            rc = GxGetGeologicalExpulsions (SubAreaName,
                                            HorizonName,
                                            LithologyName,
                                            currentSpeciesName,
                                            UpAndOrDown,
                                            NotOrCumulative,
                                            BeginTime,
                                            EndTime,
                                            expulsions);
           if (rc == GXE_NOERROR)
           {
              for (i = 0; i < nTimes; i++)
              {
                 /* Add expulsions of current species to ouytput array */
                 GroupExpulsions [i] += expulsions [i];
              }
           }
         }

         if (rc == GXE_NOERROR)
         {
            /* Get next species */
            currentSpeciesName = GetNextFromList (speciesList);
         }
      }
   }

   /* Free allocated memory */
   if (expulsions != NULL)
   {
      rc = GxFreeModelHistory (expulsions);
   }
   if (speciesList != NULL)
   {
      DeleteList (speciesList);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxGetGeologicalGroupFluxes - Get fluxes history for a group of species
------------------------------------------------------------------------------

GxGetGeologicalGroupFluxes returns the fluxes data for the group of
species <group> in the submodel of the geological model identified by
<subarea>, <horizon> and <lithology>.

Which expulsion fluxes are returned is controlled by <updown>.

The expulsion fluxes history array passed should be allocated by calling
GxCreateModelHistory.
------------------------------------------------------------------------------
*/
GxErrorCode GxGetGeologicalGroupFluxes 
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    GxUpDown       UpAndOrDown,
    double         BeginTime,
    double         EndTime,
    double *       GroupFluxes)
{
   GxErrorCode  rc;
   GxBool       addSpeciesHistory;
   double *     fluxes;
   List         speciesList;
   int          nTimes;
   char *       currentSpeciesName;
   int          i;

   rc = GXE_NOERROR;
   addSpeciesHistory = GxFalse;
   fluxes = NULL;
   speciesList = NULL;

   /* Check arguments */
   if    (!SubAreaName
      || (strlen (SubAreaName) == 0)
      || (strlen (SubAreaName) > GXL_SUBAREANAME)
      || !HorizonName
      || (strlen (HorizonName) == 0)
      || (strlen (HorizonName) > GXL_HORIZONNAME)
      || !LithologyName
      || (strlen (LithologyName) == 0)
      || (strlen (LithologyName) > GXL_LITHOLOGYNAME)
      || !GroupFluxes)
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* Determine the length of the history array */
      rc = GxGetGeologicalTimesteps (BeginTime,
                                     EndTime,
                                     & nTimes);
      if (rc == GXE_NOERROR)
      {
         /* Create local array to store expulsions of one species */
         rc = GxCreateModelHistory (nTimes, & fluxes);
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Initialise the returned results */
      for (i = 0; i < nTimes; i++)
      {
         GroupFluxes [i] = 0.0;
      }
   }


   if (rc == GXE_NOERROR)
   {
      /* Get all mobile species */
      rc = GxGetMobileSpeciesNameList (& speciesList);
   }

   if (rc == GXE_NOERROR)
   {
      /* Get first name of first species */
      currentSpeciesName = GetFirstFromList (speciesList);

      /* Check all mobile species */
      while   ((currentSpeciesName != NULL)
            && (rc == GXE_NOERROR))
      {
         /* Add the expulsion history of this species to the output array ? */
         rc = GxSpeciesInSpeciesGroup (SpeciesGroup,
                                       currentSpeciesName,
                                       & addSpeciesHistory);

         if   ((rc == GXE_NOERROR)
            && (addSpeciesHistory) == GxTrue)
         {
            /* Get history for the current species */
            rc = GxGetGeologicalFluxes (SubAreaName,
                                        HorizonName,
                                        LithologyName,
                                        currentSpeciesName,
                                        UpAndOrDown,
                                        BeginTime,
                                        EndTime,
                                        fluxes);
           if (rc == GXE_NOERROR)
            {
               for (i = 0; i < nTimes; i++)
               {
                  /* Add expulsions of current species to ouytput array */
                  GroupFluxes [i] += fluxes [i];
               }
            }
         }

         if (rc == GXE_NOERROR)
         {
            /* Get next species */
            currentSpeciesName = GetNextFromList (speciesList);
         }
      }
   }

   /* Free allocated memory */
   if (fluxes != NULL)
   {
      rc = GxFreeModelHistory (fluxes);
   }
   if (speciesList != NULL)
   {
      DeleteList (speciesList);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxGetGeologicalGroupGenCumRates -	Get Generated mass history for a group
					of species
------------------------------------------------------------------------------

GxGetGeologicalGroupGenCumRates returns the cumulative masses data for the
group of species <group> in the submodel of the geological model identified by
<subarea>, <horizon> and <lithology>.

The expulsion history array passed should be allocated by calling
GxCreateModelHistory.
------------------------------------------------------------------------------
*/
GxErrorCode GxGetGeologicalGroupGenCumRates
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    double         BeginTime,
    double         EndTime,
    double *       GroupCumRates)
{
   GxErrorCode  rc;
   GxBool       addSpeciesHistory;
   double *     cumRates;
   List         speciesList;
   int          nTimes;
   char *       currentSpeciesName;
   int          i;

   rc = GXE_NOERROR;
   addSpeciesHistory = GxFalse;
   cumRates = NULL;
   speciesList = NULL;

   /* Check arguments */
   if    (!SubAreaName
      || (strlen (SubAreaName) == 0)
      || (strlen (SubAreaName) > GXL_SUBAREANAME)
      || !HorizonName
      || (strlen (HorizonName) == 0)
      || (strlen (HorizonName) > GXL_HORIZONNAME)
      || !LithologyName
      || (strlen (LithologyName) == 0)
      || (strlen (LithologyName) > GXL_LITHOLOGYNAME)
      || !GroupCumRates)
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* Determine the length of the history array */
      rc = GxGetGeologicalTimesteps (BeginTime,
                                     EndTime,
                                     & nTimes);
      if (rc == GXE_NOERROR)
      {
         /* Create local array to store expulsions of one species */
         rc = GxCreateModelHistory (nTimes, & cumRates);
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Initialise the returned results */
      for (i = 0; i < nTimes; i++)
      {
         GroupCumRates [i] = 0.0;
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Get all mobile species */
      rc = GxGetMobileSpeciesNameList (& speciesList);
   }

   if (rc == GXE_NOERROR)
   {
      /* Get first name of first species */
      currentSpeciesName = GetFirstFromList (speciesList);

      /* Check all mobile species */
      while   ((currentSpeciesName != NULL)
            && (rc == GXE_NOERROR))
      {
         /* Add the expulsion history of this species to the output array ? */
         rc = GxSpeciesInSpeciesGroup (SpeciesGroup,
                                       currentSpeciesName,
                                       & addSpeciesHistory);

         if   ((rc == GXE_NOERROR)
            && (addSpeciesHistory == GxTrue))
         {
            /* Get history for the current species */
            rc = GxGetGeologicalGenCumRate (SubAreaName,
                                            HorizonName,
                                            LithologyName,
                                            currentSpeciesName,
                                            BeginTime,
                                            EndTime,
                                            cumRates);
           if (rc == GXE_NOERROR)
            {
               for (i = 0; i < nTimes; i++)
               {
                  /* Add expulsions of current species to ouytput array */
                  GroupCumRates [i] += cumRates [i];
               }
            }
         }

         if (rc == GXE_NOERROR)
         {
            /* Get next species */
            currentSpeciesName = GetNextFromList (speciesList);
         }
      }
   }

   /* Free allocated memory */
   if (cumRates != NULL)
   {
      rc = GxFreeModelHistory (cumRates);
   }
   if (speciesList != NULL)
   {
      DeleteList (speciesList);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxGetGeologicalGroupGenRates -	Get Generated rate history for a group
				of species
------------------------------------------------------------------------------

GxGetGeologicalGroupGenRates returns the cumulative rates for the
group of species <group> in the submodel of the geological model identified by
<subarea>, <horizon> and <lithology>.

The rates history array passed should be allocated by calling
GxCreateModelHistory.
------------------------------------------------------------------------------
*/
GxErrorCode GxGetGeologicalGroupGenRates
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    double         BeginTime,
    double         EndTime,
    double *       GroupRates)
{
   GxErrorCode  rc;
   GxBool       addSpeciesHistory;
   double *     rates;
   List         speciesList;
   int          nTimes;
   char *       currentSpeciesName;
   int          i;

   rc = GXE_NOERROR;
   addSpeciesHistory = GxFalse;
   rates = NULL;
   speciesList = NULL;

   /* Check arguments */
   if    (!SubAreaName
      || (strlen (SubAreaName) == 0)
      || (strlen (SubAreaName) > GXL_SUBAREANAME)
      || !HorizonName
      || (strlen (HorizonName) == 0)
      || (strlen (HorizonName) > GXL_HORIZONNAME)
      || !LithologyName
      || (strlen (LithologyName) == 0)
      || (strlen (LithologyName) > GXL_LITHOLOGYNAME)
      || !GroupRates)
   {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   }

   if (rc == GXE_NOERROR)
   {
      /* Determine the length of the history array */
      rc = GxGetGeologicalTimesteps (BeginTime,
                                     EndTime,
                                     & nTimes);
      if (rc == GXE_NOERROR)
      {
         /* Create local array to store expulsions of one species */
         rc = GxCreateModelHistory (nTimes, & rates);
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Initialise the returned results */
      for (i = 0; i < nTimes; i++)
      {
         GroupRates [i] = 0.0;
      }
   }

   if (rc == GXE_NOERROR)
   {
      /* Get all mobile species */
      rc = GxGetMobileSpeciesNameList (& speciesList);
   }

   if (rc == GXE_NOERROR)
   {
      /* Get first name of first species */
      currentSpeciesName = GetFirstFromList (speciesList);

      /* Check all mobile species */
      while   ((currentSpeciesName != NULL)
            && (rc == GXE_NOERROR))
      {
         /* Add the expulsion history of this species to the output array ? */
         rc = GxSpeciesInSpeciesGroup (SpeciesGroup,
                                       currentSpeciesName,
                                       & addSpeciesHistory);

         if   ((rc == GXE_NOERROR)
            && (addSpeciesHistory == GxTrue))
         {
            /* Get history for the current species */
            rc = GxGetGeologicalGenRate (SubAreaName,
                                         HorizonName,
                                         LithologyName,
                                         currentSpeciesName,
                                         BeginTime,
                                         EndTime,
                                         rates);
           if (rc == GXE_NOERROR)
            {
               for (i = 0; i < nTimes; i++)
               {
                  /* Add expulsions of current species to ouytput array */
                  GroupRates [i] += rates [i];
               }
            }
         }

         if (rc == GXE_NOERROR)
         {
            /* Get next species */
            currentSpeciesName = GetNextFromList (speciesList);
         }
      }
   }

   /* Free allocated memory */
   if (rates != NULL)
   {
      rc = GxFreeModelHistory (rates);
   }
   if (speciesList != NULL)
   {
      DeleteList (speciesList);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxIsInit - Determines whether GENEX has been opened
------------------------------------------------------------------------------

GxInit determines whether the GENEX has been initialised.

GxIsInit returns GxTrue if GxOpen has been called succesfully.
It returns False if it wasn't called succesfully.
------------------------------------------------------------------------------
*/
GxBool GxIsInit
   (void)
{
   return (genexInitialised);
}

/*
------------------------------------------------------------------------------
GxIsSpeciesCOx - Check whether species is a COx species
------------------------------------------------------------------------------

GxIsSpeciesCOx determines whether the species <SpeciesName>
is member of one of the COx species available in GENEX.

The function returns GxTrue if the SpeciesName is a COx species.
It returns GxFalse if it isn't.

-------------------------------------------------------------------------------
*/
GxBool GxIsSpeciesCOx
   (char * SpeciesName)
{
   GxBool is;

   is = GxFalse;

   /* Due to the fact that a Species struct can't be retrieved from the kernel
      the following dirty tric is used to determine whether the species is COx.
      If the kernel is extended it can be determined in an other way.
   */
   if (strcmp (SpeciesName, GX_SPECIES_COX_1) == 0)
   {
      is = GxTrue;
   }
   return (is);
}

/*
------------------------------------------------------------------------------
GxIsSpeciesDryGas - Check whether species is a dry gas species
------------------------------------------------------------------------------

GxIsSpeciesDryGas determines whether the species <SpeciesName>
is member of one of the dry gas species available in GENEX.

The function returns GxTrue if the SpeciesName is a dry gas species.
It returns GxFalse if it isn't.

-------------------------------------------------------------------------------
*/
GxBool GxIsSpeciesDryGas
   (char * SpeciesName)
{
   GxBool is;

   is = GxFalse;

   /* Due to the fact that a Species struct can't be retrieved from the kernel
      the following dirty tric is used to determine whether the species is Dry
      gas. If the kernel is extended it can be determined in an other way.
   */
   if (strcmp (SpeciesName, GX_SPECIES_DRY_GAS_1) == 0 ||
       strcmp (SpeciesName, GX_SPECIES_DRY_GAS_2) == 0)
   {
      is = GxTrue;
   }

   return (is);
}

/*
------------------------------------------------------------------------------
GxIsSpeciesHCGasNam - Check whether species is a hydrocarbon gas species
------------------------------------------------------------------------------

GxIsSpeciesHCGasNam determines whether the species <SpeciesName>
is member of one of the hydrocarbon gas species available in GENEX.

The function returns GxTrue if the SpeciesName is a hydrocarbon gas species.
It returns GxFalse if it isn't.

-------------------------------------------------------------------------------
*/
GxBool GxIsSpeciesHCGasNam
   (char * SpeciesName)
{
   GxBool is;

   /* Due to the fact that a Species struct can't be retrieved from the kernel
      the following dirty tric is used to determine whether the species is COx.
      If the kernel is extended it can be determined in an other way.
   */ 
   is = GxIsSpeciesDryGas (SpeciesName) || GxIsSpeciesWetGas (SpeciesName);

   return (is);
}

/*
------------------------------------------------------------------------------
GxIsSpeciesOilNam - Check whether species is an oil species
------------------------------------------------------------------------------

GxIsSpeciesOilNam determines whether the species <SpeciesName>
is member of one of the oil species available in GENEX.

The function returns GxTrue if the SpeciesName is an oil species.
It returns GxFalse if it isn't.

-------------------------------------------------------------------------------
*/
GxBool GxIsSpeciesOilNam
   (char * SpeciesName)
{
   GxBool is;

   is = GxIsSpeciesOilGas (SpeciesName);

   /* The species is an Oil/Gas species; check whether it is a gas */
   if   ((is)
      && (GxIsSpeciesHCGasNam (SpeciesName)))
   {
      /* The species is a gas species: therefore it isn't an oil species */
      is = GxFalse;
   }

   return (is);
}

/*
------------------------------------------------------------------------------
GxIsSpeciesWetGas - Check whether species is a wet gas species
------------------------------------------------------------------------------

GxIsSpeciesWetGas determines whether the species <SpeciesName>
is member of one of the wet gas species available in GENEX.

The function returns GxTrue if the SpeciesName is a wet gas species.
It returns GxFalse if it isn't.

-------------------------------------------------------------------------------
*/
GxBool GxIsSpeciesWetGas
   (char * SpeciesName)
{
   GxBool is;

   is = GxFalse;

   /* Due to the fact that a Species struct can't be retrieved from the kernel
      the following dirty tric is used to determine whether the species is Wet
      gas. If the kernel is extended it can be determined in an other way.
   */
   if (strcmp (SpeciesName, GX_SPECIES_WET_GAS_1) == 0||
       strcmp (SpeciesName, GX_SPECIES_WET_GAS_2) == 0||
       strcmp (SpeciesName, GX_SPECIES_WET_GAS_3) == 0||
       strcmp (SpeciesName, GX_SPECIES_WET_GAS_4) == 0)
   {
      is = GxTrue;
   }

   return (is);
}

/*
------------------------------------------------------------------------------
GxSetInit - Sets the boolean which indicated whether GENEX is initialised 
------------------------------------------------------------------------------

GxSetInit sets the local boolean to initialised if this is correct.

GxSetInit sets gxinitialised to GxTrue if it had GxFalse as value.
------------------------------------------------------------------------------
*/
GxErrorCode GxSetInit
   (void)
{
   GxErrorCode rc;

   rc = GXE_NOERROR;
   if (! GxIsInit ())
   {
      genexInitialised = GxTrue;
   }
   else
   {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   }
   return (rc);
}

/*
------------------------------------------------------------------------------
GxSpeciesInSpeciesGroup - Check whether species is member of group of species
------------------------------------------------------------------------------

GxSpeciesInSpeciesGroup determines whether the species <SpeciesName>
is member of the set of species identified by the <SpeciesGroup>.

The function returns GxTrue if the SpeciesName is a member of the group. It
returns GxFalse if it isn't.

Supported groups are:
GX_OIL_SPECIES     - Oil species
GX_HC_GAS_SPECIES  - HC Gas species
GX_DRY_GAS_SPECIES - C species
GX_WET_GAS_SPECIES - C2-C5 species
GX_COX_SPECIES     - COx species
-------------------------------------------------------------------------------
*/

static GxErrorCode GxSpeciesInSpeciesGroup
   (GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    GxBool *       SpeciesInGroup)
{
   GxErrorCode rc;
   GxBool      translated;

   rc = GXE_NOERROR;
   *SpeciesInGroup = GxFalse;
   translated = GxFalse;

   if (SpeciesGroup == GX_OIL_SPECIES)
   {
      *SpeciesInGroup = GxIsSpeciesOilNam (SpeciesName);
      translated = GxTrue;
/*      if(*SpeciesInGroup)printf(" Species is OIL : %s\n", SpeciesName); */
   }

   if (SpeciesGroup == GX_HC_GAS_SPECIES)
   {
      *SpeciesInGroup = GxIsSpeciesHCGasNam (SpeciesName);
      translated = GxTrue;
   }

   if (SpeciesGroup == GX_DRY_GAS_SPECIES)
   {
      *SpeciesInGroup = GxIsSpeciesDryGas (SpeciesName);
      translated = GxTrue;
   }

   if (SpeciesGroup == GX_WET_GAS_SPECIES)
   {
      *SpeciesInGroup = GxIsSpeciesWetGas (SpeciesName);
      translated = GxTrue;
   }

   if (SpeciesGroup == GX_COX_SPECIES)
   {
      *SpeciesInGroup = GxIsSpeciesCOx (SpeciesName);
      translated = GxTrue;
   }

   if (translated == GxFalse)
   {
      /* An unkown species group was specified */
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
      printf(" Species is UNKNOWN : %s\n", SpeciesName); 
   }

   return (rc);
}

