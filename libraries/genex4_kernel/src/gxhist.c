/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxhist.c,v 25.0 2005/07/05 08:03:45 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxhist.c,v $
 * Revision 25.0  2005/07/05 08:03:45  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:55  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:22  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:08  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:41  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:17  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:18  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:29  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:37  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:06  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:52  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:07  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:32  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.3  1998/01/28  15:40:14  ibs
 * Back to the original. No negative depth allowed.
 * Fix should be made in Cauldron. Maybe supply the pressure to genex kernel..
 *
 * Revision 14.2  1998/01/28  13:00:48  ibs
 * typo
 *
 * Revision 14.1  1998/01/28  12:56:12  ibs
 * try-out for negative deths allowing.
 *
 * Revision 14.0  1997/09/16  18:10:03  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:37  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:52  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:05  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:32  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxHist.c     Genex horizon history data functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxHist.c contains the Genex functions for handling horizon
              history data. The following functions are available:

                GxCopyHorizonHistoryData    - copy one history to another
                GxCopyHorizonHistoryToAreas - copy history to selected areas
                GxCreateHorizonHistory      - create a new horizon history
                GxCreateHorizonHistoryData  - create horizon history data
                GxDeleteHorizonHistoryItem  - delete item from horizon history
                GxDestroyHorizonHistoryData - destroy history data
                GxFreeHorizonHistory        - free memory for horizon history
                GxGetHistoryFromLocation    - get the location history data
                GxGetHorizonHistory         - return horizon history
                GxGetHorizonHistoryData     - get horizon history data
                GxInsertHorizonHistoryItem  - insert item in horizon history
                GxReadHorizonHistoryData    - read history data from file
                GxReplaceHorizonHistory     - replaces a horizon history
                GxWriteHorizonHistoryData   - write history data to file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxhist.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 12-DEC-1993  Th. Berkers When reallocating memory first check if some memory
                          had already been allocated, if so use realloc 
                          otherwise use malloc (On SUN platform realloc with 
                          NULL pointer generates an application crash).

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <float.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxerror.h"
#include  "gxfile.h"
#include  "gxhist.h"
#include  "gxnumer.h"
#include  "gxprjct.h"
#include  "gxutils.h"

/* Macro definitions */
/* ----------------- */
#define GXN_ITEMSPERALLOC  10

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocHorizonHistoryArrays (int, HorizonHistory *);
static GxErrorCode  GxAllocHorizonHistoryData (int, GxHorizonHistory *);
static GxErrorCode  GxFindHorizonHistory (const char *, const char *,
                                          GxSubArea **, GxHorizon **,
                                          GxHorizonHistory **);
static GxErrorCode  GxFreeHorizonHistoryArrays (HorizonHistory *);
static GxErrorCode  GxGetHistoryFromLocation (HorizonHistory *, double,
                                              GxLocation *); 



/*
--------------------------------------------------------------------------------
 GxAllocHorizonHistoryArrays - allocate arrays for the horizon history
--------------------------------------------------------------------------------

 GxAllocHorizonHistoryArrays allocates arrays for a new horizon history
 data structure.

 Arguments
 ---------
 ntimes - number of time entries in horizon history data structure
 horhis - the history for which the arrays should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocHorizonHistoryArrays (int ntimes,
                                                HorizonHistory *horhis)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize;
   size_t       newmemsize;

   assert (ntimes >= 0);
   assert (horhis);

/* Calculate mimimum size of array in items and bytes */
   newsize = GXN_ITEMSPERALLOC * (ntimes / GXN_ITEMSPERALLOC + 1);
   newmemsize = newsize * sizeof (double);

/* If current size is smaller than minimum size reallocate the arrays */
   if (horhis->size < newsize) {
      rc = GxFreeHorizonHistoryArrays (horhis);
      if (!rc) {
         horhis->size        = newsize;
         horhis->time        = (double *) malloc (newmemsize);
         horhis->depth       = (double *) malloc (newmemsize);
         horhis->temperature = (double *) malloc (newmemsize);
         horhis->pressure    = (double *) malloc (newmemsize);
         if (!horhis->time || !horhis->depth || !horhis->temperature
            || !horhis->pressure) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
            (void) GxFreeHorizonHistoryArrays (horhis);
         }
      }
   }

/* There are no items in the array */
   horhis->ntimes = 0;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocHorizonHistoryData - allocate a new horizon history data structure
--------------------------------------------------------------------------------

 GxAllocHorizonHistoryData allocates memory for a new horizon history data
 structure.

 Arguments
 ---------
 ntimes  - number of time entries for the horizon history
 history - the horizon history for which the memory should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocHorizonHistoryData (int ntimes,
                                              GxHorizonHistory *history)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (ntimes >= 0);
   assert (history);

   history->ntimes   = ntimes;
   history->physprop = NULL;

/* Allocate a single two dimensional array of the right size for all data */
   if (ntimes > 0)
      rc = GxAllocArray (&history->physprop, sizeof (double), 2,
                         GXN_HISTPHYSPROP, ntimes);

/* Free it in case of error */
   if (rc) (void) GxDestroyHorizonHistoryData (history);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyHorizonHistoryData - copy data from one horizon history to another
--------------------------------------------------------------------------------

 GxCopyHorizonHistoryData copies horizon history data from one horizon history
 struct to another.

 Arguments
 ---------
 destination - destination for horizon history data
 source      - source of the horizon history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyHorizonHistoryData (GxHorizonHistory *destination,
                                      const GxHorizonHistory *source)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (destination);
   assert (source);

/* Allocate the memory needed for the destination */
   rc = GxAllocHorizonHistoryData (source->ntimes, destination);
   if (!rc && (source->ntimes > 0)) {

/* -- Copy the two-dimensional array with data, row by row */
      for (i=0; i<GXN_HISTPHYSPROP; i++)
         memcpy (destination->physprop[i], source->physprop[i],
                 source->ntimes * sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyHorizonHistoryToAreas - copy horizon history data to selected subareas
--------------------------------------------------------------------------------

 GxCopyHorizonHistoryToAreas copies the horizon history data identified by
 <srhorizonname> and <subareaname> to all the subareas whose names are listed
 in <subarealist>.

 Arguments
 ---------
 srhorizonname - name of the source rock horizon
 subareaname   - name of the subarea
 subarealist   - list of subarea names

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyHorizonHistoryToAreas (const char *srhorizonname,
                                         const char *subareaname,
                                         List subarealist)
{
   GxErrorCode     rc=GXE_NOERROR;
   GxBool          initialized;
   HorizonHistory  *horhis;
   char            *othersubareaname;

   if (!srhorizonname || (strlen (srhorizonname) == 0)
      || (strlen (srhorizonname) > GXL_HORIZONNAME) || !subareaname
      || (strlen (subareaname) == 0)
      || (strlen (subareaname) > GXL_SUBAREANAME) || !subarealist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create a temporary horizon history structure */
         rc = GxCreateHorizonHistory (&horhis);
         if (!rc) {

/* -------- Get the horizon history from the project */
            rc = GxGetHorizonHistory (srhorizonname, subareaname, horhis);

/* -------- Replace all horizon histories with the same source rock horizon */
            othersubareaname = GetFirstFromList (subarealist);
            while (!rc && othersubareaname) {
               if (strcmp (othersubareaname, subareaname) != 0) {
                  strcpy (horhis->subarea, othersubareaname);
                  rc = GxReplaceHorizonHistory (horhis);
               }
               othersubareaname = GetNextFromList (subarealist);
            }
            GxFreeHorizonHistory (horhis);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateHorizonHistory - create a new horizon history structure
--------------------------------------------------------------------------------

 GxCreateHorizonHistory allocates memory for a new horizon history
 structure and initializes that memory.

 Arguments
 ---------
 horhisptr - pointer for the horizon history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateHorizonHistory (HorizonHistory **horhisptr)
{
   GxErrorCode     rc=GXE_NOERROR;
   HorizonHistory  *horhis;

   if (!horhisptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      horhis = ( HorizonHistory * ) malloc (sizeof (HorizonHistory));
      if (!horhis) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         horhis->size = 0;
         rc = GxAllocHorizonHistoryArrays (0, horhis);
         if (!rc) {
            horhis->horizon[0] = '\0';
            horhis->subarea[0] = '\0';
            horhis->ntimes     = 0;
            *horhisptr         = horhis;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateHorizonHistoryData - create horizon history data from user data
--------------------------------------------------------------------------------

 GxCreateHorizonHistoryData allocates memory for a horizon history data item
 by using the sizes defined in the user data and copies the data into the
 horizon history.

 Arguments
 ---------
 horhisdata - horizon history to be created
 horhis     - user data for the horizon history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateHorizonHistoryData (GxHorizonHistory *horhisdata,
                                        const HorizonHistory *horhis)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (horhisdata);
   assert (horhis);

/* Allocate the memory needed for the data */
   rc = GxAllocHorizonHistoryData (horhis->ntimes, horhisdata);
   if (!rc && (horhis->ntimes > 0)) {

/* -- Copy the individual data arrays into rows of the two-dim array */
      memcpy (horhisdata->physprop[GX_HISTTIME], horhis->time,
              horhis->ntimes * sizeof(double));
      memcpy (horhisdata->physprop[GX_HISTDEPTH], horhis->depth,
              horhis->ntimes * sizeof(double));
      memcpy (horhisdata->physprop[GX_HISTTEMPERATURE], horhis->temperature,
              horhis->ntimes*sizeof(double));
      memcpy (horhisdata->physprop[GX_HISTPRESSURE], horhis->pressure,
              horhis->ntimes*sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteHorizonHistoryItem - delete an item from the horizon history
--------------------------------------------------------------------------------

 GxDeleteHorizonHistoryItem removes an horizon history item from a horizon
 history. The size of the arrays in the horizon history is not changed.

 Arguments
 ---------
 horhis - user data for the horizon history
 itemnr - the number of the item to be removed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteHorizonHistoryItem (HorizonHistory *horhis, int itemnr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   if (!horhis || (itemnr < 0) || (itemnr >= horhis->ntimes)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Shift items after one to be deleted forward */
      for (i=itemnr; i<horhis->ntimes-1; i++) {
         horhis->time[i]        = horhis->time[i+1];
         horhis->depth[i]       = horhis->depth[i+1];
         horhis->temperature[i] = horhis->temperature[i+1];
         horhis->pressure[i]    = horhis->pressure[i+1];
      }

/* -- Decrease number of items */
      horhis->ntimes--;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDestroyHorizonHistoryData - destroy horizon history data by freeing memory
--------------------------------------------------------------------------------

 GxDestroyHorizonHistoryData frees the memory allocated for a horizon history
 data item.

 Arguments
 ---------
 horhis - horizon history to be destroyed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDestroyHorizonHistoryData (GxHorizonHistory *horhis)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (horhis);

   if (horhis->ntimes > 0) {
      GxFreeArray (horhis->physprop, 2, GXN_HISTPHYSPROP, horhis->ntimes);
      horhis->physprop = NULL;
      horhis->ntimes = 0;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFindHorizonHistory - find a horizon history in the project
--------------------------------------------------------------------------------

 GxFindHorizonHistory searches for a particular horizon history identified
 by subarea and source rock horizon name in the project. If the subarea or
 source rock horizon are not found an error is returned. Pointers to the
 subarea and source rock horizon dataitems are returned. If the horizon
 history itself is found a pointer to it is returned, otherwise NULL is
 returned.

 Arguments
 ---------
 srhorizonname - name of the source rock horizon
 subareaname   - name of the subarea
 subareaptr    - pointer to the subarea
 horizonptr    - pointer to source rock
 horhisptr     - pointer to the horizon history or NULL if not present

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFindHorizonHistory (const char *srhorizonname,
                                         const char *subareaname,
                                         GxSubArea **subareaptr,
                                         GxHorizon **horizonptr,
                                         GxHorizonHistory **horhisptr)
{
   GxErrorCode       rc=GXE_NOERROR;
   GxHorizonHistory  searchhorhis;
   GxHorizon         searchhorizon;
   GxSubArea         searchsubarea;

   assert (srhorizonname);
   assert (subareaname);
   assert (subareaptr);
   assert (horizonptr);
   assert (horhisptr);

/* Find the source rock horizon in the project */
   strcpy (searchhorizon.name, srhorizonname);
   *horizonptr = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                 (GxDataItem *) &searchhorizon);
   if (!*horizonptr || ((*horizonptr)->type != GX_SOURCEROCK)) {
        rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                    srhorizonname);
   } else {

/* -- Find the subarea in the project */
      strcpy (searchsubarea.name, subareaname);
      *subareaptr = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                    (GxDataItem *) &searchsubarea);
      if (!*subareaptr) {
         rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                       subareaname);
      } else {

/* ----- Find the horizon history in the project */
         searchhorhis.horizon = *horizonptr;
         searchhorhis.subarea = *subareaptr;
         *horhisptr =
            (GxHorizonHistory *) GxSearchDataItem (GX_HORIZONHISTORY,
            (GxDataItem *) &searchhorhis);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeHorizonHistory - free the memory for the data of a horizon history
--------------------------------------------------------------------------------

 GxFreeHorizonHistory frees the memory that was allocated for the data of a
 horizonhistory in the project.

 Arguments
 ---------
 horhis - the horizon history data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeHorizonHistory (HorizonHistory *horhis)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!horhis) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxFreeHorizonHistoryArrays (horhis);
      free (horhis);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeHorizonHistoryArrays - free arrays for the data of a horizon history
--------------------------------------------------------------------------------

 GxFreeHorizonHistoryArrays frees the arrays that were allocated for the data
 of a horizon history in the project.

 Arguments
 ---------
 horhis - the horizon history data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeHorizonHistoryArrays (HorizonHistory *horhis)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (horhis);

   if (horhis->size > 0) {
      if (horhis->time)        free (horhis->time);
      if (horhis->depth)       free (horhis->depth);
      if (horhis->temperature) free (horhis->temperature);
      if (horhis->pressure)    free (horhis->pressure);
   }
   horhis->ntimes      = 0;
   horhis->size        = 0;
   horhis->time        = NULL;
   horhis->depth       = NULL;
   horhis->temperature = NULL;
   horhis->pressure    = NULL;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetHistoryFromLocation - get history data from location into history
--------------------------------------------------------------------------------

 GxGetHistoryFromLocation retrieves horizon history data from the location
 data. The location horizon with age closest to the age of the source rock
 horizon is returned

 Arguments
 ---------
 horhis   - user data for the horizon history
 age      - the of the source rock horizon
 location - the location from which the data should be taken

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetHistoryFromLocation (HorizonHistory *horhis, double age,
                                      GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          horizonnr, i, ntimes;
   double       agediff, diff;

   assert (horhis);
   assert (age >= 0.0);
   assert (location);

   horizonnr = -1;
   agediff   = DBL_MAX;

/* Find the horizon in the location with the age closest to age */
   for (i=0; i<location->nhorizons; i++) {
      if (location->ntimes[i] > 0) {
         diff = ABS (location->physprop[i][GX_LOCATTIME][0] - age);
         if (agediff > diff) {
            horizonnr = i;
            agediff = diff;
         }
      }
   }

/* If nothing found set number of times to zero */
   if (horizonnr == -1) {
      ntimes = 0;
   } else {
      ntimes = location->ntimes[horizonnr];
   }

/* Allocate arrays for the horizon data and copy the data */
   rc = GxAllocHorizonHistoryArrays (ntimes, horhis);
   if (!rc && (ntimes > 0)) {
      memcpy (horhis->time, location->physprop[horizonnr][GX_LOCATTIME],
              ntimes * sizeof(double));
      memcpy (horhis->depth, location->physprop[horizonnr][GX_LOCATDEPTH],
              ntimes * sizeof(double));
      memcpy (horhis->temperature,
              location->physprop[horizonnr][GX_LOCATTEMPERATURE],
              ntimes * sizeof(double));
      memcpy (horhis->pressure,
              location->physprop[horizonnr][GX_LOCATPRESSURE],
              ntimes * sizeof(double));
      horhis->ntimes = ntimes;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetHorizonHistory - return contents of one horizon history the project
--------------------------------------------------------------------------------

 GxGetHorizonHistory returns the contents of one of the horizon histories in
 the project. If it is not present, information from the raw data file is
 returned if available.

 Arguments
 ---------
 srhorizonname - the name of the source rock horizon
 subareaname   - the name of the subarea
 horhis        - horizon history to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetHorizonHistory (const char *srhorizonname,
                                 const char *subareaname,
                                 HorizonHistory *horhis)
{
   GxErrorCode       rc=GXE_NOERROR;
   GxBool            initialized;
   GxSubArea         *subarea;
   GxHorizon         *horizon;
   GxHorizonHistory  *oldhorhis;

   if (!srhorizonname || (strlen (srhorizonname) == 0)
      || (strlen (srhorizonname) > GXL_HORIZONNAME) || !subareaname
      || (strlen (subareaname) == 0)
      || (strlen (subareaname) > GXL_SUBAREANAME) || !horhis) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the horizon history in the project */
         rc = GxFindHorizonHistory (srhorizonname, subareaname, &subarea,
                                    &horizon, &oldhorhis);
         if (!rc) {

/* -------- Prepare a named empty horizon history */
            strcpy (horhis->horizon, srhorizonname);
            strcpy (horhis->subarea, subareaname);
            horhis->ntimes = 0;
            if (oldhorhis) {

/* ----------- Horizon history is present: allocate space and copy data */
               rc = GxAllocHorizonHistoryArrays (oldhorhis->ntimes, horhis);
               if (!rc) rc = GxGetHorizonHistoryData (horhis, oldhorhis);
            }

/* -------- If no history data is available get it from location (if any) */
            if ((horhis->ntimes == 0) && (subarea->location))
               rc = GxGetHistoryFromLocation (horhis, horizon->age,
                                              subarea->location);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetHorizonHistoryData - get user data from horizon history data
--------------------------------------------------------------------------------

 GxGetHorizonHistoryData copies the horizon history data from <horhisdata>
 into the user data <horhis>.

 Arguments
 ---------
 horhis     - user data for the horizon history
 horhisdata - horizon history data to be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetHorizonHistoryData (HorizonHistory *horhis,
                                     const GxHorizonHistory *horhisdata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (horhis);
   assert (horhisdata);

   if (horhisdata->ntimes > 0) {

/* -- Copy the rows of the two-dim array into individual data arrays */
      memcpy (horhis->time, horhisdata->physprop[GX_HISTTIME],
              horhisdata->ntimes * sizeof(double));
      memcpy (horhis->depth, horhisdata->physprop[GX_HISTDEPTH],
              horhisdata->ntimes * sizeof(double));
      memcpy (horhis->temperature, horhisdata->physprop[GX_HISTTEMPERATURE],
              horhisdata->ntimes * sizeof(double));
      memcpy (horhis->pressure, horhisdata->physprop[GX_HISTPRESSURE],
              horhisdata->ntimes * sizeof(double));
   }
   horhis->ntimes = horhisdata->ntimes;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertHorizonHistoryItem - insert an item into the horizon history
--------------------------------------------------------------------------------

 GxInsertHorizonHistoryItem inserts a horizon history item into a horizon
 history. The size of the arrays in the horizon history is changed if
 they are not large enough.

 Arguments
 ---------
 horhis      - user data for the horizon history
 time        - the time for the item to be added
 depth       - the depth for the item to be added
 temperature - the temperature for the item to be added
 pressure    - the pressure for the item to be added

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertHorizonHistoryItem (HorizonHistory *horhis, double time,
                                        double depth, double temperature,
                                        double pressure)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize, itemnr, i;
   size_t       newmemsize;
   double       *newtime, *newdepth, *newtemperature, *newpressure;

   if (!horhis || (depth < 0.0) || (temperature < 0.0) || (pressure < 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if ((horhis->ntimes > 0)
         && (horhis->time[0] < horhis->time[horhis->ntimes-1])) {
         rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
      } else {
         if (horhis->ntimes == horhis->size) {

/*--------- Arrays are full: reallocate with a bigger size */
            newsize        = GXN_ITEMSPERALLOC *
                             ((horhis->ntimes + 1) / GXN_ITEMSPERALLOC + 1);
            newmemsize     = newsize * sizeof (double);
            if (horhis->time) {
                newtime = (double *) realloc (horhis->time, newmemsize);
            } else {
                newtime = (double *) malloc (newmemsize);
            }
            if (horhis->depth) {
                newdepth = (double *) realloc (horhis->depth, newmemsize);
            } else {
                newdepth = (double *) malloc (newmemsize);
            }
            if (horhis->temperature) {
                newtemperature = (double *) realloc (horhis->temperature,
                                                     newmemsize);
            } else {
                newtemperature = (double *) malloc (newmemsize);
            }
            if (horhis->pressure) {
                newpressure = (double *) realloc (horhis->pressure,
                                                  newmemsize);
            } else {
                newpressure = (double *) malloc (newmemsize);
            }
            if (newtime)        horhis->time        = newtime;
            if (newdepth)       horhis->depth       = newdepth;
            if (newtemperature) horhis->temperature = newtemperature;
            if (newpressure)    horhis->pressure    = newpressure;
            if (!newtime || !newdepth || !newtemperature || !newpressure) {
               rc = GxError (GXE_MEMORY, GXT_MEMORY);
            } else {
               horhis->size = newsize;
            }
         }
         if (!rc) {

/* -------- Find the position where the item should be inserted */
            itemnr = MAX (0, GxSearchDoubleD (horhis->ntimes, horhis->time,
                                              time));

/* -------- Shift items to make room for new item */
            for (i=horhis->ntimes-1; i>=itemnr; i--) {
               horhis->time[i+1]        = horhis->time[i];
               horhis->depth[i+1]       = horhis->depth[i];
               horhis->temperature[i+1] = horhis->temperature[i];
               horhis->pressure[i+1]    = horhis->pressure[i];
            }

/* -------- Insert the new item */
            horhis->time[itemnr]        = time;
            horhis->depth[itemnr]       = depth;
            horhis->temperature[itemnr] = temperature;
            horhis->pressure[itemnr]    = pressure;

/* -------- Increase the number of items */
            horhis->ntimes++;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadHorizonHistoryData - read horizon history data from a project file
--------------------------------------------------------------------------------

 GxReadHorizonHistoryData reads a horizon history data item from an already
 open project file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 filename   - the name of the file
 history    - horizon history to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadHorizonHistoryData (FILE *filehandle, const char *filename,
                                      GxHorizonHistory *history)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          ntimes, i, j;

   assert (filehandle);
   assert (filename);
   assert (history);

/* Read the number of history items */
   rc = GxGetInteger (filehandle, &ntimes, filename);
   if (!rc) {
      if (ntimes < 0) {
         rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
      } else {

/* ----- Allocate array big enough to hold the data */
         rc = GxAllocHorizonHistoryData (ntimes, history);
         if (ntimes > 0) {

/* -------- Read in the array data row by row */
	    for (i=0; (!rc)&&(i<ntimes);i++)
               for (j=0; (!rc)&&(j<GXN_HISTPHYSPROP); j++)
                  rc = GxGetDouble (filehandle, &history->physprop[j][i], 
								    filename);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceHorizonHistory - replaces a horizon history in the project
--------------------------------------------------------------------------------

 GxReplaceHorizonHistory replaces information from a horizon history
 with new information.

 Arguments
 ---------
 horhis - the horizon history to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceHorizonHistory (const HorizonHistory *horhis)
{
   GxErrorCode       rc=GXE_NOERROR;
   GxBool            initialized;
   GxSubArea         *subarea;
   GxHorizon         *horizon;
   GxHorizonHistory  *oldhorhis, *insertedhorhis, newhorhis;

   if (!horhis || (strlen (horhis->horizon) == 0)
      || (strlen (horhis->horizon) > GXL_HORIZONNAME)
      || (strlen (horhis->subarea) == 0)
      || (strlen (horhis->subarea) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the horizon history in the project */
         rc = GxFindHorizonHistory (horhis->horizon, horhis->subarea,
                                    &subarea, &horizon, &oldhorhis);
         if (!rc) {
            if (oldhorhis) {

/* ----------- Get rid of the old data and create the new data */
               rc = GxDestroyHorizonHistoryData (oldhorhis);
               if (!rc) rc = GxCreateHorizonHistoryData (oldhorhis, horhis);

/* ----------- Indicate to project that lab experiment has been changed */
               GxChangeProject ();
            } else {

/* ----------- Horizon history not yet present: fill and insert it */
               newhorhis.horizon = horizon;
               newhorhis.subarea = subarea;
               rc = GxInsertDataItem (GX_HORIZONHISTORY,
                                     (GxDataItem *) &newhorhis,
                                     (GxDataItem **) &insertedhorhis);
               if (!rc) rc = GxCreateHorizonHistoryData (insertedhorhis,
                                                         horhis);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxWriteHorizonHistoryData - write horizon history data to a project file
--------------------------------------------------------------------------------

 GxWriteHorizonHistoryData writes data for one location to an already open
 project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 history    - horizon history for which data should be written

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteHorizonHistoryData (FILE *filehandle, const GxHorizonHistory *history)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;

   if (!filehandle || !history) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Write the size of the history */
      fprintf (filehandle, "%d\n", history->ntimes);
      if (history->ntimes > 0) {

/* ----- Write the array data */
         for (i=0; i<history->ntimes; i++)  {
            for (j=0; j<GXN_HISTPHYSPROP; j++) {
	        fprintf (filehandle, "%f ", history->physprop[j][i]);
	    }
	    fprintf (filehandle, "\n");
	 }
      }
   }
   return (rc);
}
