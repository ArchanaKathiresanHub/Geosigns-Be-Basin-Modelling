/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxtrap.c,v 25.0 2005/07/05 08:03:51 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxtrap.c,v $
 * Revision 25.0  2005/07/05 08:03:51  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:02  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:36  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:15  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:46  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:40  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:31  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:49  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:07  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:25  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:30  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:26  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:04  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:01  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:06  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:23  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:44  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:20  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxTrap.c     Genex trap data functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxTrap.c contains the Genex functions for handling trap
              data. The following functions are available:

                GxCopyTrapData          - copy data for one trap to another
                GxCreateTrap            - create a new trap structure
                GxCreateTrapData        - create trap data from user data
                GxDeleteTrap            - remove a trap from the project
                GxDeleteTrapDepthItem   - delete depth item from trap
                GxDeleteTrapHistoryItem - delete history item from trap
                GxDestroyTrapData       - destroy trap data for a trap
                GxFreeTrap              - free memory for trap contents
                GxGetTrap               - return contents of one trap
                GxGetTrapData           - get user data from trap data
                GxGetTrapNameList       - return trap names in project
                GxInsertTrap            - inserts a new trap in project
                GxInsertTrapDepthItem   - insert depth item into trap
                GxInsertTrapHistoryItem - insert history item into trap
		GxReadTrap              - read trap from project file
                GxReadTrapData          - read trap data from project file
                GxRenameTrap            - change the name of a trap
                GxReplaceTrap           - replaces a trap in project
                GxWriteTrap             - write trap struct to project file
                GxWriteTrapData         - write trap data to project file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxtrap.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 24-MAR-1993  M. van Houtert GxWriteTrap added
 28-MAR-1993  M. van Houtert GxReadTrap added
 12-DEC-1993  Th. Berkers When reallocating memory first check if some memory
                          had already been allocated, if so use realloc 
                          otherwise use malloc (On SUN platform realloc with 
                          NULL pointer generates an application crash).
 20-MAR-1995  Th. Berkers Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <float.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxfile.h"
#include  "gxnumer.h"
#include  "gxprjct.h"
#include  "gxtrap.h"
#include  "gxutils.h"
/* Macro definitions */
/* ----------------- */
#define  GX_DEFAULTFORMATIONTIME   150.0
#define  GX_DEFAULTGASGRAVITY      0.7
#define  GX_DEFAULTGIIP            1.0E10
#define  GX_DEFAULTHWCPRESSURE     25.0E6
#define  GX_DEFAULTHWCTEMPERATURE  (100.0+GXC_ZEROCELCIUS)
#define  GX_DEFAULTOILGRAVITY      35.0
#define  GX_DEFAULTSTOIIP          1.0E8
#define  GX_DEFAULTTRAPTYPE        GX_PROSPECT
#define  GXN_DEPTHITEMSPERALLOC    5
#define  GXN_HISTITEMSPERALLOC     10
/* -- error text definitions -- */
#define  GXT_DUPLICATETRAP   "Trap %s is already present in project.\n"
#define  GXT_NOTOPDEPTH      "The top depth for trap %s is not specified.\n"
#define  GXT_TOOMANYTRAPS    "The maximum number of traps (" \
                             STRNG(GXN_TRAP) \
                             ") has already been reached.\n"
#define  GXT_TRAPNOTPRESENT  "Trap %s is not present in project.\n"

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocTrapDepthArrays (int, Trap *);
static GxErrorCode  GxAllocTrapHistoryArrays (int, Trap *);
static GxErrorCode  GxAllocTrapData (int, int, GxTrap *);
static GxErrorCode  GxFreeTrapDepthArrays (Trap *);
static GxErrorCode  GxFreeTrapHistoryArrays (Trap *);
static GxErrorCode  GxGetHistoryFromLocation (Trap *, double, GxLocation *);



/*
--------------------------------------------------------------------------------
 GxAllocTrapDepthArrays - allocate arrays for depth data for the trap
--------------------------------------------------------------------------------

 GxAllocTrapDepthArrays allocates arrays for the depth data of a trap.

 Arguments
 ---------
 nslices - number of depths in trap data structure
 trap    - the trap for which the arrays should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocTrapDepthArrays (int nslices, Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize;
   size_t       newmemsize;

   assert (nslices >= 0);
   assert (trap);

/* Calculate mimimum size of array in items and bytes */
   newsize = GXN_DEPTHITEMSPERALLOC *
             (nslices / GXN_DEPTHITEMSPERALLOC + 1);
   newmemsize = newsize * sizeof (double);

/* If current size is smaller than minimum size reallocate the arrays */
   if (trap->depthsize < newsize) {
      rc = GxFreeTrapDepthArrays (trap);
      if (!rc) {
         trap->depthsize = newsize;
         trap->depth     = (double *) malloc (newmemsize);
         trap->area      = (double *) malloc (newmemsize);
         if (!trap->depth || !trap->area) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
            (void) GxFreeTrapDepthArrays (trap);
         }
      }
   }

/* There are no items in the array */
   trap->nslices = 0;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocTrapHistoryArrays - allocate arrays for history data for the trap
--------------------------------------------------------------------------------

 GxAllocTrapHistoryArrays allocates arrays for the history data of a trap.

 Arguments
 ---------
 ntimes - number of time entries in trap data structure
 trap   - the trap for which the arrays should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocTrapHistoryArrays (int ntimes, Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize;
   size_t       newmemsize;

   assert (ntimes >= 0);
   assert (trap);

/* Calculate mimimum size of array in items and bytes */
   newsize = GXN_HISTITEMSPERALLOC * (ntimes / GXN_HISTITEMSPERALLOC + 1);
   newmemsize = newsize * sizeof (double);

/* If current size is smaller than minimum size reallocate the arrays */
   if (trap->historysize < newsize) {
      rc = GxFreeTrapHistoryArrays (trap);
      if (!rc) {
         trap->historysize = newsize;
         trap->time        = (double *) malloc (newmemsize);
         trap->temperature = (double *) malloc (newmemsize);
         trap->pressure    = (double *) malloc (newmemsize);
         trap->structvol   = (double *) malloc (newmemsize);
         if (!trap->time || !trap->temperature || !trap->pressure
            || !trap->structvol) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
            (void) GxFreeTrapHistoryArrays (trap);
         }
      }
   }

/* There are no items in the array */
   trap->ntimes = 0;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocTrapData - allocate a new trap data structure
--------------------------------------------------------------------------------

 GxAllocTrapData allocates memory for a new trap data structure.

 Arguments
 ---------
 nslices  - number of slices in trap data structure
 ntimes   - number of time entries in trap data structure
 trapdata - the trap for which the memory should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocTrapData (int nslices, int ntimes, GxTrap *trapdata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (nslices >= 0);
   assert (ntimes >= 0);
   assert (trapdata);

   trapdata->nslices  = nslices;
   trapdata->ntimes   = ntimes;
   trapdata->depth    = NULL;
   trapdata->area     = NULL;
   trapdata->physprop = NULL;

/* Allocate arrays for the area versus depth data */
   if (nslices > 0) {
      rc = GxAllocArray (&trapdata->depth, sizeof (double), 1, nslices);
      if (!rc) rc = GxAllocArray (&trapdata->area, sizeof (double),
                                  1, nslices);
   }

/* Allocate a single two-dimensional array for the history data */
   if (!rc && (ntimes > 0))
      rc = GxAllocArray (&trapdata->physprop, sizeof (double),
                         2, GXN_TRAPPHYSPROP, ntimes);

/* Free arrays in case of error */
   if (rc) (void) GxDestroyTrapData (trapdata);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyTrapData - copy data from one trap to another
--------------------------------------------------------------------------------

 GxCopyTrapData copies trap data from one trap struct to another.

 Arguments
 ---------
 destination - destination for trap data
 source      - source of the trap data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyTrapData (GxTrap *destination, const GxTrap *source)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (destination);
   assert (source);

/* Allocate the memory needed for the destination */
   rc = GxAllocTrapData (source->nslices, source->ntimes, destination);
   if (!rc) {

/* -- Copy the area versus depth data */
      if (source->nslices > 0) {
         memcpy (destination->depth, source->depth,
                 source->nslices * sizeof(double));
         memcpy (destination->area, source->area,
                 source->nslices * sizeof(double));
      }

/* -- Copy the two-dimensional array with data, row by row */
      if (source->ntimes > 0)
         for (i=0; i<GXN_TRAPPHYSPROP; i++)
            memcpy (destination->physprop[i], source->physprop[i],
                    source->ntimes * sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateTrap - create a new trap structure
--------------------------------------------------------------------------------

 GxCreateTrap allocates memory for a new trap structure and initializes that
 memory.

 Arguments
 ---------
 trapptr - pointer for the trap

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateTrap (Trap **trapptr)
{
   int          i;
   GxErrorCode  rc=GXE_NOERROR;
   Trap         *trap;

   if (!trapptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      trap = ( Trap * ) malloc (sizeof (Trap));
      if (!trap) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         trap->depthsize   = 0;
         trap->historysize = 0;
         rc = GxAllocTrapDepthArrays (0, trap);
         if (!rc) rc = GxAllocTrapHistoryArrays (0, trap);
         if (!rc) {
            trap->name[0]          = '\0';
            trap->type             = GX_PROSPECT;
            trap->reservoir[0]     = '\0';
            trap->crest[0]         = '\0';
            trap->formationtime    = GX_DEFAULTFORMATIONTIME;
            trap->hwctemperature   = GX_DEFAULTHWCTEMPERATURE;
            trap->hwcpressure      = GX_DEFAULTHWCPRESSURE;
            trap->stoiip           = GX_DEFAULTSTOIIP;
            trap->giip             = GX_DEFAULTGIIP;
            trap->oilgravity       = GX_DEFAULTOILGRAVITY;
            trap->gasgravity       = GX_DEFAULTGASGRAVITY;
            trap->nslices          = 0;
            trap->ntimes           = 0;
            for (i=0; i<GXC_MAX_CARBONNR;i++) {
               trap->gascomp[i].molpercentage = GX_DEFAULTMOLPERC;
               trap->gascomp[i].fractionation = GX_DEFAULTFRACT;
            }
            *trapptr               = trap;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateTrapData - create trap data from user data
--------------------------------------------------------------------------------

 GxCreateTrapData allocates memory for a trap data item by using the
 sizes defined in the user data and copies the data into the trap.

 Arguments
 ---------
 trapdata - trap to be created
 trap     - user data for the trap

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateTrapData (GxTrap *trapdata, const Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (trapdata);
   assert (trap);


/* Allocate the memory needed for the trap data */
   rc = GxAllocTrapData (trap->nslices, trap->ntimes, trapdata);
   if (!rc) {

/* -- Copy the area versus depth data */
      if (trap->nslices > 0) {
         memcpy (trapdata->depth, trap->depth,
                 trap->nslices * sizeof(double));
         memcpy (trapdata->area, trap->area,
                 trap->nslices * sizeof(double));
      }

/* -- Copy the history data */
      if (trap->ntimes > 0) {
         memcpy (trapdata->physprop[GX_TRAPTIME], trap->time,
                 trap->ntimes*sizeof(double));
         memcpy (trapdata->physprop[GX_TRAPTEMPERATURE],
                 trap->temperature, trap->ntimes*sizeof(double));
         memcpy (trapdata->physprop[GX_TRAPPRESSURE],
                 trap->pressure, trap->ntimes*sizeof(double));
         memcpy (trapdata->physprop[GX_TRAPSTRUCTVOL],
                 trap->structvol, trap->ntimes*sizeof(double));
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteTrap - remove a trap from the project
--------------------------------------------------------------------------------

 GxDeleteTrap removes all the information for the trap with the name <name>
 from the project.

 Arguments
 ---------
 name - name of the trap to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteTrap (const char *name)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxTrap       *trap, searchtrap;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_TRAPNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the trap structure from project */
         strcpy (searchtrap.name, name);
         trap = (GxTrap *) GxSearchDataItem (GX_TRAP,
                                             (GxDataItem *) &searchtrap);
         if (!trap) {
            rc = GxError (GXE_TRAPNOTPRESENT, GXT_TRAPNOTPRESENT, name);
         } else {

/* -------- Free the trap data and remove the trap from the project */
            rc = GxDestroyTrapData (trap);
            GxRemoveDataItem (GX_TRAP);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteTrapDepthItem - delete an depth item from the trap
--------------------------------------------------------------------------------

 GxDeleteTrapDepthItem removes an depth item from a trap. The size of the
 arrays in the trap depth data is not changed.

 Arguments
 ---------
 trap   - user data for the trap
 itemnr - the number of the item to be removed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteTrapDepthItem (Trap *trap, int itemnr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   if (!trap || (itemnr < 0) || (itemnr >= trap->nslices)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Shift items after one to be deleted forward */
      for (i=itemnr; i<trap->nslices-1; i++) {
         trap->depth[i] = trap->depth[i+1];
         trap->area[i]  = trap->area[i+1];
      }
      trap->nslices--;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteTrapHistoryItem - delete an history item from the trap
--------------------------------------------------------------------------------

 GxDeleteTrapHistoryItem removes an history item from a trap. The size of the
 arrays in the trap history is not changed.

 Arguments
 ---------
 trap   - user data for the trap
 itemnr - the number of the item to be removed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteTrapHistoryItem (Trap *trap, int itemnr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   if (!trap || (itemnr < 0) || (itemnr >= trap->ntimes)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Shift items after one to be deleted forward */
      for (i=itemnr; i<trap->ntimes-1; i++) {
         trap->time[i]        = trap->time[i+1];
         trap->temperature[i] = trap->temperature[i+1];
         trap->pressure[i]    = trap->pressure[i+1];
         trap->structvol[i]   = trap->structvol[i+1];
      }
      trap->ntimes--;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDestroyTrapData - destroy trap data by freeing memory
--------------------------------------------------------------------------------

 GxDestroyTrapData frees the memory allocated for a trap data item.

 Arguments
 ---------
 trapdata - trap data structureto be destroyed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDestroyTrapData (GxTrap *trapdata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (trapdata);

   if (trapdata->nslices > 0) {
      GxFreeArray (trapdata->depth, 1, trapdata->nslices);
      GxFreeArray (trapdata->area, 1, trapdata->nslices);
      trapdata->depth = NULL;
      trapdata->area = NULL;
      trapdata->nslices = 0;
   }
   if (trapdata->ntimes > 0) {
      GxFreeArray (trapdata->physprop, 2, GXN_TRAPPHYSPROP,
                   trapdata->ntimes);
      trapdata->physprop = NULL;
      trapdata->ntimes = 0;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeTrap - free the memory for the data of a trap
--------------------------------------------------------------------------------

 GxFreeTrap frees the memory that was allocated for the data of a trap in
 the project.

 Arguments
 ---------
 trap - the trap data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeTrap (Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;

   if (!trap) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxFreeTrapDepthArrays (trap);
      rc2 = GxFreeTrapHistoryArrays (trap);
      if (!rc) rc = rc2;
      free (trap);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeTrapDepthArrays - free the arrays for the depth data of a trap
--------------------------------------------------------------------------------

 GxFreeTrapDepthArrays frees the arrays that were allocated for the depth
 data of a trap in the project.

 Arguments
 ---------
 trap - the trap data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeTrapDepthArrays (Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (trap);

   if (trap->depthsize > 0) {
      if (trap->depth) free (trap->depth);
      if (trap->area)  free (trap->area);
   }
   trap->nslices   = 0;
   trap->depthsize = 0;
   trap->depth     = NULL;
   trap->area      = NULL;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeTrapHistoryArrays - free the arrays for the history data of a trap
--------------------------------------------------------------------------------

 GxFreeTrapHistoryArrays frees the arrays that were allocated for the history
 data of a trap in the project.

 Arguments
 ---------
 trap - the trap data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeTrapHistoryArrays (Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (trap);

   if (trap->historysize > 0) {
      if (trap->time)        free (trap->time);
      if (trap->temperature) free (trap->temperature);
      if (trap->pressure)    free (trap->pressure);
      if (trap->structvol)   free (trap->structvol);
   }
   trap->ntimes      = 0;
   trap->historysize = 0;
   trap->time        = NULL;
   trap->temperature = NULL;
   trap->pressure    = NULL;
   trap->structvol   = NULL;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetHistoryFromLocation - get history data from location into trap
--------------------------------------------------------------------------------

 GxGetHistoryFromLocation retrieves trap history data from the location
 data. The location horizon with age closest to the age of the trap is
 returned

 Arguments
 ---------
 trap     - user data for the trap
 age      - the of the reservoir to which the trap belongs
 location - the location from which the data should be taken

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetHistoryFromLocation (Trap *trap, double age,
                                      GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          horizonnr, i, ntimes;
   double       agediff, diff;

   assert (trap);
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
      ntimes  = 0;
   } else {
      ntimes = location->ntimes[horizonnr];
   }

/* Allocate arrays for the trap history data and copy the data */
   rc = GxAllocTrapHistoryArrays (ntimes, trap);
   if (!rc && (ntimes > 0)) {
      memcpy (trap->time, location->physprop[horizonnr][GX_LOCATTIME],
              ntimes * sizeof(double));
      memcpy (trap->temperature,
              location->physprop[horizonnr][GX_LOCATTEMPERATURE],
              ntimes * sizeof(double));
      memcpy (trap->pressure, location->physprop[horizonnr][GX_LOCATPRESSURE],
              ntimes * sizeof(double));

/* -- No structural volume data available: set to zero */
      for (i=0; i<ntimes; i++)
         trap->structvol[i] = 0.0;
      trap->ntimes = ntimes;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTrap - return data for a trap in the project
--------------------------------------------------------------------------------

 GxGetTrap returns the data for one of the traps in the project.

 Arguments
 ---------
 name - name of the trap
 trap - the trap returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTrap (const char *name,  Trap *trap)
{
   int          i;
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxTrap       *trapdata, searchtrapdata;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_TRAPNAME)
      || !trap) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the trap structure from the project */
         strcpy (searchtrapdata.name, name);
         trapdata = (GxTrap *) GxSearchDataItem (GX_TRAP,
                       (GxDataItem *) &searchtrapdata);
         if (!trapdata) {
            rc = GxError (GXE_TRAPNOTPRESENT, GXT_TRAPNOTPRESENT, name);
         } else {

/* -------- Fill the trap structure with data from the project */
            strcpy (trap->name,      trapdata->name);
            trap->type             = trapdata->type;
            strcpy (trap->reservoir, trapdata->horizon->name);
            strcpy (trap->crest,     trapdata->subarea->name);
            trap->formationtime    = trapdata->formationtime;
            trap->hwctemperature   = trapdata->hwctemperature;
            trap->hwcpressure      = trapdata->hwcpressure;
            trap->stoiip           = trapdata->stoiip;
            trap->giip             = trapdata->giip;
            trap->oilgravity       = trapdata->oilgravity;
            trap->gasgravity       = trapdata->gasgravity;
            for (i=0; i<GXC_MAX_CARBONNR;i++) {
               trap->gascomp[i].molpercentage =
                                 trapdata->gascomp[i].molpercentage;
               trap->gascomp[i].fractionation =
                                 trapdata->gascomp[i].fractionation;
            }

/* -------- Allocate arrays for depth and history data */
            rc = GxAllocTrapDepthArrays (trapdata->nslices, trap);
            if (!rc) rc = GxAllocTrapHistoryArrays (trapdata->ntimes, trap);

/* -------- Fill in the data from the project */
            if (!rc) rc = GxGetTrapData (trap, trapdata);

/* -------- If there is no such data get it from the location (if any) that is
   -------- linked with the subarea where the trap has its crest */
            if (!rc && (trapdata->ntimes == 0)
               && trapdata->subarea->location)
               rc = GxGetHistoryFromLocation (trap, trapdata->horizon->age,
                                              trapdata->subarea->location);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTrapData - get user data from trap data
--------------------------------------------------------------------------------

 GxGetTrapData copies the trap data from <trapdata> into the user data <trap>.

 Arguments
 ---------
 trap     - user data for the trap
 trapdata - trap data to be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTrapData (Trap *trap, const GxTrap *trapdata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (trap);
   assert (trapdata);

/* Copy the area versus depth data */
   if (trapdata->nslices > 0) {
      memcpy (trap->depth, trapdata->depth,
              trapdata->nslices * sizeof(double));
      memcpy (trap->area, trapdata->area,
              trapdata->nslices * sizeof(double));
   }

/* Copy the history data */
   if (trapdata->ntimes > 0) {
      memcpy (trap->time, trapdata->physprop[GX_TRAPTIME],
              trapdata->ntimes * sizeof(double));
      memcpy (trap->temperature, trapdata->physprop[GX_TRAPTEMPERATURE],
              trapdata->ntimes * sizeof(double));
      memcpy (trap->pressure, trapdata->physprop[GX_TRAPPRESSURE],
              trapdata->ntimes * sizeof(double));
      memcpy (trap->structvol, trapdata->physprop[GX_TRAPSTRUCTVOL],
              trapdata->ntimes*sizeof(double));
   }
   trap->nslices = trapdata->nslices;
   trap->ntimes  = trapdata->ntimes;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTrapNameList - return the trap names in the project
--------------------------------------------------------------------------------

 GxGetTrapNameList returns a list with the names of the traps present in
 the project.

 Arguments
 ---------
 traplistptr - pointer to the trap list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTrapNameList (SortedList *traplistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   SortedList   traplist;
   GxTrap       *trapdata;

   if (!traplistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create an empty sorted list for the trap names */
         traplist = CreateSortedList (GXL_TRAPNAME+1,
                    (int (*) (const void *, const void *)) strcmp);
         if (!traplist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Insert the trap names into the list */
            trapdata = (GxTrap *) GxGetFirstDataItem (GX_TRAP);
            while (trapdata && AppendToList (traplist, trapdata->name))
               trapdata = (GxTrap *) GxGetNextDataItem (GX_TRAP);
            if (trapdata) {
               DeleteSortedList (traplist);
               rc = GxError (GXE_LIST, GXT_LIST);
            } else {
               *traplistptr = traplist;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertTrap - inserts a new trap into the project
--------------------------------------------------------------------------------

 GxInsertTrap inserts information from the trap <trap> into the project.

 Arguments
 ---------
 trap - the trap to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertTrap (const Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxTrap       newtrapdata, *oldtrapdata, *trapdata;
   GxHorizon    newhorizon, *oldhorizon;
   GxSubArea    newsubarea, *oldsubarea;
   int          i, j;
   GxBool       found;

   if (!trap || (strlen (trap->name) == 0)
      || (strlen (trap->name) > GXL_TRAPNAME)
      || (strlen (trap->reservoir) == 0)
      || (strlen (trap->reservoir) > GXL_HORIZONNAME)
      || (strlen (trap->crest) == 0)
      || (strlen (trap->crest) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the trap is not yet present in the project */
         strcpy (newtrapdata.name, trap->name);
         oldtrapdata = (GxTrap *) GxSearchDataItem (GX_TRAP,
                       (GxDataItem *) &newtrapdata);
         if (oldtrapdata) {
            rc = GxError (GXE_DUPLICATETRAP, GXT_DUPLICATETRAP,
                          newtrapdata.name);
         } else {

/* -------- Check that the reservoir horizon is present */
            strcpy (newhorizon.name, trap->reservoir);
            oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                         (GxDataItem *) &newhorizon);
            if (!oldhorizon || (oldhorizon->type != GX_RESERVOIR)) {
               rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                             newhorizon.name);
            } else {

/* ----------- Check that the subarea is present */
               strcpy (newsubarea.name, trap->crest);
               oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                             (GxDataItem *) &newsubarea);
               if (!oldsubarea) {
                  rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                                newsubarea.name);
               } else {

/* -------------- Check that the top depth for the trap is specified */
                  if (trap->nslices <= 0) {
                      rc = GxError (GXE_NOTOPDEPTH, GXT_NOTOPDEPTH,
                                   trap->name);
                  } else {
                     found = GxFalse;
                     for (i=0; (i<trap->nslices)&&!found; i++)
                        if (trap->area[i] <= FLT_MIN) found = GxTrue;
                     if (!found) {
                         rc = GxError (GXE_NOTOPDEPTH, GXT_NOTOPDEPTH,
                                      trap->name);
                     } else {

/* -------------------- Maximum number of traps reached ? */
                        if (GXN_TRAP <= GxGetDataItemCount (GX_TRAP)) {
/*                             rc = GxError (GXE_TOOMANYTRAPS, GXT_TOOMANYTRAPS);*/
                         rc = GxError (GXE_NOTOPDEPTH, GXT_NOTOPDEPTH,
                                      trap->name);
                        } else {

/* ----------------------- Fill in the trap data */
                           newtrapdata.type          = trap->type;
                           newtrapdata.horizon       = oldhorizon;
                           newtrapdata.subarea       = oldsubarea;
                           newtrapdata.formationtime = trap->formationtime;
                           if (trap->type == GX_FIELD) {

/* -------------------------- Trap is a field: copy field data */
                              newtrapdata.hwctemperature =
                                 trap->hwctemperature;
                              newtrapdata.hwcpressure = trap->hwcpressure;
                              newtrapdata.stoiip      = trap->stoiip;
                              newtrapdata.giip        = trap->giip;
                              newtrapdata.oilgravity  = trap->oilgravity;
                              newtrapdata.gasgravity  = trap->gasgravity;
                              for (j=0; j<GXC_MAX_CARBONNR;j++) { 
                                newtrapdata.gascomp[j].molpercentage =trap->gascomp[j].molpercentage;
                                newtrapdata.gascomp[j].fractionation =trap->gascomp[j].fractionation;
                              }
} else {

/* -------------------------- Trap is a prospect: set field data to default */
                              newtrapdata.hwctemperature =
                                 GX_DEFAULTHWCTEMPERATURE;
                              newtrapdata.hwcpressure = GX_DEFAULTHWCPRESSURE;
                              newtrapdata.stoiip      = GX_DEFAULTSTOIIP;
                              newtrapdata.giip        = GX_DEFAULTGIIP;
                              newtrapdata.oilgravity  = GX_DEFAULTOILGRAVITY;
                              newtrapdata.gasgravity  = GX_DEFAULTGASGRAVITY;
                              for (j=0; j<GXC_MAX_CARBONNR;j++) {
                                 newtrapdata.gascomp[j].molpercentage =GX_DEFAULTMOLPERC;
                                 newtrapdata.gascomp[j].fractionation =GX_DEFAULTFRACT;
                             }
        }

/* ----------------------- Copy the depth and history data and insert trap */
                           rc = GxCreateTrapData (&newtrapdata, trap);
                           if (!rc) {
                              rc = GxInsertDataItem (GX_TRAP,
                                      (GxDataItem *) &newtrapdata,
                                      (GxDataItem **) &trapdata);
                              if (rc) (void) GxDestroyTrapData (&newtrapdata);
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertTrapDepthItem - insert a depth item into the trap
--------------------------------------------------------------------------------

 GxInsertTrapDepthItem inserts a depth item into a trap. The size of the
 arrays in the trap is changed if they are not large enough.

 Arguments
 ---------
 trap  - user data for the trap
 depth - the depth for the item to be added
 area  - the area for the item to be added

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertTrapDepthItem (Trap *trap, double depth, double area)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize, itemnr, i;
   size_t       newmemsize;
   double       *newdepth, *newarea;

   if (!trap || (depth < 0.0) || (area < 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if ((trap->nslices > 0)
         && (trap->depth[0] > trap->depth[trap->nslices-1])) {
         rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
      } else {
         if (trap->nslices == trap->depthsize) {

/*--------- Arrays are full: reallocate with a bigger size */
            newsize = GXN_DEPTHITEMSPERALLOC *
                      ((trap->nslices + 1) / GXN_DEPTHITEMSPERALLOC + 1);
            newmemsize = newsize * sizeof (double);
            if (trap->depth) {
                newdepth = (double *) realloc (trap->depth, newmemsize);
            } else {
                newdepth = (double *) malloc (newmemsize);
            }
            if (trap->area) {
                newarea  = (double *) realloc (trap->area, newmemsize);
            } else {
                newarea  = (double *) malloc (newmemsize);
            }
            if (newdepth) trap->depth = newdepth;
            if (newarea)  trap->area  = newarea;
            if (!newdepth || !newarea) {
               rc = GxError (GXE_MEMORY, GXT_MEMORY);
            } else {
               trap->depthsize = newsize;
            }
         }
         if (!rc) {

/* -------- Find the position where the item should be inserted */
            itemnr = MAX (0, GxSearchDoubleA (trap->nslices, trap->depth,
                                              depth));

/* -------- Shift items to make room for new item */
            for (i=trap->nslices-1; i>=itemnr; i--) {
               trap->depth[i+1] = trap->depth[i];
               trap->area[i+1]  = trap->area[i];
            }

/* -------- Insert the new item */
            trap->depth[itemnr] = depth;
            trap->area[itemnr]  = area;

/* -------- Increase the number of items */
            trap->nslices++;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertTrapHistoryItem - insert a history item into the trap
--------------------------------------------------------------------------------

 GxInsertTrapHistoryItem inserts a history item into a trap. The size of the
 arrays in the trap is changed if they are not large enough.

 Arguments
 ---------
 trap        - user data for the trap
 time        - the time for the item to be added
 temperature - the temperature for the item to be added
 pressure    - the pressure for the item to be added
 structvol   - the structural volume for the item to be added

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertTrapHistoryItem (Trap *trap, double time,
                                     double temperature, double pressure,
                                     double structvol)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize, itemnr, i;
   size_t       newmemsize;
   double       *newtime, *newtemperature, *newpressure, *newstructvol;

   if (!trap || (temperature < 0.0) || (pressure < 0.0)
      || (structvol < 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if ((trap->ntimes > 0)
         && (trap->time[0] < trap->time[trap->ntimes-1])) {
         rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
      } else {
         if (trap->ntimes == trap->historysize) {

/*--------- Arrays are full: reallocate with a bigger size */
            newsize = GXN_HISTITEMSPERALLOC *
                      ((trap->ntimes + 1) / GXN_HISTITEMSPERALLOC + 1);
            newmemsize = newsize * sizeof (double);
            if (trap->time) {
                newtime = (double *) realloc (trap->time, newmemsize);
            } else {
                newtime = (double *) malloc (newmemsize);
            }
            if (trap->temperature) {
                newtemperature = (double *) realloc (trap->temperature,
                                                     newmemsize);
            } else {
                newtemperature = (double *) malloc (newmemsize);
            }
            if (trap->pressure) {
                newpressure = (double *) realloc (trap->pressure, newmemsize);
            } else {
                newpressure = (double *) malloc (newmemsize);
            }
            if (trap->structvol) {
                newstructvol = (double *) realloc (trap->structvol, newmemsize);
            } else {
                newstructvol = (double *) malloc (newmemsize);
            }
            if (newtime)        trap->time        = newtime;
            if (newtemperature) trap->temperature = newtemperature;
            if (newpressure)    trap->pressure    = newpressure;
            if (newstructvol)   trap->structvol   = newstructvol;
            if (!newtime || !newtemperature || !newpressure
               || !newstructvol) {
               rc = GxError (GXE_MEMORY, GXT_MEMORY);
            } else {
               trap->historysize = newsize;
            }
         }
         if (!rc) {

/* -------- Find the position where the item should be inserted */
            itemnr = MAX (0, GxSearchDoubleD (trap->ntimes, trap->time,
                                              time));

/* -------- Shift items to make room for new item */
            for (i=trap->ntimes-1; i>=itemnr; i--) {
               trap->time[i+1]        = trap->time[i];
               trap->temperature[i+1] = trap->temperature[i];
               trap->pressure[i+1]    = trap->pressure[i];
               trap->structvol[i+1]   = trap->structvol[i];
            }

/* -------- Insert the new item */
            trap->time[itemnr]        = time;
            trap->temperature[itemnr] = temperature;
            trap->pressure[itemnr]    = pressure;
            trap->structvol[itemnr]   = structvol;

/* -------- Increase the number of items */
            trap->ntimes++;
         }
      }
   }
   return (rc);
}

/*
-------------------------------------------------------------------------------
-
 GxReadTrap - read trap from an open project file
-------------------------------------------------------------------------------
-

 GxReadTrap reads struct for one trap from an already open project
 file.

 Arguments
 ---------
 filehandle - handle of file from which instance data should be read
 trap       - trap for which data should be written
 filename   - name of the project file
 version    - version of the project file

 Return value
 ------------
 GXE_FILEREAD   token not found by scanner

-------------------------------------------------------------------------------
-
*/

GxErrorCode GxReadTrap (FILE *filehandle, GxTrap *trap, const char *filename,
                        GxPosGenexVersion version)
{
   int          i;
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (trap);
   assert (filename);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

   rc = GxGetInteger (filehandle, &trap->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, trap->name, filename);
   if (!rc) rc = GxGetInteger (filehandle, &trap->type, filename);
   if (!rc) rc = GxGetInteger (filehandle, &trap->horizonnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &trap->subareanr, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->formationtime, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->hwctemperature, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->hwcpressure, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->stoiip, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->giip, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->oilgravity, filename);
   if (!rc) rc = GxGetDouble (filehandle, &trap->gasgravity, filename);
   if (!rc) {
      switch (version) {
         case GX_VERSION_3:
            for (i=0; i<GXC_MAX_CARBONNR;i++) {
               trap->gascomp[i].molpercentage = GX_DEFAULTMOLPERC;
               trap->gascomp[i].fractionation = GX_DEFAULTFRACT;
            }
            break;
         case GX_VERSION_4:
            for (i=0; i<GXC_MAX_CARBONNR && !rc;i++) {
               rc = GxGetDouble (filehandle, &trap->gascomp[i].molpercentage,
                                 filename);
               if (!rc) {
                   rc = GxGetDouble (filehandle,
                                     &trap->gascomp[i].fractionation, filename);
               }
            }
         break;
      }
   }

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReadTrapData - read trap data from an open project file
--------------------------------------------------------------------------------

 GxReadTrapData reads a trap data item from an already open project
 file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 filename   - the name of the file
 trapdata   - trapdata to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadTrapData (FILE *filehandle, const char *filename,
                            GxTrap *trapdata)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          nslices, ntimes, i, j;

   assert (filehandle);
   assert (filename);
   assert (trapdata);


/* Read the number of depth items */
   rc = GxGetInteger (filehandle, &nslices, filename);
   if (!rc) {

/* -- Read the number of history items */
      rc = GxGetInteger (filehandle, &ntimes, filename);
      if (!rc) {
         if ((nslices < 0) || (ntimes < 0)) {
            rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
         } else {

/* -------- Allocate arrays big enough to hold the data */
            rc = GxAllocTrapData (nslices, ntimes, trapdata);
            if (!rc && (nslices > 0)) {

/* ----------- Read in the area versus depth data */
	       for (i=0; (!rc)&&(i<nslices); i++) 
                  rc = GxGetDouble (filehandle, &trapdata->depth[i], filename);

	       for (i=0; (!rc)&&(i<nslices); i++) 
                  rc = GxGetDouble (filehandle, &trapdata->area[i], filename);
            }

/* -------- Read in the history data row by row */
            if (!rc && (ntimes > 0))
               for (i=0; (!rc)&&(i<ntimes); i++)
                  for (j=0;(!rc) && j<GXN_TRAPPHYSPROP; j++)
                     rc = GxGetDouble (filehandle, &trapdata->physprop[j][i],
								filename);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxRenameTrap - change the name of a trap
--------------------------------------------------------------------------------

 GxRenameTrap changes the name of a trap in the project. If the new name
 is already a name of another trap an error occurs.

 Arguments
 ---------
 oldname - name of the trap to be renamed
 newname - new name to be given to the trap

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameTrap (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxTrap       *trap, *oldtrap, searchtrap;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_TRAPNAME) || !newname
      || (strlen (newname) == 0) || (strlen (newname) > GXL_TRAPNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no trap with the new name is present in project */
         strcpy (searchtrap.name, newname);
         oldtrap = (GxTrap *) GxSearchDataItem (GX_TRAP,
                                                (GxDataItem *) &searchtrap);
         if (oldtrap) {
            rc = GxError (GXE_DUPLICATETRAP, GXT_DUPLICATETRAP, newname);
         } else {

/* -------- Check that the trap to be renamed is present */
            strcpy (searchtrap.name, oldname);
            trap = (GxTrap *) GxSearchDataItem (GX_TRAP,
                                               (GxDataItem *) &searchtrap);
            if (!trap) {
               rc = GxError (GXE_TRAPNOTPRESENT, GXT_TRAPNOTPRESENT, oldname);
            } else {

/* ----------- Change the name and adjust the position of the trap */
               strcpy (trap->name, newname);
               GxRepositionDataItem (GX_TRAP);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceTrap - replaces a trap in the project
--------------------------------------------------------------------------------

 GxReplaceTrap replaces information for a trap with new information.

 Arguments
 ---------
 trap - the trap to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceTrap (const Trap *trap)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxTrap       *trapdata, searchtrapdata;
   GxHorizon    newhorizon, *oldhorizon;
   GxSubArea    newsubarea, *oldsubarea;
   int          i;
   GxBool       found;

   if (!trap || (strlen (trap->name) == 0)
      || (strlen (trap->name) > GXL_TRAPNAME)
      || (strlen (trap->reservoir) == 0)
      || (strlen (trap->reservoir) > GXL_HORIZONNAME)
      || (strlen (trap->crest) == 0)
      || (strlen (trap->crest) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the trap to be replaced is present */
         strcpy (searchtrapdata.name, trap->name);
         trapdata = (GxTrap *) GxSearchDataItem (GX_TRAP,
                    (GxDataItem *) &searchtrapdata);
         if (!trapdata) {
            rc = GxError (GXE_TRAPNOTPRESENT, GXT_TRAPNOTPRESENT,
                          searchtrapdata.name);
         } else {

/* -------- Check that the new reservoir horizon is present */
            strcpy (newhorizon.name, trap->reservoir);
            oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                         (GxDataItem *) &newhorizon);
            if (!oldhorizon || (oldhorizon->type != GX_RESERVOIR)) {
               rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                             newhorizon.name);
            } else {

/* ----------- Check that the new subarea is present */
               strcpy (newsubarea.name, trap->crest);
               oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                             (GxDataItem *) &newsubarea);
               if (!oldsubarea) {
                  rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                                 newsubarea.name);
               } else {

/* -------------- Check that the top depth of the trap is specified */
                  if (trap->nslices <= 0) {
                      rc = GxError (GXE_NOTOPDEPTH, GXT_NOTOPDEPTH,
                                   trap->name);
                  } else {
                     found = GxFalse;
                     for (i=0; (i<trap->nslices)&&!found; i++)
                        if (trap->area[i] <= FLT_MIN) found = GxTrue;
                     if (!found) {
                        rc = GxError (GXE_NOTOPDEPTH, GXT_NOTOPDEPTH,
                                      trap->name);
                     } else {

/* -------------------- Replace the old trap data */
                        trapdata->type          = trap->type;
                        trapdata->horizon       = oldhorizon;
                        trapdata->subarea       = oldsubarea;
                        trapdata->formationtime = trap->formationtime;
                        if (trap->type == GX_FIELD) {

/* ----------------------- Trap is a field: replace the field data */
                           trapdata->hwctemperature = trap->hwctemperature;
                           trapdata->hwcpressure    = trap->hwcpressure;
                           trapdata->stoiip         = trap->stoiip;
                           trapdata->giip           = trap->giip;
                           trapdata->oilgravity     = trap->oilgravity;
                           trapdata->gasgravity     = trap->gasgravity;
                           for (i=0;i<GXC_MAX_CARBONNR;i++) {
                              trapdata->gascomp[i].molpercentage =
                                               trap->gascomp[i].molpercentage;
                              trapdata->gascomp[i].fractionation =
                                               trap->gascomp[i].fractionation;
                           }
                        } else {

/* ----------------------- Trap is a prospect: set field data to default */
                           trapdata->hwctemperature =
                              GX_DEFAULTHWCTEMPERATURE;
                           trapdata->hwcpressure    = GX_DEFAULTHWCPRESSURE;
                           trapdata->stoiip         = GX_DEFAULTSTOIIP;
                           trapdata->giip           = GX_DEFAULTGIIP;
                           trapdata->oilgravity     = GX_DEFAULTOILGRAVITY;
                           trapdata->gasgravity     = GX_DEFAULTGASGRAVITY;
                           for (i=0;i<GXC_MAX_CARBONNR;i++) {
                              trapdata->gascomp[i].molpercentage =
                                                      GX_DEFAULTMOLPERC; 
                              trapdata->gascomp[i].fractionation =
                                                      GX_DEFAULTFRACT; 
                           }
                        }

/* -------------------- Delete old depth and history data and insert new */
                        rc = GxDestroyTrapData (trapdata);
                        if (!rc) rc = GxCreateTrapData (trapdata, trap);

/* -------------------- Notify project that its data has changed */
                        GxChangeProject ();
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteTrap - write trap to an open project file
--------------------------------------------------------------------------------

 GxWriteTrap writes struct for one trap to an already open project
 file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 trap       - trap for which data should be written
 version    - version format to save this trap

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteTrap (FILE *filehandle, const GxTrap *trap, 
                  GxPosGenexVersion version)
{
   int  i;

   assert (filehandle);
   assert (trap);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

   fprintf (filehandle, "%d ", trap->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %d ", trap->name, trap->type);
   fprintf (filehandle, "%d %d ", trap->horizonnr, trap->subareanr);
   fprintf (filehandle, "%f ", trap->formationtime);
   fprintf (filehandle, "%f ", trap->hwctemperature);
   fprintf (filehandle, "%f ", trap->hwcpressure);
   fprintf (filehandle, "%f %f ", trap->stoiip, trap->giip);
   fprintf (filehandle, "%f %f ", trap->oilgravity, trap->gasgravity);
   if (version == GX_VERSION_4) {
      for (i=0;i<GXC_MAX_CARBONNR;i++) {
         fprintf (filehandle, "%f %f ", trap->gascomp[i].molpercentage,
                                        trap->gascomp[i].fractionation);
      }
   }
   fprintf (filehandle, "\n");

}


/*
--------------------------------------------------------------------------------
 GxWriteTrapData - write trap data to an open project file
--------------------------------------------------------------------------------

 GxWriteTrapData writes data for one trap to an already open project
 file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 trapdata   - trap for which data should be written

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteTrapData (FILE *filehandle, const GxTrap *trapdata)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;

   assert (filehandle);
   assert (trapdata);

/* Write the size of the depth data */
   fprintf (filehandle, "%d %d\n", trapdata->nslices, trapdata->ntimes);
   if (trapdata->nslices > 0) {
      for (i=0; i<trapdata->nslices; i++) {
         fprintf (filehandle, "%f ", trapdata->depth[i]);
      }
      fprintf (filehandle, "\n");
      for (i=0; i<trapdata->nslices; i++) {
         fprintf (filehandle, "%f ", trapdata->area[i]);
      }
      fprintf (filehandle, "\n");

      if (trapdata->ntimes > 0) {
          for (i=0; i<trapdata->ntimes; i++) {
	     for (j=0; j<GXN_TRAPPHYSPROP; j++) {
		fprintf (filehandle, "%f ", trapdata->physprop[j][i]);
	     }
	     fprintf (filehandle, "\n");
	 }
      }
   }
   return (rc);
}
