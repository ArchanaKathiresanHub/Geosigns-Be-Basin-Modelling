/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxlabexp.c,v 25.0 2005/07/05 08:03:46 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxlabexp.c,v $
 * Revision 25.0  2005/07/05 08:03:46  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:57  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:27  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:10  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:42  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:23  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:21  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:33  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:43  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:11  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:01  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:12  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:41  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:17  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:02:27  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:02  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:17  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:27  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxLabExp.c   Genex laboratory experiment data functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxLabExp.c contains the Genex functions for handling
              laboratory experiment data. The following functions are
              available:

                GxCopyLabExperimentData    - copy a lab experiment to another
                GxCreateLabExperiment      - create a new lab experiment
                GxCreateLabExperimentData  - create lab experiment data
                GxDeleteLabExperimentItem  - delete item from lab experiment
                GxDestroyLabExperimentData - destroy lab experiment data
                GxFreeLabExperiment        - free memory for lab experiment
                GxGetLabExperiment         - return laboratory experiment
                GxGetLabExperimentData     - get laboratory experiment data
                GxGetLabExperimentTypeList - get list of lab experiment types
                GxGetStandardLabExperiment - get a standard lab experiment
                GxInsertLabExperimentItem  - insert item in lab experiment
		GxReadLabExperiment        - read lab struct from file
                GxReadLabExperimentData    - read lab data from file
                GxReplaceLabExperiment     - replaces a lab experiment
                GxWriteLabExperiment       - write lab data struct to file
                GxWriteLabExperimentData   - write lab data to file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxlabexp.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 24-JUN-1992  P. Alphenaar  initial version
 04-MAR-1993  M. van Houtert Check for / in pathname added
 22-MAR-1993  M. van Houtert GxWriteLabExperimentData changed into ascii
 23-MAR-1993  M. van Houtert GxWriteLabExperiment added
 28-MAR-1993  M. van Houtert GxReadLabExperiment added
 12-DEC-1993  Th. Berkers When reallocating memory first check if some memory
                          had already been allocated, if so use realloc 
                          otherwise use malloc (On SUN platform realloc with 
                          NULL pointer generates an application crash).

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxfile.h"
#include  "gxlabexp.h"
#include  "gxnumer.h"
#include  "gxprjct.h"
#include  "gxutils.h"

/* Macro definitions */
/* ----------------- */
#define  GXN_ITEMSPERALLOC        10
#define  GX_DEFAULTLABEXPERIMENT  GXLE_ROCKEVAL2
#define  NUMBEROF(a)              (sizeof (a) / sizeof (a[0]))

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef struct ExperimentDescription  ExperimentDescription;
struct ExperimentDescription {
   char  experimentname[GXL_LABEXPERIMENTNAME+1], filename[GXL_FILENAME+1];
};

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocLabExperimentArrays (int, LabExperiment *);
static GxErrorCode  GxAllocLabExperimentData (int, GxLabExperiment *);
static GxErrorCode  GxFreeLabExperimentArrays (LabExperiment *);
static GxErrorCode  GxReadStandardLabExperiment (const char *,
                                                 LabExperiment *);

/* Global variables */
/* ---------------- */
/* -- file scope variables -- */
static ExperimentDescription  theExperiments[]={
   {"Rock-Eval II", "re2.tmp"},
   {"PFID",         "pfid.tmp"},
   {"User defined", ""}
};



/*
--------------------------------------------------------------------------------
 GxAllocLabExperimentArrays - allocate arrays for the laboratory experiment
--------------------------------------------------------------------------------

 GxAllocLabExperimentArrays allocates arrays for a new laboratory experiment
 data structure.

 Arguments
 ---------
 ntimes - number of time entries in laboratory experiment data structure
 labexp - the lab experiment for which the arrays should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocLabExperimentArrays (int ntimes,
                                               LabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize;
   size_t       newmemsize;

   assert (ntimes >= 0);
   assert (labexp);

/* Calculate mimimum size of array in items and bytes */
   newsize = GXN_ITEMSPERALLOC * (ntimes / GXN_ITEMSPERALLOC + 1);
   newmemsize = newsize * sizeof (double);

/* If current size is smaller than minimum size reallocate the arrays */
   if (labexp->size < newsize) {
      rc = GxFreeLabExperimentArrays (labexp);
      if (!rc) {
         labexp->size        = newsize;
         labexp->time        = (double *) malloc (newmemsize);
         labexp->temperature = (double *) malloc (newmemsize);
         labexp->pressure    = (double *) malloc (newmemsize);
         if (!labexp->time || !labexp->temperature || !labexp->pressure) {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
            (void) GxFreeLabExperimentArrays (labexp);
         }
      }
   }

/* There are no items in the array */
   labexp->ntimes = 0;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocLabExperimentData - allocate a new lab experiment data structure
--------------------------------------------------------------------------------

 GxAllocLabExperimentData allocates memory for a new project lab experiment
 data structure.

 Arguments
 ---------
 ntimes  - number of time entries for the laboratory experiment
 history - the laboratory experiment for which the memory should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocLabExperimentData (int ntimes,
                                             GxLabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (ntimes >= 0);
   assert (labexp);

   labexp->ntimes   = ntimes;
   labexp->physprop = NULL;

/* Allocate a single two dimensional array of the right size for all data */
   if (ntimes > 0)
      rc = GxAllocArray (&labexp->physprop, sizeof (double), 2,
                         GXN_LABPHYSPROP, ntimes);

/* Free it in case of error */
   if (rc) (void) GxDestroyLabExperimentData (labexp);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyLabExperimentData - copy data from one laboratory experiment to another
--------------------------------------------------------------------------------

 GxCopyLabExperimentData copies laboratory experiment data from one project
 laboratory experiment struct to another.

 Arguments
 ---------
 destination - destination for laboratory experiment data
 source      - source of the laboratory experiment data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyLabExperimentData (GxLabExperiment *destination,
                                     const GxLabExperiment *source)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (destination);
   assert (source);

/* Allocate the memory needed for the destination */
   rc = GxAllocLabExperimentData (source->ntimes, destination);
   if (!rc && (source->ntimes > 0)) {

/* -- Copy the the two-dimensional array with data, row by row */
      for (i=0; i<GXN_LABPHYSPROP; i++)
         memcpy (destination->physprop[i], source->physprop[i],
                 source->ntimes * sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateLabExperiment - create a new laboratory experiment structure
--------------------------------------------------------------------------------

 GxCreateLabExperiment allocates memory for a new laboratory experiment
 structure and initializes that memory.

 Arguments
 ---------
 labexpptr - pointer for the laboratory experiment

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateLabExperiment (LabExperiment **labexpptr)
{
   GxErrorCode    rc=GXE_NOERROR;
   LabExperiment  *labexp;

   if (!labexpptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      labexp = ( LabExperiment * ) malloc (sizeof (LabExperiment));
      if (!labexp) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         labexp->ntimes      = 0;
         labexp->size        = 0;
         labexp->time        = NULL;
         labexp->temperature = NULL;
         labexp->pressure    = NULL;
         labexp->type        = GX_DEFAULTLABEXPERIMENT;

/* ----- Read the parameters of the standard laboratory from file */
         rc = GxReadStandardLabExperiment
                 (theExperiments[labexp->type].filename, labexp);
         if (!rc) *labexpptr = labexp;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateLabExperimentData - create laboratory experiment data from user data
--------------------------------------------------------------------------------

 GxCreateLabExperimentData allocates memory for a project laboratory
 experiment data item by using the sizes defined in the user data and copies
 the data into the laboratory experiment.

 Arguments
 ---------
 labexpdata - laboratory experiment to be created
 labexp     - user data for the laboratory experiment data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateLabExperimentData (GxLabExperiment *labexpdata,
                                       const LabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (labexpdata);
   assert (labexp);

/* Allocate the memory needed for the data */
   rc = GxAllocLabExperimentData (labexp->ntimes, labexpdata);
   if (!rc && (labexp->ntimes > 0)) {

/* -- Copy the individual data arrays into rows of the two-dim array */
      memcpy (labexpdata->physprop[GX_LABTIME], labexp->time,
              labexp->ntimes * sizeof(double));
      memcpy (labexpdata->physprop[GX_LABTEMPERATURE], labexp->temperature,
              labexp->ntimes*sizeof(double));
      memcpy (labexpdata->physprop[GX_LABPRESSURE], labexp->pressure,
              labexp->ntimes*sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteLabExperimentItem - delete an item from the laboratory experiment
--------------------------------------------------------------------------------

 GxDeleteLabExperimentItem removes an history item from the laboratory
 experiment. The size of the arrays in the laboratory experiment is not
 changed.

 Arguments
 ---------
 labexp - user data for the laboratory experiment
 itemnr - the number of the item to be removed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteLabExperimentItem (LabExperiment *labexp, int itemnr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   if (!labexp || (labexp->type != GXLE_USERDEFINED) || (itemnr < 0)
      || (itemnr >= labexp->ntimes)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Shift items after one to be deleted forward */
      for (i=itemnr; i<labexp->ntimes-1; i++) {
         labexp->time[i]        = labexp->time[i+1];
         labexp->temperature[i] = labexp->temperature[i+1];
         labexp->pressure[i]    = labexp->pressure[i+1];
      }

/* -- Decrease number of items */
      labexp->ntimes--;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDestroyLabExperimentData - destroy lab experiment data by freeing memory
--------------------------------------------------------------------------------

 GxDestroyLabExperimentData frees the memory allocated for a laboratory
 experiment data item.

 Arguments
 ---------
 labexp - laboratory experiment to be destroyed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDestroyLabExperimentData (GxLabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (labexp);

   if (labexp->ntimes > 0) {
      GxFreeArray (labexp->physprop, 2, GXN_LABPHYSPROP, labexp->ntimes);
      labexp->physprop = NULL;
      labexp->ntimes = 0;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeLabExperiment - free the memory for the data of a laboratory experiment
--------------------------------------------------------------------------------

 GxFreeLabExperiment frees the memory that was allocated for the data of a
 laboratory experiment in the project.

 Arguments
 ---------
 labexp - the laboratory experiment data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeLabExperiment (LabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!labexp) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxFreeLabExperimentArrays (labexp);
      if (!rc) free (labexp);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeLabExperimentArrays - free arrays for the data of a lab experiment
--------------------------------------------------------------------------------

 GxFreeLabExperimentArrays frees the arrays that were allocated for the data
 of a laboratory experiment in the project.

 Arguments
 ---------
 labexp - the laboratory experiment data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeLabExperimentArrays (LabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (labexp);

   if (labexp->size > 0) {
      if (labexp->time)        free (labexp->time);
      if (labexp->temperature) free (labexp->temperature);
      if (labexp->pressure)    free (labexp->pressure);
   }
   labexp->ntimes      = 0;
   labexp->size        = 0;
   labexp->time        = NULL;
   labexp->temperature = NULL;
   labexp->pressure    = NULL;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLabExperiment - return contents of one laboratory experiment
--------------------------------------------------------------------------------

 GxGetLabExperiment returns the contents of laboratory experiment in the
 project. If it is not present, default information is returned.

 Arguments
 ---------
 labexp - laboratory experiment to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLabExperiment (LabExperiment *labexp)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxBool           initialized;
   GxLabExperiment  *oldlabexp;
   int              i, nexp;

   if (!labexp) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) laboratory experiment structure from project */
         oldlabexp =
            (GxLabExperiment *) GxGetFirstDataItem (GX_LABEXPERIMENT);
         if (oldlabexp) {

/* -------- It is present: determine the number of the experiment type */
            labexp->type = -1;
            nexp = NUMBEROF (theExperiments);
            for (i=0; i<nexp; i++)
               if (strcmp (theExperiments[i].experimentname,
                           oldlabexp->name) == 0) labexp->type = i;
            if (labexp->type == -1)
               rc = GxError (GXE_INCONSIST, GXT_INCONSIST,
                             __FILE__, __LINE__);
         } else {

/* -------- It is not present: number of the experiment type is default */
            labexp->type = GX_DEFAULTLABEXPERIMENT;
         }
         if (!rc) {
            if (labexp->type == GXLE_USERDEFINED) {

/* ----------- The experiment is user defined: copy parameters from project */
               labexp->charlength   = oldlabexp->charlength;
               labexp->netthickness = oldlabexp->netthickness;
               labexp->upperbiot    = oldlabexp->upperbiot;
               labexp->lowerbiot    = oldlabexp->lowerbiot;
               rc = GxAllocLabExperimentArrays (oldlabexp->ntimes, labexp);
               if (!rc) rc = GxGetLabExperimentData (labexp, oldlabexp);
            } else {

/* ----------- Experiment is not user defined: read parameters from file */
               rc = GxReadStandardLabExperiment
                       (theExperiments[labexp->type].filename, labexp);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLabExperimentData - get user data from laboratory experiment data
--------------------------------------------------------------------------------

 GxGetLabExperimentData copies the laboratory experiment data from
 <labexpdata> into the user data <labexp>.

 Arguments
 ---------
 labexp     - user data for the laboratory experiment
 labexpdata - laboratory experiment data to be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLabExperimentData (LabExperiment *labexp,
                                    const GxLabExperiment *labexpdata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (labexp);
   assert (labexpdata);

   if (labexpdata->ntimes) {

/* -- Copy the rows of the two-dim array into individual data arrays */
      memcpy (labexp->time, labexpdata->physprop[GX_LABTIME],
              labexpdata->ntimes * sizeof(double));
      memcpy (labexp->temperature, labexpdata->physprop[GX_LABTEMPERATURE],
              labexpdata->ntimes * sizeof(double));
      memcpy (labexp->pressure, labexpdata->physprop[GX_LABPRESSURE],
              labexpdata->ntimes * sizeof(double));
   }
   labexp->ntimes = labexpdata->ntimes;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLabExperimentTypeList - return the names of the lab experiment types
--------------------------------------------------------------------------------

 GxGetLabExperimentTypeList returns a list with the names of the different
 laboratory experiment types.

 Arguments
 ---------
 typelistptr - pointer to the type list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLabExperimentTypeList (List *typelistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   List         typelist;
   int          i, nexp;

   if (!typelistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create an empty searchable list */
         typelist = CreateList (GXL_LABEXPERIMENTNAME+1,
                       (int (*) (const void *, const void *)) strcmp);
         if (!typelist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Append the experiment names to the list */
            nexp = NUMBEROF (theExperiments);
            for (i=0; (!rc)&&(i<nexp); i++) {
               if (!AppendToList (typelist,
                                  theExperiments[i].experimentname)) {
                  DeleteList (typelist);
                  rc = GxError (GXE_LIST, GXT_LIST);
               }
            }
            if (!rc) *typelistptr = typelist;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetStandardLabExperiment - return contents of a standard lab experiment
--------------------------------------------------------------------------------

 GxGetStandardLabExperiment returns the contents of a standard laboratory
 experiment. The standard experiment is read the corresponding file in
 the Genex directory.

 Arguments
 ---------
 type   - the type of the standard laboratory experiment
 labexp - standard laboratory experiment to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetStandardLabExperiment (int type, LabExperiment *labexp)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxBool           initialized;

   if ((type < 0) || (type >= NUMBEROF (theExperiments))
      || (type == GXLE_USERDEFINED) || !labexp) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Read the parameters for the standard lab experiment from file */
         rc = GxReadStandardLabExperiment (theExperiments[type].filename,
                                           labexp);
      }
      if (!rc) labexp->type = type;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertLabExperimentItem - insert an item into the laboratory experiment
--------------------------------------------------------------------------------

 GxInsertLabExperimentItem inserts a history item into a laboratory
 experiment. The size of the arrays in the laboratory experiment is changed
 if they are not large enough.

 Arguments
 ---------
 labexp      - user data for the laboratory experiment
 time        - the time for the item to be added
 temperature - the temperature for the item to be added
 pressure    - the pressure for the item to be added

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertLabExperimentItem (LabExperiment *labexp, double time,
                                       double temperature, double pressure)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          newsize, itemnr, i;
   size_t       newmemsize;
   double       *newtime, *newtemperature, *newpressure;

   if (!labexp || (labexp->type != GXLE_USERDEFINED) || (temperature < 0.0)
      || (pressure < 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if ((labexp->ntimes > 0)
         && (labexp->time[0] > labexp->time[labexp->ntimes-1])) {
         rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
      } else {
         if (labexp->ntimes == labexp->size) {

/* -------- Arrays are full: reallocate with a bigger size */
            newsize        = GXN_ITEMSPERALLOC *
                             ((labexp->ntimes + 1) / GXN_ITEMSPERALLOC + 1);
            newmemsize     = newsize * sizeof (double);
            if (labexp->time) {
                newtime = (double *) realloc (labexp->time, newmemsize);
            } else {
                newtime = (double *) malloc (newmemsize);
            }
            if (labexp->temperature) {
                newtemperature = (double *) realloc (labexp->temperature,
                                                     newmemsize);
            } else {
                newtemperature = (double *) malloc (newmemsize);
            }
            if (labexp->pressure) {
                newpressure = (double *) realloc (labexp->pressure,
                                                  newmemsize);
            } else {
                newpressure = (double *) malloc (newmemsize);
            }
            if (newtime)        labexp->time        = newtime;
            if (newtemperature) labexp->temperature = newtemperature;
            if (newpressure)    labexp->pressure    = newpressure;
            if (!newtime || !newtemperature || !newpressure) {
               rc = GxError (GXE_MEMORY, GXT_MEMORY);
            } else {
               labexp->size = newsize;
            }
         }
         if (!rc) {

/* -------- Find the position where the item should be inserted */
            itemnr = MAX (0, GxSearchDoubleA (labexp->ntimes, labexp->time,
                                              time));

/* -------- Shift items to make room for new item */
            for (i=labexp->ntimes-1; i>=itemnr; i--) {
               labexp->time[i+1]        = labexp->time[i];
               labexp->temperature[i+1] = labexp->temperature[i];
               labexp->pressure[i+1]    = labexp->pressure[i];
            }

/* -------- Insert the new item */
            labexp->time[itemnr]        = time;
            labexp->temperature[itemnr] = temperature;
            labexp->pressure[itemnr]    = pressure;

/* -------- Increase the number of items */
            labexp->ntimes++;
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxReadLabExperiment - read lab experiment struct from a project file
--------------------------------------------------------------------------------

 GxReadLabExperiment reads a project laboratory experiment struct from
 an already open project file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 labexp     - laboratory experiment to be read
 filename   - the name of the file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadLabExperiment (FILE *filehandle, GxLabExperiment *labexp,
				const char *filename)
{
   GxErrorCode   rc;

   assert (filehandle);
   assert (labexp);
   assert (filename);

   rc = GxGetInteger (filehandle, &labexp->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, labexp->name, filename);
   if (!rc) rc = GxGetDouble (filehandle, &labexp->charlength ,filename);
   if (!rc) rc = GxGetDouble (filehandle, &labexp->lowerbiot ,filename);
   if (!rc) rc = GxGetDouble (filehandle, &labexp->netthickness ,filename);
   if (!rc) rc = GxGetDouble (filehandle, &labexp->upperbiot ,filename);

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReadLabExperimentData - read lab experiment data from a project file
--------------------------------------------------------------------------------

 GxReadLabExperimentData reads a project laboratory experiment data item from
 an already open project file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 filename   - the name of the file
 labexp     - laboratory experiment to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadLabExperimentData (FILE *filehandle, const char *filename,
                                     GxLabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          ntimes, i, j;

   assert (filehandle);
   assert (filename);
   assert (labexp);

/* Read the number of temperature and pressure items */
   rc = GxGetInteger (filehandle, &ntimes, filename);
   if (!rc) {
      if (ntimes < 0) {
         rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
      } else {

/* ----- Allocate array big enough to hold the data */
         rc = GxAllocLabExperimentData (ntimes, labexp);
         if (ntimes > 0) {

	    for (i=0; (!rc)&&(i<ntimes);i++)
                for (j=0; (!rc)&&(j<GXN_LABPHYSPROP); j++)
                   rc = GxGetDouble (filehandle, &labexp->physprop[j][i], 
								   filename);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadStandardLabExperiment - read a standard lab experiment from file
--------------------------------------------------------------------------------

 GxReadStandardLabExperiment reads a standard laboratory experiment from its
 file into the laboratory experiment data structure.

 Arguments
 ---------
 filename - the name of the file
 labexp   - laboratory experiment to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadStandardLabExperiment (const char *filename,
                                         LabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         *directory, expfilename[GXL_FILENAME+1];
   FILE         *filehandle;
   int          i, len, ntimes;

   assert (filename);
   assert (labexp);

/* Determine the path by translating the environment variable */
   directory = getenv (GX_DIRECTORYVARIABLE);
   if (!directory) {
      rc = GxError (GXE_VARUNDEFINED, GXT_VARUNDEFINED,
                    GX_DIRECTORYVARIABLE);
   } else {

/* -- Build the complete file name */
      (void) strcpy (expfilename, directory);
      len = strlen (expfilename);
      if (expfilename[len - 1] != '/') {
          (void) strcat (expfilename, "/");
      }
      (void) strcat (expfilename, filename);

/* -- Open the file */
      filehandle = fopen (expfilename, "rt");
      if (!filehandle) {
         rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, expfilename);
      } else {

/* ----- Read in the size of the t&p history and allocate the arrays */
         if (fscanf (filehandle, "%d\n", &ntimes) != 1)
            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, expfilename);
         if (!rc) rc = GxAllocLabExperimentArrays (ntimes, labexp);

/* ----- Read in the parameters, looping for the t&p history items */
         if (!rc && (fscanf (filehandle, "%lf %lf %lf %lf\n",
            &labexp->charlength, &labexp->netthickness, &labexp->upperbiot,
            &labexp->lowerbiot) != 4))
            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, expfilename);
         for (i=0; (!rc)&&(i<ntimes); i++) {
            if (fscanf (filehandle, "%lf %lf %lf\n", &labexp->time[i],
               &labexp->temperature[i], &labexp->pressure[i]) != 3)
               rc = GxError (GXE_FILEREAD, GXT_FILEREAD, expfilename);
            if (!rc) labexp->temperature[i] += GXC_ZEROCELCIUS;
         }

/* ----- Set the number of t&p items and close the file */
         if (!rc) labexp->ntimes = ntimes;
         if (fclose (filehandle) && !rc)
            rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE, expfilename);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceLabExperiment - replaces a laboratory experiment in the project
--------------------------------------------------------------------------------

 GxReplaceLabExperiment replaces information from a laboratory experiment
 with new information.

 Arguments
 ---------
 labexp - the laboratory experiment to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceLabExperiment (const LabExperiment *labexp)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxBool           initialized;
   GxLabExperiment  *oldlabexp, *insertedlabexp, newlabexp;

   if (!labexp || (labexp->type < 0)
      || (labexp->type >= NUMBEROF (theExperiments))
      || (labexp->charlength <= 0.0) || (labexp->netthickness <= 0.0)
      || (labexp->upperbiot <= 0.0) || (labexp->lowerbiot <= 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) laboratory experiment structure from project */
         oldlabexp =
            (GxLabExperiment *) GxGetFirstDataItem (GX_LABEXPERIMENT);
         if (oldlabexp) {

/* -------- Get rid of old experiment data */
            rc = GxDestroyLabExperimentData (oldlabexp);
            if (!rc) {

/* ----------- Set the name of the experiment type */
               strcpy (oldlabexp->name,
                       theExperiments[labexp->type].experimentname);
               if (labexp->type == GXLE_USERDEFINED) {

/* -------------- User defined experiment, so user data must also be copied */
                  oldlabexp->charlength   = labexp->charlength;
                  oldlabexp->netthickness = labexp->netthickness;
                  oldlabexp->upperbiot    = labexp->upperbiot;
                  oldlabexp->lowerbiot    = labexp->lowerbiot;
                  rc = GxCreateLabExperimentData (oldlabexp, labexp);
               }

/* ----------- Indicate to project that lab experiment has been changed */
               GxChangeProject ();
            }
         } else {

/* -------- No lab experiment present: fill and insert a new one */
            strcpy (newlabexp.name,
                    theExperiments[labexp->type].experimentname);
            newlabexp.ntimes   = 0;
            newlabexp.physprop = NULL;
            if (labexp->type == GXLE_USERDEFINED) {

/* ----------- User defined experiment, so user data must also be copied */
               newlabexp.charlength    = labexp->charlength;
               newlabexp.netthickness  = labexp->netthickness;
               newlabexp.upperbiot     = labexp->upperbiot;
               newlabexp.lowerbiot     = labexp->lowerbiot;
               rc = GxCreateLabExperimentData (&newlabexp, labexp);
            }
            rc = GxInsertDataItem (GX_LABEXPERIMENT,
                                  (GxDataItem *) &newlabexp,
                                  (GxDataItem **) &insertedlabexp);
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteLabExperiment - write lab experiment to a project file
--------------------------------------------------------------------------------

 GxWriteLabExperimentData writes a laboratory experiment to an
 already open project file. The actual properties is not saved here

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 labexp     - laboratory experiment for which data should be written

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteLabExperiment (FILE *filehandle, const GxLabExperiment *labexp)
{
   assert (filehandle);
   assert (labexp);

   fprintf (filehandle, "%d ", labexp->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %f ", labexp->name, labexp->charlength);
   fprintf (filehandle, "%f %f ", labexp->lowerbiot, labexp->netthickness);
   fprintf (filehandle, "%f\n",   labexp->upperbiot);
}


/*
--------------------------------------------------------------------------------
 GxWriteLabExperimentData - write lab experiment data to a project file
--------------------------------------------------------------------------------

 GxWriteLabExperimentData writes a laboratory experiment data item to an
 already open project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 labexp     - laboratory experiment for which data should be written

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteLabExperimentData (FILE *filehandle, 
                                      const GxLabExperiment *labexp)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;

   if (!filehandle || !labexp) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Write the size of the t&p history */
      fprintf (filehandle, "%d\n", labexp->ntimes);
      if (labexp->ntimes > 0) {

/* ----- Write the array data row by row */
         for (i=0; i<labexp->ntimes; i++) {
            for (j=0; j<GXN_LABPHYSPROP; j++) {
               fprintf (filehandle, "%f ", labexp->physprop[j][i]);
            }
            fprintf (filehandle, "\n");
         }
      }
   }
   return (rc);
}
