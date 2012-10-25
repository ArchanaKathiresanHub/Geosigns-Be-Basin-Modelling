/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxfile.c,v 25.0 2005/07/05 08:03:44 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxfile.c,v $
 * Revision 25.0  2005/07/05 08:03:44  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:54  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:21  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:08  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:40  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:15  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:17  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:27  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:36  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:05  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:49  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:06  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:30  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:00  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:21  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:51  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:03  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:12  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxFile.c     Genex file reading and writing functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxFile.c contains the Genex functions for reading and writing
              project files. The following functions are available:

		GxCloseF77File     - Close a previous open F77 file
		GxFindF77Label     - Find a label in the F77 file
		GxOpenF77File      - Open a unformatted fortran file
                GxRead             - read data from the project file
                GxReadFileHeader   - read the header of the project file
		GxReadF77Record    - Read a string from unformatted file
                GxWrite            - write data to the project file
                GxWriteFileHeader  - write the header of the project file

 Usage notes: - In order to use the functions in this source file the header
                file gxfile.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 24-JUN-1992  P. Alphenaar  initial version
 01-SEP-1992  M.J.A van Houtert GxOpenF77File added
 01-SEP-1992  M.J.A van Houtert GxReadF77Record added
 01-SEP-1992  M.J.A van Houtert GxCloseF77File added
 01-SEP-1992  M.J.A van Houtert GxFindF77Label added
 22-MAR-1992  M.J.A van Houtert GxWriteFileTrailer removed
 09-DEC-1993  Th. Berkers GxCloseF77File setting fileBuffer to NULL after 
                          freeing fileBuffer memory.
 12-DEC-1993  Th. Berkers When reallocating memory first check if some memory
                          had already been allocated, if so use realloc 
                          otherwise use malloc (On SUN platform realloc with 
                          NULL pointer generates an application crash).
 20-MAR-1995  Th. Berkers Integration of Isotopes Fractionation:
                          Genex version in project file checked
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  "gxerror.h"
#include  "gxfile.h"
/* Macro definitions */
/* ----------------- */
/* -- project file type id and revision id -- */
#define  M_FILETYPE           25
#define  PROJECTFILEID3       "GENEX3/PROJECT/"
#define  PROJECTFILEID4       "GENEX4/PROJECT/"
#define  PROJECTFILEREVID     1
/* -- error text definitions -- */
#define  GXT_INVFILEREVISION  "The project file %s is saved by a " \
                              "different version of Genex.\n" \
                              "The version of Genex you are using is " \
                              "unable to read this file.\n"

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef  struct FileHeader   FileHeader;
struct FileHeader {
   char  type[M_FILETYPE+1];
   int   revision;
};

/* Local definitions */
/* ----------------- */
static void  *fileBuffer = NULL;
static int    bufferSize = 0;

/*
--------------------------------------------------------------------------------
 GxCloseF77File - Closes a unformatted FORTRAN file
--------------------------------------------------------------------------------

 Closes a Fortran 77 unformatted file and frees the buffer

 Arguments
 ---------
 filehandle - handle of file from which data should be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/
void GxCloseF77File (FILE *fileHandle)
{
    bufferSize = 0;
    free (fileBuffer);
    fileBuffer = NULL;
    fclose (fileHandle);
}


/*
--------------------------------------------------------------------------------
 GxFindF77Label - Skip until label found
--------------------------------------------------------------------------------

 Skip everything untill the label is found in the Fortran 77 unformatted file

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 label      - Label to be found

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/


GxErrorCode GxFindF77Label (FILE *fileHandle, char *filename, char *label)
{
    GxErrorCode  rc = GXE_NOERROR;
    GxBool       found = GxFalse;
    char        *buffer;
    int          size, length;
  
    if (!fileHandle || !label) {
       rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
    } else {

        length = strlen (label);
	buffer = malloc (length);
	if (buffer) {
	    while (!found && !rc) {
                if (fread (&size, 4, 1, fileHandle) != 1) {
                    rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
	        } else {
	            if (size != length) {
	                if (fseek(fileHandle, size + 4, SEEK_CUR)) {
           	            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
		        }
		    } else {
			if (fread(buffer, size, 1, fileHandle) != 1) {
           	            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
			} else if (strncmp (label, buffer, length) == 0) {
			    found = GxTrue;
			}
			if (fread(&size, 4, 1, fileHandle) != 1) {
           	            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
		 	}
		    }
		}
	    }
	    free (buffer);
	} else {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
	}
    }
    return rc;
}


        
/*
--------------------------------------------------------------------------------
 GxOpenF77File - Opens a unformatted Fortran 77 file
--------------------------------------------------------------------------------

 Opens a unformatted Fortran-77 file and initialises the buffer

 Arguments
 ---------
 filename   - the name of the file
 filehandle - handle of file from which data should be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxOpenF77File (char *filename, FILE  **fileHandle)
{
    GxErrorCode rc=GXE_NOERROR;

    if (!filename || (strlen(filename) == 0)
       || (strlen(filename) > GXL_FILENAME) || !fileHandle) {
       rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
    } else {
        if (fileBuffer) {
            free (fileBuffer);
            fileBuffer = NULL;
            bufferSize = 0;
        }
        *fileHandle = fopen (filename, "rb");
        if (*fileHandle == NULL) {
            rc = GxError (GXE_FILEOPENR, GXT_FILEOPENR, filename);
        }
    }
    return rc;
}




/*
--------------------------------------------------------------------------------
 GxRead - read data from a file to a buffer
--------------------------------------------------------------------------------

 GxRead reads data from an open file into a buffer.

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 filename   - the name of the file
 buffer     - buffer into which data should be read
 size       - size of data items to be read
 count      - number of data items to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRead (FILE *filehandle, const char *filename,
                    void *buffer, size_t size, size_t count)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (buffer);

   if (fread (buffer, size, count, filehandle) != count) {
      if (feof (filehandle)) {
         rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
      } else {
         rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadFileHeader - read and check the file header of an open project file
--------------------------------------------------------------------------------

 GxReadFileHeader reads a standard file header from an already open project
 file and checks that it is correct. The header contains a file type
 identifier and a file type revision identifier.

 Arguments
 ---------
 filehandle - handle of the file from which header should be read
 filename   - the name of the file
 version    - Genex version of project file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadFileHeader (FILE *filehandle, const char *filename, 
                              GxPosGenexVersion *version)
{
   GxErrorCode  rc=GXE_NOERROR;
   FileHeader   header;

   assert (filehandle);
   assert (filename);

/* Read the file header information */
   rc = GxGetString (filehandle, &header.type, filename);

/* Check the file type information identifying it as a GENEX project file */
   if (!rc) {
      if (strncmp (header.type, PROJECTFILEID3, M_FILETYPE) == 0) {
         *version = GX_VERSION_3;
      } else if (strncmp (header.type, PROJECTFILEID4, M_FILETYPE) == 0) {
         *version = GX_VERSION_4;
      } else {
         rc = GxError (GXE_INVALIDPROJECT, GXT_INVALIDPROJECT, filename);
      }
   }

/* Check that the revision number of the program that wrote the file matches
 * that of the current program
 */
   if (!rc) rc = GxGetInteger (filehandle, &header.revision, filename);
   if (!rc && (header.revision != PROJECTFILEREVID))
      rc = GxError (GXE_INVFILEREVISION, GXT_INVFILEREVISION, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadF77Record - read data from a unformatted Fortran 77 file to a buffer
--------------------------------------------------------------------------------

 GxReadF77Record reads data from an open file into a buffer. It skips the 
 record length fields and resizes the buffer if necessary

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 filename   - the name of the file
 buffer     - buffer into which data should be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadF77Record (FILE  *fileHandle, char *filename, void **buffer)
{
    size_t       size;
    GxErrorCode  rc = GXE_NOERROR;

    if (!fileHandle || !filename || (strlen(filename) == 0)
       || (strlen (filename) > GXL_FILENAME)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
    } else {

        if (fread (&size, 4, 1, fileHandle) != 1) {
            rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
        } else if (size > bufferSize) {
           if (fileBuffer) {
               fileBuffer = realloc (fileBuffer, size);
           } else {
               fileBuffer = malloc (size);
           }
           bufferSize = size;
        }
        if (!rc && fileBuffer) {
            if (fread (fileBuffer, size, 1, fileHandle) != 1) {
                rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
            } else  {
                *buffer = fileBuffer;
            }
            if (fread (&size, 4, 1, fileHandle) != 1) {
                rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
            }
        } else {
            rc = GxError (GXE_MEMORY, GXT_MEMORY);
        }
    }
    return rc;
}



/*
--------------------------------------------------------------------------------
 GxWrite - write data from a buffer to a file
--------------------------------------------------------------------------------

 GxWrite writes data from a buffer to an open file.

 Arguments
 ---------
 filehandle - handle of file to which data should be written
 filename   - the name of the file
 buffer     - data that should be written
 size       - size of data items to be written
 count      - number of data items to be written

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWrite (FILE *filehandle, const char *filename,
                     const void *buffer, size_t size, size_t count)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (buffer);

   if (count != fwrite (buffer, size, count, filehandle))
      rc = GxError (GXE_FILEWRITE, GXT_FILEWRITE, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxWriteFileHeader - write a file header to an open project file
--------------------------------------------------------------------------------

 GxWriteFileHeader writes a standard file header to an already open file.
 The header contains a file type identifier and a file type revision
 identifier.

 Arguments
 ---------
 filehandle - handle of file to which header should be written
 version    - the genex version of the file

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxWriteFileHeader (FILE *filehandle, GxPosGenexVersion version)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (version == GX_VERSION_3 || version == GX_VERSION_4);
 
   if (version == GX_VERSION_3) {
      fprintf (filehandle, "\"%s\" %d\n", PROJECTFILEID3, PROJECTFILEREVID);
   } else {
      fprintf (filehandle, "\"%s\" %d\n", PROJECTFILEID4, PROJECTFILEREVID);
   }

   return (rc);
}

