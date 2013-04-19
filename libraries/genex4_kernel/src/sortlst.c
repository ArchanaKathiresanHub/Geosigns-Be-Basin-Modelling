/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/sortlst.c,v 25.0 2005/07/05 08:03:53 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: sortlst.c,v $
 * Revision 25.0  2005/07/05 08:03:53  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:05  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:39  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:16  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:48  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:44  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:34  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:53  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:14  ibs
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
 * Revision 14.0  1997/09/16  18:11:12  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:47  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:31  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:53  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:10  ibs
 * Add headers.
 * */
/* ================================================================== */
#include <stdio.h>

#include "error.h"
#include "sortlst.h"

#ifndef NULL
#define NULL    0
#endif

void *SearchInSortedList (SortedList list, void *searchKey)
{
   void  *data=NULL;
   int   result;

   if (!list->compare) {
      ErrorHandler ( NONFATAL,
            "SearchInSortedList : Can't search without compare routine !!!" );
   } else {
      data = GetFirstFromSortedList (list);
      if (data != 0) {
	 while (data && ((result = list->compare (searchKey, data)) > 0))
         data = GetNextFromSortedList (list);
         if (result != 0) data = NULL;
         } 
   }
   return (data);
}


void *InsertInSortedList (SortedList list, void *newData)
{
   void  *data;

   if (!list->compare) {
      ErrorHandler ( NONFATAL,
            "InsertInSortedList : Can't insert without compare routine !!!" );
   } else {
      data = SearchInSortedList (list, newData);
      data = InsertInList (list, newData);
   }
   return (data);
}



void RepositionInSortedList (SortedList list)
{
   void  *data;

   if (!list->compare) {
      ErrorHandler ( NONFATAL,
 "RepositionInSortedList : Can't reposition without compare routine !!!" );
   } else {
      data = UnlinkFromList (list);
      SearchInSortedList (list, data);
      LinkIntoList (list, data);
   }
}
