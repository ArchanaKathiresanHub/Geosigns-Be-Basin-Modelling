/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/gxgpar.c,v 25.0 2005/07/05 08:03:55 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxgpar.c,v $
 * Revision 25.0  2005/07/05 08:03:55  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:43  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:17  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:48  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:47  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:44:47  ibs
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
 * Revision 18.0  1999/08/01 14:30:31  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:44  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:33  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:13  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:20  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:01  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:09  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:40  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:08:30  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		23-SEP-1993

Description:	Get the parameters from the GENEX datastructures

Usage notes:	

History:
--------
23-SEP-1993	ErKo	initial version
------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxoption.h"
#include  "ibsgxkernel.h"
#include  "ibsgx.h"

/* Prototypes of local functions */
/* ----------------------------- */

/* Local variables */
/* --------------- */

/*
------------------------------------------------------------------------------
gxgpar - Get GENEX parameters from the GENEX project and kernel
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
#ifdef RS6000
   int gxgpar
#else
   int gxgpar_
#endif
      (int * NSlices,
       int * NGxTimeSteps)
{
   GxErrorCode rc;
   int         gxGParRc;
   RunOptions  sRunOptions;

   rc = GXE_NOERROR;
   gxGParRc = IBSGXE_NOERROR;

   rc = GxGetRunOptions (& sRunOptions);
   if (rc != GXE_NOERROR)
   {
      gxGParRc = IBSGXE_GETDATAERROR;
   }

   if (gxGParRc == IBSGXE_NOERROR)
   {
      *NSlices      = sRUNOPTIONS_NSlices    (sRunOptions);
      *NGxTimeSteps = sRUNOPTIONS_NTimeSteps (sRunOptions);
   }

   return (gxGParRc);
}
