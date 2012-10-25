/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxlocat.c,v 25.0 2005/07/05 08:03:47 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxlocat.c,v $
 * Revision 25.0  2005/07/05 08:03:47  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:58  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:28  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:11  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:43  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:25  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:22  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:35  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:45  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:13  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:04  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:14  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:45  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:23  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:02:53  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:04  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:20  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:01  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxLocat.c    Genex location data functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxLocat.c contains the Genex functions for handling location
              data. The following functions are available:

                GxCopyLocationData    - copy data for one location to another
                GxCreateLocationData  - create location data from raw data
                GxDestroyLocationData - destroy location data for a location
                GxGetLocationData     - get raw data from location data
		GxReadLocation        - read location struct from project file
                GxReadLocationData    - read location data from project file
                GxWriteRawDataFile    - read the file struct from to file
		GxWriteLocation       - write location data to project file
                GxWriteLocationData   - write location data to project file
		GxWriteRawDataFile    - write the file struct to project file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxlocat.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 23-MAR-1993  M. van Houtert GxWriteLocation added
 28-MAR-1993  M. van Houtert GxReadRawDataFile added
 28-MAR_1993  M. van Houtert GxReadLocation added.

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxerror.h"
#include  "gxfile.h"
#include  "gxlocat.h"
#include  "gxutils.h"

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocLocationData (int, int *, GxLocation *);



/*
--------------------------------------------------------------------------------
 GxAllocLocationData - allocate a new location data structure
--------------------------------------------------------------------------------

 GxAllocLocationData allocates memory for a new location data structure.

 Arguments
 ---------
 nhorizons - number of horizons in location data structure
 ntimes    - number of time entries in each of the horizons
 location  - the location for which the memory should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocLocationData (int nhorizons, int *ntimes,
                                        GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (nhorizons >= 0);
   assert ((nhorizons == 0) || ntimes);
   assert (location);

   location->nhorizons = nhorizons;
   location->ntimes    = NULL;
   location->physprop  = NULL;

/* Allocate arrays for the horizon data */
   if (nhorizons > 0) {
      rc = GxAllocArray (&location->ntimes, sizeof (int), 1, nhorizons);
      if (!rc) rc = GxAllocArray (&location->physprop, sizeof (double), 1,
                                  nhorizons);
      if (!rc) {
         for (i=0; i<nhorizons; i++) {
            location->physprop[i] = NULL;
            location->ntimes[i] = 0;
            if (!rc && (ntimes[i] > 0)) {
               location->ntimes[i] = ntimes[i];
               rc = GxAllocArray (&location->physprop[i], sizeof(double),
                                  2, GXN_LOCATPHYSPROP, ntimes[i]);
            }
         }
      }
   }

/* Free it in case of error */
   if (rc) (void) GxDestroyLocationData (location);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCopyLocationData - copy data from one location to another
--------------------------------------------------------------------------------

 GxCopyLocationData copies location data from one location struct to another.

 Arguments
 ---------
 destination - destination for location data
 source      - source of the location data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCopyLocationData (GxLocation *destination,
                                const GxLocation *source)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;

   assert (destination);
   assert (source);


/* Allocate the memory needed for the destination */
   rc = GxAllocLocationData (source->nhorizons, source->ntimes,
                             destination);
   if (!rc) {

/* -- Copy the the data horizon by horizon */
      for (i=0; i<source->nhorizons; i++)
         for (j=0; j<GXN_LOCATPHYSPROP; j++)
            memcpy (destination->physprop[i][j], source->physprop[i][j],
                    source->ntimes[i]*sizeof(double));
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateLocationData - create location data from horizon data
--------------------------------------------------------------------------------

 GxCreateLocationData allocates memory for a location data item by using the
 sizes defined in the raw data and copies the data into the location.

 Arguments
 ---------
 location - location to be created
 rdfile   - raw data with the horizon data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateLocationData (GxLocation *location,
                                  const RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, *ntimes;

   assert (location);
   assert (rdfile);


/* Create temporary array to hold the size of the data */
   rc = GxAllocArray (&ntimes, sizeof (int), 1, rdfile->nHorizonPoints);
   if (!rc) {
      for (i=0; i<rdfile->nHorizonPoints; i++)
         ntimes[i] = rdfile->horizonPoint[i].nTimeSteps;

/* -- Allocate the memory needed for the data */
      rc = GxAllocLocationData (rdfile->nHorizonPoints, ntimes, location);
      if (!rc) {

/* ----- Copy the horizon data into the allocated array */
         for (i=0; i<rdfile->nHorizonPoints; i++) {
            if (ntimes[i] > 0) {
               memcpy (location->physprop[i][GX_LOCATTIME],
                       rdfile->horizonPoint[i].time,
                       ntimes[i]*sizeof(double));
               memcpy (location->physprop[i][GX_LOCATDEPTH],
                       rdfile->horizonPoint[i].depth,
                       ntimes[i]*sizeof(double));
               memcpy (location->physprop[i][GX_LOCATTEMPERATURE],
                       rdfile->horizonPoint[i].temperature,
                       ntimes[i]*sizeof(double));
               memcpy (location->physprop[i][GX_LOCATPRESSURE],
                       rdfile->horizonPoint[i].pressure,
                       ntimes[i]*sizeof(double));
               memcpy (location->physprop[i][GX_LOCATVRE],
                       rdfile->horizonPoint[i].vre,
                       ntimes[i]*sizeof(double));
            }
         }
      }

/* -- Free the temporary array */
      GxFreeArray (ntimes, 1, rdfile->nHorizonPoints);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDestroyLocationData - destroy location data by freeing memory
--------------------------------------------------------------------------------

 GxDestroyLocationData frees the memory allocated for a location data item.

 Arguments
 ---------
 location - location to be destroyed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDestroyLocationData (GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (location);

   if (location->nhorizons > 0) {
      if (location->physprop) {

/* ----- Free the arrays with the data */
         for (i=0; i<location->nhorizons; i++)
            GxFreeArray (location->physprop[i], 2, GXN_LOCATPHYSPROP,
                         location->ntimes[i]);
         GxFreeArray (location->physprop, 1, location->nhorizons);
         location->physprop = NULL;
      }
      if (location->ntimes) {

/* ----- Free the array with the sizes of the horizons */
         GxFreeArray (location->ntimes, 1, location->nhorizons);
         location->ntimes = NULL;
      }
      location->nhorizons = 0;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLocationData - get horizon data from location data
--------------------------------------------------------------------------------

 GxGetLocationData copies horizon data from the location data to the raw
 data file structure.

 Arguments
 ---------
 rdfile   - raw data with horizon data to be created
 location - location data to be copied

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLocationData (RawDataFile *rdfile,
                               const GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, ntime;

   assert (rdfile);
   assert (location);


/* Copy the data for the horizons in the location into individual arrays */
   for (i=0; i<location->nhorizons; i++) {
      ntime = location->ntimes[i];
      memcpy (rdfile->horizonPoint[i].time,
              location->physprop[i][GX_LOCATTIME], ntime*sizeof(double));
      memcpy (rdfile->horizonPoint[i].depth,
              location->physprop[i][GX_LOCATDEPTH], ntime*sizeof(double));
      memcpy (rdfile->horizonPoint[i].temperature,
              location->physprop[i][GX_LOCATTEMPERATURE],
              ntime*sizeof(double));
      memcpy (rdfile->horizonPoint[i].pressure,
              location->physprop[i][GX_LOCATPRESSURE], ntime*sizeof(double));
      memcpy (rdfile->horizonPoint[i].vre,
              location->physprop[i][GX_LOCATVRE], ntime*sizeof(double));
   }
   return (rc);
}

/*
-------------------------------------------------------------------------------
-
 GxReadLocation - read location from an open project file
-------------------------------------------------------------------------------
-

 GxReadLocation reads a location from an already open project
 file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 location   - location to be read
 filename   - the name of the file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered
 GXE_FILEREAD token not found by scanner

-------------------------------------------------------------------------------
-
*/

GxErrorCode GxReadLocation (FILE *filehandle, GxLocation *location,
			    const char *filename)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (location);
   assert (filename);

   rc = GxGetInteger (filehandle, &location->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, location->name, filename);
   if (!rc) rc = GxGetDouble (filehandle, &location->easting, filename);
   if (!rc) rc = GxGetDouble (filehandle, &location->northing, filename);
   if (!rc) rc = GxGetInteger (filehandle, &location->filenr, filename);
   if (!rc) rc = GxGetInteger (filehandle, &location->subareanr, filename);
 
   return rc;
}


/*
--------------------------------------------------------------------------------
 GxReadLocationData - read location data from an open project file
--------------------------------------------------------------------------------

 GxReadLocationData reads a location data item from an already open project
 file into memory.

 Arguments
 ---------
 filehandle - handle of file from which project should be read
 filename   - the name of the file
 location   - location to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadLocationData (FILE *filehandle, const char *filename,
                                GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, nhorizons, *ntimes;

   assert (filehandle);
   assert (filename);
   assert (location);

/* Read the number of horizons */
   rc = GxGetInteger (filehandle, &nhorizons, filename);
   if (!rc) {
      if (nhorizons < 0) {
         rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
      } else {
         if (nhorizons > 0) {

/* -------- Allocate temporary array for the horizon size information */
            rc = GxAllocArray (&ntimes, sizeof (int), 1, nhorizons);
            if (!rc) {

/* ----------- Read the size of the data of the horizons */
               for (i=0; (!rc) && i<nhorizons; i++)
                  rc = GxGetInteger (filehandle, &ntimes[i], filename);

               for (i=0; (!rc)&&(i<nhorizons); i++)
                  if (ntimes[i] < 0)
                     rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT,
                                   filename);
            }
         }

/* ----- Allocate arrays big enough to hold the data */
         if (!rc) rc = GxAllocLocationData (nhorizons, ntimes, location);

         for (i=0; (!rc)&&(i<nhorizons); i++)
            if (ntimes[i] > 0)
               for (j=0; j<ntimes[i]; j++)
                  for (k=0; (!rc)&&(k<GXN_LOCATPHYSPROP); k++)
                     rc = GxGetDouble (filehandle, &location->physprop[i][k][j],
                               					filename);

/* ----- Free temporary array */
         GxFreeArray (ntimes, 1, nhorizons);
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxReadRawDataFile - read rawdata from an open project file
--------------------------------------------------------------------------------

 GxReadRawDataFile reads the struct of a rawdata file from an open project
 file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 rawdata    - struct to store the data in
 filename   - the name of the project file

 Return value
 ------------

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadRawDataFile  (FILE *filehandle, GxFile *file, 
   				const char *filename)
{
   GxErrorCode   rc=GXE_NOERROR;

   assert (filehandle);
   assert (file);
   assert (filename);

   rc = GxGetInteger (filehandle, &file->dataitem.seqnr, filename);
   if (!rc) rc = GxGetString (filehandle, file->name, filename);
   if (!rc) rc = GxGetInteger (filehandle, &file->locationnr, filename);
   return rc;
} 


/*
--------------------------------------------------------------------------------
 GxWriteLocation - write location to an open project file
--------------------------------------------------------------------------------

 GxWriteLocation writes one location to an already open project
 file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 location   - location for which data should be written

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteLocation (FILE *filehandle, const GxLocation *location)
{
   assert (filehandle);
   assert (location);

   fprintf (filehandle, "%d ", location->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %f ", location->name, location->easting); 
   fprintf (filehandle, "%f %d ", location->northing, location->filenr);
   fprintf (filehandle, "%d\n",   location->subareanr);
}



/*
--------------------------------------------------------------------------------
 GxWriteLocationData - write location data to an open project file
--------------------------------------------------------------------------------

 GxWriteLocationData writes data for one location to an already open project
 file.

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 location   - location for which data should be written

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteLocationData (FILE *filehandle, const GxLocation *location)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k;

   assert (filehandle);
   assert (location);

/* Write the number of horizons */
   fprintf (filehandle, "%d\n", location->nhorizons);
   if (location->nhorizons > 0) {

/* -- Write the size of the horizons */
      for (i = 0; i < location->nhorizons; i++) {
         fprintf (filehandle, "%d ", location->ntimes[i]);
      }
      fprintf (filehandle, "\n");
      for (i=0; i<location->nhorizons; i++) {
         if (location->ntimes[i] > 0) {
	    for (j=0; j<location->ntimes[i]; j++) {

                for (k=0; k<GXN_LOCATPHYSPROP; k++) {
		    fprintf (filehandle, "%f ", location->physprop[i][k][j]);
		}
		fprintf (filehandle, "\n");
	    }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxWriteRawDataFile - write imported rawdata file data to an open project file
--------------------------------------------------------------------------------

 GxWriteRawDataFile - write an imported burial file definition to an open
 project file. The name and coupled location number will be saved

 Arguments
 ---------
 filehandle - handle of file to which instance data should be written
 file       - Struct to be saved.

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxWriteRawDataFile (FILE *filehandle, const GxFile *file)
{
   assert (filehandle);
   assert (file);

   fprintf (filehandle, "%d ", file->dataitem.seqnr);
   fprintf (filehandle, "\"%s\" %d\n", file->name, file->locationnr);
}
  
