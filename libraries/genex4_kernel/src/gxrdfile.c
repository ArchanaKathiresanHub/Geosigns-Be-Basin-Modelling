/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxrdfile.c,v 25.0 2005/07/05 08:03:49 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxrdfile.c,v $
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
 * Revision 22.0  2002/06/28 12:09:33  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:27  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:43  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:56  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:19  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:17  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:21  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:56  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:46  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:13  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:15  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.1  1996/07/01  09:33:32  ibs
 * Extended 64 length to 512 for the file / directory names.
 *
 * Revision 10.0  1996/03/11  12:41:34  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:46  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxRdFile.c   Genex raw data file functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        23-JUN-1992

 Description: GxRdFile.c contains the Genex functions for handling raw data
              files such as importing files from external programs, for
              example Burmag. The following functions are available:

                GxCreateRawDataFile  - create a new raw data file structure
                GxDeleteRawDataFile  - remove raw data file from the project
                GxFreeRawDataFile    - free memory for raw data file contents
                GxGetRawDataFile     - return contents of one raw data file
                GxGetRawDataFileList - return raw data file names in project
                GxInsertRawDataFile  - inserts new raw data file in project
                GxReadBurmag30File   - reads foreign raw data file from disk
                GxReadBurmag40File   - read new burmag format from disk
                GxReadGHSFile        - reads foreigh Heat/Tisa file from disk
                GxRenameRawDataFile  - change the name of a raw data file
                GxReplaceRawDataFile - replaces a raw data file in project

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxrdfile.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 23-JUN-1992  P. Alphenaar  initial version
 01-SEP-1992  M.J.A. van Houtert GxReadRawDataFile renamed to GxReadBurmagFile
 01-SEP-1992  M.J.A. van Houtert GxReadGHSFile added
 03-SEP-1992  M.J.A. van Houtert GxConvertGHSData added
 03-SEP-1992  M.J.A. van Houtert GxFindTemperature added
 07-APR-1993  M.J.A. van Houtert Burmag 4.0 support
 09-DEC-1993  Th. Berkers GxConvertGHSData, vre data initialisation added
 12-DEC-1993  Th. Berkers When reallocating memory first check if some memory
                          had already been allocated, if so use realloc 
                          otherwise use malloc (On SUN platform realloc with 
                          NULL pointer generates an application crash).
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <float.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include  <malloc.h>
#include  <ctype.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxlocat.h"
#include  "gxprjct.h"
#include  "gxrdfile.h"
#include  "gxutils.h"
#include  "gxnumer.h"

#ifndef SEEK_SET

#define SEEK_SET        0

#endif

/* Macro definitions */
/* ----------------- */
/* -- definitions for Burmag history file -- */
#define  GXN_BURMAGHEADER        7
#define  GXN_BURMAGTRAILER       1
/* -- error text definitions -- */
#define  GXT_DUPLICATEFILE       "Raw data file %s is already " \
                                 "present in project.\n"
#define  GXT_DUPLICATELOC        "Location %s is already present " \
                                 "in project.\n"
#define  GXT_FILENOTPRESENT      "Raw data file %s is not " \
                                 "present in project.\n"
#define  GXT_INVBURMAGHIST       "The file %s is not a valid Burmag " \
                                 "history file.\n"
#define  GXT_LOCATIONREFERRED    "Location %s is referred to by " \
                                 "subarea %s.\n"
/* -- default values -- */
#define  GX_DEFAULTEASTING       0.0
#define  GX_DEFAULTNORTHING      0.0



/* Function prototypes */
/* ------------------- */
static double       GxFindTemperature (double, int, float *, float *);
static GxErrorCode  GxAllocHorizons (int, int *, RawDataFile *);
static GxErrorCode  GxConvertGHSData (FILE *, const char *, RawDataFile *, 
                                                int, float *, int, int *);
static GxErrorCode  GxFreeHorizons (RawDataFile *);



/*
--------------------------------------------------------------------------------
 GxAllocHorizons - allocate the horizons for a raw data file structure
--------------------------------------------------------------------------------

 GxAllocHorizons allocates memory for the horizon data of a raw data file.

 Arguments
 ---------
 nhorizons - number of horizons in raw data file
 ntimes    - number of time entries for each horizon
 rdfile    - the raw data file for which horizons should be allocated

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocHorizons (int nhorizons, int *ntimes,
                                    RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (nhorizons >= 0);
   assert ((nhorizons == 0) || ntimes);
   assert (rdfile);

   rdfile->nHorizonPoints = nhorizons;
   rdfile->horizonPoint   = NULL;
   if (nhorizons > 0) {

/* -- Allocate an array of structures, one structure for each horizon */
      rc = GxAllocArray (&rdfile->horizonPoint, sizeof (HorizonPoint),
                         1, nhorizons);
      if (!rc) {
         for (i=0; i<nhorizons; i++) {

/* -------- Initialize the structures (for error recovery later on) */
            rdfile->horizonPoint[i].time        = NULL;
            rdfile->horizonPoint[i].depth       = NULL;
            rdfile->horizonPoint[i].temperature = NULL;
            rdfile->horizonPoint[i].pressure    = NULL;
            rdfile->horizonPoint[i].vre         = NULL;
            rdfile->horizonPoint[i].nTimeSteps  = ntimes[i];
            if (!rc && (ntimes[i] > 0)) {

/* ----------- Allocate the arrays for the different properties */
               rc = GxAllocArray (&rdfile->horizonPoint[i].time,
                                  sizeof (double), 1, ntimes[i]);
               if (!rc) rc = GxAllocArray (&rdfile->horizonPoint[i].depth,
                                           sizeof (double), 1, ntimes[i]);
               if (!rc)
                  rc = GxAllocArray (&rdfile->horizonPoint[i].temperature,
                                     sizeof (double), 1, ntimes[i]);
               if (!rc) rc = GxAllocArray (&rdfile->horizonPoint[i].pressure,
                                           sizeof (double), 1, ntimes[i]);
               if (!rc) rc = GxAllocArray (&rdfile->horizonPoint[i].vre,
                                           sizeof (double), 1, ntimes[i]);
            }
         }
      }
   }

/* In case of error free any memory successfully allocated */
   if (rc) (void) GxFreeHorizons (rdfile);
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxConvertGHSData - Read the data and translate it into Genex data
--------------------------------------------------------------------------------

 Read the data from the Heat/Tisa file and convert it into Genex data.
 Memory is already allocated.

 Arguments
 ---------
 filehandle - handle of GHS file
 filename   - name of the GHS file
 rdfile     - pointer for the raw data file
 ncount     - # of timesteps
 timeTable  - Table with timesteps
 nhorizons  - # of horizons
 ntimes     - the size of the arrays

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode  GxConvertGHSData (FILE *filehandle, const char *filename, 
                                      RawDataFile *rdfile, int ncount,
                                      float *timeTable,    int nhorizons,
                                      int   *ntimes)
{
    GxErrorCode   rc=GXE_NOERROR;
    void         *buffer;
    double        depth;
    float        *floatPtr, *coord, *temp, time, max;
    int          *intPtr, i, j, npoint, offset, size;

    coord = NULL;
    temp  = NULL;
    size  = 0;

    for (i=0; i<nhorizons; i++) {
        offset = ncount - ntimes[i];
        for (j=0; j < ntimes[i]; j++) {
            rdfile->horizonPoint[i].time[j] = timeTable[offset+j];
        }
    }
        
    if (fseek(filehandle, 0L, SEEK_SET) != 0) {
        rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
    }
    if (!rc) rc = GxFindF77Label (filehandle, filename, "A3");
    if ( rc ) {
	GxClearError ();
        if (fseek(filehandle, 0L, SEEK_SET) != 0) {
            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
        } else {
            rc = GxFindF77Label (filehandle, filename, "A3_GHS");
        }
    }

    if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
    if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
    if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
    for (i = 0; i < nhorizons && !rc; i++) {
        rc = GxReadF77Record (filehandle, filename, &buffer);
        if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
  	if (!rc) {
            floatPtr = buffer;
            offset = ncount - ntimes[i];
            for (j=0; j < ntimes[i]; j++) {
                rdfile->horizonPoint[i].depth[j] = (double) floatPtr[j+offset];
		rdfile->horizonPoint[i].pressure[j] = 0.0;
		rdfile->horizonPoint[i].vre[j] = 0.0;
            }
            /* Check if layer is not completely eroded */
            j = ntimes[i];
            while ((rdfile->horizonPoint[i].depth[j - 1] == 0.0) && 
                   (j > 1)) {
                j--;
            }
            if (ntimes[i] != j) {
                rdfile->horizonPoint[i].nTimeSteps = j + 1;
            }
            for (j=0; j < ntimes[i]; j++) {
                if (rdfile->horizonPoint[i].depth[j] < 1.0) {
                   rdfile->horizonPoint[i].depth[j] = 1.0;
                }
            }
	}
    }
    if (!rc) {
        if (fseek(filehandle, 0L, SEEK_SET) != 0) {
            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
        } else {
            rc = GxFindF77Label (filehandle, filename, "A2");
            if ( rc ) {
		GxClearError ();
                if (fseek(filehandle, 0L, SEEK_SET) != 0) {
                    rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
                } else {
                    rc = GxFindF77Label (filehandle, filename, "A2_GHS");
                }
            }
            for (i = 0; !rc && i < ncount; i++) {
                rc = GxReadF77Record (filehandle, filename, &buffer);
                if (!rc) {
                    floatPtr = buffer;
                    time = *floatPtr++;
                    intPtr = (int *)floatPtr;
                    npoint = *intPtr;
                    if (size < npoint) {
                        if (coord) {
                            coord = (float *)realloc (coord, 
                                                      npoint * sizeof (float));
                        } else {
                            coord = (float *)malloc (npoint * sizeof (float));
                        }
                        if (temp) {
                            temp  = (float *)realloc (temp,  
                                                      npoint * sizeof (float));
                        } else {
                            temp  = (float *)malloc (npoint * sizeof (float));
                        }
                        size = npoint;
                    }
                    if (coord && temp) {
                        rc = GxReadF77Record(filehandle, filename, &buffer);
                        if (!rc) {
                            memcpy (coord, buffer, sizeof(float) * npoint);
                            max = coord[npoint - 1];
                            for (j = 0; j < npoint; j++) {
                                coord[j] = max - coord[j];
                            }
                        }
			if (!rc) rc = GxReadF77Record(filehandle, filename, 
								       &buffer);
			if (!rc) rc = GxReadF77Record(filehandle, filename, 
								       &buffer);
			if (!rc) rc = GxReadF77Record(filehandle, filename, 
								       &buffer);
                        if (!rc) {
                            rc = GxReadF77Record(filehandle, filename, &buffer);
                        }
                        if (!rc) {
                            memcpy (temp, buffer, sizeof(float) * npoint);
                        }
			if (!rc) rc = GxReadF77Record(filehandle, filename, 
								      &buffer);
			if (!rc) rc = GxReadF77Record(filehandle, filename, 
								      &buffer);
			if (!rc) {
			    for (j = 0; j < nhorizons; j++) {
	
				/* Check iftime is in range */
				if ((ncount - i) <= ntimes[j]) {
				    offset = ncount - ntimes[j];
				    depth = rdfile->horizonPoint[j].depth[i - offset];
				    rdfile->horizonPoint[j].temperature[i - offset] = 
				    GxFindTemperature (depth, npoint, coord, temp);
				}
			    }
			}
                    } else {
                        rc = GxError (GXE_MEMORY, GXT_MEMORY);
                    }
		}
	    }
 	}
    } 

    if (coord)     free (coord);
    if (temp)      free (temp);
            
    return rc;
}

/*
--------------------------------------------------------------------------------
 GxCreateRawDataFile - create a new raw data file structure
--------------------------------------------------------------------------------

 GxCreateRawDataFile allocates memory for a new raw data file structure and
 initializes that memory.

 Arguments
 ---------
 rdfileptr - pointer for the raw data file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateRawDataFile (RawDataFile **rdfileptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   RawDataFile  *rdfile;

   if (!rdfileptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rdfile = ( RawDataFile * ) malloc (sizeof (RawDataFile));
      if (!rdfile) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         rdfile->fileName[0]    = '\0';
         rdfile->locationId[0]  = '\0';
         rdfile->easting        = GX_DEFAULTEASTING;
         rdfile->northing       = GX_DEFAULTNORTHING;
         rdfile->nHorizonPoints = 0;
         rdfile->horizonPoint   = NULL;
         *rdfileptr = rdfile;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDeleteRawDataFile - remove a raw data file from the project
--------------------------------------------------------------------------------

 GxDeleteRawDataFile removes all the information that originated from the
 data file <name> from the project.

 Arguments
 ---------
 name - name of the file to be removed from the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDeleteRawDataFile (const char *name)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxFile       *file, searchfile;
   GxLocation   *location, searchloc;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the raw data file structure from project */
         strcpy (searchfile.name, name);
         file = (GxFile *) GxSearchDataItem (GX_FILE,
                                             (GxDataItem *) &searchfile);
         if (!file) {
            rc = GxError (GXE_FILENOTPRESENT, GXT_FILENOTPRESENT, name);
         } else {

/* -------- Check that the bidirection link is intact */
            if (!file->location || (file->location->file != file)) {
               rc = GxError (GXE_LINK, GXT_LINK);
            } else {

/* ----------- Get the location structure from project */
               strcpy (searchloc.name, file->location->name);
               location = (GxLocation *) GxSearchDataItem (GX_LOCATION,
                          (GxDataItem *) &searchloc);

/* ----------- Check that the right location was found */
               if (location != file->location) {
                  rc = GxError (GXE_LINK, GXT_LINK);
               } else {

/* -------------- Check that no subarea refers to this location */
                  if (location->subarea) {
                      rc = GxError (GXE_LOCATIONREFERRED, GXT_LOCATIONREFERRED,
                                   location->name, location->subarea->name);
                  } else {

/* ----------------- Free the data and remove location and raw data file */
                     rc = GxDestroyLocationData (location);
                     GxRemoveDataItem (GX_FILE);
                     GxRemoveDataItem (GX_LOCATION);
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
 GxFindTemperature - Find the temperature in a given range
--------------------------------------------------------------------------------

 Finds a temperature in a given range. If the requested depth is outside
 the range, 0.0 or the max temperature will be returned. If the requested
 depth is not available, the value is interpolated

 Arguments
 ---------
 depth   - Requested depth
 npoints - size of the tables
 depths  - table of given depths
 temps   - table of given temps

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static double GxFindTemperature ( double depth, int npoints, float * depths,
			  float *temps )
{
    double result;
    int    i;

    if (depth >= depths[0]) {
	result = (double)temps[0];
    } else if (depth <= depths[npoints - 1]) {
	result = (double)temps[npoints - 1];
    } else {
	for (i=1; i < npoints; i++) {
	    if (depth > depths[i]) {
		result = GxLinearInterpolate (depth, (double)depths[i],
				(double)depths[i-1], (double)temps[i],
				(double)temps[i-1]);
	        break;
	    } else if (depth == depths[i]) {
		result = (double)temps[i];
		break;
	    }
	}
    }
    return result + GXC_ZEROCELCIUS;
}
/*
--------------------------------------------------------------------------------
 GxFreeHorizons - free the memory for the horizons of a raw data file
--------------------------------------------------------------------------------

 GxFreeHorizons frees the memory that was allocated for the horizons of
 one raw data file in the project.

 Arguments
 ---------
 rdfile - the raw data file for which the horizons should be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeHorizons (RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (rdfile);

   if (rdfile->horizonPoint) {

/* -- Free the memory allocated for each of the horizons */
      for (i=0; i<rdfile->nHorizonPoints; i++) {
         GxFreeArray (rdfile->horizonPoint[i].time, 1,
                      rdfile->horizonPoint[i].nTimeSteps);
         GxFreeArray (rdfile->horizonPoint[i].depth, 1,
                      rdfile->horizonPoint[i].nTimeSteps);
         GxFreeArray (rdfile->horizonPoint[i].temperature, 1,
                      rdfile->horizonPoint[i].nTimeSteps);
         GxFreeArray (rdfile->horizonPoint[i].pressure, 1,
                      rdfile->horizonPoint[i].nTimeSteps);
         GxFreeArray (rdfile->horizonPoint[i].vre, 1,
                      rdfile->horizonPoint[i].nTimeSteps);
      }
   }

/* Free the array with the horizon point structures */
   if (rdfile->nHorizonPoints > 0)
      GxFreeArray (rdfile->horizonPoint, 1, rdfile->nHorizonPoints);
   rdfile->horizonPoint = NULL;
   rdfile->nHorizonPoints = 0;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeRawDataFile - free the memory for the contents of a raw data file
--------------------------------------------------------------------------------

 GxFreeRawDataFile frees the memory that was allocated for the contents of
 one raw data file in the project.

 Arguments
 ---------
 rdfile - the raw data file contents to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeRawDataFile (RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!rdfile) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxFreeHorizons (rdfile);
      if (!rc) free (rdfile);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRawDataFile - return contents of one raw data file in the project
--------------------------------------------------------------------------------

 GxGetRawDataFile returns the contents of one of the raw data files in the
 project.

 Arguments
 ---------
 name   - name of the raw data file
 rdfile - raw data file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRawDataFile (const char *name,  RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxFile       *file, searchfile;

   if (!name || (strlen (name) == 0) || (strlen (name) > GXL_FILENAME)
      || !rdfile) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Get the raw data file structure from the project */
         strcpy (searchfile.name, name);
         file = (GxFile *) GxSearchDataItem (GX_FILE,
                                             (GxDataItem *) &searchfile);
         if (!file) {
            rc = GxError (GXE_FILENOTPRESENT, GXT_FILENOTPRESENT, name);
         } else {

/* -------- Empty the structure in which the data should be returned */
            rc = GxFreeHorizons (rdfile);
            if (!rc) {

/* ----------- Re-allocate the structure and fill it with data from project */
               strcpy (rdfile->fileName,   file->name);
               strcpy (rdfile->locationId, file->location->name);
               rdfile->easting  = file->location->easting;
               rdfile->northing = file->location->northing;
               rc = GxAllocHorizons (file->location->nhorizons,
                                     file->location->ntimes, rdfile);
               if (!rc) rc = GxGetLocationData (rdfile, file->location);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRawDataFileList - return the raw data file names in the project
--------------------------------------------------------------------------------

 GxGetRawDataFileList returns a list with the names of the raw data files
 present in the project.

 Arguments
 ---------
 filelistptr - pointer to the file list

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRawDataFileList (SortedList *filelistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   SortedList   filelist;
   GxFile       *file;

   if (!filelistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Create an empty sorted list for the raw data file names */
         filelist = CreateSortedList (GXL_FILENAME+1,
                    (int (*) (const void *, const void *)) strcmp);
         if (!filelist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Insert the raw data file names into the list */
            file = (GxFile *) GxGetFirstDataItem (GX_FILE);
            while (file && AppendToList (filelist, file->name))
               file = (GxFile *) GxGetNextDataItem (GX_FILE);
            if (file) {
               DeleteSortedList (filelist);
               rc = GxError (GXE_LIST, GXT_LIST);
            } else {
               *filelistptr = filelist;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInsertRawDataFile - inserts a new raw data file into the project
--------------------------------------------------------------------------------

 GxInsertRawDataFile inserts information from the raw data file <rdfile> into
 the project.

 Arguments
 ---------
 rdfile - the file to be inserted into the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInsertRawDataFile (const RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxFile       newfile, *oldfile, *file;
   GxLocation   newloc, *oldloc, *location;

   if (!rdfile || (strlen (rdfile->fileName) == 0)
      || (strlen (rdfile->fileName) > GXL_FILENAME)
      || (strlen (rdfile->locationId) == 0)
      || (strlen (rdfile->locationId) > GXL_LOCATIONID)
      || (rdfile->nHorizonPoints == 0) || !rdfile->horizonPoint) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the raw data file is not yet present in the project */
         strcpy (newfile.name, rdfile->fileName);
         oldfile = (GxFile *) GxSearchDataItem (GX_FILE,
                                                (GxDataItem *) &newfile);
         if (oldfile) {
            rc = GxError (GXE_DUPLICATEFILE, GXT_DUPLICATEFILE, newfile.name);
         } else {

/* -------- Allocate and fill in the raw data file */
            strcpy (newloc.name, rdfile->locationId);
            newloc.subarea  = NULL;
            newloc.easting  = rdfile->easting;
            newloc.northing = rdfile->northing;
            rc = GxCreateLocationData (&newloc, rdfile);
            if (!rc) {

/* ----------- Check that the location is not yet present in the project */
               oldloc = (GxLocation *) GxSearchDataItem (GX_LOCATION,
                        (GxDataItem *) &newloc);
               if (oldloc) {
                  rc = GxError (GXE_DUPLICATELOC, GXT_DUPLICATELOC,
                                newloc.name);
               } else {

/* -------------- Insert the raw data file into the project */
                  rc = GxInsertDataItem (GX_FILE, (GxDataItem *) &newfile,
                                         (GxDataItem **) &file);
                  if (!rc) {

/* ----------------- Insert the location into the project */
                     rc = GxInsertDataItem (GX_LOCATION,
                                            (GxDataItem *) &newloc,
                                            (GxDataItem **) &location);
                     if (rc) {
                        GxRemoveDataItem (GX_FILE);
                     } else {

/* -------------------- Set up the link between raw data file and location */
                        file->location = location;
                        location->file = file;
                     }
                  }
               }
               if (rc) (void) GxDestroyLocationData (&newloc);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadBurmag30File - reads a foreign raw data file from disk
--------------------------------------------------------------------------------

 GxReadBurmag30File reads the Burmag history file <filename> from disk and
 returns it in the raw data file structure <rdfile>.

 Arguments
 ---------
 filename - name of Burmag history file
 rdfile   - the contents of the file <filename>

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadBurmag30File (const char *filename, RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized, notWhite;
   FILE         *filehandle;
   int          i, n, c, ntimes[1], result;
   double       dummy;


   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME) || !rdfile) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Open the Burmag file */
         filehandle = fopen (filename, "rt");
         if (!filehandle) {
            rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, filename);
         } else {

/* -------- Count the number of non-empty lines in the file */
/*
            (void) fscanf (filehandle, "\n");
            n = 1;
            while (fscanf (filehandle, "%*[^\n]\n") != EOF) n++;
*/

/* -------- Read 'till first dashed line */
	    n = 0;
	    c = fgetc (filehandle);
	    while (c != '-' && c != EOF) {
		while (c != EOF && c != '\n') {
		    c = fgetc (filehandle);
		}
		if (c != EOF) {
		    c = fgetc (filehandle);
		}
	    }
	    if ( c == '-' ) {

/* ------------ Skip dashed line and search the next one */
		while (c != EOF && c != '\n') {
		    c = fgetc (filehandle);
		}
   		if (c != EOF) {
		    c = fgetc (filehandle);
		    while ( c != EOF && c != '-') {
			while (c != EOF && c != '\n') {
			    c = fgetc (filehandle);
			}
			if (c == '\n') {
			    n++;
			    c = fgetc (filehandle);
			}
		    }
		}
	    }

/* -------- Check that there are at least two horizon data lines */
            if (n < 2) {
               rc = GxError (GXE_INVBURMAGHIST, GXT_INVBURMAGHIST, filename);
            } else {

/* ----------- Re-allocate the raw data file structure to the right size */
               ntimes[0] = n;
               rc = GxFreeHorizons (rdfile);
               if (!rc) rc = GxAllocHorizons (1, ntimes, rdfile);
               if (!rc) {

/* -------------- Rewind the Burmag file to the beginning */
                  if (fseek (filehandle, 0L, SEEK_SET) != 0) {
                     rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
                  } else {

/* ----------------- Set the name of the raw data file structure */
                     strcpy (rdfile->fileName, filename);

/* ----------------- Read the location name from the Burmag file header */
                     (void) fscanf (filehandle, "%*[^\n]\n");
                     (void) fscanf (filehandle, "Date: %*s Location: ");
                     if (fscanf (filehandle,
                                 "%" "20" "[^\n]",                       
                                 rdfile->locationId) != 1) {
                        rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
                     } else {

/* -------------------- Reset the position of the location */
                        rdfile->easting  = 0.0;
                        rdfile->northing = 0.0;

/* -------------------- Skip the rest of the Burmag file header */
                        (void) fscanf (filehandle, "%*[^\n]");
                        (void) fscanf (filehandle, "\n");
                        for (i=0; i<GXN_BURMAGHEADER-2; i++)
                           (void) fscanf (filehandle, "%*[^\n]\n");

/* -------------------- Read the horizon data skipping undecompacted depth */
                        for (i=0; (!rc)&&(i<ntimes[0]); i++) {
                           (void) fscanf (filehandle, "%lf %lf %lf %lf %lf\n",
                                      &rdfile->horizonPoint[0].time[i],
                                      &dummy,
                                      &rdfile->horizonPoint[0].depth[i],
                                      &rdfile->horizonPoint[0].temperature[i],
                                      &rdfile->horizonPoint[0].vre[i]);
                        }
                     }
                  }
               }
            }

/* -------- Close the Burmag file */
            if (fclose (filehandle) && !rc)
               rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE, filename);
         }
      }
   }
   if (!rc) {

/* -- Convert the temperatures to Kelvin and reset the pressures */
      for (i=0; i<ntimes[0]; i++) {
         rdfile->horizonPoint[0].temperature[i] += GXC_ZEROCELCIUS;
         rdfile->horizonPoint[0].pressure[i]     = 0.0;
/* ----- Minimal depth in Genex is 1.0 meter */
         if (rdfile->horizonPoint[0].depth[i] < 1.0) {
             rdfile->horizonPoint[0].depth[i] = 1.0;
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxReadBurmag40File - reads a foreign raw data file from disk
--------------------------------------------------------------------------------

 GxReadBurmag40File reads the Burmag history file <filename> from disk and
 returns it in the raw data file structure <rdfile>. The routine handles
 the new Unix version of burmag and supports multiple layers

 Arguments
 ---------
 filename - name of Burmag history file
 rdfile   - the contents of the file <filename>

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadBurmag40File (const char *filename, RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   FILE         *filehandle;
   int          i, j, nLayer;
   int         *nTimes = NULL;
   double       dummy;
   char         dummyStr[512];


   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME) || !rdfile) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Open the Burmag file */
         filehandle = fopen (filename, "rt");
         if (!filehandle) {
            rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, filename);
         } else {
            strcpy (rdfile->fileName, filename);
            rdfile->easting  = 0.0;
            rdfile->northing = 0.0;
            rc = GxGetString (filehandle, &rdfile->locationId, filename);
            if (!rc) rc = GxGetString (filehandle, &dummyStr, filename);
            if (!rc) rc = GxGetDouble (filehandle, &dummy, filename);
            if (!rc) rc = GxGetInteger (filehandle, &nLayer, filename);
            rc = GxAllocArray (&nTimes, sizeof (int), 1, nLayer);
            if (!rc) {
               for (i=0; (i<nLayer)&&(!rc); i++) {
                  rc = GxGetInteger (filehandle, &nTimes[i], filename);
               }
            } 
            if (!rc) {
               rc = GxFreeHorizons (rdfile);
               if (!rc) rc = GxAllocHorizons (nLayer, nTimes, rdfile);
               if (!rc) {
	          for (i=0; (i<nLayer)&&(!rc); i++) {
                     for (j=0; (j<nTimes[i])&&(!rc); j++) {
                         rc = GxGetDouble (filehandle, 
                             &rdfile->horizonPoint[i].time[j], filename);
                         if (!rc) rc = GxGetDouble (filehandle, &dummy, 
                                                                     filename);
                         if (!rc) rc = GxGetDouble (filehandle, 
                             &rdfile->horizonPoint[i].depth[j], filename);
                         if (!rc) rc = GxGetDouble (filehandle, 
                             &rdfile->horizonPoint[i].temperature[j], filename);
                         if (!rc) rc = GxGetDouble (filehandle, 
                             &rdfile->horizonPoint[i].vre[j], filename);
                      }
                  }   
               }
            }
         }

/* -------- Close the Burmag file */
         GxCloseFile (filehandle);
      }
   }
   if (!rc) {

/* -- Convert the temperatures to Kelvin and reset the pressures */
      for (i=0; i<nLayer; i++) {
         for (j=0; j<nTimes[i]; j++) {
            rdfile->horizonPoint[i].temperature[j] += GXC_ZEROCELCIUS;
            rdfile->horizonPoint[i].pressure[j]     = 0.0;
/* -------- Minimal depth in Genex is 1.0 meter */
            if (rdfile->horizonPoint[i].depth[j] < 1.0) {
               rdfile->horizonPoint[i].depth[j] = 1.0;
            }
         }
      }
   }
   if (nTimes) {
      GxFreeArray (nTimes, 1, nLayer);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxReadGHSFile - reads a foreign raw data file from disk
--------------------------------------------------------------------------------

 GxReadGHSFile reads the Heat/Tisa history file <filename> from disk and
 returns it in the raw data file structure <rdfile>.

 Arguments
 ---------
 filename - name of GHS history file
 rdfile   - the contents of the file <filename>

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadGHSFile (const char *filename, RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   FILE         *filehandle;
   void         *buffer;
   float        *floatPtr, *timeTable, age;
   int          *intPtr, *ntimes, i, j, ncount, nhorizons;


   if (!filename || (strlen (filename) == 0)
      || (strlen (filename) > GXL_FILENAME) || !rdfile) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {
         rc = GxOpenF77File (filename, &filehandle);
         if (!rc) {
            rc = GxReadF77Record (filehandle, filename, &buffer);
            if (strncmp (buffer, "A1", 2) == 0 || 
                strncmp (buffer, "A1_GHS", 6) == 0 && !rc) {
                strcpy  (rdfile->fileName, filename);
                strncpy (rdfile->locationId, buffer, GXL_LOCATIONID);
                rdfile->locationId[GXL_LOCATIONID] = '\0';
                rc = GxReadF77Record (filehandle, filename, &buffer);
                if (!rc) {
                    intPtr = buffer;
                    ncount = *intPtr++;
                    floatPtr = (float *) intPtr;
                    timeTable = (float *)malloc (sizeof(float) * ncount);
                    if (timeTable) {
                        for (i=0; i < ncount; i++) {
                            timeTable[i] = *floatPtr++;
                        }
                    } else {
                        rc = GxError (GXE_MEMORY, GXT_MEMORY);
                    }
                }
                if (!rc) rc = GxFindF77Label (filehandle, filename, "A3"); 
                if ( rc ) {
                    GxClearError ();
                    if (fseek(filehandle, 0L, SEEK_SET) != 0) {
                        rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
                    } else {
                        rc = GxFindF77Label (filehandle, filename, "A3_GHS");
                    }
                }

                if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
                if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
                if (!rc) rc = GxReadF77Record (filehandle, filename, &buffer);
                if (!rc) {
                    intPtr = buffer;
                    nhorizons = *intPtr;
                    ntimes = (int *)malloc (sizeof(int) * nhorizons);
                    if (ntimes) {
                        for (i=0; i < nhorizons && !rc; i++) {
                            rc = GxReadF77Record(filehandle, filename, &buffer);
                            floatPtr = buffer;
                            age = *floatPtr;
                            if (!rc) {
                                for (j=0; j < ncount; j++) {
                                    if (age >= timeTable[j]) {
                                        break;
                                    }
                                }
                                ntimes[i] = ncount - j;
                            }
                            if (!rc) rc = GxReadF77Record(filehandle, 
                                                          filename, &buffer);
                        }
                        if (!rc) rc = GxFreeHorizons (rdfile);
                        if (!rc) {
                            rc = GxAllocHorizons (nhorizons, ntimes, rdfile);
                        }
                        /* Memory allocated, start the actual reading */
                        if (!rc) rc = GxConvertGHSData (filehandle, filename, 
				rdfile, ncount, timeTable,nhorizons, ntimes);
                        
                    } else {
                        rc = GxError (GXE_MEMORY, GXT_MEMORY);
                    }
                }
                
            }
            GxCloseF77File (filehandle);
         }
      }
   }
   if (timeTable) free (timeTable);
   if (ntimes)    free (ntimes);
   return rc;
}
         

/*
--------------------------------------------------------------------------------
 GxRenameRawDataFile - change the name of a raw data file
--------------------------------------------------------------------------------


 GxRenameRawDataFile changes the name of a raw data file in the project.
 If the new name is already a name of another raw data file an error occurs.

 Arguments
 ---------
 oldname - name of the raw data file to be renamed
 newname - new name to be given to the raw data file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRenameRawDataFile (const char *oldname, const char *newname)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxFile       *file, *oldfile, searchfile;

   if (!oldname || (strlen (oldname) == 0)
      || (strlen (oldname) > GXL_FILENAME) || !newname
      || (strlen (newname) == 0) || (strlen (newname) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that no raw data file with the new name is already present */
         strcpy (searchfile.name, newname);
         oldfile = (GxFile *) GxSearchDataItem (GX_FILE,
                                                (GxDataItem *) &searchfile);
         if (oldfile) {
            rc = GxError (GXE_DUPLICATEFILE, GXT_DUPLICATEFILE, newname);
         } else {

/* -------- Check that the raw data file to be renamed is present */
            strcpy (searchfile.name, oldname);
            file = (GxFile *) GxSearchDataItem (GX_FILE,
                                               (GxDataItem *) &searchfile);
            if (!file) {
               rc = GxError (GXE_FILENOTPRESENT, GXT_FILENOTPRESENT, oldname);
            } else {

/* ----------- Change the name and adjust the position of the raw data file */
               strcpy (file->name, newname);
               GxRepositionDataItem (GX_FILE);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReplaceRawDataFile - replaces a raw data file in the project
--------------------------------------------------------------------------------

 GxReplaceRawDataFile replaces information from a raw data file with new
 information.

 Arguments
 ---------
 rdfile - the file to be replaced in the project

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReplaceRawDataFile (const RawDataFile *rdfile)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxBool       initialized;
   GxFile       *file, searchfile;
   GxLocation   *location, searchloc;

   if (!rdfile || (strlen (rdfile->fileName) == 0)
      || (strlen (rdfile->fileName) > GXL_FILENAME)
      || (strlen (rdfile->locationId) == 0)
      || (strlen (rdfile->locationId) > GXL_LOCATIONID)
      || (rdfile->nHorizonPoints == 0) || !rdfile->horizonPoint) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the project has been initialized */
      rc = GxProjectInitialized (&initialized);
      if (!rc && !initialized)
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      if (!rc) {

/* ----- Check that the raw data file to be replaced is present */
         strcpy (searchfile.name, rdfile->fileName);
         file = (GxFile *) GxSearchDataItem (GX_FILE,
                                             (GxDataItem *) &searchfile);
         if (!file) {
            rc = GxError (GXE_FILENOTPRESENT, GXT_FILENOTPRESENT,
                          searchfile.name);
         } else {

/* -------- Check that no location with the new name is present in project
   -------- unless it is the same location */
            strcpy (searchloc.name, rdfile->locationId);
            location = (GxLocation *) GxSearchDataItem (GX_LOCATION,
                       (GxDataItem *)  &searchloc);
            if (location && (location != file->location)) {
               rc = GxError (GXE_DUPLICATELOC, GXT_DUPLICATELOC,
                             searchloc.name);
            } else {

/* ----------- Check that the bi-directional link is intact */
               if (!file->location || (file->location->file != file)) {
                  rc = GxError (GXE_LINK, GXT_LINK);
               } else {

/* -------------- Free the memory in use for the current data */
                  rc = GxDestroyLocationData (file->location);
                  if (!rc) {

/* ----------------- Allocate memory for the new data and fill it with data */
                     strcpy (file->location->name, rdfile->locationId);
                     file->location->easting  = rdfile->easting;
                     file->location->northing = rdfile->northing;
                     rc = GxCreateLocationData (file->location, rdfile);
                  }

/* -------------- Indicate to the project that it has changed */
                  GxChangeProject ();
               }
            }
         }
      }
   }
   return (rc);
}
