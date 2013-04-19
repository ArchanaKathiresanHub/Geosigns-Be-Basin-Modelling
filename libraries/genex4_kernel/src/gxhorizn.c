/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxhorizn.c,v 25.0 2005/07/05 08:03:45 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxhorizn.c,v $
 * Revision 25.0  2005/07/05 08:03:45  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:55  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:25  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:09  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:41  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:18  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:18  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:30  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:38  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:07  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:53  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:08  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:35  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:06  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:47  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:54  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:06  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:59  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxHorizn.c   Genex horizon functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        22-JUN-1992

 Description: GxHorizn.c contains the Genex functions for handling horizons.
              The following functions are available:

                GxCreateRVHorizon      - create a reservoir horizon structure
                GxCreateSRHorizon      - create a new rock horizon structure
                GxDeleteRVHorizon      - remove reservoir horizon from project
                GxDeleteSRHorizon      - remove rock horizon from the project
                GxFreeRVHorizon        - free reservoir horizon contents
                GxFreeSRHorizon        - free memory for rock horizon contents
                GxGetRVHorizon         - return contents of reservoir horizon
                GxGetRVHorizonNameList - return reservoir horizon names
                GxGetSRHorizon         - return contents of one rock horizon
                GxGetSRHorizonNameList - return rock horizon names in project
                GxInsertRVHorizon      - inserts a reservoir horizon in project
                GxInsertSRHorizon      - inserts a new rock horizon in project
                GxReadHorizon          - read the horizon struct from file
		GxReadHorizonHistory   - read the horizon data from file
                GxRenameRVHorizon      - change name of a reservoir horizon
                GxRenameSRHorizon      - change name of a source rock horizon
                GxReplaceRVHorizon     - replaces reservoir horizon in project
                GxReplaceSRHorizon     - replaces a rock horizon in project
                GxWriteHorizon         - Write horizon data in project
                GxWriteHorizonHistory  - Write horizon data in project

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxhorizn.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 22-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M.J.A. van Houtert GxWriteHorizon added
 28-MAR-1993  M.J.A. van Houtert GxReadHorizon added
 04-DEC-1993  Th. Berkers        When deleting an item form a list the next item
                                 item in this list will afterwards be the 
                                 current. When using next item function one item
                                 in the list will be skipped which can cause 
                                 a system crash after saving and reloading the
                                 project file (Changes in GxDeleteRVHorizon and
                                 GxDeleteSRHorizon)
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxhorizn.h"
#include  "gxprjct.h"
#include  "sortlst.h"

/* Macro definitions */
/* ----------------- */
/* -- error text definitions -- */
#define  GXT_DUPLICATEHORIZON   "Horizon %s is already present " \
                                "in project.\n"
#define  GXT_RVHORIZONREFERRED  "Reservoir horizon %s is referred to " \
                                "by another data unit.\n"
/* -- default values -- */
#define  GX_DEFAULTAGE          150.0

/* Function prototypes */
/* ------------------- */
static int          GxCmpHorizon (const void *horizon1,
                                  const void *horizon2);
static GxErrorCode  GxGetHorizonNameList (List *, HorizonType);



/*
--------------------------------------------------------------------------------
 GxCmpHorizon - compare two horizons for sorting
--------------------------------------------------------------------------------

 GxCmpHorizon compares two horizons ages with each other in order to sort
 them in a sorted list.

 Arguments
 ---------
 horizon1 - first horizon to be compared
 horizon2 - second horizon to be compared

 Return value
 ------------
 a negative value if <horizon1> is less than <horizon2>, 0 if <horizon1> is
 equal to <horizon2>, or a positive value if <horizon1> is greater than
 <horizon2>

--------------------------------------------------------------------------------
*/

static int GxCmpHorizon (const void *horizon1, const void *horizon2)
{
   int  cmp;

   assert (horizon1);
   assert (horizon2);

   if (((GxHorizon *) horizon1)->age < ((GxHorizon *) horizon2)->age) {
      cmp = -1;
   } else if (((GxHorizon *) horizon1)->age > ((GxHorizon *) horizon2)->age){
      cmp = 1;
   } else {
      cmp = strcmp (((GxHorizon *) horizon1)->name,
                    ((GxHorizon *) horizon2)->name);
   }
   return (cmp);
}



/*
--------------------------------------------------------------------------------
 GxCreateRVHorizon - create a new reservoir horizon structure
--------------------------------------------------------------------------------

 GxCreateRVHorizon allocates memory for a new reservoir horizon structure
 and initializes that memory.

 Arguments
 ---------
 rvhorizonptr - pointer for the reservoir horizon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateRVHorizon (RVHorizon **rvhorizonptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   RVHorizon    *rvhorizon;

   if (!rvhorizonptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rvhorizon = (RVHorizon *) malloc (sizeof (RVHorizon));
      if (!rvhorizon) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         rvhorizon->name[0] = '\0';
         rvhorizon->age     = GX_DEFAULTAGE;
         *rvhorizonptr      = rvhorizon;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateSRHorizon - create a new source rock horizon structure
--------------------------------------------------------------------------------

 GxCreateSRHorizon allocates memory for a new source rock horizon structure
 and initializes that memory.

 Arguments
 ---------
 srhorizonptr - pointer for the source rock horizon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateSRHorizon (SRHorizon **srhorizonptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   SRHorizon    *srhorizon;

   if (!srhorizonptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      srhorizon = (SRHorizon *) malloc (sizeof (SRHorizon));
      if (!srhorizon) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         srhorizon->name[0] = '\0';
         srhorizon->age     = GX_DEFAULTAGE;
         *srhorizonptr      = srhorizon;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteRVHorizon - remove a reservoir horizon from the project
--------------------------------------------------------------------------------

 GxDeleteRVHorizon removes the reservoir horizon <name> from the project.
 If the reservoir horizon is referred to by another data unit an error will
 occur and the reservoir horizon will not be removed.

 Arguments
 ---------
 name - name of the reservoir horizon to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteRVHorizon (const char *name)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxBool              initialized;
   GxHorizon           *oldhorizon, searchhorizon;
   GxReservoirSubArea  *resarea;
   GxTrap              *trap;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the reservoir horizon structure from project */
         strcpy (searchhorizon.name, name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_RESERVOIR)) {
            rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                          name);
         } else {

/* -------- Check that there are no traps with a reference to this horizon */
            trap = (GxTrap *) GxGetFirstDataItem (GX_TRAP);
            while (trap && (trap->horizon != oldhorizon))
               trap = (GxTrap *) GxGetNextDataItem (GX_TRAP);

            if (trap) {
               rc = GxError (GXE_RVHORIZONREFERRED, GXT_RVHORIZONREFERRED,
                             name);
            } else {

/* ----------- Remove any reservoir subareas that refer to this horizon */
               resarea = (GxReservoirSubArea *)
                         GxGetFirstDataItem (GX_RESERVOIRSUBAREA);
               while (resarea) {
                   if (resarea->horizon == oldhorizon) {
                       GxRemoveDataItem (GX_RESERVOIRSUBAREA);
                       resarea = (GxReservoirSubArea *)
                                 GxGetCurrentDataItem (GX_RESERVOIRSUBAREA);
                   } else {
                       resarea = (GxReservoirSubArea *)
                                 GxGetNextDataItem (GX_RESERVOIRSUBAREA);
                   }
               }

/* ----------- Remove the reservoir horizon from the project */
               GxRemoveDataItem (GX_HORIZON);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteSRHorizon - remove a source rock horizon from the project
--------------------------------------------------------------------------------

 GxDeleteSRHorizon removes the source rock horizon <name> from the project.
 If the source rock horizon is referred to by another data unit an error will
 occur and the source rock horizon will not be removed.

 Arguments
 ---------
 name - name of the source rock horizon to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteSRHorizon (const char *name)
{
   GxErrorCode       rc=GXE_NOERROR;
   GxBool            initialized;
   GxHorizon         *oldhorizon, searchhorizon;
   GxHorizonHistory  *history;
   GxRock            *rock;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the source rock horizon structure from project */
         strcpy (searchhorizon.name, name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_SOURCEROCK)) {
            rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                          name);
         } else {

/* -------- Remove any rock structures that refer to this horizon */
            rock = (GxRock *) GxGetFirstDataItem (GX_ROCK);
            while (rock) {
                if (rock->horizon == oldhorizon) {
                    GxRemoveDataItem (GX_ROCK);
                    rock = (GxRock *) GxGetCurrentDataItem (GX_ROCK);
                } else {
                    rock = (GxRock *) GxGetNextDataItem (GX_ROCK);
                }
            }


/* -------- Remove any horizon histories that refer to this horizon */
            history = (GxHorizonHistory *)
                      GxGetFirstDataItem (GX_HORIZONHISTORY);
            while (!rc && history) {
                if (history->horizon == oldhorizon) {
                    rc = GxDestroyHorizonHistoryData (history);
                    GxRemoveDataItem (GX_HORIZONHISTORY);
                    history = (GxHorizonHistory *)
                              GxGetCurrentDataItem (GX_HORIZONHISTORY);
                } else {
                    history = (GxHorizonHistory *)
                              GxGetNextDataItem (GX_HORIZONHISTORY);
                }
            }
            if (!rc) {

/* ----------- Remove the source rock horizon from the project */
               GxRemoveDataItem (GX_HORIZON);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeRVHorizon - free the memory for the contents of a reservoir horizon
--------------------------------------------------------------------------------

 GxFreeRVHorizon frees the memory that was allocated for the contents of
 one reservoir horizon in the project.

 Arguments
 ---------
 rvhorizon - the reservoir horizon contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeRVHorizon (RVHorizon *rvhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!rvhorizon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (rvhorizon);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeSRHorizon - free the memory for the contents of a source rock horizon
--------------------------------------------------------------------------------

 GxFreeSRHorizon frees the memory that was allocated for the contents of
 one source rock horizon in the project.

 Arguments
 ---------
 srhorizon - the source rock horizon contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeSRHorizon (SRHorizon *srhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!srhorizon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (srhorizon);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetHorizonNameList - return the horizon names in the project
-------------------------------------------------------------------------------

 GxGetHorizonNameList returns a list with the names of a particular type of
 horizons present in the project.

 Arguments
 ---------
 horizonlistptr - pointer to the horizon list
 horizontype    - type of horizon requested

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetHorizonNameList (List *horizonlistptr,
                                         HorizonType horizontype)
{
   GxErrorCode  rc=GXE_NOERROR;
   List         horizonnamelist;
   SortedList   horizonnameagelist;
   GxHorizon    *horizon, *horizonptr;
   char         *nameptr;

   assert (horizonlistptr);
   assert ((horizontype == GX_SOURCEROCK) || (horizontype == GX_RESERVOIR));

/* Create an temporary sorted list for the horizons */
   horizonnameagelist = CreateSortedList (sizeof (GxHorizon), GxCmpHorizon);
   if (!horizonnameagelist) {
      rc = GxError (GXE_LIST, GXT_LIST);
   } else {

/* -- Insert the horizons into the list */
      horizon = (GxHorizon *) GxGetFirstDataItem (GX_HORIZON);
      while (horizon && !rc) {
         if (horizon->type == horizontype) {
            horizonptr = InsertInSortedList (horizonnameagelist, horizon);
            if (!horizonptr) rc = GxError (GXE_LIST, GXT_LIST);
         }
         horizon = (GxHorizon *) GxGetNextDataItem (GX_HORIZON);
      }
      if (!rc) {

/* ----- Create an empty searchable list for the horizon names */
         horizonnamelist = CreateList (GXL_HORIZONNAME+1,
                             (int (*) (const void *, const void *)) strcmp);
         if (!horizonnamelist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Append the horizon names to the list */
            horizon =
               (GxHorizon *) GetFirstFromSortedList (horizonnameagelist);
            while (horizon && !rc) {
               nameptr = AppendToList (horizonnamelist, horizon->name);
               if (!nameptr) rc = GxError (GXE_LIST, GXT_LIST);
               horizon = (GxHorizon *) GetNextFromList (horizonnameagelist);
            }
            if (rc) {
               DeleteList (horizonnamelist);
            } else {
               *horizonlistptr = horizonnamelist;
            }
         }
      }

/* -- Delete the temporary list */
      DeleteSortedList (horizonnameagelist);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRVHorizon - return contents of one reservoir horizon in the project
--------------------------------------------------------------------------------

 GxGetRVHorizon returns the contents of one of the reservoir horizons in
 the project.

 Arguments
 ---------
 name      - name of the reservoir horizon
 rvhorizon - reservoir horizon to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRVHorizon (const char *name, RVHorizon *rvhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *oldhorizon, searchhorizon;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_HORIZONNAME)
      || !rvhorizon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the horizon structure from the project */
         strcpy (searchhorizon.name, name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_RESERVOIR)) {
            rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                          name);
         } else {

/* -------- Copy the horizon data */
            strcpy (rvhorizon->name, oldhorizon->name);
            rvhorizon->age = oldhorizon->age;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRVHorizonNameList - return the reservoir horizon names in the project
-------------------------------------------------------------------------------

 GxGetRVHorizonNameList returns a list with the names of the reservoir
 horizons present in the project.

 Arguments
 ---------
 rvhorizonlistptr - pointer to the reservoir horizon list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRVHorizonNameList (List *rvhorizonlistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;

   if (!rvhorizonlistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);

/* -- Get the list */
      if (!rc) rc = GxGetHorizonNameList (rvhorizonlistptr, GX_RESERVOIR);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetSRHorizon - return contents of one source rock horizon in the project
--------------------------------------------------------------------------------

 GxGetSRHorizon returns the contents of one of the source rock horizons in
 the project.

 Arguments
 ---------
 name      - name of the source rock horizon
 srhorizon - source rock horizon to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSRHorizon (const char *name, SRHorizon *srhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *oldhorizon, searchhorizon;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_HORIZONNAME)
      || !srhorizon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the horizon structure from the project */
         strcpy (searchhorizon.name, name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_SOURCEROCK)) {
            rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                          name);
         } else {

/* -------- Copy the horizon data */
            strcpy (srhorizon->name, oldhorizon->name);
            srhorizon->age = oldhorizon->age;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetSRHorizonNameList - return the source rock horizon names in the project
-------------------------------------------------------------------------------

 GxGetSRHorizonNameList returns a list with the names of the source rock
 horizons present in the project.

 Arguments
 ---------
 srhorizonlistptr - pointer to the source rock horizon list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSRHorizonNameList (List *srhorizonlistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;

   if (!srhorizonlistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);

/* -- Get the list */
      if (!rc) rc = GxGetHorizonNameList (srhorizonlistptr, GX_SOURCEROCK);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertRVHorizon - inserts a new reservoir horizon into the project
--------------------------------------------------------------------------------

 GxInsertRVHorizon inserts information from the reservoir horizon
 <rvhorizon> into the project.

 Arguments
 ---------
 rvhorizon - the reservoir horizon to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertRVHorizon (const RVHorizon *rvhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    newhorizon, *oldhorizon, *insertedhorizon;

   if (!rvhorizon || (strlen (rvhorizon->name) == 0)
      || (strlen (rvhorizon->name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the horizon is not yet present in the project */
         strcpy (newhorizon.name, rvhorizon->name);
         newhorizon.type = GX_RESERVOIR;
         newhorizon.age  = rvhorizon->age;
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &newhorizon);
         if (oldhorizon) {
            rc = GxError (GXE_DUPLICATEHORIZON, GXT_DUPLICATEHORIZON,
                          newhorizon.name);
         } else {

/* -------- Insert the horizon into the project */
            rc = GxInsertDataItem (GX_HORIZON, (GxDataItem *) &newhorizon,
                                   (GxDataItem **) &insertedhorizon);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertSRHorizon - inserts a new source rock horizon into the project
--------------------------------------------------------------------------------

 GxInsertSRHorizon inserts information from the source rock horizon
 <srhorizon> into the project.

 Arguments
 ---------
 srhorizon - the source rock horizon to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertSRHorizon (const SRHorizon *srhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    newhorizon, *oldhorizon, *insertedhorizon;

   if (!srhorizon || (strlen (srhorizon->name) == 0)
      || (strlen (srhorizon->name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the horizon is not yet present in the project */
         strcpy (newhorizon.name, srhorizon->name);
         newhorizon.type = GX_SOURCEROCK;
         newhorizon.age  = srhorizon->age;
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &newhorizon);
         if (oldhorizon) {
            rc = GxError (GXE_DUPLICATEHORIZON, GXT_DUPLICATEHORIZON,
                          newhorizon.name);
         } else {

/* -------- Insert the horizon into the project */
            rc = GxInsertDataItem (GX_HORIZON, (GxDataItem *) &newhorizon,
                                  (GxDataItem **) &insertedhorizon);
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxReadHorizon - read horizon data from an open project file
--------------------------------------------------------------------------------

 GxReadHorizon - read a horizon data struct from an ascii file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be read
 horizon    - Struct to be read.
 filename   - name of the project file

 Return value
 ------------
 GXE_FILEREAD - token not found by scanner

--------------------------------------------------------------------------------
*/


GxErrorCode GxReadHorizon (FILE *filehandle, GxHorizon *horizon, 
			   const char *filename)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (horizon);
   assert (filename);

   rc = GxGetInteger (filehandle, &horizon->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, horizon->name, filename);
   if (!rc) rc = GxGetInteger (filehandle, &horizon->type, filename);
   if (!rc) rc = GxGetDouble (filehandle, &horizon->age, filename);

   return rc;
}

/*
--------------------------------------------------------------------------------
 GxReadHorizonHistory - read horizon history data from an open project file
--------------------------------------------------------------------------------

 GxReadHorizonHistory - read a horizon history data struct from an ascii file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be read
 horizon    - Struct to be read.
 filename   - name of the project file

 Return value
 ------------
 GXE_FILEREAD - token not found by scanner

--------------------------------------------------------------------------------
*/


GxErrorCode GxReadHorizonHistory (FILE *filehandle, GxHorizonHistory *horizon,
                           const char *filename)
{
   GxErrorCode rc=GXE_NOERROR;

   assert (filehandle);
   assert (horizon);
   assert (filename);

   rc = GxGetInteger (filehandle, &horizon->horizonnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &horizon->subareanr, filename);

   return rc;
}



/*
--------------------------------------------------------------------------------
 GxRenameRVHorizon - change the name of a reservoir horizon
--------------------------------------------------------------------------------

 GxRenameRVHorizon changes the name of a reservoir horizon in the project.
 If the new name is already a name of another horizon an error occurs.

 Arguments
 ---------
 oldname - name of the reservoir horizon to be renamed
 newname - new name to be given to the reservoir horizon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameRVHorizon (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *horizon, *oldhorizon, searchhorizon;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_HORIZONNAME) || !newname
      || (strlen (newname) == 0) || (strlen (newname) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no horizon with the new name is present in the project */
         strcpy (searchhorizon.name, newname);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                                                (GxDataItem *) &searchhorizon);
         if (oldhorizon) {
            rc = GxError (GXE_DUPLICATEHORIZON, GXT_DUPLICATEHORIZON,
                          newname);
         } else {

/* -------- Check that the reservoir horizon to be renamed is present */
            strcpy (searchhorizon.name, oldname);
            horizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
            if (!horizon || (horizon->type != GX_RESERVOIR)) {
               rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                             oldname);
            } else {

/* ----------- Change the name and adjust the position of the horizon */
               strcpy (horizon->name, newname);
               GxRepositionDataItem (GX_HORIZON);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxRenameSRHorizon - change the name of a source rock horizon
--------------------------------------------------------------------------------

 GxRenameSRHorizon changes the name of a source rock horizon in the project.
 If the new name is already a name of another horizon an error occurs.

 Arguments
 ---------
 oldname - name of the source rock horizon to be renamed
 newname - new name to be given to the source rock horizon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameSRHorizon (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *horizon, *oldhorizon, searchhorizon;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_HORIZONNAME) || !newname
      || (strlen (newname) == 0) || (strlen (newname) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no horizon with the new name is present in the project */
         strcpy (searchhorizon.name, newname);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                                                (GxDataItem *) &searchhorizon);
         if (oldhorizon) {
            rc = GxError (GXE_DUPLICATEHORIZON, GXT_DUPLICATEHORIZON,
                          newname);
         } else {

/* -------- Check that the source rock horizon to be renamed is present */
            strcpy (searchhorizon.name, oldname);
            horizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
            if (!horizon || (horizon->type != GX_SOURCEROCK)) {
               rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                             oldname);
            } else {

/* ----------- Change the name and adjust the position of the horizon */
               strcpy (horizon->name, newname);
               GxRepositionDataItem (GX_HORIZON);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceRVHorizon - replaces a reservoir horizon in the project
--------------------------------------------------------------------------------

 GxReplaceRVHorizon replaces information from a reservoir with new
 information.

 Arguments
 ---------
 rvhorizon - the reservoir horizon to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceRVHorizon (const RVHorizon *rvhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *oldhorizon, searchhorizon;

   if (!rvhorizon || (strlen (rvhorizon->name) == 0)
      || (strlen (rvhorizon->name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the reservoir horizon to be replaced is present */
         strcpy (searchhorizon.name, rvhorizon->name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_RESERVOIR)) {
            rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                          searchhorizon.name);
         } else {

/* -------- Change the data of the horizon and inform the project */
            oldhorizon->age = rvhorizon->age;
            GxChangeProject ();
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceSRHorizon - replaces a source rock horizon in the project
--------------------------------------------------------------------------------

 GxReplaceSRHorizon replaces information from a source rock with new
 information.

 Arguments
 ---------
 srhorizon - the source rock horizon to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceSRHorizon (const SRHorizon *srhorizon)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxHorizon    *oldhorizon, searchhorizon;

   if (!srhorizon || (strlen (srhorizon->name) == 0)
      || (strlen (srhorizon->name) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the source rock horizon to be replaced is present */
         strcpy (searchhorizon.name, srhorizon->name);
         oldhorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                      (GxDataItem *) &searchhorizon);
         if (!oldhorizon || (oldhorizon->type != GX_SOURCEROCK)) {
            rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                          searchhorizon.name);
         } else {

/* -------- Change the data of the horizon and inform the project */
            oldhorizon->age = srhorizon->age;
            GxChangeProject ();
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteHorizon - write horizon data to an open project file
--------------------------------------------------------------------------------

 GxWriteHorizon - write a horizon data struct in an ascii file 

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 horizon    - Struct to be saved.

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteHorizon (FILE *filehandle, const GxHorizon *horizon)
{
   assert (filehandle);
   assert (horizon);
   
   fprintf (filehandle, "%d ", horizon->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %d ", horizon->name, horizon->type);
   fprintf (filehandle, "%f\n",   horizon->age);
}

/*
--------------------------------------------------------------------------------
 GxWriteHorizonHistory - write horizon data to an open project file
--------------------------------------------------------------------------------

 GxWriteHorizonHistory - write a horizon data struct in an ascii file, the
                         actual data will not be saved here!!!

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 horizon    - Struct to be saved.

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteHorizonHistory (FILE *filehandle, const GxHorizonHistory *horizon)
{
   assert (filehandle);
   assert (horizon);

   fprintf (filehandle, "%d %d\n", horizon->horizonnr, horizon->subareanr);
}

