/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/list.h,v 25.0 2005/07/05 08:03:51 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: list.h,v $
 * Revision 25.0  2005/07/05 08:03:51  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:47:56  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:38  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:15  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:42  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:32  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:52  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:11  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:27  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:35  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:28  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:07  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:07  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:30  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:00  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:29  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.4  1996/01/04  15:18:22  ibs
 * cplusplus corrections added.
 *
 * Revision 9.3  1995/11/20  14:23:44  ibs
 * list define changed to genex_list define.
 *
 * Revision 9.2  1995/10/30  09:53:09  ibs
 * Add headers.
 * */
/* ================================================================== */

/************************************************************************/
/*                           */
/*   Author      : M v Houtert         Date : 9 Sept 1991   */
/*                           */
/*   File        : list.h                  */
/*                           */
/*   Version     : 1.0                     */
/*                           */
/*   Description :                     */
/*                           */
/*   Linked list definitions !                  */
/*                           */
/************************************************************************/


#ifndef GENEX_LIST

#define GENEX_LIST

#ifdef __cplusplus
extern "C" {
#endif
    

#include <stddef.h>

typedef struct ListElement *ListElementPtr;

typedef struct ListElement 
{
    void             *data;
    ListElementPtr   next;
} ListElement;

typedef struct ListStruct *List;

typedef struct ListStruct {
   long            elements;
   size_t          size;
   ListElementPtr  first, current, last;
   int             (*compare) (const void *, const void *);
} ListStruct;


List  CreateList (size_t, int (*) (const void *, const void *));

void  *InsertInList (List, const void *);
void  *AppendToList (List, const void *);
void  LinkIntoList (List, const void *);
List  CopyList (List, const List);
void  DeleteList (List);
void  ClearList (List);
void  RemoveFromList (List);
void  *UnlinkFromList (List);
void  DumpList (const List);

void  *GetCurrentFromList (List);
void  *GetFirstFromList (List);
void  *GetLastFromList (List);
void  *GetNextFromList (List);

void  *SearchInList (List, const void *);

int   IsListEmpty (const List);
long  CountList (const List);


#ifdef __cplusplus
}
#endif

#endif

