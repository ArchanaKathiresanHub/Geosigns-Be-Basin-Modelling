/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxlithol.c,v 25.0 2005/07/05 08:03:47 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxlithol.c,v $
 * Revision 25.0  2005/07/05 08:03:47  ibs
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
 * Revision 22.0  2002/06/28 12:09:24  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:22  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:34  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:44  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:12  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:02  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:13  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:43  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:19  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:02:39  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:03  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:18  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:34  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxLithol.c   Genex lithology functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        22-JUN-1992

 Description: GxLithol.c contains the Genex functions for handling
              lithologies. The following functions are available:

                GxCreateLithology      - create a new lithology structure
                GxDeleteLithology      - remove a lithology from the project
                GxFreeLithology        - free memory for lithology contents
                GxGetLithology         - return contents of one lithology
                GxGetLithologyNameList - return lithology names in project
                GxInsertLithology      - inserts a new lithology in project
		GxReadLithology        - read a lithology from project
                GxRenameLithology      - change the name of a lithology
                GxReplaceLithology     - replaces a lithology in project
                GxWriteLithology       - Writes a lithology to projectfile

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxlithol.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 22-JUN-1992  P. Alphenaar  initial version
 23-MAR-1992  M. van Houtert GxWriteLithology added
 29-MAR-1993  M. van Houtert GxReadLithology added
 04-DEC-1993  Th. Berkers    When deleting a lithology, the source rock present
                             day and thickness info is scanned for references 
                             to this lithology. When a reference is found that
                             info had to be deleted but then the next item in
                             the list was skipped resulting in unreferenced 
                             source rock information which causes a crash after
                             reloading a file. 
 22-DEC-1993  Th. Berkers    The default N/C ratio changed in 0.016 (was 0.0)
 05-JAN-1994  Th. Berkers    The default S1 for initial properties must be 5.0
                             (was 50.0)
 02-MAR-1995  Th. Berkers    Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdlib.h>
#include  <string.h>
#include  <assert.h>
#include  "gxkernel.h"
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxprjct.h"
#include  "gxlithol.h"

/* Macro definitions */
/* ----------------- */
/* -- default values -- */
#define GX_DEFAULTASPHALTHACT    220000.0
#define GX_DEFAULTASPHALTLACT    205500.0
#define GX_DEFAULTCHARLENGTH     1.0
#define GX_DEFAULTHCI            1.2
#define GX_DEFAULTKEROGENHACT    235000.0
#define GX_DEFAULTKEROGENLACT    170000.0
#define GX_DEFAULTLOWERBIOT      10.0
#define GX_DEFAULTNCI            0.016
#define GX_DEFAULTOCI            0.18
#define GX_DEFAULTSCI            0.0
#define GX_DEFAULTS1I            5.0
#define GX_DEFAULTS2I            10.0
#define GX_DEFAULTS3I            1.0
#define GX_DEFAULTTMAXI          (450.0+GXC_ZEROCELCIUS)
#define GX_DEFAULTTOCI           0.1
#define GX_DEFAULTUPPERBIOT      10.0
#define GX_DEFAULTCHARLENGTH     1.0
#define GX_DEFAULTINITSRFRACT    -26.0
/* -- error text definitions -- */
#define  GXT_DUPLICATELITHOLOGY  "Lithology %s is already present " \
                                 "in project.\n"
#define  GXT_TOOMANYLITHOLOGIES  "The maximum number of lithologies has already been reached.\n" 


/*
--------------------------------------------------------------------------------
 GxCreateLithology - create a new lithology structure
--------------------------------------------------------------------------------

 GxCreateLithology allocates memory for a new lithology structure and
 initializes that memory.

 Arguments
 ---------
 lithologyptr - pointer for the lithology

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateLithology (Lithology **lithologyptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   Lithology    *lithology;

   if (!lithologyptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      lithology = ( Lithology * ) malloc (sizeof (Lithology));
      if (!lithology) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         lithology->name[0]             = '\0';
         lithology->toci                = GX_DEFAULTTOCI;
         lithology->s1i                 = GX_DEFAULTS1I;
         lithology->s2i                 = GX_DEFAULTS2I;
         lithology->s3i                 = GX_DEFAULTS3I;
         lithology->tmaxi               = GX_DEFAULTTMAXI;
         lithology->hci                 = GX_DEFAULTHCI;
         lithology->oci                 = GX_DEFAULTOCI;
         lithology->sci                 = GX_DEFAULTSCI;
         lithology->nci                 = GX_DEFAULTNCI;
         lithology->charlength          = GX_DEFAULTCHARLENGTH;
         lithology->upperbiot           = GX_DEFAULTUPPERBIOT;
         lithology->lowerbiot           = GX_DEFAULTLOWERBIOT;
         lithology->kerogenlowact       = GX_DEFAULTKEROGENLACT;
         lithology->kerogenhighact      = GX_DEFAULTKEROGENHACT;
         lithology->asphaltenelowact    = GX_DEFAULTASPHALTLACT;
         lithology->asphaltenehighact   = GX_DEFAULTASPHALTHACT;
         lithology->initfractionation   = GX_DEFAULTINITSRFRACT;
         lithology->fractionationfactor = GxGetDefIsoFractFactor ();
         *lithologyptr                  = lithology;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteLithology - remove a lithology from the project
--------------------------------------------------------------------------------

 GxDeleteLithology removes the lithology <name> from the project. If the
 lithology is referred to by another data unit an error will occur and the
 lithology will not be removed.

 Arguments
 ---------
 name - name of the lithology to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteLithology (const char *name)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *oldlithology, searchlithology;
   GxRock       *rock;

   if (!name || (strlen (name) == 0)
      || (strlen (name) > GXL_LITHOLOGYNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the lithology structure from project */
         strcpy (searchlithology.name, name);
         oldlithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                        (GxDataItem *) &searchlithology);
         if (!oldlithology) {
            rc = GxError (GXE_LITHOLOGYNOTPRESENT, GXT_LITHOLOGYNOTPRESENT,
                          name);
         } else {

/* -------- Remove any rock structures that refer to this lithology */
            rock = (GxRock *) GxGetFirstDataItem (GX_ROCK);
            while (rock) {
               if (rock->lithology == oldlithology) {
                   GxRemoveDataItem (GX_ROCK);
                   rock = (GxRock *) GxGetCurrentDataItem (GX_ROCK);
               } else {
                   rock = (GxRock *) GxGetNextDataItem (GX_ROCK);
               }
            }

/* -------- Remove the lithology from the project */
            GxRemoveDataItem (GX_LITHOLOGY);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeLithology - free the memory for the contents of a lithology
--------------------------------------------------------------------------------

 GxFreeLithology frees the memory that was allocated for the contents of
 one lithology in the project.

 Arguments
 ---------
 lithology - the lithology contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeLithology (Lithology *lithology)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!lithology) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (lithology);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLithology - return contents of one lithology in the project
--------------------------------------------------------------------------------

 GxGetLithology returns the contents of one of the lithologies in the project.

 Arguments
 ---------
 name      - name of the lithology
 lithology - lithology to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLithology (const char *name, Lithology *lithology)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *oldlithology, searchlithology;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_LITHOLOGYNAME)
      || !lithology) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the lithology structure from the project */
         strcpy (searchlithology.name, name);
         oldlithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                        (GxDataItem *) &searchlithology);
         if (!oldlithology) {
            rc = GxError (GXE_LITHOLOGYNOTPRESENT, GXT_LITHOLOGYNOTPRESENT,
                          name);
         } else {

/* -------- Copy the lithology data */
            strcpy (lithology->name, oldlithology->name);
            lithology->toci              = oldlithology->toci;
            lithology->s1i               = oldlithology->s1i;
            lithology->s2i               = oldlithology->s2i;
            lithology->s3i               = oldlithology->s3i;
            lithology->tmaxi             = oldlithology->tmaxi;
            lithology->hci               = oldlithology->hci;
            lithology->oci               = oldlithology->oci;
            lithology->sci               = oldlithology->sci;
            lithology->nci               = oldlithology->nci;
            lithology->charlength        = oldlithology->charlength;
            lithology->upperbiot         = oldlithology->upperbiot;
            lithology->lowerbiot         = oldlithology->lowerbiot;
            lithology->kerogenlowact     = oldlithology->kerogenlowact;
            lithology->kerogenhighact    = oldlithology->kerogenhighact;
            lithology->asphaltenelowact  = oldlithology->asphaltenelowact;
            lithology->asphaltenehighact = oldlithology->asphaltenehighact;
            lithology->initfractionation = oldlithology->initfractionation;
            lithology->fractionationfactor 
                                         = oldlithology->fractionationfactor;
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLithologyNameList - return the lithology names in the project
--------------------------------------------------------------------------------

 GxGetLithologyNameList returns a list with the names of the lithologies
 present in the project.

 Arguments
 ---------
 lithologylistptr - pointer to the lithology list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLithologyNameList (SortedList *lithologylistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   SortedList   lithologylist;
   GxLithology  *lithology;

   if (!lithologylistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create an empty sorted list for the lithology names */
         lithologylist = CreateSortedList (GXL_LITHOLOGYNAME+1,
                         (int (*) (const void *, const void *)) strcmp);
         if (!lithologylist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Insert the lithology names into the list */
            lithology = (GxLithology *) GxGetFirstDataItem (GX_LITHOLOGY);
            while (lithology
               && AppendToList (lithologylist, lithology->name))
               lithology = (GxLithology *) GxGetNextDataItem (GX_LITHOLOGY);
            if (lithology) {
               DeleteSortedList (lithologylist);
               rc = GxError (GXE_LIST, GXT_LIST);
            } else {
               *lithologylistptr = lithologylist;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertLithology - inserts a new lithology into the project
--------------------------------------------------------------------------------

 GxInsertLithology inserts information from the lithology <lithology> into
 the project.

 Arguments
 ---------
 lithology - the lithology to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertLithology (const Lithology *lithology)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  newlithology, *oldlithology, *insertedlithology;

   if (!lithology || (strlen (lithology->name) == 0)
      || (strlen (lithology->name) > GXL_LITHOLOGYNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the lithology is not yet present in the project */
         strcpy (newlithology.name, lithology->name);
         oldlithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                        (GxDataItem *) &newlithology);
         if (oldlithology) {
            rc = GxError (GXE_DUPLICATELITHOLOGY, GXT_DUPLICATELITHOLOGY,
                          newlithology.name);
         } else {

/* -------- Check that the maximum number of lithologies is not yet reached */
            if (GXN_LITHOLOGY <= GxGetDataItemCount (GX_LITHOLOGY)) {
                 rc = GxError (GXE_TOOMANYLITHOLOGIES,
                               GXT_TOOMANYLITHOLOGIES);
            } else {

/* ----------- Fill in the lithology and insert it into the project */
               newlithology.toci              = lithology->toci;
               newlithology.s1i               = lithology->s1i;
               newlithology.s2i               = lithology->s2i;
               newlithology.s3i               = lithology->s3i;
               newlithology.tmaxi             = lithology->tmaxi;
               newlithology.hci               = lithology->hci;
               newlithology.oci               = lithology->oci;
               newlithology.sci               = lithology->sci;
               newlithology.nci               = lithology->nci;
               newlithology.charlength        = lithology->charlength;
               newlithology.upperbiot         = lithology->upperbiot;
               newlithology.lowerbiot         = lithology->lowerbiot;
               newlithology.kerogenlowact     = lithology->kerogenlowact;
               newlithology.kerogenhighact    = lithology->kerogenhighact;
               newlithology.asphaltenelowact  = lithology->asphaltenelowact;
               newlithology.asphaltenehighact = lithology->asphaltenehighact;
               newlithology.initfractionation = lithology->initfractionation;
               newlithology.fractionationfactor 
                                              = lithology->fractionationfactor;
               rc = GxInsertDataItem (GX_LITHOLOGY,
                                     (GxDataItem *) &newlithology,
                                     (GxDataItem **) &insertedlithology);
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxReadLithology - read lithology from a project file
--------------------------------------------------------------------------------

 GxReadLithology - read the lithology from an ascii projectfile
 Arguments
 ---------
 filehandle - handle of file to which instance data should be read
 lithology  - lithology for which data should be read
 filename   - name of the projectfile
 version    - version of the projectfile

 Return value
 ------------
 GXE_FILEREAD  token not found by scanner

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadLithology (FILE *filehandle, GxLithology *lithology, 
		      const char *filename, GxPosGenexVersion version)
{
   GxErrorCode  rc;

   assert (filehandle);
   assert (lithology);
   assert (filename);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

   rc = GxGetInteger (filehandle, &lithology->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, lithology->name, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->toci, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->s1i, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->s2i, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->s3i, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->tmaxi, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->hci, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->oci, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->sci, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->nci, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->charlength, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->upperbiot, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->lowerbiot, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->kerogenlowact, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->kerogenhighact, filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->asphaltenelowact, 
                              filename);
   if (!rc) rc = GxGetDouble (filehandle, &lithology->asphaltenehighact, 
                              filename);
   if (!rc) {
      switch (version) {
         case GX_VERSION_3:
               lithology->initfractionation   = GX_DEFAULTINITSRFRACT;
               lithology->fractionationfactor = GxGetDefIsoFractFactor ();
            break;
         case GX_VERSION_4:
            rc = GxGetDouble (filehandle, &lithology->initfractionation,
                              filename);
            if (!rc) {
               rc = GxGetDouble (filehandle, &lithology->fractionationfactor, 
                                 filename);
            }
         break;
      }
   }

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxRenameLithology - change the name of a lithology
--------------------------------------------------------------------------------

 GxRenameLithology changes the name of a lithology in the project.
 If the new name is already a name of another lithology an error occurs.

 Arguments
 ---------
 oldname - name of the lithology to be renamed
 newname - new name to be given to the lithology

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameLithology (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *lithology, *oldlithology, searchlithology;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_LITHOLOGYNAME) || !newname
      || (strlen (newname) == 0)
      || (strlen (newname) > GXL_LITHOLOGYNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no lithology with the new name is present in project */
         strcpy (searchlithology.name, newname);
         oldlithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                                             (GxDataItem *) &searchlithology);
         if (oldlithology) {
            rc = GxError (GXE_DUPLICATELITHOLOGY, GXT_DUPLICATELITHOLOGY,
                          newname);
         } else {

/* -------- Check that the lithology to be renamed is present */
            strcpy (searchlithology.name, oldname);
            lithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                                           (GxDataItem *) &searchlithology);
            if (!lithology) {
               rc = GxError (GXE_LITHOLOGYNOTPRESENT,
                             GXT_LITHOLOGYNOTPRESENT, oldname);
            } else {

/* ----------- Change the name and adjust the position of the lithology */
               strcpy (lithology->name, newname);
               GxRepositionDataItem (GX_LITHOLOGY);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceLithology - replaces a lithology in the project
--------------------------------------------------------------------------------

 GxReplaceLithology replaces information from a lithology with new
 information.

 Arguments
 ---------
 lithology - the lithology to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceLithology (const Lithology *lithology)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxLithology  *oldlithology, searchlithology;

   if (!lithology || (strlen (lithology->name) == 0)
      || (strlen (lithology->name) > GXL_LITHOLOGYNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the lithology to be replaced is present */
         strcpy (searchlithology.name, lithology->name);
         oldlithology = (GxLithology *) GxSearchDataItem (GX_LITHOLOGY,
                        (GxDataItem *) &searchlithology);
         if (!oldlithology) {
            rc = GxError (GXE_LITHOLOGYNOTPRESENT, GXT_LITHOLOGYNOTPRESENT,
                          searchlithology.name);
         } else {

/* -------- Change the data of the lithology */
            oldlithology->toci              = lithology->toci;
            oldlithology->s1i               = lithology->s1i;
            oldlithology->s2i               = lithology->s2i;
            oldlithology->s3i               = lithology->s3i;
            oldlithology->tmaxi             = lithology->tmaxi;
            oldlithology->hci               = lithology->hci;
            oldlithology->oci               = lithology->oci;
            oldlithology->sci               = lithology->sci;
            oldlithology->nci               = lithology->nci;
            oldlithology->charlength        = lithology->charlength;
            oldlithology->upperbiot         = lithology->upperbiot;
            oldlithology->lowerbiot         = lithology->lowerbiot;
            oldlithology->kerogenlowact     = lithology->kerogenlowact;
            oldlithology->kerogenhighact    = lithology->kerogenhighact;
            oldlithology->asphaltenelowact  = lithology->asphaltenelowact;
            oldlithology->asphaltenehighact = lithology->asphaltenehighact;
            oldlithology->initfractionation = lithology->initfractionation;
            oldlithology->fractionationfactor =
                                              lithology->fractionationfactor;

/* -------- Indicate to the project that the lithology has changed */
            GxChangeProject ();
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteLithology - write lithology to a project file
--------------------------------------------------------------------------------

 GxWriteLithology - writes the lithology in an ascii projectfile
 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 lithology  - lithology for which data should be written
 version    - version format to save this lithology

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteLithology (FILE *filehandle, const GxLithology *lithology,
                       GxPosGenexVersion version)
{
   assert (filehandle);
   assert (lithology);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);

   fprintf (filehandle, "%d ", lithology->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %f ", lithology->name, lithology->toci);
   fprintf (filehandle, "%f %f ", lithology->s1i, lithology->s2i);
   fprintf (filehandle, "%f %f ", lithology->s3i, lithology->tmaxi);
   fprintf (filehandle, "%f %f ", lithology->hci, lithology->oci);
   fprintf (filehandle, "%f %f ", lithology->sci, lithology->nci);
   fprintf (filehandle, "%f %f ", lithology->charlength, lithology->upperbiot);
   fprintf (filehandle, "%f ", lithology->lowerbiot);
   fprintf (filehandle, "%f ", lithology->kerogenlowact);
   fprintf (filehandle, "%f ", lithology->kerogenhighact);
   fprintf (filehandle, "%f ", lithology->asphaltenelowact);
   fprintf (filehandle, "%f ", lithology->asphaltenehighact);
   if (version == GX_VERSION_4) {
      fprintf (filehandle, "%f ",  lithology->initfractionation);
      fprintf (filehandle, "%f\n", lithology->fractionationfactor);
   } else {
      fprintf (filehandle, "\n");
   }
}


