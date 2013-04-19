/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsgetexpmc.c,v 25.0 2005/07/05 08:03:56 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: ibsgetexpmc.c,v $
 * Revision 25.0  2005/07/05 08:03:56  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:45  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:18  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:50  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:52  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:39  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:53:01  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:23  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:35  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:51  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.7  1998/12/02 10:57:34  ibs
 * c type commemt delimiters used...
 *
 * Revision 16.6  1998/12/02 10:54:31  ibs
 * location of //
 *
 * Revision 16.5  1998/12/02 10:48:10  ibs
 * cumulative values kept constant with negative COX expulsion rates...
 *
 * Revision 16.4  1998/12/01 16:24:39  ibs
 * ... eh brackets.
 *
 * Revision 16.3  1998/12/01 16:20:23  ibs
 * COX expelled cumulative set to zero now when negative.
 *
 * Revision 16.2  1998/11/25 15:24:26  ibs
 * log message keyword changed: keep rcs happy.
 *
 * Revision 16.1  1998/11/25 15:23:42  ibs
 * The expelled values MUST BE => 0.0, else set to zero....
 * This fix to prevent negative COX expulsion rates and cumalative values
 * which will cause problems in the migration part.
 * They are also nonsense ...
 *
 * Revision 16.0  1998/10/15 22:26:39  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:21  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:33  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:24  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:21  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:56  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:08:58  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		29-SEP-1993

Description:	Get the cumulative masses of the expelled species as
		calculated by GENEX

Usage notes:	

History:
--------
29-SEP-1993	ErKo	initial version: moved retrieval and translation
			of GENEX results to individual routines
 7-OCT-1993	ErKo	- use IbsGetVal to determine result
			- update storage location of results according to the
			  order of time values in the OutputTimeValues array
17-DEC-1993     ErKo    store result of Horizon Age  [i] at location [i]
                        in stead of at location [i -1]
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
IbsGetExpMC -	Get the cumulative masses of the expelled species
			as calculated by the GENEX
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
GxErrorCode IbsGetExpMC
   (GxBool         IndividualSpecies,
    GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    GxUpDown       UpAndOrDown,
    double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputResults)
{
   GxErrorCode rc;
   int         i;
   double      result;
   float       lastvalue;
   if (IndividualSpecies != GxTrue)
   {
      rc = GxGetGeologicalGroupExpulsions (IBS_GX_SUBAREA, 
                                           IBS_GX_SRHORIZON,
                                           IBS_GX_LITHOLOGY,
                                           SpeciesGroup,
                                           UpAndOrDown,
                                           GX_CUMULATIVE,
                                           StartTimeFullRange,
                                           EndTimeFullRange,
                                           FullRangeResults);
   }
   else
   {
      rc = GxGetGeologicalExpulsions (IBS_GX_SUBAREA, 
                                      IBS_GX_SRHORIZON,
                                      IBS_GX_LITHOLOGY,
                                      SpeciesName,
                                      UpAndOrDown,
                                      GX_CUMULATIVE,
                                      StartTimeFullRange,
                                      EndTimeFullRange,
                                      FullRangeResults);
   }
   /* Translate retrieved results to requested time intervals */
   if (rc == GXE_NOERROR)
   {
      OutputResults [0] = (float) 0.0;
      lastvalue = 0.0;
      for (i = 1; i < NOutputTimeValues; i++)
      {
         rc = IbsGetVal ((double) OutputTimeValues [i -1],
                         (double) OutputTimeValues [i],
                         NFullRangeValues,
                         FullRangeTimeValues,
                         FullRangeResults,
                         &result);
         if (rc == GXE_NOERROR)
         {
            if (! (result < lastvalue) ) 
	    {
   /*  prevent dipping cumulative results, because of negative COX rates */
	       OutputResults [i] = (float) result;
	       lastvalue = (float) result;
            }
            else
            {
	       OutputResults [i] = lastvalue;
	    }
         }
      }
   }

   return (rc);
}
