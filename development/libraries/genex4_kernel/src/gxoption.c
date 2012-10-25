/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxoption.c,v 25.0 2005/07/05 08:03:48 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxoption.c,v $
 * Revision 25.0  2005/07/05 08:03:48  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:59  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:30  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:12  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:44  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:28  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:25  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:39  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:49  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:16  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:10  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:18  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:51  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:33  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:36  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:10  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:28  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:29  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxOption.c   Genex option data functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        22-JUN-1992

 Description: GxOption.c contains the Genex functions for handling
              option data. The following functions are available:

                GxCreateRefConditions  - create a reference condition struct
                GxCreateRunOptions     - create a run options struct
                GxFreeRefConditions    - free memory for reference conditions
                GxFreeRunOptions       - free memory for run options
                GxGetRefConditions     - return reference conditions
                GxGetRunOptions        - return run options
		GxReadRefConditions    - read the ref conditions from file
		GxReadRunOptions       - read the run options from file
                GxReplaceRefConditions - replaces the reference conditions
                GxReplaceRunOptions    - replaces the run options
	        GxWriteRefConditions   - write the ref conditions to file
	        GxWriteRunOptions      - write the run options to file

 Usage notes: - In order to use the functions in this source file the header
                file gxoption.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 22-JUN-1992  P. Alphenaar  initial version
 23-MAR-1992  M. van Houtert GxWriteRefConditions added
 23-MAR-1992  M. van Houtert GxWriteRunOptions added
 28-MAR-1993  M. van Houtert GxReadRefConditions added
 28-MAR-1993  M. van Houtert GxReadRunOptions added

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxoption.h"
#include  "gxprjct.h"

/* Macro definitions */
/* ----------------- */
/* -- default values for run options and reference conditions -- */
#define  GX_DEFAULTCHLENFACTOR         10.0
#define  GX_DEFAULTGEOLREFTEMP         (GXC_ZEROCELCIUS+140.0)
#define  GX_DEFAULTGEOLREFPRES         98000000.0
#define  GX_DEFAULTGEOLREFWBO          0.5
#define  GX_DEFAULTLABREFTEMP          (GXC_ZEROCELCIUS+450.0)
#define  GX_DEFAULTLABREFPRES          GXC_ATMOSPHERICPRESSURE
#define  GX_DEFAULTLABREFWBO           0.5
#define  GX_DEFAULTMAXCHLENERROR       0.005
#define  GX_DEFAULTMAXCHLENITERATIONS  6
#define  GX_DEFAULTNNODES              10
#define  GX_DEFAULTNTIMESTEPS          400
#define  GX_DEFAULTREFPOSITION         0.5



/*
--------------------------------------------------------------------------------
 GxCreateRefConditions - create a new reference conditions structure
--------------------------------------------------------------------------------

 GxCreateRefConditions allocates memory for a new reference conditions
 structure and initializes that memory.

 Arguments
 ---------
 refconptr - pointer for the reference conditions

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateRefConditions (RefConditions **refconptr)
{
   GxErrorCode     rc=GXE_NOERROR;
   RefConditions   *refcon;

   if (!refconptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      refcon = ( RefConditions * ) malloc (sizeof (RefConditions));
      if (!refcon) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         refcon->geolreftemperature = GX_DEFAULTGEOLREFTEMP;
         refcon->geolrefpressure    = GX_DEFAULTGEOLREFPRES;
         refcon->geolrefwbo         = GX_DEFAULTGEOLREFWBO;
         refcon->labreftemperature  = GX_DEFAULTLABREFTEMP;
         refcon->labrefpressure     = GX_DEFAULTLABREFPRES;
         refcon->labrefwbo          = GX_DEFAULTLABREFWBO;
         *refconptr                 = refcon;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateRunOptions - create a new run options structure
--------------------------------------------------------------------------------

 GxCreateRunOptions allocates memory for a new run options data structure and
 initializes that memory.

 Arguments
 ---------
 runoptptr - pointer for the run options

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateRunOptions (RunOptions **runoptptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   RunOptions   *runopt;

   if (!runoptptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      runopt = ( RunOptions * ) malloc (sizeof (RunOptions));
      if (!runopt) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         runopt->nnodes             = GX_DEFAULTNNODES;
         runopt->ntimesteps         = GX_DEFAULTNTIMESTEPS;
         runopt->maxchleniterations = GX_DEFAULTMAXCHLENITERATIONS;
         runopt->maxchlenerror      = GX_DEFAULTMAXCHLENERROR;
         runopt->chlenfactor        = GX_DEFAULTCHLENFACTOR;
         runopt->referenceposition  = GX_DEFAULTREFPOSITION;
         *runoptptr                 = runopt;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeRefConditions - free the memory for the reference conditions data
--------------------------------------------------------------------------------

 GxFreeRefConditions frees the memory that was allocated for the data of the
 reference conditions in the project.

 Arguments
 ---------
 refcon - the reference condition data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeRefConditions (RefConditions *refcon)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!refcon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (refcon);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeRunOptions - free the memory for the run options data
--------------------------------------------------------------------------------

 GxFreeRunOptions frees the memory that was allocated for the data of the
 run options in the project.

 Arguments
 ---------
 runopt - the run options data to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeRunOptions (RunOptions *runopt)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!runopt) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (runopt);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRefConditions - return contents of reference conditions in the project
--------------------------------------------------------------------------------

 GxGetRefConditions returns the contents of reference conditions in the
 project. If it is not present, default information is returned.

 Arguments
 ---------
 refcon - reference conditions to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRefConditions (RefConditions *refcon)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxBool           initialized;
   GxRefConditions  *oldrefcon;

   if (!refcon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) reference conditions structure from project */
         oldrefcon =
            (GxRefConditions *) GxGetFirstDataItem (GX_REFCONDITIONS);

/* ----- If present copy its values otherwise take defaults */
         if (oldrefcon) {
            refcon->geolreftemperature = oldrefcon->geolreftemperature;
            refcon->geolrefpressure    = oldrefcon->geolrefpressure;
            refcon->geolrefwbo         = oldrefcon->geolrefwbo;
            refcon->labreftemperature  = oldrefcon->labreftemperature;
            refcon->labrefpressure     = oldrefcon->labrefpressure;
            refcon->labrefwbo          = oldrefcon->labrefwbo;
         } else {
            refcon->geolreftemperature = GX_DEFAULTGEOLREFTEMP;
            refcon->geolrefpressure    = GX_DEFAULTGEOLREFPRES;
            refcon->geolrefwbo         = GX_DEFAULTGEOLREFWBO;
            refcon->labreftemperature  = GX_DEFAULTLABREFTEMP;
            refcon->labrefpressure     = GX_DEFAULTLABREFPRES;
            refcon->labrefwbo          = GX_DEFAULTLABREFWBO;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRunOptions - return contents of run options in the project
--------------------------------------------------------------------------------

 GxGetRunOptions returns the contents of the run options in the
 project. If it is not present, default information is returned.

 Arguments
 ---------
 runopt - run options to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRunOptions (RunOptions *runopt)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxBool        initialized;
   GxRunOptions  *oldrunopt;

   if (!runopt) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) run options structure from project */
         oldrunopt = (GxRunOptions *) GxGetFirstDataItem (GX_RUNOPTIONS);

/* ----- If present copy its values otherwise take defaults */
         if (oldrunopt) {
            runopt->nnodes             = oldrunopt->nnodes;
            runopt->ntimesteps         = oldrunopt->ntimesteps;
            runopt->maxchleniterations = oldrunopt->maxchleniterations;
            runopt->maxchlenerror      = oldrunopt->maxchlenerror;
            runopt->chlenfactor        = oldrunopt->chlenfactor;
            runopt->referenceposition  = oldrunopt->referenceposition;
         } else {
            runopt->nnodes             = GX_DEFAULTNNODES;
            runopt->ntimesteps         = GX_DEFAULTNTIMESTEPS;
            runopt->maxchleniterations = GX_DEFAULTMAXCHLENITERATIONS;
            runopt->maxchlenerror      = GX_DEFAULTMAXCHLENERROR;
            runopt->chlenfactor        = GX_DEFAULTCHLENFACTOR;
            runopt->referenceposition  = GX_DEFAULTREFPOSITION;
         }
      }
   }
   return (rc);
}
/*
-------------------------------------------------------------------------------
-
 GxReadRunOptions - read runoptions from a project file
-------------------------------------------------------------------------------
-

 GxReadRunOptions - reads the user specified run options from a project file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be read
 options    - The run options
 filename   - project filename

 Return value
 ------------
 GXE_FILEREAD  token not found by scanner

-------------------------------------------------------------------------------
-
*/

GxErrorCode GxReadRunOptions (FILE *filehandle, GxRunOptions *options,
			const char *filename)
{
   GxErrorCode rc=GXE_NOERROR;

   assert (filehandle);
   assert (options);
   assert (filename);

   rc = GxGetInteger (filehandle, &options->dataitem.seqnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &options->nnodes, filename);
   if (!rc) rc = GxGetInteger (filehandle, &options->ntimesteps, filename);
   if (!rc) rc = GxGetInteger (filehandle, &options->maxchleniterations, 
								filename);
   if (!rc) rc = GxGetDouble (filehandle, &options->referenceposition,
								filename);
   if (!rc) rc = GxGetDouble (filehandle, &options->maxchlenerror, filename);
   if (!rc) rc = GxGetDouble (filehandle, &options->chlenfactor, filename);

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReadRefConditions - read reference conditions from a project file
--------------------------------------------------------------------------------

 GxReadRefConditions reads the reference conditions from an
 already open project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 conditions - The reference conditions
 filename   - project filename

 Return value
 ------------
 GXE_FILEREAD - token not found by scanner
--------------------------------------------------------------------------------
*/

GxErrorCode GxReadRefConditions (FILE *filehandle, GxRefConditions *condition,
				 const char *filename)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (condition);
   assert (filename);

   rc = GxGetInteger (filehandle, &condition->dataitem.seqnr, filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->geolreftemperature, 
							filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->geolrefpressure, 
							filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->geolrefwbo, filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->labreftemperature, 
							filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->labreftemperature, 
							filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->labrefpressure, filename);
   if (!rc) rc = GxGetDouble (filehandle, &condition->labrefwbo, filename);

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReplaceRefConditions - replaces the reference conditions in the project
--------------------------------------------------------------------------------

 GxReplaceRefConditions replaces information for the reference conditions
 with new information.

 Arguments
 ---------
 refcon - the reference conditions to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceRefConditions (const RefConditions *refcon)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxBool           initialized;
   GxRefConditions  *oldrefcon, *insertedrefcon, newrefcon;

   if (!refcon) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) reference conditions structure from project */
         oldrefcon =
            (GxRefConditions *) GxGetFirstDataItem (GX_REFCONDITIONS);
         if (oldrefcon) {

/* -------- It was already present: copy new values to it */
            oldrefcon->geolreftemperature = refcon->geolreftemperature;
            oldrefcon->geolrefpressure    = refcon->geolrefpressure;
            oldrefcon->geolrefwbo         = refcon->geolrefwbo;
            oldrefcon->labreftemperature  = refcon->labreftemperature;
            oldrefcon->labrefpressure     = refcon->labrefpressure;
            oldrefcon->labrefwbo          = refcon->labrefwbo;

/* -------- Signal the changes to the project */
            GxChangeProject ();
         } else {

/* -------- It was not yet present: fill in values and insert it */
            newrefcon.geolreftemperature = refcon->geolreftemperature;
            newrefcon.geolrefpressure    = refcon->geolrefpressure;
            newrefcon.geolrefwbo         = refcon->geolrefwbo;
            newrefcon.labreftemperature  = refcon->labreftemperature;
            newrefcon.labrefpressure     = refcon->labrefpressure;
            newrefcon.labrefwbo          = refcon->labrefwbo;
            rc = GxInsertDataItem (GX_REFCONDITIONS,
                                  (GxDataItem *) &newrefcon,
                                  (GxDataItem **) &insertedrefcon);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceRunOptions - replaces the run options in the project
--------------------------------------------------------------------------------

 GxReplaceRunOptions replaces information for the run options with new
 information.

 Arguments
 ---------
 runopt - the run options to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceRunOptions (const RunOptions *runopt)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxBool        initialized;
   GxRunOptions  *oldrunopt, *insertedrunopt, newrunopt;

   if (!runopt || (runopt->nnodes <= 0) || (runopt->ntimesteps <= 0)
      || (runopt->maxchleniterations <= 0) || (runopt->maxchlenerror <= 0.0)
      || (runopt->chlenfactor <= 1.0) || (runopt->referenceposition < 0.0)
      || (runopt->referenceposition > 1.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the (single) run options structure from project */
         oldrunopt = (GxRunOptions *) GxGetFirstDataItem (GX_RUNOPTIONS);
         if (oldrunopt) {

/* -------- It was already present: copy new values to it */
            oldrunopt->nnodes             = runopt->nnodes;
            oldrunopt->ntimesteps         = runopt->ntimesteps;
            oldrunopt->maxchleniterations = runopt->maxchleniterations;
            oldrunopt->maxchlenerror      = runopt->maxchlenerror;
            oldrunopt->chlenfactor        = runopt->chlenfactor;
            oldrunopt->referenceposition  = runopt->referenceposition;

/* -------- Signal the changes to the project */
            GxChangeProject ();
         } else {

/* -------- It was not yet present: fill in values and insert it */
            newrunopt.nnodes              = runopt->nnodes;
            newrunopt.ntimesteps          = runopt->ntimesteps;
            newrunopt.maxchleniterations  = runopt->maxchleniterations;
            newrunopt.maxchlenerror       = runopt->maxchlenerror;
            newrunopt.chlenfactor         = runopt->chlenfactor;
            newrunopt.referenceposition   = runopt->referenceposition;
            rc = GxInsertDataItem (GX_RUNOPTIONS, (GxDataItem *) &newrunopt,
                                  (GxDataItem **) &insertedrunopt);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxWriteRefConditions - write reference conditions to a project file
--------------------------------------------------------------------------------

 GxWriteRefConditions writes the reference conditions to an
 already open project file. 

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 conditions - The reference conditions 

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteRefConditions (FILE *filehandle, const GxRefConditions *condition)
{
   assert (filehandle);
   assert (condition);

   fprintf (filehandle, "%d ", condition->dataitem.seqnr);
   fprintf (filehandle, "%f ", condition->geolreftemperature);
   fprintf (filehandle, "%f ", condition->geolrefpressure);
   fprintf (filehandle, "%f ", condition->geolrefwbo);
   fprintf (filehandle, "%f ", condition->labreftemperature);
   fprintf (filehandle, "%f ", condition->labrefpressure);
   fprintf (filehandle, "%f\n", condition->labrefwbo);
}


/*
--------------------------------------------------------------------------------
 GxWriteRunOptions - write runoptions to a project file
--------------------------------------------------------------------------------

 GxWriteRunOptions - writes the user specified run options to a project file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 options - The run options

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteRunOptions (FILE *filehandle, const GxRunOptions *options)
{
   assert (filehandle);
   assert (options);

   fprintf (filehandle, "%d ", options->dataitem.seqnr);
   fprintf (filehandle, "%d %d ", options->nnodes, options->ntimesteps);
   fprintf (filehandle, "%d ", options->maxchleniterations);
   fprintf (filehandle, "%f ", options->referenceposition);
   fprintf (filehandle, "%f ", options->maxchlenerror);
   fprintf (filehandle, "%f\n", options-> chlenfactor);
}

