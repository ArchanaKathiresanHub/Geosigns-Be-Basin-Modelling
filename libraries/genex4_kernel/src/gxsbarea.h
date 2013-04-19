/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxsbarea.h,v 25.0 2005/07/05 08:03:50 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxsbarea.h,v $
 * Revision 25.0  2005/07/05 08:03:50  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:53:34  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:14  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:46  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:37  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:46:17  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:29  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:47  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:04  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:22  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:24  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:24  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:01  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:55  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:34  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:56  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:24  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:18:11  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:53:06  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXSBAREA_H
#define   GXSBAREA_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxsbarea.h   include file for Genex subarea functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxsbarea.h contains prototype, type and macro definitions for
              the subarea functions like create, free, insert and remove

 History
 -------
 19-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteSubArea added
 28-MAR-1993  M. van Houtert GxReadSubArea added
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxprjdef.h"
#include  "sortlst.h"

/* Macro definitions */
/* ----------------- */
/* -- error definitions -- */
enum {
  GXE_DUPLICATESUBAREA = GXBE_SBAREA | GXF_APPLICATION | GXF_ERROR   | 1,
  GXE_LOCATIONLINKED, GXE_SUBAREAREFERRED
};

/* Type definitions */
/* ---------------- */
typedef  struct SubArea  SubArea;
struct SubArea {
   char    name[GXL_SUBAREANAME+1];
   char    locationId[GXL_LOCATIONID+1];
   double  easting, northing, area;
};

/* Function prototypes */
/* ------------------- */
GxErrorCode  GxCreateSubArea (SubArea **);
GxErrorCode  GxDeleteSubArea (const char *);
GxErrorCode  GxFreeSubArea (SubArea *);
GxErrorCode  GxGetSubArea (const char *, SubArea *);
GxErrorCode  GxGetSubAreaNameList (SortedList *);
GxErrorCode  GxInsertSubArea (const SubArea *);
GxErrorCode  GxReadSubArea (FILE *, GxSubArea *, const char *);
GxErrorCode  GxRenameSubArea (const char *, const char *);
GxErrorCode  GxReplaceSubArea (const SubArea *);

void         GxWriteSubArea (FILE *, const GxSubArea *);


#ifdef __cplusplus
}
#endif

#endif

