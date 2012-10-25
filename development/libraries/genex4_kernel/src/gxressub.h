/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxressub.h,v 25.0 2005/07/05 08:03:50 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxressub.h,v $
 * Revision 25.0  2005/07/05 08:03:50  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:53:33  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:13  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:45  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:35  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:45:36  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:28  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:45  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:00  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:21  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:20  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:22  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:58  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:49  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:20  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:53  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:21  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:18:06  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:54:31  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXRESSUB_H
#define   GXRESSUB_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxressub.h   include file for Genex reservoir subarea functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-MAR-1992

 Description: gxressub.h contains prototype, type and macro definitions for
              the reservoir subarea functions like create, free, and replace

 History
 -------
 19-MAR-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteReservoirSubArea added

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxprjdef.h"

/* Type definitions */
/* ---------------- */
typedef  struct ReservoirSubArea  ReservoirSubArea;
struct ReservoirSubArea {
   char    reservoir[GXL_HORIZONNAME+1];
   char    subarea[GXL_SUBAREANAME+1];
   double  topDepth, grossThickness, netToGross, netAvgPorosity,
           waterSalinity;
};

/* Function prototypes */
/* ------------------- */
GxErrorCode  GxCopyToAllReservoirs (const char *);
GxErrorCode  GxCreateReservoirSubArea (ReservoirSubArea **);
GxErrorCode  GxFreeReservoirSubArea (ReservoirSubArea *);
GxErrorCode  GxGetReservoirSubArea (const char *, const char *,
                                    ReservoirSubArea *);
GxErrorCode  GxReadReservoirSubArea (FILE *, GxReservoirSubArea *, 
					const char *);
GxErrorCode  GxReplaceReservoirSubArea (const ReservoirSubArea *);

void         GxWriteReservoirSubArea (FILE *, const GxReservoirSubArea *);


#ifdef __cplusplus
}
#endif

#endif

