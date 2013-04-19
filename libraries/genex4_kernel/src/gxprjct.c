/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxprjct.c,v 25.0 2005/07/05 08:03:49 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxprjct.c,v $
 * Revision 25.0  2005/07/05 08:03:49  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:00  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:31  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:13  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:44  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:31  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:26  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:42  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:53  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:18  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:14  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:19  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:54  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:41  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:57  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:14  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:32  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:04  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxPrjct.c    Genex project functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxPrjct.c contains the Genex functions for handling projects.
              The following functions are available:

                GxBackupProject      - Save project in backup file
                GxChangeProject      - set the project to "changed"
                GxCommitProject      - commit changes to a project
                GxDeleteProject      - delete a project file from disk
                GxFreeProject        - free the project currently in memory
                GxGetDataItemCount   - return the number of data items
                GxGetFirstDataItem   - get the first of a list of data items
                GxGetNextDataItem    - get the next of a list of data items
                GxGetProjectName     - get the current projectname
                GxInitializeProject  - initialize the project data structures
                GxInsertDataItem     - inserts item into a list of data items
                GxNewProject         - create a new project in memory
                GxOpenProject        - read a project from file into memory
                GxProjectChanged     - inquire whether project is changed
                GxProjectExists      - inquire whether project file exists
                GxProjectInitialized - inquire whether data is initialized
                GxRemoveDataItem     - removes item from a list of data items
                GxRepositionDataItem - moves an item to right place in list
                GxRollbackProject    - rollback changes to a project
                GxSaveProject        - save the project in its project file
                GxSaveProjectAs      - save project in file with a new name
                GxSearchDataItem     - searches an item in list of data items
                GxSetProjectName     - set the name of the project

         To support these functions this file contains a number of
         local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxprjct.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 24-JUN-1992  P. Alphenaar  initial version
 28-JUN-1992  M.J.A. van Houtert  Backup routine added
  1-JUL-1992  M.J.A. van Houtert  Get projectname routine added
  4-DEC-1993  Th. Berkers         GxGetCurrentDataItem added
 20-MAR-1995  Th. Berkers         Integration of Isotopes Fractionation:
                                  Genex version in project file checked
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxerror.h"
#include  "gxfile.h"
#include  "gxinstnc.h"
#include  "gxprjct.h"
#include  "gxscanner.h"
#include  "gxsysdep.h"
#include  "sortlst.h"

/* Macro definitions */
/* ----------------- */
#define  GX_DEFAULTPROJECTNAME  "noname.gnx"
#define  GX_BACKUPPROJECTNAME   "genex.bak"
/* -- error text definitions -- */
#define  GXT_FILENOTEXIST       "The project file %s does not exist.\n"
#define  GXT_NOPROJECTNAME      "The project has no name.\n"

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef struct Project  Project;
struct Project {
   GxBool      initialized, currentchanged, backupchanged;
   char        *name;
   size_t      unitsize[GXN_DATAUNITS];
   int         (*compare[GXN_DATAUNITS]) (const void *, const void *);
   SortedList  current[GXN_DATAUNITS], backup[GXN_DATAUNITS];
};

/* Function prototypes */
/* ------------------- */
static int  GxCmpFile (const void *, const void *);
static int  GxCmpHorizon (const void *, const void *);
static int  GxCmpHorizonHistory (const void *, const void *);
static int  GxCmpLithology (const void *, const void *);
static int  GxCmpLocation (const void *, const void *);
static int  GxCmpReservoirSubArea (const void *, const void *);
static int  GxCmpRock (const void *, const void *);
static int  GxCmpSubArea (const void *, const void *);
static int  GxCmpTrap (const void *, const void *);

/* Global variables */
/* ---------------- */
/* -- file scope variables -- */
static Project  theProject={
   GxFalse, GxFalse, GxFalse, NULL,
   {sizeof (GxFile), sizeof (GxHorizon), sizeof (GxHorizonHistory),
    sizeof (GxLabExperiment), sizeof (GxLithology), sizeof (GxLocation),
    sizeof (GxRefConditions), sizeof (GxReservoirSubArea), sizeof (GxRock),
    sizeof (GxRunOptions), sizeof (GxSubArea), sizeof (GxTrap)},
   {GxCmpFile, GxCmpHorizon, GxCmpHorizonHistory, NULL, GxCmpLithology,
    GxCmpLocation, NULL, GxCmpReservoirSubArea, GxCmpRock, NULL,
    GxCmpSubArea, GxCmpTrap}
};



/*
--------------------------------------------------------------------------------
 GxChangeProject - set the project changed flag to changed
--------------------------------------------------------------------------------

 GxChangeProject sets the changed flag of the project data structure to
 "changed" indicating that a change has occured in the data in the project.

--------------------------------------------------------------------------------
*/

void GxChangeProject (void)
{
   theProject.currentchanged = GxTrue;
}



/*
--------------------------------------------------------------------------------
 GxCmpFile - compare two files for sorting
--------------------------------------------------------------------------------

 GxCmpFile compares two files with each other in order to sort them in a
 sorted list.

 Arguments
 ---------
 file1 - first file to be compared
 file2 - second file to be compared

 Return value
 ------------
 a negative value if <file1> is less than <file2>, 0 if <file1> is equal
 to <file2>, or a positive value if <file1> is greater than <file2>

--------------------------------------------------------------------------------
*/

static int GxCmpFile (const void *file1, const void *file2)
{
   assert (file1);
   assert (file2);

   return (strcmp (((GxFile *) file1)->name, ((GxFile *) file2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCmpHorizon - compare two horizons for sorting
--------------------------------------------------------------------------------

 GxCmpHorizon compares two horizons with each other in order to sort them in
 a sorted list.

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
   assert (horizon1);
   assert (horizon2);

   return (strcmp (((GxHorizon *) horizon1)->name,
                   ((GxHorizon *) horizon2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCmpHorizonHistory - compare two horizon histories for sorting
--------------------------------------------------------------------------------

 GxCmpHorizonHistory compares two horizon histories with each other in
 order to sort them in a sorted list.

 Arguments
 ---------
 horhis1 - first horizon history to be compared
 horhis2 - second horizon history to be compared

 Return value
 ------------
 a negative value if <horhis1> is less than <horhis2>, 0 if <horhis1> is
 equal to <horhis2>, or a positive value if <horhis1> is greater than
 <horhis2>

--------------------------------------------------------------------------------
*/

static int GxCmpHorizonHistory (const void *horhis1, const void *horhis2)
{
   int  cmp;

   assert (horhis1);
   assert (horhis2);

   cmp = strcmp (((GxHorizonHistory *) horhis1)->horizon->name,
                 ((GxHorizonHistory *) horhis2)->horizon->name);
   if (cmp == 0)
      cmp = strcmp (((GxHorizonHistory *) horhis1)->subarea->name,
                    ((GxHorizonHistory *) horhis2)->subarea->name);
   return (cmp);
}



/*
--------------------------------------------------------------------------------
 GxCmpLithology - compare two lithologies for sorting
--------------------------------------------------------------------------------

 GxCmpLithology compares two lithologies with each other in order to sort
 them in a sorted list.

 Arguments
 ---------
 lithology1 - first lithology to be compared
 lithology2 - second lithology to be compared

 Return value
 ------------
 a negative value if <lithology1> is less than <lithology2>, 0 if <lithology1>
 is equal to <lithology2>, or a positive value if <lithology1> is greater
 than <lithology2>.

--------------------------------------------------------------------------------
*/

static int GxCmpLithology (const void *lithology1, const void *lithology2)
{
   assert (lithology1);
   assert (lithology2);

   return (strcmp (((GxLithology *) lithology1)->name,
                   ((GxLithology *) lithology2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCmpLocation - compare two locations for sorting
--------------------------------------------------------------------------------

 GxCmpLocation compares two locations with each other in order to sort them
 in a sorted list.

 Arguments
 ---------
 location1 - first location to be compared
 location2 - second location to be compared

 Return value
 ------------
 a negative value if <location1> is less than <location2>, 0 if <location1>
 is equal to <location2>, or a positive value if <location1> is greater than
 <location2>

--------------------------------------------------------------------------------
*/

static int GxCmpLocation (const void *location1, const void *location2)
{
   assert (location1);
   assert (location2);

   return (strcmp (((GxLocation *) location1)->name,
                   ((GxLocation *) location2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCmpReservoirSubArea - compare two reservoir subareas for sorting
--------------------------------------------------------------------------------

 GxCmpReservoirSubArea compares two reservoir subareas with each other in
 order to sort them in a sorted list.

 Arguments
 ---------
 ressub1 - first reservoir subarea to be compared
 ressub2 - second reservoir subarea to be compared

 Return value
 ------------
 a negative value if <ressub1> is less than <ressub2>, 0 if <ressub1> is
 equal to <ressub2>, or a positive value if <ressub1> is greater than
 <ressub2>

--------------------------------------------------------------------------------
*/

static int GxCmpReservoirSubArea (const void *ressub1, const void *ressub2)
{
   int  cmp;

   assert (ressub1);
   assert (ressub2);

   cmp = strcmp (((GxReservoirSubArea *) ressub1)->horizon->name,
                 ((GxReservoirSubArea *) ressub2)->horizon->name);
   if (cmp == 0)
      cmp = strcmp (((GxReservoirSubArea *) ressub1)->subarea->name,
                     ((GxReservoirSubArea *) ressub2)->subarea->name);
   return (cmp);
}



/*
--------------------------------------------------------------------------------
 GxCmpRock - compare two rocks for sorting
--------------------------------------------------------------------------------

 GxCmpRock compares two rocks with each other in order to sort them in a
 sorted list.

 Arguments
 ---------
 rock1 - first rock to be compared
 rock2 - second rock to be compared

 Return value
 ------------
 a negative value if <rock1> is less than <rock2>, 0 if <rock1> is
 equal to <rock2>, or a positive value if <rock1> is greater than
 <rock2>

--------------------------------------------------------------------------------
*/

static int GxCmpRock (const void *rock1, const void *rock2)
{
   int  cmp;

   assert (rock1);
   assert (rock2);

   cmp = strcmp (((GxRock *) rock1)->lithology->name,
                 ((GxRock *) rock2)->lithology->name);
   if (cmp == 0) {
      cmp = strcmp (((GxRock *) rock1)->subarea->name,
                     ((GxRock *) rock2)->subarea->name);
      if (cmp == 0)
         cmp = strcmp (((GxRock *) rock1)->horizon->name,
                        ((GxRock *) rock2)->horizon->name);
   }
   return (cmp);
}



/*
--------------------------------------------------------------------------------
 GxCmpSubArea - compare two subareas for sorting
--------------------------------------------------------------------------------

 GxCmpSubArea compares two subareas with each other in order to sort them in a
 sorted list.

 Arguments
 ---------
 subarea1 - first subarea to be compared
 subarea2 - second subarea to be compared

 Return value
 ------------
 a negative value if <subarea1> is less than <subarea2>, 0 if <subarea1> is
 equal to <subarea2>, or a positive value if <subarea1> is greater than
 <subarea2>

--------------------------------------------------------------------------------
*/

static int GxCmpSubArea (const void *subarea1, const void *subarea2)
{
   assert (subarea1);
   assert (subarea2);

   return (strcmp (((GxSubArea *) subarea1)->name,
                   ((GxSubArea *) subarea2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCmpTrap - compare two traps for sorting
--------------------------------------------------------------------------------

 GxCmpTrap compares two traps with each other in order to sort them
 in a sorted list.

 Arguments
 ---------
 trap1 - first trap to be compared
 trap2 - second trap to be compared

 Return value
 ------------
 a negative value if <trap1> is less than <trap2>, 0 if <trap1>
 is equal to <trap2>, or a positive value if <trap1> is greater than
 <trap2>

--------------------------------------------------------------------------------
*/

static int GxCmpTrap (const void *trap1, const void *trap2)
{
   assert (trap1);
   assert (trap2);

   return (strcmp (((GxTrap *) trap1)->name, ((GxTrap *) trap2)->name));
}



/*
--------------------------------------------------------------------------------
 GxCommitProject - commit the changes to a project
--------------------------------------------------------------------------------

 GxCommitProject commits the changes that were made to a project by copying
 the project to the backup project.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCommitProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;

/* Check that the project has been initialized */
   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Clear the backup instance */
      rc = GxClearInstance (theProject.backup);

/* -- Prepare the current instance for copying */
      if (!rc) rc = GxLinkNumbers (theProject.current);

/* -- Copy the current instance to the backup instance */
      if (!rc) rc = GxCopyInstance (theProject.backup, theProject.current);
      if (!rc) theProject.backupchanged = theProject.currentchanged;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteProject - delete a project file from disk
--------------------------------------------------------------------------------

 GxDeleteProject removes the project file <filename> from disk.

 Arguments
 ---------
 filename - name of the project file to be removed from disk

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteProject (const char *filename)
{
   GxErrorCode        rc=GXE_NOERROR;
   GxBool             exists;
   FILE              *filehandle;
   GxPosGenexVersion  version;

   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project file exists */
      rc = GxProjectExists (filename, &exists);
      if (!rc && !exists) {
         rc = GxError (GXE_FILENOTEXIST, GXT_FILENOTEXIST, filename);
      } else if (!rc) {

/* ----- Open the project file */
         filehandle = fopen (filename, "r");
         if (!filehandle) {
            rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, filename);
         } else {

/* -------- Read the project file header to make sure it is a project file */
            rc = GxReadFileHeader (filehandle, filename, &version);

/* -------- Close the file */
            if (fclose (filehandle) && !rc)
               rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE, filename);

/* -------- If no error encoutered so far delete the file from disk */
            if (!rc && remove (filename))
               rc = GxError (GXE_FILEDELETE, GXT_FILEDELETE, filename);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeProject - free the project currently in memory
--------------------------------------------------------------------------------

 GxFreeProject removes the current project from memory by freeing the memory
 allocated for the project.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;

/* Check that the project has been initialized */
   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Free the memory allocated for the name of the project */
      if (theProject.name) {
         free (theProject.name);
         theProject.name = NULL;
      }

/* -- Free the two project instances */
      rc = GxFreeInstance (theProject.current);
      if (!rc) rc = GxFreeInstance (theProject.backup);

/* -- Reset the initialization flag */
      theProject.initialized = GxFalse;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetDataItemCount - return the number of data items
--------------------------------------------------------------------------------

 GxGetDataItemCount returns the number of items in the list of the
 specified data items.

 Arguments
 ---------
 dataunit - the data unit of which the count should be returned

 Return value
 ------------
 the number of data items in the list

--------------------------------------------------------------------------------
*/

int GxGetDataItemCount (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   return ((int) CountSortedList (theProject.current[dataunit]));
}



/*
--------------------------------------------------------------------------------
 GxGetCurrentDataItem - return current of a list of data items
--------------------------------------------------------------------------------

 GxGetCurrentDataItem sets the specified list of data items to point to the
 current and return that data item.

 Arguments
 ---------
 dataunit - the data unit of which the current item should be returned

 Return value
 ------------
 pointer to the current data item in the list

--------------------------------------------------------------------------------
*/

GxDataItem *GxGetCurrentDataItem (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   return (GetCurrentFromSortedList (theProject.current[dataunit]));
}



/*
--------------------------------------------------------------------------------
 GxGetFirstDataItem - return the first of a list of data items
--------------------------------------------------------------------------------

 GxGetFirstDataItem sets the specified list of data items to point to the
 first and return that data item.

 Arguments
 ---------
 dataunit - the data unit of which the first item should be returned

 Return value
 ------------
 pointer to the first data item in the list

--------------------------------------------------------------------------------
*/

GxDataItem *GxGetFirstDataItem (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   return (GetFirstFromSortedList (theProject.current[dataunit]));
}



/*
--------------------------------------------------------------------------------
 GxGetNextDataItem - return the next of a list of data items
--------------------------------------------------------------------------------

 GxGetNextDataItem sets the specified list of data items to point to the
 next data item and return that data item.

 Arguments
 ---------
 dataunit - the data unit of which the next item should be returned

 Return value
 ------------
 pointer to the next data item in the list

--------------------------------------------------------------------------------
*/

GxDataItem *GxGetNextDataItem (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   return (GetNextFromSortedList (theProject.current[dataunit]));
}



/*
--------------------------------------------------------------------------------
 GxInitializeProject - create and initialize a new project
--------------------------------------------------------------------------------

 GxInitializeProject allocates the memory needed for a new project and
 initializes that memory.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInitializeProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

/* Free the project if was already initialized */
   if (theProject.initialized) rc = GxFreeProject ();
   if (!rc) {

/* -- Create a sorted list for each of the data units in the two instances */
      for (i=0; (!rc)&&(i<GXN_DATAUNITS); i++) {
         theProject.current[i] =
            CreateSortedList (theProject.unitsize[i], theProject.compare[i]);
         if (!theProject.current[i]) rc = GxError (GXE_LIST, GXT_LIST);
         if (!rc) theProject.backup[i] =
            CreateSortedList (theProject.unitsize[i], theProject.compare[i]);
         if (!rc && !theProject.backup[i]) rc = GxError (GXE_LIST, GXT_LIST);
      }
      if (!rc) {

/* ----- Set the flag to indicate that the project is initialized */
         theProject.initialized = GxTrue;

/* ----- Reset the project to an empty project */
         rc = GxNewProject ();
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertDataItem - inserts a new data item into a list of data items
--------------------------------------------------------------------------------

 GxInsertDataItem inserts a new data item into the specified list of data
 items and returns a pointer to the data item in the list.

 Arguments
 ---------
 dataunit    - the data unit of which the first item should be returned
 dataitem    - the data item that has to be inserted
 dataitemptr - pointer to the data item after it has been inserted

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertDataItem (GxDataUnit dataunit, GxDataItem *dataitem,
                              GxDataItem **dataitemptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (dataitem);
   assert (dataitemptr);
   assert (theProject.initialized);

   if (theProject.compare[dataunit]) {

/* -- There is a compare function, so insert in sorted order */
      *dataitemptr = InsertInSortedList (theProject.current[dataunit],
                                         dataitem);
   } else {

/* -- There is no compare function, so append at end of list */
      *dataitemptr = AppendToList (theProject.current[dataunit], dataitem);
   }
   if (*dataitemptr) {

/* -- Set the "changed" flag of the current instance */
      GxChangeProject ();
   } else {
      rc = GxError (GXE_LIST, GXT_LIST);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxNewProject - create a new project in memory
--------------------------------------------------------------------------------

 GxNewProject creates a new empty project in memory by first destroying the
 current project and then allocating the memory needed for an empty project.
 The project initially has the name GX_DEFAULTPROJECTNAME.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxNewProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;

/* Check that the project has been initialized */
   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Set the name of the project to its default */
      rc = GxSetProjectName (GX_DEFAULTPROJECTNAME);

/* -- Clear the two project instances */
      if (!rc) rc = GxClearInstance (theProject.current);
      if (!rc) rc = GxClearInstance (theProject.backup);
      if (!rc) {

/* ----- Reset the "changed" flag of both instances */
         theProject.currentchanged = GxFalse;
         theProject.backupchanged  = GxFalse;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxOpenProject - read a project from file into memory
--------------------------------------------------------------------------------

 GxOpenProject reads a new project into memory from the project file
 <filename>. The name of the project is set to the name of the file.

 Arguments
 ---------
 filename - name of the project file to be read in

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxOpenProject (const char *filename)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       exists, tempchanged;
   FILE         *filehandle;

   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      if (!theProject.initialized) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Check that the project file exists */
         rc = GxProjectExists (filename, &exists);
         if (!rc && !exists) {
            rc = GxError (GXE_FILENOTEXIST, GXT_FILENOTEXIST, filename);
         } else if (!rc) {

/* -------- Open the file for reading in binary format */
            filehandle = fopen (filename, "r");
            if (!filehandle) {
               rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, filename);
            } else {

/* ----------- Swap the two instances for error recovery, in the current
   ----------- situation this equivalent to a GxCommitProject but faster */
               rc = GxSwapInstance (theProject.current, theProject.backup);
               if (!rc) {
                  tempchanged               = theProject.currentchanged;
                  theProject.currentchanged = theProject.backupchanged;
                  theProject.backupchanged  = tempchanged;

/* -------------- Clear current instance and read data for it from file  */
                  if (!rc) rc = GxClearInstance (theProject.current);
                  if (!rc) rc = GxReadInstance (filehandle, filename,
                                                theProject.unitsize,
                                                theProject.current);

/* -------------- Make the necessary pointer links in the instance */
                  if (!rc) rc = GxLinkPointers (theProject.current);

/* -------------- Set the name of the project to the name of the file */
                  if (!rc) rc = GxSetProjectName (filename);
                  if (rc) {

/* ----------------- Roll back the instance swapped earlier */
                     (void) GxRollbackProject ();
                  } else {

/* ----------------- Reset the "changed" flag and commit the changes */
                     theProject.currentchanged = GxFalse;
                     rc = GxCommitProject ();
                  }
               }

/* ----------- Close the file even if an error has occurred */
	       GxCloseFile (filehandle);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxProjectChanged - inquire whether the project is changed
--------------------------------------------------------------------------------

 GxProjectChanged returns whether the project in memory was changed since the
 last time it was either read from disk, written to disk or cleared.

 Arguments
 ---------
 changed - pointer to boolean variable for result: GxTrue if project changed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxProjectChanged (GxBool *changed)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!changed) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theProject.initialized) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {
         *changed = theProject.currentchanged;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxProjectExists - inquire whether a project exists
--------------------------------------------------------------------------------

 GxProjectExists returns whether the specified project file exists on disk.

 Arguments
 ---------
 exists - pointer to boolean variable for result: GxTrue if file exists

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxProjectExists (const char *filename, GxBool *exists)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME) || !exists) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxFileExists (filename, exists);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxProjectInitialized - inquire whether project data structure is initialized
--------------------------------------------------------------------------------

 GxProjectInitialized returns whether the project data structures have been
 initialized by calling GxInitializeProject.

 Arguments
 ---------
 initialized - pointer to boolean variable for result:
               GxTrue if project data structures have been initialized

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxProjectInitialized (GxBool *initialized)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!initialized) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *initialized = theProject.initialized;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxRemoveDataItem - removes the current data item from a list of data items
--------------------------------------------------------------------------------

 GxRemoveDataItem removes the current data item from the specified list of
 data items.

 Arguments
 ---------
 dataunit - the data unit of which the current item should be removed

--------------------------------------------------------------------------------
*/

void GxRemoveDataItem (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   RemoveFromSortedList (theProject.current[dataunit]);
   GxChangeProject ();
}



/*
--------------------------------------------------------------------------------
 GxRepositionDataItem - moves the current data item to its right position
--------------------------------------------------------------------------------

 GxRepositionDataItem takes the current data item and moves it to the right
 position in the data item list. This function is useful when the key of
 a data item has changed and the data item should be moved without affecting
 any links to it.

 Arguments
 ---------
 dataunit - the data unit of which the current item should be repositioned

--------------------------------------------------------------------------------
*/

void GxRepositionDataItem (GxDataUnit dataunit)
{
   assert (theProject.initialized);

   RepositionInSortedList (theProject.current[dataunit]);
   GxChangeProject ();
}



/*
--------------------------------------------------------------------------------
 GxRollbackProject - rollback the changes to a project
--------------------------------------------------------------------------------

 GxRollbackProject rolls back the changes that were made to a project by
 copying the backup project to the project.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRollbackProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;

/* Check that the project has been initialized */
   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Clear the current instance of the project */
      rc = GxClearInstance (theProject.current);

/* -- Copy the backup instance to the current instance */
      if (!rc) rc = GxCopyInstance (theProject.current, theProject.backup);

/* -- Make the necessary pointer links in the instance */
      if (!rc) rc = GxLinkPointers (theProject.current);

/* -- Also restore the changed flag of the backup instance */
      if (!rc) theProject.currentchanged = theProject.backupchanged;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSaveProject - save the project in its project file
--------------------------------------------------------------------------------

 GxSaveProject writes the project from memory to its project file. The
 project should have a name before it can be written. If the project file
 already exists it will be overwritten.
 
 Arguments
 ---------
 version  - version identifier to save the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxSaveProject (GxPosGenexVersion version)
{
   GxErrorCode  rc=GXE_NOERROR;
   FILE         *filehandle;
   GxBool       oldchanged;

   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

/* Check that the project has been initialized */
   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Check that the project has a name */
      if (!theProject.name) {
         rc = GxError (GXE_NOPROJECTNAME, GXT_NOPROJECTNAME);
      } else {

/* ----- Keep the "changed" flag in case an error occurs */
         oldchanged = theProject.currentchanged;

/* ----- Reset the "changed" flag and commit the changes made */
         theProject.currentchanged = GxFalse;
         rc = GxCommitProject ();
         if (!rc) {

/* -------- Open the project file for writing in binary format */
            filehandle = fopen (theProject.name, "w");
            if (!filehandle) {
               rc = GxError (GXE_FILEOPENW, GXT_FILEOPENW, theProject.name);
            } else {

/* ----------- Write the instance into the project file */
               if (!rc) rc = GxWriteInstance (filehandle, theProject.name,
                  theProject.unitsize, theProject.current, version);

/* ----------- Close the file even if an error occurred */
               if (fclose (filehandle) && !rc)
                  rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE,
                                theProject.name);
            }

/* -------- Return to the old "changed" flag if an error occurred */
            if (rc) theProject.currentchanged = oldchanged;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSaveProjectAs - save the project in a file with a new name
--------------------------------------------------------------------------------

 GxSaveProjectAs writes the project from memory to the project file
 <filename>. The name of the project is changed to the name of the project
 file.

 Arguments
 ---------
 filename - name of the project file to be saved
 version  - version identifier to save the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxSaveProjectAs (const char *filename, GxPosGenexVersion version)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME) 
      || ((version != GX_VERSION_3) && (version != GX_VERSION_4))) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      if (!theProject.initialized) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Set the project name the name passed in the argument */
         rc = GxSetProjectName (filename);

/* ----- Save the current instance of the project */
         if (!rc) rc = GxSaveProject (version);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSearchDataItem - searches a data item in a list of data items
--------------------------------------------------------------------------------

 GxSearchDataItem searches the specified list of data items for a data item
 with the same key as the data item <dataitem>. If one is found it is
 returned.

 Arguments
 ---------
 dataunit - the data unit in which the item should be searched
 dataitem - the data to be searched

 Return value
 ------------
 pointer to the data item found

--------------------------------------------------------------------------------
*/

GxDataItem *GxSearchDataItem (GxDataUnit dataunit, GxDataItem *dataitem)
{
   GxDataItem  *founditem=NULL;

   assert (dataitem);
   assert (theProject.initialized);

   if (theProject.compare[dataunit])
      founditem = SearchInSortedList (theProject.current[dataunit],
                                      dataitem);
   return (founditem);
}



/*
--------------------------------------------------------------------------------
 GxSetProjectName - sets the name of the project in memory
--------------------------------------------------------------------------------

 GxSetProjectName changes the name of the project in memory by freeing the
 memory used for the old name (if any) and allocating memory for the new name.

 Arguments
 ---------
 name - new name of the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxSetProjectName (const char *name)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         *newname=NULL;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      if (!theProject.initialized) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Allocate memory for the name */
         newname = malloc (strlen(name)+1);
         if (!newname) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
         } else {

/* -------- Copy the name to the new memory */
            strcpy (newname, name);
         }
         if (!rc) {

/* -------- Free the memory for the old name and install the new name */
            if (theProject.name) free (theProject.name);
            theProject.name = newname;

/* -------- Set the "changed" flag for the current instance */
            GxChangeProject ();
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetProjectName - gets the name of the project in memory
--------------------------------------------------------------------------------

 GxGetProjectName returns the name of the project in memory. 

 Arguments
 ---------
 name - pointer for the name

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetProjectName (char **name)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         *newname=NULL;

   if (!name) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theProject.initialized) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {
         newname = malloc (strlen(theProject.name)+1);
         if (!newname) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
         } else {
            strcpy (newname, theProject.name);
           *name = newname;
         }
      }
   }
   return (rc);
}





/*
--------------------------------------------------------------------------------
 GxBackupProject - save the project in a backup file named 'genex.bak'
--------------------------------------------------------------------------------

 GxBackupProject writes the project from memory to a backup file. 
 If the backup file already exists it will be overwritten.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxBackupProject (void)
{
   GxErrorCode  rc=GXE_NOERROR;
   FILE         *filehandle;

   if (!theProject.initialized) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {
      filehandle = fopen (GX_BACKUPPROJECTNAME, "w");
      if (!filehandle) {
         rc = GxError (GXE_FILEOPENW, GXT_FILEOPENW, GX_BACKUPPROJECTNAME);
      } else {
         if (!rc) 
            rc = GxWriteInstance (filehandle, GX_BACKUPPROJECTNAME, 
                                  theProject.unitsize, theProject.current, 
                                  GX_VERSION_4);
         if (fclose (filehandle) && !rc)
            rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE, GX_BACKUPPROJECTNAME);
      }
   }
   return (rc);
}


