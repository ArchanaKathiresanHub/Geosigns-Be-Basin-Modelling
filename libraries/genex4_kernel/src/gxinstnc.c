/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxinstnc.c,v 25.0 2005/07/05 08:03:45 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxinstnc.c,v $
 * Revision 25.0  2005/07/05 08:03:45  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:55  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:26  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:09  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:41  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:20  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:19  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:31  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:39  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:08  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:55  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:09  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:36  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:09  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:58  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:55  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:08  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:26  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxInstnc.c   Genex project instance functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxInstnc.c contains the Genex functions for handling project
              instances. The following functions are available:

                GxClearInstance       - clear an instance of the project
                GxCopyInstance        - copy an instance of a project
                GxFreeInstance        - free an instance of the project
                GxLinkNumbers         - determine number data units links
                GxLinkPointers        - determine pointer data units links
                GxReadInstance        - read a project instance from file
                GxReadInstanceHeader  - read an instance header from file
                GxSwapInstance        - swaps two instances of a project
                GxWriteInstance       - write a project instance to file
                GxWriteInstanceHeader - write the instance header to file

 Usage notes: - In order to use the functions in this source file the header
                file gxinstnc.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 22-MAR-1993  M. van Houtert File trailer removed
 04-DEC-1993  Th. Berkers    Extra checks added when linking pointers within
                             the project data in function GxLinkPointers
 20-MAR-1995  Th. Berkers    Integration of Isotopes Fractionation:
                             Genex version in project file checked

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdlib.h>
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxfile.h"
#include  "gxlabexp.h"
#include  "gxlocat.h"
#include  "gxinstnc.h"
#include  "gxprjdef.h"
#include  "gxtrap.h"
#include  "gxutils.h"

/* -- error text definitions -- */
#define  GXT_INVPROJECT       "Inconsistency found in project file.\n" \
                              "Unable to use the project file.\n"


/*
--------------------------------------------------------------------------------
 GxClearInstance - clear an instance of the project
--------------------------------------------------------------------------------

 GxClearInstance cleares an instance of the project by initializing the
 memory for the instance.

 Arguments
 ---------
 instance - instance to be cleared

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxClearInstance (SortedList *instance)
{
   GxErrorCode       rc=GXE_NOERROR, rc2;
   int               i;
   GxHorizonHistory  *history;
   GxLabExperiment   *labexperiment;
   GxLocation        *location;
   GxTrap            *trap;

   assert (instance);

/* Delete the history data in each of the horizon histories */
   history = GetFirstFromSortedList (instance[GX_HORIZONHISTORY]);
   while (history) {
      rc2 = GxDestroyHorizonHistoryData (history);
      if (!rc) rc = rc2;
         history = GetNextFromSortedList (instance[GX_HORIZONHISTORY]);
   }

/* Delete the history data in each of the laboratory experiments */
   labexperiment = GetFirstFromSortedList (instance[GX_LABEXPERIMENT]);
   while (labexperiment) {
      rc2 = GxDestroyLabExperimentData (labexperiment);
      if (!rc) rc = rc2;
      labexperiment = GetNextFromSortedList (instance[GX_LABEXPERIMENT]);
   }

/* Delete the history data in each of the locations */
   location = GetFirstFromSortedList (instance[GX_LOCATION]);
   while (location) {
      rc2 = GxDestroyLocationData (location);
      if (!rc) rc = rc2;
      location = GetNextFromSortedList (instance[GX_LOCATION]);
   }

/* Delete the depth and history data in each of the traps */
   trap = GetFirstFromSortedList (instance[GX_TRAP]);
   while (trap) {
      rc2 = GxDestroyTrapData (trap);
      if (!rc) rc = rc2;
      trap = GetNextFromSortedList (instance[GX_TRAP]);
   }

/* Delete all the data units themselves */
   for (i=0; i<GXN_DATAUNITS; i++)
      if (instance[i]) ClearSortedList (instance[i]);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyInstance - copy an instance of a project to another instance
--------------------------------------------------------------------------------

 GxCopyInstance copies one complete project instance to another instance. The
 target instance is cleared first.

 Arguments
 ---------
 destination - destination instance
 source      - instance to be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyInstance (SortedList *destination, const SortedList *source)
{
   GxErrorCode       rc=GXE_NOERROR;
   int               i;
   GxHorizonHistory  *sourcehist, *desthist;
   GxLabExperiment   *sourcelabexp, *destlabexp;
   GxLocation        *sourceloc, *destloc;
   GxTrap            *sourcetrap, *desttrap;

   assert (source);
   assert (destination);

/* Copy all the data units themselves */
   for (i=0; (!rc)&&(i<GXN_DATAUNITS); i++)
      if (!CopyList (destination[i], source[i]))
         rc = GxError (GXE_LIST, GXT_LIST);
   if (!rc) {

/* -- Copy the history data for each of the horizon histories */
      sourcehist = GetFirstFromSortedList (source[GX_HORIZONHISTORY]);
      desthist = GetFirstFromSortedList (destination[GX_HORIZONHISTORY]);
      while (!rc && sourcehist) {
         if (!desthist) rc = GxError (GXE_LIST, GXT_LIST);
         if (!rc) rc = GxCopyHorizonHistoryData (desthist, sourcehist);
         if (!rc) sourcehist =
            GetNextFromSortedList (source[GX_HORIZONHISTORY]);
         if (!rc) desthist =
            GetNextFromSortedList (destination[GX_HORIZONHISTORY]);
      }
      if (!rc && desthist) rc = GxError (GXE_LIST, GXT_LIST);
   }
   if (!rc) {

/* -- Copy the history data for each of the laboratory experiments */
      sourcelabexp = GetFirstFromSortedList (source[GX_LABEXPERIMENT]);
      destlabexp = GetFirstFromSortedList (destination[GX_LABEXPERIMENT]);
      while (!rc && sourcelabexp) {
         if (!destlabexp) rc = GxError (GXE_LIST, GXT_LIST);
         if (!rc) rc = GxCopyLabExperimentData (destlabexp, sourcelabexp);
         if (!rc) sourcelabexp =
            GetNextFromSortedList (source[GX_LABEXPERIMENT]);
         if (!rc) destlabexp =
            GetNextFromSortedList (destination[GX_LABEXPERIMENT]);
      }
      if (!rc && destlabexp) rc = GxError (GXE_LIST, GXT_LIST);
   }
   if (!rc) {

/* -- Copy the history data for each of the locations */
      sourceloc = GetFirstFromSortedList (source[GX_LOCATION]);
      destloc = GetFirstFromSortedList (destination[GX_LOCATION]);
      while (!rc && sourceloc) {
         if (!destloc) rc = GxError (GXE_LIST, GXT_LIST);
         if (!rc) rc = GxCopyLocationData (destloc, sourceloc);
         if (!rc) sourceloc = GetNextFromSortedList (source[GX_LOCATION]);
         if (!rc) destloc = GetNextFromSortedList (destination[GX_LOCATION]);
      }
      if (!rc && destloc) rc = GxError (GXE_LIST, GXT_LIST);
   }
   if (!rc) {

/* -- Copy the depth and history data for each of the traps */
      sourcetrap = GetFirstFromSortedList (source[GX_TRAP]);
      desttrap = GetFirstFromSortedList (destination[GX_TRAP]);
      while (!rc && sourcetrap) {
         if (!desttrap) rc = GxError (GXE_LIST, GXT_LIST);
         if (!rc) rc = GxCopyTrapData (desttrap, sourcetrap);
         if (!rc) sourcetrap = GetNextFromSortedList (source[GX_TRAP]);
         if (!rc) desttrap = GetNextFromSortedList (destination[GX_TRAP]);
      }
      if (!rc && desttrap) rc = GxError (GXE_LIST, GXT_LIST);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeInstance - free an instance of the project
--------------------------------------------------------------------------------

 GxFreeInstance frees an instance of the project by deallocating the
 memory for the instance.

 Arguments
 ---------
 instance - instance to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeInstance (SortedList *instance)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (instance);

/* Remove all the elements in the sorted lists */
   rc = GxClearInstance (instance);
   if (!rc) {

/* -- Delete the sorted lists themselves */
      for (i=0; i<GXN_DATAUNITS; i++) {
         DeleteSortedList (instance[i]);
         instance[i] = NULL;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxLinkNumbers - determine links between data units in terms of numbers
--------------------------------------------------------------------------------

 GxLinkNumbers determines the links from one data unit to another in terms
 of sequence numbers in the lists from the pointer links.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxLinkNumbers (SortedList *instance)
{
   GxErrorCode         rc=GXE_NOERROR;
   int                 i, seqnr;
   GxDataItem          *dataitem;
   GxFile              *file;
   GxHorizonHistory    *history;
   GxLocation          *location;
   GxReservoirSubArea  *resarea;
   GxRock              *rock;
   GxSubArea           *subarea;
   GxTrap              *trap;

   assert (instance);

/* Determine the sequence number in the data unit list for each data unit */
   for (i=0; i<GXN_DATAUNITS; i++) {
      seqnr = 0;
      dataitem = (GxDataItem *) GetFirstFromSortedList (instance[i]);
      while (dataitem) {
         dataitem->seqnr = seqnr++;
         dataitem = (GxDataItem *) GetNextFromSortedList (instance[i]);
      }
   }

/* Loop for all raw data files in the project */
   file = GetFirstFromSortedList (instance[GX_FILE]);
   while (file) {

/* -- Reference from raw data file to location (-1 if none) */
      file->locationnr = file->location ?
                         ((GxDataItem *) file->location)->seqnr : -1;
      file = GetNextFromSortedList (instance[GX_FILE]);
   }

/* Loop for all horizon histories in the project */
   history = GetFirstFromSortedList (instance[GX_HORIZONHISTORY]);
   while (history) {

/* -- Reference from horizon history to source rock horizon (-1 if none) */
      history->horizonnr = history->horizon ?
                           ((GxDataItem *) history->horizon)->seqnr : -1;

/* -- Reference from horizon history to subarea (-1 if none) */
      history->subareanr = history->subarea ?
                           ((GxDataItem *) history->subarea)->seqnr : -1;
      history = GetNextFromSortedList (instance[GX_HORIZONHISTORY]);
   }

/* Loop for all locations in the project */
   location = GetFirstFromSortedList (instance[GX_LOCATION]);
   while (location) {

/* -- Reference from location to raw data file (-1 if none) */
      location->filenr    = location->file ?
                            ((GxDataItem *) location->file)->seqnr : -1;

/* -- Reference from location to subarea (-1 if none) */
      location->subareanr = location->subarea ?
                            ((GxDataItem *) location->subarea)->seqnr : -1;
      location = GetNextFromSortedList (instance[GX_LOCATION]);
   }

/* Loop for all reservoir subareas in the project */
   resarea = GetFirstFromSortedList (instance[GX_RESERVOIRSUBAREA]);
   while (resarea) {

/* -- Reference from reservoir subarea to reservoir horizon (-1 if none) */
      resarea->horizonnr = resarea->horizon ?
                           ((GxDataItem *) resarea->horizon)->seqnr : -1;

/* -- Reference from reservoir subarea to subarea (-1 if none) */
      resarea->subareanr = resarea->subarea ?
                           ((GxDataItem *) resarea->subarea)->seqnr : -1;
      resarea = GetNextFromSortedList (instance[GX_RESERVOIRSUBAREA]);
   }

/* Loop for all rocks in the project */
   rock = GetFirstFromSortedList (instance[GX_ROCK]);
   while (rock) {

/* -- Reference from rock to source rock horizon (-1 if none) */
      rock->horizonnr   = rock->horizon ?
                          ((GxDataItem *) rock->horizon)->seqnr : -1;

/* -- Reference from rock to lithology (-1 if none) */
      rock->lithologynr = rock->lithology ?
                          ((GxDataItem *) rock->lithology)->seqnr : -1;

/* -- Reference from rock to subarea (-1 if none) */
      rock->subareanr   = rock->subarea ?
                          ((GxDataItem *) rock->subarea)->seqnr : -1;
      rock = GetNextFromSortedList (instance[GX_ROCK]);
   }

/* Loop for all subareas in the project */
   subarea = GetFirstFromSortedList (instance[GX_SUBAREA]);
   while (subarea) {

/* -- Reference from subarea to location (-1 if none) */
      subarea->locationnr = subarea->location ?
                            ((GxDataItem *) subarea->location)->seqnr : -1;
      subarea = GetNextFromSortedList (instance[GX_SUBAREA]);
   }

/* Loop for all traps in the project */
   trap = GetFirstFromSortedList (instance[GX_TRAP]);
   while (trap) {

/* -- Reference from trap to reservoir horizon (-1 if none) */
      trap->horizonnr = trap->horizon ?
                        ((GxDataItem *) trap->horizon)->seqnr : -1;

/* -- Reference from trap to subarea (-1 if none) */
      trap->subareanr = trap->subarea ?
                        ((GxDataItem *) trap->subarea)->seqnr : -1;
      trap = GetNextFromSortedList (instance[GX_TRAP]);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxLinkPointers - determine links between data units in terms of pointers
--------------------------------------------------------------------------------

 GxLinkPointers determines the links from one data unit to another in terms
 of struct pointers from the sequence numbers of these structs in the lists.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxLinkPointers (SortedList *instance)
{
   GxErrorCode         rc=GXE_NOERROR;
   int                 i, j, nitems, maxitems;
   GxDataItem          *dataitem, ***itemptr;
   GxFile              *file;
   GxHorizonHistory    *history;
   GxLocation          *location;
   GxReservoirSubArea  *resarea;
   GxRock              *rock;
   GxSubArea           *subarea;
   GxTrap              *trap;

   assert (instance);

/* Determine the maximum of the sizes of the data unit lists */
   maxitems = 0;
   for (i=0; i<GXN_DATAUNITS; i++) {
      nitems   = (int) CountSortedList (instance[i]);
      maxitems = MAX (maxitems, nitems);
   }

/* Allocate temporary array for translation of sequence numbers to pointers */
   if (maxitems > 0)
      rc = GxAllocArray (&itemptr, sizeof (GxDataItem *), 2,
                         GXN_DATAUNITS, maxitems);
   
   if (!rc && (maxitems > 0)) {

/* -- Fill complete array with NULL pointers */
      for (i=0; i<GXN_DATAUNITS; i++) {
          for (j=0; j<maxitems; j++) {
              itemptr[i][j] = NULL;
          }
      }

/* -- Fill in the pointers in the array */
      for (i=0; i<GXN_DATAUNITS; i++) {
         dataitem = (GxDataItem *) GetFirstFromSortedList (instance[i]);
         while (dataitem) {
            itemptr[i][dataitem->seqnr] = dataitem;
            dataitem = (GxDataItem *) GetNextFromSortedList (instance[i]);
         }
      }

/* -- Translate sequence numbers in the raw data files to pointer links */
      file = GetFirstFromSortedList (instance[GX_FILE]);
      while (file && !rc) {
          if (file->locationnr >= maxitems) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from raw data file to location */
              file->location = (file->locationnr < 0) ? NULL :
                  (GxLocation *) itemptr[GX_LOCATION][file->locationnr];
              file = GetNextFromSortedList (instance[GX_FILE]);
          }
      }

/* -- Translate sequence numbers in the horizon histories to pointer links */
      history = GetFirstFromSortedList (instance[GX_HORIZONHISTORY]);
      while (history && !rc) {
          if ((history->horizonnr >= maxitems) ||
              (history->subareanr >= maxitems)) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from horizon history to source rock horizon */
              history->horizon = (history->horizonnr < 0) ? NULL :
                 (GxHorizon *) itemptr[GX_HORIZON][history->horizonnr];

/* ---------- Reference from horizon history to subarea */
              history->subarea = (history->subareanr < 0) ? NULL :
                 (GxSubArea *) itemptr[GX_SUBAREA][history->subareanr];
              history = GetNextFromSortedList (instance[GX_HORIZONHISTORY]);
          }
      }

/* -- Translate sequence numbers in the locations to pointer links */
      location = GetFirstFromSortedList (instance[GX_LOCATION]);
      while (location && !rc) {
          if ((location->filenr >= maxitems) ||
              (location->subareanr >= maxitems)) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from location to raw data file */
              location->file = (location->filenr < 0) ? NULL :
                               (GxFile *) itemptr[GX_FILE][location->filenr];

/* ---------- Reference from location to subarea */
              location->subarea = (location->subareanr < 0) ? NULL :
                    (GxSubArea *) itemptr[GX_SUBAREA][location->subareanr];
              location = GetNextFromSortedList (instance[GX_LOCATION]);
          }
      }

/* -- Translate sequence numbers in the reservoir subareas to pointer links */
      resarea = GetFirstFromSortedList (instance[GX_RESERVOIRSUBAREA]);
      while (resarea && !rc) {
          if ((resarea->horizonnr >= maxitems) ||
              (resarea->subareanr >= maxitems)) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from reservoir subarea to reservoir horizon */
              resarea->horizon = (resarea->horizonnr < 0) ? NULL :
                 (GxHorizon *) itemptr[GX_HORIZON][resarea->horizonnr];

/* ---------- Reference from reservoir subarea to subarea */
              resarea->subarea = (resarea->subareanr < 0) ? NULL :
                 (GxSubArea *) itemptr[GX_SUBAREA][resarea->subareanr];
              resarea = GetNextFromSortedList (instance[GX_RESERVOIRSUBAREA]);
          }
      }

/* -- Translate sequence numbers in the rocks to pointer links */
      rock = GetFirstFromSortedList (instance[GX_ROCK]);
      while (rock && !rc) {
          if ((rock->horizonnr >= maxitems) ||
              (rock->lithologynr >= maxitems) ||  
              (rock->subareanr >= maxitems)) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from rock to source rock horizon */
              rock->horizon = (rock->horizonnr < 0) ? NULL :
                 (GxHorizon *) itemptr[GX_HORIZON][rock->horizonnr];

/* ---------- Reference from rock to lithology */
              rock->lithology = (rock->lithologynr < 0) ? NULL :
                 (GxLithology *) itemptr[GX_LITHOLOGY][rock->lithologynr];

/* ---------- Reference from rock to subarea */
              rock->subarea = (rock->subareanr < 0) ? NULL :
                 (GxSubArea *) itemptr[GX_SUBAREA][rock->subareanr];
              rock = GetNextFromSortedList (instance[GX_ROCK]);
          }
      }

/* -- Translate sequence numbers in the subareas to pointer links */
      subarea = GetFirstFromSortedList (instance[GX_SUBAREA]);
      while (subarea && !rc) {
          if (subarea->locationnr >= maxitems) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from subarea to location */
              subarea->location = (subarea->locationnr < 0) ? NULL :
                 (GxLocation *) itemptr[GX_LOCATION][subarea->locationnr];
              subarea = GetNextFromSortedList (instance[GX_SUBAREA]);
          }
      }

/* -- Translate sequence numbers in the traps to pointer links */
      trap = GetFirstFromSortedList (instance[GX_TRAP]);
      while (trap && !rc) {
          if ((trap->horizonnr >= maxitems) ||
              (trap->subareanr >= maxitems)) { 
              rc = GxError (GXE_INVPROJECT, GXT_INVPROJECT);
          } else { 

/* ---------- Reference from trap to reservoir horizon */
              trap->horizon = (trap->horizonnr < 0) ? NULL :
                  (GxHorizon *) itemptr[GX_HORIZON][trap->horizonnr];

/* ---------- Reference from trap to subarea */
              trap->subarea = (trap->subareanr < 0) ? NULL :
                  (GxSubArea *) itemptr[GX_SUBAREA][trap->subareanr];
              trap = GetNextFromSortedList (instance[GX_TRAP]);
          }
      }

/* -- Free the temporary pointer array */
      GxFreeArray (itemptr, 2, GXN_DATAUNITS, maxitems);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadInstance - read a project instance from an open project file
--------------------------------------------------------------------------------

 GxReadInstance reads a project instance from an already open project file
 into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 filename   - the name of the file
 unitsize   - the sizes for each of the data units
 instance   - pointer to the instance to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadInstance (FILE *filehandle, const char *filename,
                            size_t *unitsize, SortedList *instance)
{
   GxErrorCode       rc=GXE_NOERROR;
   int               nunits[GXN_DATAUNITS], i;
   size_t            maxsize;
   void              *dataitem;
   GxHorizonHistory  *history;
   GxLabExperiment   *labexperiment;
   GxLocation        *location;
   GxTrap            *trap;
   GxPosGenexVersion  version;

   assert (filehandle);
   assert (filename);
   assert (unitsize);
   assert (instance);

/* Determine the size of the largest data unit structure */
   maxsize = 0;
   for (i=0; i<GXN_DATAUNITS; i++)
      maxsize = MAX (maxsize, unitsize[i]);

   dataitem = malloc (maxsize);
   if (!dataitem) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Read and check the header of the project file */
      rc = GxReadFileHeader (filehandle, filename, &version);

/* -- Read the instance header with the number of data items of each type */
      if (!rc) rc = GxReadInstanceHeader (filehandle, filename, nunits); 

/* Loop for all data units */
      for (i=0; (!rc)&&(i<GXN_DATAUNITS); i++) {

/* -- Loop for all data items of a particular data unit */
         while (!rc && (nunits[i]-- > 0)) {

/* ----- Write the data item to file */
            switch (i) {
                case GX_FILE:
		  rc = GxReadRawDataFile (filehandle, (GxFile *) dataitem, 
								filename);
   	          break;
   	       case GX_HORIZON:
		  rc = GxReadHorizon (filehandle, (GxHorizon *) dataitem,
								filename);
                  break;
               case GX_HORIZONHISTORY:
		  rc = GxReadHorizonHistory (filehandle, (GxHorizonHistory *)
				dataitem, filename);
                  break;
               case GX_LABEXPERIMENT:
		  rc = GxReadLabExperiment (filehandle, (GxLabExperiment *)
				dataitem, filename);
                  break;
               case GX_LITHOLOGY:
		  rc = GxReadLithology (filehandle, (GxLithology *)dataitem,
					filename, version);
                  break;
               case GX_LOCATION:
		  rc = GxReadLocation (filehandle, (GxLocation *) dataitem, 
				       filename);
                  break;
               case GX_REFCONDITIONS:
		  rc = GxReadRefConditions (filehandle, (GxRefConditions *)
							dataitem, filename);
                  break;
               case GX_RESERVOIRSUBAREA:
		  rc = GxReadReservoirSubArea (filehandle, 
				(GxReservoirSubArea *) dataitem, filename);
                  break;
               case GX_ROCK:
		  rc = GxReadRock (filehandle, (GxRock *) dataitem, filename);
                  break;
               case GX_RUNOPTIONS:
		  rc = GxReadRunOptions (filehandle,  
					(GxRunOptions *) dataitem, filename);
                  break;
               case GX_SUBAREA:
		  rc = GxReadSubArea (filehandle,  (GxSubArea *) dataitem,
								filename);
                  break;
               case GX_TRAP:
		  rc = GxReadTrap (filehandle, (GxTrap *) dataitem, filename,
                                   version);
                  break;
            }
	    if (!rc && !AppendToList (instance[i], dataitem))
               rc = GxError (GXE_LIST, GXT_LIST);
         }   
      }
/* -- Free the temporary generic data item */
      free (dataitem);

/* -- Read in the additional history data for each horizon history */
      history = GetFirstFromSortedList (instance[GX_HORIZONHISTORY]);
      while (history) {
         history->ntimes = 0;
         if (!rc) 
	    rc = GxReadHorizonHistoryData (filehandle, filename, history);
         history = GetNextFromSortedList (instance[GX_HORIZONHISTORY]);
      }

/* -- Read in additional history data for each laboratory experiment */
      labexperiment = GetFirstFromSortedList (instance[GX_LABEXPERIMENT]);
      while (labexperiment) {
         labexperiment->ntimes = 0;
         if (!rc)
            rc = GxReadLabExperimentData (filehandle, filename, labexperiment);
	 labexperiment = GetNextFromSortedList (instance[GX_LABEXPERIMENT]);
      }

/* -- Read in the additional history data for each location */
      location = GetFirstFromSortedList (instance[GX_LOCATION]);
      while (location) {
         location->nhorizons = 0;
         if (!rc)
            rc = GxReadLocationData (filehandle, filename, location);
         location = GetNextFromSortedList (instance[GX_LOCATION]);
      }

/* -- Read in the additional depth and history data for each trap */
      trap = GetFirstFromSortedList (instance[GX_TRAP]);
      while (trap) {
         trap->nslices = 0;
         trap->ntimes  = 0;
         if (!rc)
            rc = GxReadTrapData (filehandle, filename, trap);
         trap = GetNextFromSortedList (instance[GX_TRAP]);
      }
   }

   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadInstanceHeader - read an instance header from an open project file
--------------------------------------------------------------------------------

 GxReadInstanceHeader reads the instance header from an already open project
 file into memory.

 Arguments
 ---------
 filehandle - handle of file from which the instance header should be read
 filename   - the name of the file
 nunits     - number of data units in the instance

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadInstanceHeader (FILE *filehandle, const char *filename,
                                  int *nunits)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (filehandle);
   assert (filename);
   assert (nunits);

/* Read the number of data items for each of the data units */
   
   for (i=0; i<GXN_DATAUNITS && !rc; i++) {
      rc = GxGetInteger (filehandle, &nunits[i], filename);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSwapInstance - swaps two instances of a project
--------------------------------------------------------------------------------

 GxSwapInstance swaps two complete project instances.

 Arguments
 ---------
 instance1 - first instance
 instance2 - second instance

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxSwapInstance (SortedList *instance1, SortedList *instance2)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;
   List         templist;

   assert (instance1);
   assert (instance2);

/* Swapping instances is simply swapping the data item lists */
   for (i=0; i<GXN_DATAUNITS; i++) {
      templist     = instance1[i];
      instance1[i] = instance2[i];
      instance2[i] = templist;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxWriteInstance - write a project instance into an open project file
--------------------------------------------------------------------------------

 GxWriteInstance writes a project instance from memory to an already open
 project file.

 Arguments
 ---------
 filehandle - handle of file to which project should be written
 filename   - the name of the file
 unitsize   - the sizes for each of the data units
 instance   - the project instance to be written
 version    - the version format to write project instance in the project file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteInstance (FILE *filehandle, const char *filename,
                             size_t *unitsize, const SortedList *instance,
                             GxPosGenexVersion version)
{
   GxErrorCode       rc=GXE_NOERROR;
   int               nunits[GXN_DATAUNITS], i;
   void              *dataitem;
   GxHorizonHistory  *history;
   GxLabExperiment   *labexperiment;
   GxLocation        *location;
   GxTrap            *trap;

   assert (filehandle);
   assert (filename);
   assert (unitsize);
   assert (instance);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

/* Determine the number of data items for each of the data units */
   for (i=0; i<GXN_DATAUNITS; i++)
      nunits[i] = (int) CountSortedList (instance[i]);

/* Write the header of the project file */
   rc = GxWriteFileHeader (filehandle, version);

/* Write the instance header with the number of data items */
   if (!rc) rc = GxWriteInstanceHeader (filehandle, nunits);

/* Loop for all data units */
   for (i=0; (!rc)&&(i<GXN_DATAUNITS); i++) {

/* -- Loop for all data items of a particular data unit */
      dataitem = GetFirstFromSortedList (instance[i]);
      while (!rc && dataitem) {

/* ----- Write the data item to file */
         switch (i) {
            case GX_FILE:
               GxWriteRawDataFile (filehandle, (GxFile *) dataitem);
	       break;
	    case GX_HORIZON:
	       GxWriteHorizon (filehandle, (GxHorizon *) dataitem);
               break;
            case GX_HORIZONHISTORY:
               GxWriteHorizonHistory (filehandle, (GxHorizonHistory *)dataitem);
               break;
            case GX_LABEXPERIMENT:
               GxWriteLabExperiment (filehandle, (GxLabExperiment *) dataitem);
               break;
            case GX_LITHOLOGY:
               GxWriteLithology (filehandle, (GxLithology *) dataitem, version);
               break;
            case GX_LOCATION:
	       GxWriteLocation (filehandle, (GxLocation *) dataitem);
               break;
            case GX_REFCONDITIONS:
 	       GxWriteRefConditions (filehandle, (GxRefConditions *) dataitem);
               break;
            case GX_RESERVOIRSUBAREA:
	       GxWriteReservoirSubArea (filehandle, 
					 (GxReservoirSubArea *) dataitem);
               break;
            case GX_ROCK:
	       GxWriteRock (filehandle, (GxRock *) dataitem);
               break;
            case GX_RUNOPTIONS:
	       GxWriteRunOptions (filehandle, (GxRunOptions *) dataitem);
               break;
            case GX_SUBAREA:
	       GxWriteSubArea (filehandle, (GxSubArea *) dataitem);
               break;
            case GX_TRAP:
	       GxWriteTrap (filehandle, (GxTrap *) dataitem, version);
               break;

         }
         dataitem = GetNextFromSortedList (instance[i]);
      }
   }

/* Write the additional history data for each horizon history */
   history = GetFirstFromSortedList (instance[GX_HORIZONHISTORY]);
   while (!rc && history) {
      rc = GxWriteHorizonHistoryData (filehandle, history);
      history = GetNextFromSortedList (instance[GX_HORIZONHISTORY]);
   }

/* Write the additional history data for each laboratory experiment */
   labexperiment = GetFirstFromSortedList (instance[GX_LABEXPERIMENT]);
   while (!rc && labexperiment) {
      rc = GxWriteLabExperimentData (filehandle, labexperiment);
      labexperiment = GetNextFromSortedList (instance[GX_LABEXPERIMENT]);
   }

/* Write the additional history data for each location */
   location = GetFirstFromSortedList (instance[GX_LOCATION]);
   while (!rc && location) {
      rc = GxWriteLocationData (filehandle, location);
      location = GetNextFromSortedList (instance[GX_LOCATION]);
   }

/* Write the additional depth and history data for each trap */
   trap = GetFirstFromSortedList (instance[GX_TRAP]);
   while (!rc && trap) {
      rc = GxWriteTrapData (filehandle, trap);
      trap = GetNextFromSortedList (instance[GX_TRAP]);
   }

   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxWriteInstanceHeader - write the instance header into an open project file
--------------------------------------------------------------------------------

 GxWriteInstanceHeader writes the header for a project instance from memory
 to an already open project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 nunits     - number of data units in the instance

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteInstanceHeader (FILE *filehandle, int *nunits)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (filehandle);
   assert (nunits);

   for (i = 0; i < GXN_DATAUNITS; i++) {
       fprintf (filehandle, "%d ", nunits[i]);
   }
   fprintf (filehandle, "\n");
 
   return (rc);
}
