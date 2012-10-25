/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/gxggrs.c,v 25.0 2005/07/05 08:03:55 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxggrs.c,v $
 * Revision 25.0  2005/07/05 08:03:55  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:42  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:17  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:48  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:46  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:44:43  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:35  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:56  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:17  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:30  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:42  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:32  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:12  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:17  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:56  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:07  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.2  1996/06/18  12:59:50  ibs
 * Slipped.. fell...
 *
 * Revision 10.1  1996/06/18  12:54:20  ibs
 * Free memory.
 *
 * Revision 10.0  1996/03/11  12:42:37  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:08:27  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		30-SEP-1993

Description:	Get the location specific general results calculated by GENEX
		The letters in the routine name GxGSRs stand for:
		- Genex                         Gx
		- Get                             G
		- General                          G
		- Results                           Rs

Usage notes:	The time values for which results must be returned can be
		defined in two ways: They are described in ibsutil.c. Note that
		a 0 value of NOutputTimeValues is overwritten with the
		determined number of values.

		Note that the Output..Values arrays must have appropriate
		lengths in the calling software. If the time values are passed
		by the calling software the length of the Output..Values arrays
		must be equal to NOutputTimeValues. If the time values are
		specified using OutputStartTime, OutputEndTime and
		OutputDeltaTime the lengths of the Output..Values arrays must
		be equal to:

		                       OutputStartTime - OutputEndTime
		  NOutputTimeValues =  -------------------------------   + 1
		                                OutputDeltaTime

History:
--------
30-SEP-1993	ErKo	initial version: based on gxgsrs.c
 6-OCT-1993	ErKo	removed TR2 output segment (asphaltene conversion) and
			renamed TR1 output segment to TRG
20-DEC-1993     ErKo    updated the types of the arguments of
                        GXGetMainExpulsion: GxExpulsion sOil ->
                        GxExpulsion * Oil
------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxlithol.h"
#include  "gxsbarea.h"
#include  "gxkernel.h"
#include  "ibsgxkernel.h"
#include  "ibsgx.h"
#include  "ibsutil.h"

/* Prototypes of local functions */
/* ----------------------------- */

/* Local variables */
/* --------------- */

/*
------------------------------------------------------------------------------
gxggrs - Get the location specific genreal results calculated by the GENEX
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
#ifdef RS6000
   int gxggrs
#else
   int gxggrs_
#endif
      (int *   NOutputTimeValues, /* I/O Value :
                                     If NOutputTimeValues equals 0:
                                     delta time is used
                                   */
       float * OutputTimeValues,  /* I/O Array :
                                     Should be large enough to hold:
                                     - <NOutputTimeValues>
                                       if <NOutputTimeValues> != 0
                                     OR
                                     - (StartTime - EndTime) / DeltaTime values
                                       if <NOutputTimeValues> == 0
                                   */
       float * OutputStartTime,   /* I   Value */
       float * OutputEndTime,     /* I   Value */
       float * OutputDeltaTime,   /* I   Value */
       float * OutputXGOValue,    /*   O Value : Expulsion Gas-Oil ratio */
       float * OutputXCGValue,    /*   O Value : Expulsion Condesate-Gas ratio */
       float * OutputTRGValues,   /*   O Array : Kerogen conversion ratio's */
       float * OutputAPIValues,   /*   O Array : Expulsion API's */
       float * OutputVREValues)   /*   O Array : Maturity %'s */
{
   GxErrorCode    rc;
   int            gxGGRsRc;
   double         startTimeFullRange;
   double         endTimeFullRange;
   int            nFullRangeValues;
   double *       fullRangeTimeValues;
   double *       fullRangeResults1;
   double *       fullRangeResults2;
   GxExpulsion *  oilExpulsion;
   GxExpulsion *  gasExpulsion;
   GxExpulsion *  nonHCExpulsion;
   double         localGORSm3Sm3; /* expelled Gas-Oil ratio [Sm3/Sm3] */
   double         localGORScfSTB; /* expelled Gas-Oil ratio [Scf/STB] */
   double         localCGR;       /* Condensate-Gas ratio */

   rc = GXE_NOERROR;
   gxGGRsRc = IBSGXE_NOERROR;
   fullRangeTimeValues = NULL;
   fullRangeResults1 = NULL;
   fullRangeResults2 = NULL;

/* ----- BEGIN PREPARATIONS ----- */
/* ----- Determine full range time range Values ----- */
   /* Get time range available in gxkernel */
   rc = IbsGetFullTimeRange (& startTimeFullRange,
                             & endTimeFullRange,
                             & nFullRangeValues,
                             & fullRangeTimeValues);
   if (rc != GXE_NOERROR)
   {
      gxGGRsRc = IBSGXE_GETDATAERROR;
   }

/* ----- Allocate local storage for full range results ----- */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = GxCreateModelHistory (nFullRangeValues, & fullRangeResults1);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_MEMORYALLOCERROR;
      }
   }
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = GxCreateModelHistory (nFullRangeValues, & fullRangeResults2);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_MEMORYALLOCERROR;
      }
   }

/* ----- Determine output time range Values ----- */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = IbsGetOutputTimeRange (NOutputTimeValues,
                                  *OutputStartTime,
                                  *OutputEndTime,
                                  *OutputDeltaTime,
                                  OutputTimeValues);
      if (rc != GXE_NOERROR)
      {
          gxGGRsRc = IBSGXE_GETDATAERROR;
      }
   }
/* ----- END OF PREPARATIONS ----- */

/* ----- BEGIN RETRIEVING GENERAL RESULTS ----- */
   /* Expulsion GOR and Condensate-gas ratio: the XGO and XCG output segments */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = GxGetMainExpulsion (OutputTimeValues [0],
                               & oilExpulsion,
                               & gasExpulsion,
                               & nonHCExpulsion,
                               & localGORSm3Sm3,
                               & localGORScfSTB,
                               & localCGR);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_GETDATAERROR;
      }
      else
      {
         *OutputXGOValue = (float) localGORSm3Sm3;
         *OutputXCGValue = (float) localCGR;
      }

/* ---- free the memory used ------ */
      rc = GxFreeExpulsion (oilExpulsion,
                            gasExpulsion,
                            nonHCExpulsion);
   }

   /* Geological Conversion: the TRG output segment */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = IbsGetCnvrs (startTimeFullRange,
                        endTimeFullRange,
                        nFullRangeValues,
                        fullRangeTimeValues,
                        fullRangeResults1,
                        fullRangeResults2,
                        *NOutputTimeValues,
                        OutputTimeValues,
                        OutputTRGValues);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_GETDATAERROR;
      }
   }

   /* Expulsion API: the API output segment */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = IbsGetExpAPI (startTimeFullRange,
                         endTimeFullRange,
                         nFullRangeValues,
                         fullRangeTimeValues,
                         fullRangeResults1,
                         *NOutputTimeValues,
                         OutputTimeValues,
                         OutputAPIValues);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_GETDATAERROR;
      }
   }

   /* Geological VRE: the VRE output segment */
   if (gxGGRsRc == IBSGXE_NOERROR)
   {
      rc = IbsGetVRE (startTimeFullRange,
                      endTimeFullRange,
                      nFullRangeValues,
                      fullRangeTimeValues,
                      fullRangeResults1,
                      *NOutputTimeValues,
                      OutputTimeValues,
                      OutputVREValues);
      if (rc != GXE_NOERROR)
      {
         gxGGRsRc = IBSGXE_GETDATAERROR;
      }
   }
/* ----- END OF RETRIEVING GENERAL RESULTS ----- */

/* ----- BEGIN CLEAN UP ----- */
   if (fullRangeTimeValues != NULL)
   {
      rc = GxFreeModelHistory (fullRangeTimeValues);
   }
   if (fullRangeResults1 != NULL)
   {
      rc = GxFreeModelHistory (fullRangeResults1);
   }
   if (fullRangeResults2 != NULL)
   {
      rc = GxFreeModelHistory (fullRangeResults2);
   }
/* ----- END OF CLEANUP ----- */

   return (gxGGRsRc);
}
