/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/list.c,v 25.0 2005/07/05 08:03:51 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: list.c,v $
 * Revision 25.0  2005/07/05 08:03:51  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:03  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:37  ibs
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
 * Revision 20.0  2000/08/15 09:52:51  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:10  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:26  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:34  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:28  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:06  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:06  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:24  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:26  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:47  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:54  ibs
 * Add headers.
 * */
/* ================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "list.h"

static ListElementPtr createElement (void);
static void deleteElement (ListElementPtr element);

static ListElementPtr createElement ()
{
    ListElementPtr tmpPtr;

    tmpPtr = ( ListElementPtr ) malloc (( size_t ) sizeof ( ListElement ));
    if ( tmpPtr )
    {
        tmpPtr->data = ( void * ) NULL;
        tmpPtr->next = ( ListElementPtr ) NULL;
    }
    else
    {
        ErrorHandler ( NONFATAL, "createElement : Can't allocate memory for LIST element !!!" );
    }
    return ( tmpPtr );
}


static void deleteElement (ListElementPtr element)
{
    free ( element );
}


List CreateList (size_t size, int (*compare) (const void *, const void *))
{
    List  tmpPtr;

    tmpPtr = ( List ) malloc ( (size_t) sizeof ( ListStruct ));

    if ( tmpPtr )
    {
        tmpPtr->elements = 0;
        tmpPtr->size     = size;
        tmpPtr->first    = ( ListElementPtr ) NULL;
        tmpPtr->current  = ( ListElementPtr ) NULL;
        tmpPtr->last     = ( ListElementPtr ) NULL;
        tmpPtr->compare  = compare;
    }
    else
    {
        ErrorHandler ( FATAL, "CreateList : Can't allocate memory !!!" );
    }
    return ( tmpPtr );
}


void *GetCurrentFromList (List list)
{
   return (list->current ? list->current->data : NULL);
}


void *GetFirstFromList (List list)
{
   list->current = list->first;
   return (list->current ? list->current->data : NULL);
}


void *GetNextFromList (List list)
{
   if (list->current) list->current = list->current->next;
   return (list->current ? list->current->data : NULL);
}


void *GetLastFromList (List list)
{
   list->current = list->last;
   return (list->current ? list->current->data : NULL);
}


void *InsertInList (List list, const void *data)
{
   void  *dataptr;

   dataptr = malloc (list->size);
   if (!dataptr) 
   {
      ErrorHandler ( NONFATAL, "InsertInList : Can't allocate memory for data !!!" );
   } 
   else 
   {
      memcpy (dataptr, data, list->size);
      LinkIntoList (list, dataptr);
   }
   return (dataptr);
}


void LinkIntoList (List list, const void *data)
{
   ListElementPtr  element, tmpPtr;

   element = createElement ();
   if (element) 
   {
      if (list->current == list->first) 
      {
         element->next = list->first;
         list->first   = element;
      } 
      else 
      {
         tmpPtr = list->first;
         while (tmpPtr->next != list->current)
            tmpPtr = tmpPtr->next;
         tmpPtr->next  = element;
         element->next = list->current;
      }
      if (!list->current) 
      {
          list->last = element;
      }
      element->data = (void *) data;
      list->current = element;
      list->elements++;
   }
}


void *AppendToList (List list, const void *data)
{
   ListElementPtr  element;
   void            *dataptr=NULL;

   element = createElement ();
   if (element) 
   {
      dataptr = malloc (list->size);
      if (!dataptr) 
      {
         free (element);
      } 
      else 
      {
         if (list->elements++ == 0) 
         {
            list->first   = element;
            list->current = element;
         } 
         else 
         {
            list->last->next = element;
         }
         list->last    = element;
         element->data = dataptr;
         memcpy (element->data, data, list->size);
      }
   }
   return (dataptr);
}


List CopyList (List destination, const List source)
{
   List  list=NULL;
   void  *data;

   if (destination->size == source->size) 
   {
      ClearList (destination);
      data = GetFirstFromList (source);
      while (data && AppendToList (destination, data))
      {
         data = GetNextFromList (source);
      }
      if (!data) list = destination;
   }
   return (list);
}



void *SearchInList (List list, const void *searchKey)
{
   void *data=NULL;

   if (!list->compare) {
      ErrorHandler (NONFATAL, "SearchInList : No compare routine available !!!" );
   } 
   else 
   {
      data = GetFirstFromList (list);
      while (data && (list->compare (searchKey, data) != 0))
      {
         data = GetNextFromList (list);
      }
   }
   return (data);
}





void DeleteList (List list)
{
    ListElementPtr  element,
                    tmpPtr;

    element = list->first;

    while ( element )
    {
        tmpPtr = element->next;
        free ( element->data );
        deleteElement ( element );
        element = tmpPtr;
    }
    free ( list );
}


void ClearList (List list)
{
    ListElementPtr  element,
                    tmpPtr;

    element = list->first;

    while ( element )
    {
        tmpPtr = element->next;
        free ( element->data );
        deleteElement ( element );
        element = tmpPtr;
    }
    list->elements = 0;
    list->first    = ( ListElementPtr ) NULL;
    list->current  = ( ListElementPtr ) NULL;
    list->last     = ( ListElementPtr ) NULL;
}


void RemoveFromList (List list)
{
    void  *data;

    if (!list->current) 
    {
       ErrorHandler ( NONFATAL, "RemoveFromList : No current element !!!" );
    } 
    else 
    {
       data = UnlinkFromList (list);
       free (data);
    }
}


void *UnlinkFromList (List list)
{
   ListElementPtr  element, tmpPtr;
   void            *data=NULL;

   if (list->current) 
   {
      element = list->current;
      if (element == list->first) 
      {
         list->first   = element->next;
         list->current = list->first;
         if (element == list->last) 
         {
             list->last = NULL;
         }
      } 
      else 
      {
         tmpPtr = list->first;
         while (tmpPtr->next != element)
         {
            tmpPtr = tmpPtr->next;
         }
         tmpPtr->next  = element->next;
         list->current = tmpPtr->next;
         if (element == list->last) 
         {
             list->last = tmpPtr;
         }
      }
      list->elements--;
      data = element->data;
      deleteElement (element);
   }
   return (data);
}



int IsListEmpty (const List list)
{
    return ( list->elements == 0 ? 1 : 0 );
}


long CountList (const List list)
{
    return ( list->elements );
}

