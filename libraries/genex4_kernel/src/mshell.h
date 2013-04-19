/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/mshell.h,v 25.0 2005/07/05 08:03:52 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: mshell.h,v $
 * Revision 25.0  2005/07/05 08:03:52  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:47:56  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:39  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:16  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:44  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:33  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:53  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:13  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:28  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:38  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:30  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:09  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:11  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:45  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:02  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:32  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:18:24  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:53:50  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   MSHELL_H
#define   MSHELL_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 mshell.h - include file for memory allcation debugging tool MShell
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie)

 Date:        25-NOV-1991

 Description: mshell.h defines a number of macros with which the usual
              memory allocation functions are replaced by MShell functions.
              In addition mshell.h defines two functions to inquire and
              display memory usage.

 History
 -------
 25-NOV-1991  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>

/* Function prototypes */
/* ------------------- */
/* -- memory allocation functions -- */
void    *MShellCalloc (size_t, size_t, const char *, int);
void    MShellFree (const void *, const char *, int);
void    *MShellMalloc (size_t, const char *, int);
void    *MShellRealloc (const void *, size_t, const char *, int);
char    *MShellStrdup (const char *, const char *, int);
/* -- memory usage inquiry functions -- */
void    MShellDisplay (FILE *);
size_t  MShellUsed (void);

/* Macro definitions */
/* ----------------- */
#define  calloc(n,s)   MShellCalloc ((n), (s), __FILE__, __LINE__)
#define  free(p)       MShellFree ((p), __FILE__, __LINE__)
#define  malloc(s)     MShellMalloc ((s), __FILE__, __LINE__)
#define  realloc(p,s)  MShellRealloc ((p), (s), __FILE__, __LINE__)
#define  strdup(s)     MShellStrdup ((s), __FILE__, __LINE__)


#ifdef __cplusplus
}
#endif

#endif

