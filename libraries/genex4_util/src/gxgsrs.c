/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/gxgsrs.c,v 25.0 2005/07/05 08:03:55 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxgsrs.c,v $
 * Revision 25.0  2005/07/05 08:03:55  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:43  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:17  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:49  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:48  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:44:49  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:36  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:57  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:18  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:32  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:44  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:33  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:14  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.1  1997/09/18  15:15:21  kspre0
 * Modif used by XMIG to get more species.
 *
 * Revision 14.0  1997/09/16  18:11:44  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:04  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:10  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.2  1996/06/26  08:14:57  cs319
 * initialise.
 *
 * Revision 10.1  1996/06/18  12:00:55  ibs
 * Initial value set for speciesGroup.
 *
 * Revision 10.0  1996/03/11  12:42:42  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.5  1996/01/18  12:33:20  cs319
 * Initialised a string.
 *
 * Revision 9.4  1995/12/07  07:55:44  ibs
 * Added the rs6000/else ifdef structure.
 *
 * Revision 9.3  1995/11/21  10:06:37  ibs
 * Name change : cauldron_gxgsrs to gxgsrs
 *
 * Revision 9.2  1995/11/21  09:58:51  ibs
 * Fortran relations out.
 *
 * Revision 9.1  1995/11/20  13:08:31  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		P.J. Huysse   (BSO Rotterdam)        ( 3-Nov-1995)
Original Author:E. de Koster  (BSO/AT Zoetermeer bv) (24-SEP-1993)

=============================================================================
NOTE		This routine is analogue the Fortran / C routine written
		by E. de Koster.
		The Fortran specific items are removed etc.		
=============================================================================

Description:	Get the location specific species results calculated by GENEX
		The letters in the routine name GxGSRs stand for:
		- Genex                         Gx
		- Get                             G
		- Species                          S
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

		The species and speciesgroups can be extended by defining new
		cases in the 'switch (*OutputSpecies)' statement. The names of
		the generated output segments will require modification if
		individual species are to be retrieved.		

		The output segments for which data is retrieved from the GENEX
		kernel can be extended by adding appropriate Output..Values
		arrays and extending the OutputSegments array.
		Appropriate 'if (OutputSegments [OUTSEG_...] == FORTRAN_TRUE)'
		and retrieval functions must be added at the end of this
		routine.
History:
--------
24-SEP-1993	ErKo	initial version: determination of requested results
27-SEP-1993	ErKo	added retrieval of GENEX results
28-SEP-1993	ErKo	- added determination of time-values if ntimes == 0
			- added translation of GENEX results according to
			  requested time values
29-SEP-1993	ErKo	moved retrieval and translation of GENEX results to
			individual routines
30-SEP-1993	ErKo	- added arguments to the retrieval and translation
			  routines
			- added description of arguments and the way in which
			  new species(groups) and outputsegments can be added
			- moved retrieval of fullrange time values and the
			  determination of output time values to ibsutil.c
31-MAR-1994	ErKo	added translation of N2 species indicator to species
			name
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
gxgsrs - Get the location specific species results calculated by the GENEX
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
#ifdef RS6000
   int gxgsrs
#else
   int gxgsrs_
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
       int *   OutputSpecies,     /* I   Value : Species to get results for */
       int *   OutputSegments,    /* I   Array : Segments to get results for */
       float * OutputGCValues,    /*   O Array : Generated cumulative */
       float * OutputGRValues,    /*   O Array : Generated rate */
       float * OutputXCValues,    /*   O Array : Expelled cumulative */
       float * OutputXRValues,    /*   O Array : Expulsion rate */
       float * OutputUCValues,    /*   O Array : Expelled upward cumulative */
       float * OutputDCValues,    /*   O Array : Expelled downward cumulative */
       float * OutputURValues,    /*   O Array : Expulsion upward rate */
       float * OutputDRValues)    /*   O Array : Expulsion downward rate */
{
   GxErrorCode    rc;
   int            gxGSRsRc;
   GxSpeciesGroup speciesGroup=GX_OIL_SPECIES;
   char *         speciesName=NULL;
   GxBool         individualSpecies;
   double         startTimeFullRange;
   double         endTimeFullRange;
   int            nFullRangeValues;
   double *       fullRangeTimeValues;
   double *       fullRangeResults;

   fullRangeTimeValues = NULL;
   fullRangeResults = NULL;

/* ----- BEGIN PREPARATIONS ----- */
/* ----- Determine full range time range Values ----- */
   /* Get time range available in gxkernel */

   rc = IbsGetFullTimeRange (& startTimeFullRange,
                             & endTimeFullRange,
                             & nFullRangeValues,
                             & fullRangeTimeValues);

/* ----- Allocate local storage for full range results ----- */

   if (!rc) rc = GxCreateModelHistory (nFullRangeValues, & fullRangeResults);

/* ----- Determine output time range Values ----- */
   if (!rc) rc = IbsGetOutputTimeRange (NOutputTimeValues,
                                  *OutputStartTime,
                                  *OutputEndTime,
                                  *OutputDeltaTime,
                                  OutputTimeValues);

/* ----- Determine Species (group) to retrieve results of ----- */

/*   === NEW VERSION:   ===== */

/*
      if ( *OutputSpecies == GX_N2_SPECIESNAME )
      {
         individualSpecies = GxTrue;
      else
         individualSpecies = GxFalse;
      }
*/

      switch (*OutputSpecies)
      {
      /* ----- Species groups ----- */
      case OIL_SPECIESGROUP :
         speciesGroup = GX_OIL_SPECIES;
         individualSpecies = GxFalse;
         break;

      case HC_GAS_SPECIESGROUP :
         speciesGroup = GX_HC_GAS_SPECIES;
         individualSpecies = GxFalse;
         break;

      case DRY_GAS_SPECIESGROUP :
         speciesGroup = GX_DRY_GAS_SPECIES;
         individualSpecies = GxFalse;
         break;

      case WET_GAS_SPECIESGROUP :
         speciesGroup = GX_WET_GAS_SPECIES;
         individualSpecies = GxFalse;
         break;

      case COX_SPECIESGROUP :
         speciesGroup = GX_COX_SPECIES;
         individualSpecies = GxFalse;
         break;

      /* ----- Individual Species ----- */
      case N2_SPECIES :
         speciesName = GX_N2_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILAH_SPECIES :
         speciesName = GX_OILAH_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILAM_SPECIES :
         speciesName = GX_OILAM_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILAL_SPECIES :
         speciesName = GX_OILAL_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILPH_SPECIES :
         speciesName = GX_OILPH_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILPM_SPECIES :
         speciesName = GX_OILPM_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case OILPL_SPECIES :
         speciesName = GX_OILPL_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case C5_SPECIES :
         speciesName = GX_C5_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case C4_SPECIES :
         speciesName = GX_C4_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case C3_SPECIES :
         speciesName = GX_C3_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case C2_SPECIES :
         speciesName = GX_C2_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case C1_SPECIES :
         speciesName = GX_C1_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      case COX_SPECIES :
         speciesName = GX_COX_SPECIESNAME;
         individualSpecies = GxTrue;
         break;

      /* ----- other --> return error ----- */
      default :
         gxGSRsRc = IBSGXE_UNKNOWNSPECIESNUMBER;
         break;
      }
/* ----- END OF PREPARATIONS ----- */

/* ----- BEGIN RETRIEVING REQUESTED SPECIES(GROUP) RESULTS ----- */
   /* Cumulative generated: the .GC output segment */

      if (OutputSegments [OUTSEG_GEN_CUM_TOTAL])
      {
         rc = IbsGetGenMC (individualSpecies,
                                speciesGroup,
                                speciesName,
                                startTimeFullRange,
                                endTimeFullRange,
                                nFullRangeValues,
                                fullRangeTimeValues,
                                fullRangeResults,
                                *NOutputTimeValues,
                                OutputTimeValues,
                                OutputGCValues);
     }

    /* Generation rate: the .GR output segment */

      if (OutputSegments [OUTSEG_GEN_RATE])
      {
         rc = IbsGetGenR (individualSpecies,
                              speciesGroup,
                              speciesName,
                              startTimeFullRange,
                              endTimeFullRange,
                              nFullRangeValues,
                              fullRangeTimeValues,
                              fullRangeResults,
                              *NOutputTimeValues,
                              OutputTimeValues,
                              OutputGRValues);
      }

   /* Cumulative expelled: the .XC output segment */

      if (OutputSegments [OUTSEG_EXP_CUM_TOTAL])
      {
         rc = IbsGetExpMC (individualSpecies,
                                speciesGroup,
                                speciesName,
                                GX_UPANDDOWN,
                                startTimeFullRange,
                                endTimeFullRange,
                                nFullRangeValues,
                                fullRangeTimeValues,
                                fullRangeResults,
                                *NOutputTimeValues,
                                OutputTimeValues,
                                OutputXCValues);
     }

   /* Expulsion rate: the .XR output segment */

      if (OutputSegments [OUTSEG_EXP_RATE])
      {
         rc = IbsGetExpR (individualSpecies,
                              speciesGroup,
                              speciesName,
                              GX_UPANDDOWN,
                              startTimeFullRange,
                              endTimeFullRange,
                              nFullRangeValues,
                              fullRangeTimeValues,
                              fullRangeResults,
                              *NOutputTimeValues,
                              OutputTimeValues,
                              OutputXRValues);
      }

   /* Cumulative expelled upward: the .UC output segment */

      if (OutputSegments [OUTSEG_EXP_UP_CUM_TOTAL])
      {
         rc = IbsGetExpMC (individualSpecies,
                                speciesGroup,
                                speciesName,
                                GX_UP,
                                startTimeFullRange,
                                endTimeFullRange,
                                nFullRangeValues,
                                fullRangeTimeValues,
                                fullRangeResults,
                                *NOutputTimeValues,
                                OutputTimeValues,
                                OutputUCValues);
      }

   /* Cumulative expelled downward: the .DC output segment */

      if (OutputSegments [OUTSEG_EXP_DOWN_CUM_TOTAL])
      {
         rc = IbsGetExpMC (individualSpecies,
                                speciesGroup,
                                speciesName,
                                GX_DOWN,
                                startTimeFullRange,
                                endTimeFullRange,
                                nFullRangeValues,
                                fullRangeTimeValues,
                                fullRangeResults,
                                *NOutputTimeValues,
                                OutputTimeValues,
                                OutputDCValues);
      }

   /* Expulsion upward rate: the .UR output segment */

      if (OutputSegments [OUTSEG_EXP_UP_RATE])
      {
         rc = IbsGetExpR (individualSpecies,
                              speciesGroup,
                              speciesName,
                              GX_UP,
                              startTimeFullRange,
                              endTimeFullRange,
                              nFullRangeValues,
                              fullRangeTimeValues,
                              fullRangeResults,
                              *NOutputTimeValues,
                              OutputTimeValues,
                              OutputURValues);
      }

   /* Expulsion downward rate: the .DR output segment */

      if (OutputSegments [OUTSEG_EXP_DOWN_RATE] )
      {
         rc = IbsGetExpR (individualSpecies,
                              speciesGroup,
                              speciesName,
                              GX_DOWN,
                              startTimeFullRange,
                              endTimeFullRange,
                              nFullRangeValues,
                              fullRangeTimeValues,
                              fullRangeResults,
                              *NOutputTimeValues,
                              OutputTimeValues,
                              OutputDRValues);
      }

/* ----- END OF RETRIEVING REQUESTED SPECIES(GROUP) RESULTS ----- */

/* ----- BEGIN CLEAN UP ----- */
   if (fullRangeTimeValues != NULL)
   {
      rc = GxFreeModelHistory (fullRangeTimeValues);
   }
   if (fullRangeResults != NULL)
   {
      rc = GxFreeModelHistory (fullRangeResults);
   }
/* ----- END OF CLEANUP ----- */

   return (rc);
}
