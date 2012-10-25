/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxerror.c,v 25.0 2005/07/05 08:03:44 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxerror.c,v $
 * Revision 25.0  2005/07/05 08:03:44  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:54  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:20  ibs
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
 * Revision 21.0  2001/05/02 08:59:16  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:26  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:35  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:04  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:46  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:05  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:29  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:09:57  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:08  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:50  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.1  1996/06/12  14:57:54  ibs
 * Genex name in the error messages now.
 *
 * Revision 10.0  1996/03/11  12:41:02  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:40  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxError.c    Genex error handling functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxError.c contains the Genex functions for handling errors.
              The following functions are available:

                GxClearError      - clear the current error
                GxError           - create a new error or add information
                GxReportError     - report the current error
                GxSetErrorHandler - sets the error handler function

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: In order to use the functions in this source file the header
              file gxerror.h should be included.

 History
 -------
 24-JUN-1992  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdarg.h>
#include  <stdio.h>
#include  <string.h>
#include  "gxerror.h"

/* Macro definitions */
/* ----------------- */
/* -- maximum length of total error message -- */
#define   GXM_ERRORTEXT    500
/* -- error text definitions -- */
#define   GXT_ERRORLENGTH     "The error message overflows the error buffer.\n"
#define   GXT_NOERRORPRESENT  "There is no error to report !!\n"

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxConcatError (GxErrorCode, const char *, va_list);
static void         GxStandardErrorHandler (GxErrorCode, const char *);

/* Global variables */
/* ---------------- */
/* -- file scope variables -- */
static void         (*theErrorHandler) (GxErrorCode, const char *)=NULL;
static GxErrorCode  theErrorCode=GXE_NOERROR;
static char         theErrorText[GXM_ERRORTEXT+1]="";



/*
--------------------------------------------------------------------------------
 GxClearError - clear the current error
--------------------------------------------------------------------------------

 GxClearError clear the current error by setting it to "no error" and making
 the error text an empty string.

--------------------------------------------------------------------------------
*/

void GxClearError (void)
{
   theErrorCode    = GXE_NOERROR;
   theErrorText[0] = '\0';
}



/*
--------------------------------------------------------------------------------
 GxConcatError - concatenate extra error information
--------------------------------------------------------------------------------

 GxConcatError concatenates the text of the additional information to the
 text of the current error. If the code of the additional error information
 is not equal to GXE_NOERROR this code will supersede the code of the current
 error. The error code returned is either the code of the error encountered
 during error handling (if any) or the code of the error passed to the
 function. GxConcatError makes use of the variable argument list
 functionality of ANSI C.

 Arguments
 ---------
 code       - the error code for the error
 descriptor - format string for the text for the error
 argptr     - variable number of arguments to be included in error text

 Return value
 ------------
 code of the most severe of the error passed and error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxConcatError (GxErrorCode code, const char *descriptor,
                                  va_list argptr)
{
   size_t  length;
   int     addlength;

   if (!descriptor) {
      code = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Determine the length of the message text and check for errors */
      length    = strlen (theErrorText);
      vsprintf (&theErrorText[length], descriptor, argptr);
      addlength = strlen (&theErrorText[length]);
      if (addlength < 0) {
         code = GxError (GXE_INTERNWRITE, GXT_INTERNWRITE,
                         __FILE__, __LINE__);
      } else if (length + addlength > GXM_ERRORTEXT) {
         GxClearError ();
         code = GxError (GXE_ERRORLENGTH, GXT_ERRORLENGTH);
      } else {
         if (GX_SEVERITY (code) > GX_SEVERITY (theErrorCode))
            theErrorCode = code;
      }
   }
   return (code);
}



/*
--------------------------------------------------------------------------------
 GxError - create a new current error or add information
--------------------------------------------------------------------------------

 GxError creates a new current error if there is no current error or adds
 information if there is one. If there is no current error the error code
 <code> must be unequal to GXE_ERROR. If the error code <code> is unequal to
 GXE_NOERROR the most severe of the current and the additional error will be
 retained. Errors discovered by GxError will be treated as additional errors.
 GxError makes use of the variable argument list functionality of ANSI C.

 Arguments
 ---------
 code       - the error code for the error
 descriptor - format string for the text for the error
 ...        - variable number of arguments to be included in error text

 Return value
 ------------
 code of the most severe of the error passed and error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxError (GxErrorCode code, const char *descriptor, ...)
{
   va_list  argptr;

   if (!descriptor) {
      code = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else if ((theErrorCode == GXE_NOERROR) && (code == GXE_NOERROR)) {
      code = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {
      va_start (argptr, descriptor);
      code = GxConcatError (code, descriptor, argptr);
      va_end (argptr);
   }
   return (code);
}



/*
--------------------------------------------------------------------------------
 GxReportError - report the current error through the current error handler
--------------------------------------------------------------------------------

 GxReportError reports the current error to the user by calling the current
 error handler. After the error has been reported the error is cleared. In
 case there is no current error to report an error is generated to signal
 this fact. If no error handler is defined, the default error handler is used
 instead. The error handler is called with two arguments: the error code and
 the error text.

--------------------------------------------------------------------------------
*/

void GxReportError (void)
{
   if (theErrorCode == GXE_NOERROR)
      GxError (GXE_NOERRORPRESENT, GXT_NOERRORPRESENT);
   if (theErrorHandler) {
      theErrorHandler (theErrorCode, theErrorText);
   } else {
      GxStandardErrorHandler (theErrorCode, theErrorText);
   }
   GxClearError ();
}



/*
--------------------------------------------------------------------------------
 GxSetErrorHandler - set the function to handle the displaying of errors
--------------------------------------------------------------------------------

 GxSetErrorHandler defines the function that will be used to report the
 errors to the user. This function can be called more than once. If
 GxSetErrorHandler is not called or if it is called with the argument NULL,
 the default error handler will be used.

 Arguments
 ---------
 errorhandler - pointer to the error handler function

--------------------------------------------------------------------------------
*/

void GxSetErrorHandler (void (*errorhandler) (GxErrorCode, const char *))
{
   theErrorHandler = errorhandler;
}



/*
--------------------------------------------------------------------------------
 GxStandardErrorHandler - standard function for handling errors
--------------------------------------------------------------------------------

 GxStandardErrorHandler is the standard function for reporting errors to
 the user. It writes the information of the error to the standard error output
 stderr.

 Arguments
 ---------
 code - the error code for the error
 text - the text for the error

--------------------------------------------------------------------------------
*/

static void GxStandardErrorHandler (GxErrorCode code, const char *text)
{
   fprintf (stderr, "\nGENEX: ");
   switch (GX_SOURCE (code)) {
      case GXC_APPLICATION:
           fprintf (stderr, "APPLICATION ");
           break;

      case GXC_SYSTEM:
           fprintf (stderr, "SYSTEM ");
           break;

      case GXC_TOOLBOX:
           fprintf (stderr, "TOOLBOX ");
           break;

      case GXC_USER:
           fprintf (stderr, "USER ");
           break;
   }

   switch (GX_SEVERITY (code)) {
      case GXC_INFORMATIONAL:
           fprintf (stderr, "INFORMATIONAL ");
           break;

      case GXC_ERROR:
           fprintf (stderr, "ERROR ");
           break;

      case GXC_SEVEREERROR:
           fprintf (stderr, "SEVERE ERROR ");
           break;

      case GXC_FATAL:
           fprintf (stderr, "FATAL ");
           break;
   }

   fprintf (stderr, "MESSAGE: %d\n", GX_NUMBER (code));
   if (text) fprintf (stderr, "%s\n", text);
}
