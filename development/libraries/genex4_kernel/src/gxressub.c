/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxressub.c,v 25.0 2005/07/05 08:03:49 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxressub.c,v $
 * Revision 25.0  2005/07/05 08:03:49  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:00  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:32  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:13  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:45  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:34  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:28  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:44  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:59  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:20  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:19  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:22  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:58  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:48  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:17  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:17  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:35  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:18  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxResSub.c   Genex reservoir subarea functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxResSub.c contains the Genex functions for handling reservoir
              subareas. The following functions are available:

                GxCopyToAllReservoirs     - copy data to all reservoirs
                GxCreateReservoirSubArea  - create new reservoir subarea
                GxFreeReservoirSubArea    - free a reservoir subarea
                GxGetReservoirSubArea     - return a reservoir subarea
		GxReadReservoirSubArea    - read struct from a file
                GxReplaceReservoirSubArea - replaces a reservoir subarea
	        GxWriteReservoirSubArea   - write struct to file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxressub.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteReservoirSubArea added
 28-MAR-1993  M. van Houtert GxReadReservoirSubArea added

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxerror.h"
#include  "gxprjct.h"
#include  "gxressub.h"

/* Macro definitions */
/* ----------------- */
#define GX_DEFAULTTOPDEPTH        3000.0
#define GX_DEFAULTGROSSTHICKNESS  500.0
#define GX_DEFAULTNETTOGROSS      0.70
#define GX_DEFAULTNETAVGPOROSITY  0.15
#define GX_DEFAULTWATERSALINITY   0.1

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxFindReservoirSubArea (const char *, const char *,
                                            GxSubArea **, GxHorizon **,
                                            GxReservoirSubArea **);



/*
--------------------------------------------------------------------------------
 GxCopyToAllReservoirs - copy reservoir subarea data to all reservoirs
--------------------------------------------------------------------------------

 GxCopyToAllReservoirs copies the reservoir subarea data for reservoir
 <reservoirname> to all other reservoirs.

 Arguments
 ---------
 reservoirname - name of the reservoir for which the data should be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyToAllReservoirs (const char *reservoirname)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxBool              initialized;
   GxHorizon           *sourcehorizon, searchhorizon, *horizon;
   GxSubArea           *subarea;
   GxReservoirSubArea  searchressub, *ressub, newressub;
   double              topdepth, grossthickness, nettogross, netavgporosity,
                       watersalinity;

   if (!reservoirname || (strlen (reservoirname) == 0)
      || (strlen (reservoirname) > GXL_HORIZONNAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the reservoir horizon is present */
         strcpy (searchhorizon.name, reservoirname);
         sourcehorizon = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                         (GxDataItem *) &searchhorizon);
         if (!sourcehorizon || (sourcehorizon->type != GX_RESERVOIR)) {
            rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                          reservoirname);
         } else {

/* -------- Loop over all subareas in the project */
            subarea = (GxSubArea *) GxGetFirstDataItem (GX_SUBAREA);
            while (!rc && subarea) {
               searchressub.horizon = sourcehorizon;
               searchressub.subarea = subarea;

/* ----------- Get the reservoir subarea from the project */
               ressub = (GxReservoirSubArea *)
                  GxSearchDataItem (GX_RESERVOIRSUBAREA,
                     (GxDataItem *) &searchressub);
               if (ressub) {

/* -------------- It is present: get its data from the structure */
                  topdepth        = ressub->topDepth;
                  grossthickness  = ressub->grossThickness;
                  nettogross      = ressub->netToGross;
                  netavgporosity  = ressub->netAvgPorosity;
                  watersalinity   = ressub->waterSalinity;
               } else {

/* -------------- It is not present: use the default values for the data */
                  topdepth        = GX_DEFAULTTOPDEPTH;
                  grossthickness  = GX_DEFAULTGROSSTHICKNESS;
                  nettogross      = GX_DEFAULTNETTOGROSS;
                  netavgporosity  = GX_DEFAULTNETAVGPOROSITY;
                  watersalinity   = GX_DEFAULTWATERSALINITY;
               }

/* ----------- Loop over all reservoir horizons in the project */
               horizon = (GxHorizon *) GxGetFirstDataItem (GX_HORIZON);
               while (!rc && horizon) {
                  if ((horizon != sourcehorizon)
                     && (horizon->type == GX_RESERVOIR)) {

/* ----------------- Get the reservoir subarea from the project */
                     searchressub.horizon = horizon;
                     searchressub.subarea = subarea;
                     ressub = (GxReservoirSubArea *)
                        GxSearchDataItem (GX_RESERVOIRSUBAREA,
                           (GxDataItem *) &searchressub);
                     if (ressub) {

/* -------------------- It is present: replace its data */
                        ressub->topDepth       = topdepth;
                        ressub->grossThickness = grossthickness;
                        ressub->netToGross     = nettogross;
                        ressub->netAvgPorosity = netavgporosity;
                        ressub->waterSalinity  = watersalinity;

/* -------------------- Indicate to the project that something has changed */
                        GxChangeProject ();
                     } else {

/* -------------------- It is not present: create and insert a new one */
                        newressub.horizon        = horizon;
                        newressub.subarea        = subarea;
                        newressub.topDepth       = topdepth;
                        newressub.grossThickness = grossthickness;
                        newressub.netToGross     = nettogross;
                        newressub.netAvgPorosity = netavgporosity;
                        newressub.waterSalinity  = watersalinity;
                        rc = GxInsertDataItem (GX_RESERVOIRSUBAREA,
                                (GxDataItem *) &newressub,
                                (GxDataItem **) &ressub);
                     }
                  }
                  horizon = (GxHorizon *) GxGetNextDataItem (GX_HORIZON);
               }
               subarea = (GxSubArea *) GxGetNextDataItem (GX_SUBAREA);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateReservoirSubArea - create a new reservoir subarea structure
--------------------------------------------------------------------------------

 GxCreateReservoirSubArea allocates memory for a new reservoir subarea
 structure and initializes that memory.

 Arguments
 ---------
 ressubptr - pointer for the reservoir subarea

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateReservoirSubArea (ReservoirSubArea **ressubptr)
{
   GxErrorCode       rc=GXE_NOERROR;
   ReservoirSubArea  *ressub;

   if (!ressubptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      ressub = ( ReservoirSubArea * )malloc (sizeof (ReservoirSubArea));
      if (!ressub) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         ressub->reservoir[0]   = '\0';
         ressub->subarea[0]     = '\0';
         ressub->topDepth       = GX_DEFAULTTOPDEPTH;
         ressub->grossThickness = GX_DEFAULTGROSSTHICKNESS;
         ressub->netToGross     = GX_DEFAULTNETTOGROSS;
         ressub->netAvgPorosity = GX_DEFAULTNETAVGPOROSITY;
         ressub->waterSalinity  = GX_DEFAULTWATERSALINITY;
         *ressubptr             = ressub;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFindReservoirSubArea - find a reservoir subarea in the project
--------------------------------------------------------------------------------

 GxFindReservoirSubArea searches for a particular reservoir subarea identified
 by subarea and reservoir horizon name in the project. If the subarea or
 reservoir horizon are not found an error is returned. Pointers to the
 subarea and reservoir horizon dataitems are returned. If the reservoir
 subarea itself is found a pointer to it is returned, otherwise NULL is
 returned.

 Arguments
 ---------
 reservoirname - name of the reservoir horizon
 subareaname   - name of the subarea
 subareaptr    - pointer to the subarea
 horizonptr    - pointer to the source rock horizon
 ressubptr     - pointer to the reservoir subarea or NULL if not present

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFindReservoirSubArea (const char *reservoirname,
                                           const char *subareaname,
                                           GxSubArea **subareaptr,
                                           GxHorizon **horizonptr,
                                           GxReservoirSubArea **ressubptr)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxReservoirSubArea  searchressub;
   GxHorizon           searchhorizon;
   GxSubArea           searchsubarea;

   assert (reservoirname);
   assert (subareaname);
   assert (subareaptr);
   assert (horizonptr);
   assert (ressubptr);

/* Find the reservoir horizon in the project */
   strcpy (searchhorizon.name, reservoirname);
   *horizonptr = (GxHorizon *) GxSearchDataItem (GX_HORIZON,
                 (GxDataItem *) &searchhorizon);
   if (!*horizonptr || ((*horizonptr)->type != GX_RESERVOIR)) {
        rc = GxError (GXE_RVHORIZONNOTPRESENT, GXT_RVHORIZONNOTPRESENT,
                    reservoirname);
   } else {

/* -- Find the subarea in the project */
      strcpy (searchsubarea.name, subareaname);
      *subareaptr = (GxSubArea *) GxSearchDataItem (GX_SUBAREA,
                    (GxDataItem *) &searchsubarea);
      if (!*subareaptr) {
         rc = GxError (GXE_SUBAREANOTPRESENT, GXT_SUBAREANOTPRESENT,
                       subareaname);
      } else {

/* ----- Find the reservoir subarea in the project */
         searchressub.horizon = *horizonptr;
         searchressub.subarea = *subareaptr;
         *ressubptr =
            (GxReservoirSubArea *) GxSearchDataItem (GX_RESERVOIRSUBAREA,
            (GxDataItem *) &searchressub);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeReservoirSubArea - free the memory for contents of a reservoir subarea
--------------------------------------------------------------------------------

 GxFreeReservoirSubArea frees the memory that was allocated for the contents
 of one reservoir subarea in the project.

 Arguments
 ---------
 ressub - the reservoir subarea contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeReservoirSubArea (ReservoirSubArea *ressub)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!ressub) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      free (ressub);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetReservoirSubArea - return contents of one reservoir subarea the project
--------------------------------------------------------------------------------

 GxGetReservoirSubArea returns the contents of one of the reservoir subareas
 in the project.

 Arguments
 ---------
 reservoirname - the name of the reservoir
 subareaname   - the name of the subarea
 ressub        - reservoir subarea to be returned

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetReservoirSubArea (const char *reservoirname,
                                   const char *subareaname,
                                   ReservoirSubArea *ressub)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxBool              initialized;
   GxSubArea           *subarea;
   GxHorizon           *horizon;
   GxReservoirSubArea  *oldressub;

   if (!reservoirname || (strlen (reservoirname) == 0)
      || (strlen (reservoirname) > GXL_HORIZONNAME) || !subareaname
      || (strlen (subareaname) == 0)
      || (strlen (subareaname) > GXL_SUBAREANAME) || !ressub) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the reservoir subarea in the project */
         rc = GxFindReservoirSubArea (reservoirname, subareaname, &subarea,
                                      &horizon, &oldressub);
         if (!rc) {

/* -------- Copy the identification of the reservoir subarea */
            strcpy (ressub->reservoir, reservoirname);
            strcpy (ressub->subarea,   subareaname);
            if (oldressub) {

/* ----------- Reservoir subarea was found: copy its data */
               ressub->topDepth       = oldressub->topDepth;
               ressub->grossThickness = oldressub->grossThickness;
               ressub->netToGross     = oldressub->netToGross;
               ressub->netAvgPorosity = oldressub->netAvgPorosity;
               ressub->waterSalinity  = oldressub->waterSalinity;
            } else {

/* ----------- Reservoir subarea was not found: assign default values */
               ressub->topDepth       = GX_DEFAULTTOPDEPTH;
               ressub->grossThickness = GX_DEFAULTGROSSTHICKNESS;
               ressub->netToGross     = GX_DEFAULTNETTOGROSS;
               ressub->netAvgPorosity = GX_DEFAULTNETAVGPOROSITY;
               ressub->waterSalinity  = GX_DEFAULTWATERSALINITY;
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxReadReservoirSubArea - read reservoir subarea from a project file
--------------------------------------------------------------------------------

 GxReadReservoirSubArea reads the reservoir subarea properties from an
 already open project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 reservoir  - The reference conditions
 filename   - Name of the projectfile

 Return value
 ------------

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadReservoirSubArea (FILE *filehandle, 
		GxReservoirSubArea *reservoir, const char *filename)
{
   GxErrorCode rc=GXE_NOERROR;

   assert (filehandle);
   assert (reservoir);
   assert (filename);

   rc = GxGetInteger (filehandle, &reservoir->dataitem.seqnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &reservoir->horizonnr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &reservoir->subareanr, filehandle);
   if (!rc) rc = GxGetDouble (filehandle, &reservoir->topDepth, filehandle);
   if (!rc) rc = GxGetDouble (filehandle, &reservoir->grossThickness, 
								filehandle);
   if (!rc) rc = GxGetDouble (filehandle, &reservoir->netToGross, 
								filehandle);
   if (!rc) rc = GxGetDouble (filehandle, &reservoir->netAvgPorosity, 
								filehandle);
   if (!rc) rc = GxGetDouble (filehandle, &reservoir->waterSalinity, 
								filehandle);

   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReplaceReservoirSubArea - replaces a reservoir subarea in the project
--------------------------------------------------------------------------------

 GxReplaceReservoirSubArea replaces information from a reservoir subarea
 with new information.

 Arguments
 ---------
 ressub - the reservoir subarea to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceReservoirSubArea (const ReservoirSubArea *ressub)
{
   GxErrorCode         rc=GXE_NOERROR;
   GxBool              initialized;
   GxSubArea           *subarea;
   GxHorizon           *horizon;
   GxReservoirSubArea  *oldressub, *insertedressub, newressub;

   if (!ressub || (strlen (ressub->reservoir) == 0)
      || (strlen (ressub->reservoir) > GXL_HORIZONNAME)
      || (strlen (ressub->subarea) == 0)
      || (strlen (ressub->subarea) > GXL_SUBAREANAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Find the reservoir subarea in the project */
         rc = GxFindReservoirSubArea (ressub->reservoir, ressub->subarea,
                                      &subarea, &horizon, &oldressub);
         if (!rc) {
            if (oldressub) {

/* ----------- It was found: copy the new data */
               oldressub->topDepth       = ressub->topDepth;
               oldressub->grossThickness = ressub->grossThickness;
               oldressub->netToGross     = ressub->netToGross;
               oldressub->netAvgPorosity = ressub->netAvgPorosity;
               oldressub->waterSalinity  = ressub->waterSalinity;

/* ----------- Indicate to the project that it has changed */
               GxChangeProject ();
            } else {

/* ----------- It was not found: insert a new reservoir subarea */
               newressub.horizon         = horizon;
               newressub.subarea         = subarea;
               newressub.topDepth        = ressub->topDepth;
               newressub.grossThickness  = ressub->grossThickness;
               newressub.netToGross      = ressub->netToGross;
               newressub.netAvgPorosity  = ressub->netAvgPorosity;
               newressub.waterSalinity   = ressub->waterSalinity;
               rc = GxInsertDataItem (GX_RESERVOIRSUBAREA,
                                     (GxDataItem *) &newressub,
                                     (GxDataItem **) &insertedressub);
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxWriteReservoirSubArea - write reservoir subarea to a project file
--------------------------------------------------------------------------------

 GxWriteReservoirSubArea writes the reservoir subarea properties to an
 already open project file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 reservoir  - The reference conditions

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteReservoirSubArea (FILE *filehandle, 
    const GxReservoirSubArea *reservoir)
{
   assert (filehandle);
   assert (reservoir);

   fprintf (filehandle, "%d ", reservoir->dataitem.seqnr);
   fprintf (filehandle, "%d %d ", reservoir->horizonnr, reservoir->subareanr);
   fprintf (filehandle, "%f ", reservoir->topDepth);
   fprintf (filehandle, "%f ", reservoir->grossThickness);
   fprintf (filehandle, "%f ", reservoir->netToGross);
   fprintf (filehandle, "%f ", reservoir->netAvgPorosity);
   fprintf (filehandle, "%f\n", reservoir->waterSalinity);
}
