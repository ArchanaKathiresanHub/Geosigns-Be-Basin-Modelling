/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxrock.c,v 25.0 2005/07/05 08:03:50 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxrock.c,v $
 * Revision 25.0  2005/07/05 08:03:50  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:01  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:33  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:14  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:45  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:35  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:28  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:45  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:01  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:21  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:21  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:23  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:59  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:50  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:22  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:18  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:37  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:41  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxRock.c     Genex rock functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxRock.c contains the Genex functions for handling rocks.
              The following functions are available:

                GxCopyThicknessToAllAreas - copy thickness data to all rocks
                GxCreateRock              - create a new rock data structure
                GxFreeRock                - free memory for rock structure
                GxGetRock                 - return a rock from the project
		GxReadRock                - read rock struct from project
                GxReplaceRock             - replaces a rock in the project
	        GxWriteRock               - write rock struct to project file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxrock.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteRock
 28-MAR-1993  M. van Houtert GxReadRock
 07-SEP-1994  Th. Berkers    Default values moved to gxrock include file

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxprjct.h"
#include  "gxrock.h"

/* Macro definitions */
/* ----------------- */


/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxFindRock (const char *, const char *, const char *,
                                GxLithology **, GxSubArea **, GxHorizon **,
                                GxRock **);



/*
--------------------------------------------------------------------------------
 GxCopyThicknessToAllAreas - copy thickness data to all subareas
--------------------------------------------------------------------------------

 GxCopyThicknessToAllAreas copies the net thickness data for subarea
 <subareaname> to all other subareas.

 Arguments
 ---------
 subareaname - name of the subarea for which the thickness should be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyThicknessToAllAreas (const char *subareaname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxSubArea    *sourcesubarea, searchsubarea, *subarea;
   GxLithology  *lithology;
   GxHorizon    *horizon;
   GxRock       searchrock, *rock, newrock;
   double       thickness;

   if ((!subareaname) || (strlen (subareaname) == 0)
      || (strlen (subareaname) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the subarea is present */
         strcpy (searchsubarea.name, subareaname);
         sourcesubarea = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                         (GxDataItem *) &searchsubarea);
         if (!sourcesubarea) {
            rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                          subareaname);
         } else {

/* -------- Loop over all source rock horizons */
            horizon = (GxHorizon *) GxGetFirstDataItem (GX_HORIZON);
            while (!rc && horizon) {
               if (horizon->type == GX_SOURCEROCK) {

/* -------------- Loop over all lithologies */
                  lithology =
                     (GxLithology *) GxGetFirstDataItem (GX_LITHOLOGY);
                  while (!rc && lithology) {

/* ----------------- Get the source rock structure from the project */
                     searchrock.lithology = lithology;
                     searchrock.subarea   = sourcesubarea;
                     searchrock.horizon   = horizon;
                     rock = (GxRock *) GxSearchDataItem (GX_ROCK,
                                          (GxDataItem *) &searchrock);

/* ----------------- If present use its thickness, otherwise use default */
                     thickness = (rock != NULL) ?
                                 rock->thickness : GX_DEFAULTTHICKNESS;

/* ----------------- Loop over all subareas */
                     subarea = (GxSubArea *) GxGetFirstDataItem (GX_SUBAREA);
                     while (!rc && subarea) {

/* -------------------- Get the destination rock structure from the project */
                        searchrock.lithology = lithology;
                        searchrock.subarea   = subarea;
                        searchrock.horizon   = horizon;
                        rock = (GxRock *) GxSearchDataItem (GX_ROCK,
                                             (GxDataItem *) &searchrock);
                        if (rock) {

/* ----------------------- If present change thickness and notify project */
                           rock->thickness    = thickness;
                           GxChangeProject ();
                        } else {

/* ----------------------- If not present assign default values */
                           newrock.lithology = lithology;
                           newrock.horizon   = horizon;
                           newrock.subarea   = subarea;
                           newrock.tocm      = GX_DEFAULTTOCM;
                           newrock.s1m       = GX_DEFAULTS1M;
                           newrock.s2m       = GX_DEFAULTS2M;
                           newrock.s3m       = GX_DEFAULTS3M;
                           newrock.tmaxm     = GX_DEFAULTTMAXM;
                           newrock.hcm       = GX_DEFAULTHCM;
                           newrock.ocm       = GX_DEFAULTOCM;
                           newrock.scm       = GX_DEFAULTSCM;
                           newrock.ncm       = GX_DEFAULTNCM;
                           newrock.vrm       = GX_DEFAULTVRM;
                           newrock.transr    = GX_DEFAULTTRANSR;
                           newrock.edited    = GX_DEFAULTEDITED;

/* ----------------------- Set value for thickness and insert into project */
                           newrock.thickness = thickness;
                           rc = GxInsertDataItem (GX_ROCK,
                                   (GxDataItem *) &newrock,
                                   (GxDataItem **) &rock);
                        }
                        subarea =
                           (GxSubArea *) GxGetNextDataItem (GX_SUBAREA);
                     }
                     lithology =
                        (GxLithology *) GxGetNextDataItem (GX_LITHOLOGY);
                  }
               }
               horizon = (GxHorizon *) GxGetNextDataItem (GX_HORIZON);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateRock - create a new rock structure
--------------------------------------------------------------------------------

 GxCreateRock allocates memory for a new rock structure and initializes that
 memory.

 Arguments
 ---------
 rockptr - pointer for the rock

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateRock (Rock **rockptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   Rock         *rock;

   if (!rockptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rock = (Rock *)malloc (sizeof (Rock));
      if (!rock) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         rock->lithology[0] = '\0';
         rock->subarea[0]   = '\0';
         rock->horizon[0]   = '\0';
         rock->tocm         = GX_DEFAULTTOCM;
         rock->s1m          = GX_DEFAULTS1M;
         rock->s2m          = GX_DEFAULTS2M;
         rock->s3m          = GX_DEFAULTS3M;
         rock->tmaxm        = GX_DEFAULTTMAXM;
         rock->hcm          = GX_DEFAULTHCM;
         rock->ocm          = GX_DEFAULTOCM;
         rock->scm          = GX_DEFAULTSCM;
         rock->ncm          = GX_DEFAULTNCM;
         rock->vrm          = GX_DEFAULTVRM;
         rock->transr       = GX_DEFAULTTRANSR;
         rock->edited       = GX_DEFAULTEDITED;
         rock->thickness    = GX_DEFAULTTHICKNESS;
         *rockptr           = rock;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFindRock - find a rock in the project
--------------------------------------------------------------------------------

 GxFindRock searches for a particular rock identified by lithology, subarea
 and source rock horizon name in the project. If the lithology, subarea or
 reservoir horizon are not found an error is returned. Pointers to the
 lithology, subarea and reservoir horizon dataitems are returned. If the rock
 itself is found a pointer to it is returned, otherwise NULL is returned.

 Arguments
 ---------
 lithologyname - name of the lithology
 subareaname   - name of the subarea
 horizonname   - name of the source rock horizon
 lithologyptr  - pointer to the lithology
 subareaptr    - pointer to the subarea
 horizonptr    - pointer to the source rock horizon
 rockptr       - pointer to the rock or NULL if not present

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFindRock (const char *lithologyname,
                               const char *subareaname,
                               const char *horizonname,
                               GxLithology **lithologyptr,
                               GxSubArea **subareaptr,
                               GxHorizon **horizonptr, GxRock **rockptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxRock       searchrock;
   GxLithology  searchlithology;
   GxSubArea    searchsubarea;
   GxHorizon    searchhorizon;

   assert (lithologyname);
   assert (subareaname);
   assert (horizonname);
   assert (lithologyptr);
   assert (subareaptr);
   assert (horizonptr);
   assert (rockptr);

/* Find the lithology in the project */
   strcpy (searchlithology.name, lithologyname);
   *lithologyptr = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                   (GxDataItem *) &searchlithology);
   if (!*lithologyptr) {
      rc = GxError (GXE_LITHOLOGYNOTPRESENT, GXT_LITHOLOGYNOTPRESENT,
                    lithologyname);
   } else {

/* -- Find the subarea in the project */
      strcpy (searchsubarea.name, subareaname);
      *subareaptr = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                    (GxDataItem *) &searchsubarea);
      if (!*subareaptr) {
         rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                       subareaname);
      } else {

/* ----- Find the source rock horizon in the project */
         strcpy (searchhorizon.name, horizonname);
         *horizonptr = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                       (GxDataItem *) &searchhorizon);
         if (!*horizonptr || ((*horizonptr)->type != GX_SOURCEROCK)) {
            rc = GxError (GXE_SRHORIZONNOTPRESENT, GXT_SRHORIZONNOTPRESENT,
                          horizonname);
         } else {

/* -------- Find the rock in the project */
            searchrock.lithology = *lithologyptr;
            searchrock.subarea   = *subareaptr;
            searchrock.horizon   = *horizonptr;
            *rockptr = (GxRock *) GxSearchDataItem (GX_ROCK,
                                                  (GxDataItem *) &searchrock);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeRock - free the memory for contents of a rock
--------------------------------------------------------------------------------

 GxFreeRock frees the memory that was allocated for the contents of one rock
 in the project.

 Arguments
 ---------
 rock - the rock contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeRock (Rock *rock)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!rock) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (rock);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRock - return contents of one rock the project
--------------------------------------------------------------------------------

 GxGetRock returns the contents of one of the rock in the project.

 Arguments
 ---------
 lithologyname - the name of the lithology
 subareaname   - the name of the subarea
 horizonname   - the name of the source rock horizon
 rock          - the rock to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRock (const char *lithologyname, const char *subareaname,
                       const char *horizonname, Rock *rock)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *lithology;
   GxSubArea    *subarea;
   GxHorizon    *horizon;
   GxRock       *oldrock;

   if (!lithologyname || (strlen (lithologyname) == 0)
      || (strlen (lithologyname) > GXL_LITHOLOGYNAME) || !subareaname
      || (strlen (subareaname) == 0)
      || (strlen (subareaname) > GXL_SUBAREANAME) || !horizonname
      || (strlen (horizonname) == 0)
      || (strlen (horizonname) > GXL_HORIZONNAME) || !rock) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the rock in the project */
         rc = GxFindRock (lithologyname, subareaname, horizonname,
                          &lithology, &subarea, &horizon, &oldrock);
         if (!rc) {

/* -------- Copy the identification of the rock */
            strcpy (rock->lithology, lithologyname);
            strcpy (rock->subarea,   subareaname);
            strcpy (rock->horizon,   horizonname);
            if (oldrock) {

/* ----------- Rock was found: copy its data */
               rock->tocm      = oldrock->tocm;
               rock->s1m       = oldrock->s1m;
               rock->s2m       = oldrock->s2m;
               rock->s3m       = oldrock->s3m;
               rock->tmaxm     = oldrock->tmaxm;
               rock->hcm       = oldrock->hcm;
               rock->ocm       = oldrock->ocm;
               rock->scm       = oldrock->scm;
               rock->ncm       = oldrock->ncm;
               rock->vrm       = oldrock->vrm;
               rock->transr    = oldrock->transr;
               rock->edited    = oldrock->edited;
               rock->thickness = oldrock->thickness;
            } else {

/* ----------- Rock was not found: assign default values */
               rock->tocm      = GX_DEFAULTTOCM;
               rock->s1m       = GX_DEFAULTS1M;
               rock->s2m       = GX_DEFAULTS2M;
               rock->s3m       = GX_DEFAULTS3M;
               rock->tmaxm     = GX_DEFAULTTMAXM;
               rock->hcm       = GX_DEFAULTHCM;
               rock->ocm       = GX_DEFAULTOCM;
               rock->scm       = GX_DEFAULTSCM;
               rock->ncm       = GX_DEFAULTNCM;
               rock->vrm       = GX_DEFAULTVRM;
               rock->transr    = GX_DEFAULTTRANSR;
               rock->edited    = GX_DEFAULTEDITED;
               rock->thickness = GX_DEFAULTTHICKNESS;
            }
         }
      }
   }
   return (rc);
}

/*
-------------------------------------------------------------------------------
-
 GxReadRock - read a rock struct from the project file
-------------------------------------------------------------------------------
-

 GxReadRock reads a rock structure cwfromin an ascii project file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 rock       - The rock structure
 filename   - name of the project file

 Return value
 ------------
 GXE_FILEREAD  token not found by scanner

-------------------------------------------------------------------------------
-
*/

GxErrorCode GxReadRock (FILE *filehandle, GxRock *rock, const char *filename)
{
   GxErrorCode  rc = GXE_NOERROR;

   assert (filehandle);
   assert (rock);
   assert (filename);

   rc = GxGetInteger (filehandle, &rock->dataitem.seqnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &rock->lithologynr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &rock->horizonnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &rock->subareanr, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->tocm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->s1m, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->s2m, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->s3m, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->tmaxm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->hcm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->ocm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->scm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->ncm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->thickness, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->vrm, filename);
   if (!rc) rc = GxGetDouble (filehandle, &rock->transr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &rock->edited, filename);

   return rc;
}



/*
--------------------------------------------------------------------------------
 GxReplaceRock - replaces a rock in the project
--------------------------------------------------------------------------------

 GxReplaceRock replaces information from a rock with new information.

 Arguments
 ---------
 rock - the rock to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceRock (const Rock *rock)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *lithology;
   GxSubArea    *subarea;
   GxHorizon    *horizon;
   GxRock       *oldrock, *insertedrock, newrock;

   if (!rock || (strlen (rock->lithology) == 0)
      || (strlen (rock->lithology) > GXL_LITHOLOGYNAME)
      || (strlen (rock->subarea) == 0)
      || (strlen (rock->subarea) > GXL_SUBAREANAME)
      || (strlen (rock->horizon) == 0)
      || (strlen (rock->horizon) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the rock in the project */
         rc = GxFindRock (rock->lithology, rock->subarea, rock->horizon,
                          &lithology, &subarea, &horizon, &oldrock);
         if (!rc) {
            if (oldrock) {

/* ----------- It was found: copy the new data */
               oldrock->tocm      = rock->tocm;
               oldrock->s1m       = rock->s1m;
               oldrock->s2m       = rock->s2m;
               oldrock->s3m       = rock->s3m;
               oldrock->tmaxm     = rock->tmaxm;
               oldrock->hcm       = rock->hcm;
               oldrock->ocm       = rock->ocm;
               oldrock->scm       = rock->scm;
               oldrock->ncm       = rock->ncm;
               oldrock->vrm       = rock->vrm;
               oldrock->transr    = rock->transr;
               oldrock->edited    = rock->edited;
               oldrock->thickness = rock->thickness;

/* ----------- Indicate to the project that it has changed */
               GxChangeProject ();
            } else {

/* ----------- It was not found: insert a new rock */
               newrock.lithology  = lithology;
               newrock.horizon    = horizon;
               newrock.subarea    = subarea;
               newrock.tocm       = rock->tocm;
               newrock.s1m        = rock->s1m;
               newrock.s2m        = rock->s2m;
               newrock.s3m        = rock->s3m;
               newrock.tmaxm      = rock->tmaxm;
               newrock.hcm        = rock->hcm;
               newrock.ocm        = rock->ocm;
               newrock.scm        = rock->scm;
               newrock.ncm        = rock->ncm;
               newrock.vrm        = rock->vrm;
               newrock.transr     = rock->transr;
               newrock.edited     = rock->edited;
               newrock.thickness  = rock->thickness;
               rc = GxInsertDataItem (GX_ROCK, (GxDataItem *) &newrock,
                                     (GxDataItem **) &insertedrock);
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteRock - write a rock struct to the project file
--------------------------------------------------------------------------------

 GxWriteRock writes a rock structure in an ascii project file

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 rock       - The rock structure

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteRock (FILE *filehandle, const GxRock *rock)
{
   assert (filehandle);
   assert (rock);

   fprintf (filehandle, "%d ", rock->dataitem.seqnr);
   fprintf (filehandle, "%d %d ", rock->lithologynr, rock->horizonnr);
   fprintf (filehandle, "%d ", rock->subareanr);
   fprintf (filehandle, "%f %f ", rock->tocm, rock->s1m);
   fprintf (filehandle, "%f %f ", rock->s2m, rock->s3m);
   fprintf (filehandle, "%f %f ", rock->tmaxm, rock->hcm);
   fprintf (filehandle, "%f %f ", rock->ocm, rock->scm);
   fprintf (filehandle, "%f %f ", rock->ncm, rock->thickness);
   fprintf (filehandle, "%f %f ", rock->vrm, rock->transr);
   fprintf (filehandle, "%d\n", rock->edited);
}



