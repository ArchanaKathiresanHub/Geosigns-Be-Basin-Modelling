/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxsbarea.c,v 25.0 2005/07/05 08:03:50 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxsbarea.c,v $
 * Revision 25.0  2005/07/05 08:03:50  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:01  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
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
 * Revision 22.0  2002/06/28 12:09:36  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:29  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:46  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:03  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:22  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:23  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:24  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:00  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:54  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:30  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:19  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:38  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:51  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxSbArea.c   Genex subarea functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxSbArea.c contains the Genex functions for handling subareas.
              The following functions are available:

                GxCreateSubArea      - create a new subarea structure
                GxDeleteSubArea      - remove a subarea from the project
                GxFreeSubArea        - free memory for subarea contents
                GxGetSubArea         - return contents of one subarea
                GxGetSubAreaNameList - return subarea names in project
                GxInsertSubArea      - inserts a new subarea in project
		GxReadSubArea        - read a subarea from a project
                GxRenameSubArea      - change the name of a subarea
                GxReplaceSubArea     - replaces a subarea in project
	        GxWriteSubArea       - writes a subarea to the projectfile

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxsbarea.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteSubArea added
 28-MAR-1993  M. van Houtert GxReadSubArea added
 04-DEC-1993  Th. Berkers    When deleting an item form a list the next item
                             item in this list will afterwards be the 
                             current. When using next item function one item
                             in the list will be skipped which can cause 
                             a system crash after saving and reloading the
                             project file (Changes in GxDeleteSubArea)

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdlib.h>
#include  <string.h>
#include  <assert.h>
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxprjct.h"
#include  "gxsbarea.h"

/* Macro definitions */
/* ----------------- */
/* -- error text definitions -- */
#define  GXT_DUPLICATESUBAREA   "Subarea %s is already present in project.\n"
#define  GXT_LOCATIONLINKED     "Location %s is already linked to " \
                                "subarea %s."
#define  GXT_SUBAREAREFERRED    "Subarea %s is referred to by another " \
                                "data unit.\n"
/* -- default values -- */
#define  GX_DEFAULTAREA          100000000.0
#define  GX_DEFAULTEASTING       0.0
#define  GX_DEFAULTNORTHING      0.0



/*
--------------------------------------------------------------------------------
 GxCreateSubArea - create a new subarea structure
--------------------------------------------------------------------------------

 GxCreateSubArea allocates memory for a new subarea structure and
 initializes that memory.

 Arguments
 ---------
 subareaptr - pointer for the subarea

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateSubArea (SubArea **subareaptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   SubArea      *subarea;

   if (!subareaptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      subarea = ( SubArea * )malloc (sizeof (SubArea));
      if (!subarea) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         subarea->name[0]       = '\0';
         subarea->locationId[0] = '\0';
         subarea->easting       = GX_DEFAULTEASTING;
         subarea->northing      = GX_DEFAULTNORTHING;
         subarea->area          = GX_DEFAULTAREA;
         *subareaptr            = subarea;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteSubArea - remove a subarea from the project
--------------------------------------------------------------------------------

 GxDeleteSubArea removes the subarea <name> from the project. If the subarea
 is referred to by another data unit an error will occur and the subarea will
 not be removed.

 Arguments
 ---------
 name - name of the subarea to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteSubArea (const char *name)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxBool              initialized;
   GxSubArea           *oldsubarea, searchsubarea;
   GxHorizonHistory    *history;
   GxReservoirSubArea  *resarea;
   GxRock              *rock;
   GxTrap              *trap;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the subarea structure from project */
         strcpy (searchsubarea.name, name);
         oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                      (GxDataItem *) &searchsubarea);
         if (!oldsubarea) {
            rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT, name);
         } else {

/* -------- Check that there are no traps with a reference to this subarea */
            trap = (GxTrap *) GxGetFirstDataItem (GX_TRAP);
            while (trap && (trap->subarea != oldsubarea))
               trap = (GxTrap *) GxGetNextDataItem (GX_TRAP);

            if (trap) {
               rc = GxError (GXE_SUBAREAREFERRED, GXT_SUBAREAREFERRED, name);
            } else {

/* ----------- Check that the optional link with a location is intact */
               if (oldsubarea->location
                  && (oldsubarea->location->subarea != oldsubarea)) {
                    rc = GxError (GXE_LINK, GXT_LINK);
               } else {

/* -------------- Remove any reservoir subarea that refer to this subarea */
                  resarea = (GxReservoirSubArea *)
                            GxGetFirstDataItem (GX_RESERVOIRSUBAREA);
                  while (resarea) {
                      if (resarea->subarea == oldsubarea) {
                          GxRemoveDataItem (GX_RESERVOIRSUBAREA);
                          resarea = (GxReservoirSubArea *)
                                    GxGetCurrentDataItem (GX_RESERVOIRSUBAREA);
                      } else {
                          resarea = (GxReservoirSubArea *)
                                    GxGetNextDataItem (GX_RESERVOIRSUBAREA);
                      }
                  }


/* -------------- Remove any rock structures that refer to this subarea */
                  rock = (GxRock *) GxGetFirstDataItem (GX_ROCK);
                  while (rock) {
                      if (rock->subarea == oldsubarea) {
                          GxRemoveDataItem (GX_ROCK);
                          rock = (GxRock *) GxGetCurrentDataItem (GX_ROCK);
                      } else {
                          rock = (GxRock *) GxGetNextDataItem (GX_ROCK);
                      }
                  }


/* -------------- Remove any horizon histories that refer to this subarea */
                  history = (GxHorizonHistory *)
                            GxGetFirstDataItem (GX_HORIZONHISTORY);
                   while (!rc && history) {
                      if (history->subarea == oldsubarea) {
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

/* ----------------- Break the link with the location (if any) */
                     if (oldsubarea->location)
                        oldsubarea->location->subarea = NULL;

/* ----------------- Remove the subarea from the project */
                     GxRemoveDataItem (GX_SUBAREA);
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
 GxFreeSubArea - free the memory for the contents of a subarea
--------------------------------------------------------------------------------

 GxFreeSubArea frees the memory that was allocated for the contents of
 one subarea in the project.

 Arguments
 ---------
 subarea - the subarea contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeSubArea (SubArea *subarea)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!subarea) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (subarea);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetSubArea - return contents of one subarea in the project
--------------------------------------------------------------------------------

 GxGetSubArea returns the contents of one of the subareas in the project.

 Arguments
 ---------
 name    - name of the subarea
 subarea - subarea to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSubArea (const char *name, SubArea *subarea)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxSubArea    *oldsubarea, searchsubarea;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_SUBAREANAME)
      || !subarea) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the subarea structure from the project */
         strcpy (searchsubarea.name, name);
         oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                      (GxDataItem *) &searchsubarea);
         if (!oldsubarea) {
            rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT, name);
         } else {

/* -------- Copy the subarea data */
            strcpy (subarea->name, oldsubarea->name);
            if (oldsubarea->location) {
               strcpy (subarea->locationId, oldsubarea->location->name);
            } else {
               subarea->locationId[0] = '\0';
            }
            subarea->easting  = oldsubarea->easting;
            subarea->northing = oldsubarea->northing;
            subarea->area     = oldsubarea->area;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetSubAreaNameList - return the subarea names in the project
--------------------------------------------------------------------------------

 GxGetSubAreaNameList returns a list with the names of the subareas
 present in the project.

 Arguments
 ---------
 subarealistptr - pointer to the subarea list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSubAreaNameList (SortedList *subarealistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   SortedList   subarealist;
   GxSubArea    *subarea;

   if (!subarealistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create an empty sorted list for the subarea names */
         subarealist = CreateSortedList (GXL_SUBAREANAME+1,
                       (int (*) (const void *, const void *)) strcmp);
         if (!subarealist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Insert the subarea names into the list */
            subarea = (GxSubArea *) GxGetFirstDataItem (GX_SUBAREA);
            while (subarea && AppendToList (subarealist, subarea->name))
               subarea = (GxSubArea *) GxGetNextDataItem (GX_SUBAREA);
            if (subarea) {
               DeleteSortedList (subarealist);
               rc = GxError (GXE_LIST, GXT_LIST);
            } else {
               *subarealistptr = subarealist;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertSubArea - inserts a new subarea into the project
--------------------------------------------------------------------------------

 GxInsertSubArea inserts information from the subarea <subarea> into
 the project.

 Arguments
 ---------
 subarea - the subarea to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertSubArea (const SubArea *subarea)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxSubArea    newsubarea, *oldsubarea, *insertedsubarea;
   GxLocation   newloc, *oldloc;

   if (!subarea || (strlen (subarea->name) == 0)
      || (strlen (subarea->name) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Fill in the new subarea structure */
         strcpy (newsubarea.name, subarea->name);
         newsubarea.easting  = subarea->easting;
         newsubarea.northing = subarea->northing;
         newsubarea.area     = subarea->area;

/* ----- Check that the subarea is not yet present in the project */
         oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                      (GxDataItem *) &newsubarea);
         if (oldsubarea) {
            rc = GxError (GXE_DUPLICATESUBAREA, GXT_DUPLICATESUBAREA,
                          newsubarea.name);
         } else {

/* -------- Find the location the subarea refers to (if any) */
            if (strlen (subarea->locationId) == 0) {
               oldloc = NULL;
            } else {
               strcpy (newloc.name, subarea->locationId);
               oldloc = (GxLocation *) GxSearchDataItem (GX_LOCATION,
                                                       (GxDataItem *) &newloc);
               if (!oldloc) {
                  rc = GxError (GXE_LOCATIONNOTPRESENT,
                                GXT_LOCATIONNOTPRESENT, newloc.name);
               } else {

/* -------------- Check that it is not already linked */
                  if (oldloc->subarea) {
                     rc = GxError (GXE_LOCATIONLINKED, GXT_LOCATIONLINKED,
                                   newloc.name, oldloc->subarea->name);
                  }
               }
            }
            if (!rc) {

/* ----------- Insert the subarea into the project */
               rc = GxInsertDataItem (GX_SUBAREA, (GxDataItem *) &newsubarea,
                                      (GxDataItem **) &insertedsubarea);
            }
            if (!rc) {

/* ----------- Set up the link with the location (if any) */
               insertedsubarea->location = oldloc;
               if (oldloc) oldloc->subarea = insertedsubarea;
            }
         }
      }
   }
   return (rc);
}

/*
-------------------------------------------------------------------------------
-
 GxReadSubArea - read a subarea struct from the project file
-------------------------------------------------------------------------------
-

 GxReadSubArea reads a subarea structure from an ascii project file

 Arguments
 ---------
 filehandle - handle of file from which instance data should be read
 area       - The subarea structure
 filename   - the project filename

 Return value
 ------------
 GXE_FILEREAD  token not found by scanner

-------------------------------------------------------------------------------
-
*/

GxErrorCode GxReadSubArea (FILE *filehandle, GxSubArea *area, 
							const char *filename)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (area);
   assert (filename);
   
   rc = GxGetInteger (filehandle, &area->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, area->name, filename);
   if (!rc) rc = GxGetInteger (filehandle, &area->locationnr, filename);
   if (!rc) rc = GxGetDouble (filehandle, &area->easting, filename);
   if (!rc) rc = GxGetDouble (filehandle, &area->northing, filename);
   if (!rc) rc = GxGetDouble (filehandle, &area->area, filename);

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxRenameSubArea - change the name of a subarea
--------------------------------------------------------------------------------

 GxRenameSubArea changes the name of a subarea in the project.
 If the new name is already a name of another subarea an error occurs.

 Arguments
 ---------
 oldname - name of the subarea to be renamed
 newname - new name to be given to the subarea

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameSubArea (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxSubArea    *subarea, *oldsubarea, searchsubarea;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_SUBAREANAME) || !newname
      || (strlen (newname) == 0) || (strlen (newname) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no subarea with the new name is present in project */
         strcpy (searchsubarea.name, newname);
         oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                                             (GxDataItem *) &searchsubarea);
         if (oldsubarea) {
            rc = GxError (GXE_DUPLICATESUBAREA, GXT_DUPLICATESUBAREA,
                          newname);
         } else {

/* -------- Check that the subarea to be renamed is present */
            strcpy (searchsubarea.name, oldname);
            subarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                                            (GxDataItem *) &searchsubarea);
            if (!subarea) {
               rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                             oldname);
            } else {

/* ----------- Change the name and adjust the position of the subarea */
               strcpy (subarea->name, newname);
               GxRepositionDataItem (GX_SUBAREA);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceSubArea - replaces a subarea in the project
--------------------------------------------------------------------------------

 GxReplaceSubArea replaces information from a subarea with new information.

 Arguments
 ---------
 subarea - the subarea to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceSubArea (const SubArea *subarea)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxSubArea    *oldsubarea, searchsubarea;
   GxLocation   newloc, *oldloc;

   if (!subarea || (strlen (subarea->name) == 0)
      || (strlen (subarea->name) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the subarea to be replaced is present */
         strcpy (searchsubarea.name, subarea->name);
         oldsubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                      (GxDataItem *) &searchsubarea);
         if (!oldsubarea) {
            rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                          searchsubarea.name);
         } else {

/* -------- Find the new location the subarea refers to (if any) */
            if (strlen (subarea->locationId) == 0) {
               oldloc = NULL;
            } else {
               strcpy (newloc.name, subarea->locationId);
               oldloc = (GxLocation *) GxSearchDataItem (GX_LOCATION,
                                                      (GxDataItem *) &newloc);
               if (!oldloc) {
                  rc = GxError (GXE_LOCATIONNOTPRESENT,
                                GXT_LOCATIONNOTPRESENT, newloc.name);
               } else {

/* -------------- Check that it is not already linked to another subarea */
                  if (oldloc->subarea && (oldloc->subarea != oldsubarea))
                     rc = GxError (GXE_LOCATIONLINKED, GXT_LOCATIONLINKED,
                                   newloc.name, oldloc->subarea->name);
               }
            }
            if (!rc) {

/* ----------- Check the existing link between the subarea and a location */
               if (oldsubarea->location
                  && (oldsubarea->location->subarea != oldsubarea)) {
                  rc = GxError (GXE_LINK, GXT_LINK);
               } else {

/* -------------- Change the link between the subarea ans a location */
                  if (oldsubarea->location)
                     oldsubarea->location->subarea = NULL;
                  oldsubarea->location = oldloc;
                  if (oldloc) oldloc->subarea = oldsubarea;

/* -------------- Change the other data of the subarea */
                  oldsubarea->easting  = subarea->easting;
                  oldsubarea->northing = subarea->northing;
                  oldsubarea->area     = subarea->area;

/* -------------- Notify the project that it has changed */
                  GxChangeProject ();
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxWriteSubArea - write a subarea struct to the project file
--------------------------------------------------------------------------------

 GxWriteSubArea writes a subarea structure in an ascii project file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 area       - The subarea structure

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteSubArea (FILE *filehandle, const GxSubArea *area)
{
   assert (filehandle);
   assert (area);

   fprintf (filehandle, "%d ", area->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %d ", area->name, area->locationnr);
   fprintf (filehandle, "%f %f ", area->easting, area->northing);
   fprintf (filehandle, "%f\n",   area->area);
}

