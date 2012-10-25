/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxscanner.c,v 25.0 2005/07/05 08:03:50 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxscanner.c,v $
 * Revision 25.0  2005/07/05 08:03:50  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:02  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:35  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:14  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:46  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:38  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:30  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:47  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:04  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:23  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:25  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:24  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:01  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:56  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:42  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:21  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:40  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:19  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 gxscanner.c   Lexical analyser
--------------------------------------------------------------------------------

 Author:      M. van Houtert (BSO-AT/Ztm)

 Date:        25-MAR-1993

 Description: Scanner that recognizes int's floats and strings

                GxCloseFile     Close the inputfile
		GxGetDouble	Retrieve a double value
		GxGetInteger    Retrieve a integer value
		GxGetString     Retrieve a string value
 History
 -------
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include <ctype.h>                      /* Macro definitions isalpha etc     */
#include <string.h>                     /* String len definitions etc        */
#include <stdlib.h>                     /* Standard library                  */
#include <math.h>                       /* Atof definitions  etc             */
#include <assert.h>			

#include "gxscanner.h"                  /* Token values                      */


#define MAXLINE         256             /* Maximum line length               */


static signed char  buffer[MAXLINE+1];  /* Buffer for line from source       */
static char    yyString[MAXLINE+1];     /* Recognized string                 */
static int     yyInt;                   /* Recognized integer value          */
static int     yyLine;                  /* Current linenumber                */
static double  yyDouble;                /* Recognized double value           */


static GxBool   empty   = GxTrue;       /* Input buffer empty True/False     */
static int      linePtr = 0;            /* Position within line              */
static int      token;                  /* Current scanned token             */


/*
--------------------------------------------------------------------------------
 getLine - Get a line!!!
--------------------------------------------------------------------------------

 Get the nextline from the inputfile

 Arguments
 ---------
 fp     - Pointer to inputfile

 Return value
 ------------

--------------------------------------------------------------------------------
*/


static void getLine (
    FILE    *fp                         /* I: File to read from              */ 
)
{
    int  i;
    int  kar;

    i = 0;
    kar = fgetc (fp);
    while (i < MAXLINE) {               /* Line length exceeds limits??      */
        buffer[i++] = kar;
        if (kar == EOF) {               /* End of file reached?              */
            break;
        } else if (kar == '\n') {       /* Stop reading at end of line       */
            break;
        }
        kar = fgetc (fp);
    }
    buffer[i] = '\0';
    linePtr = 0;
    empty = GxFalse;
}


/*
--------------------------------------------------------------------------------
 yylex - Get a token
--------------------------------------------------------------------------------

 Local routine for recognizing the tokens

 Arguments
 ---------
 fp     - Pointer to inputfile

 Return value
 ------------
 int   - the token
--------------------------------------------------------------------------------
*/



static int yylex (
    FILE *fp                            /* I: The source file                */
)
{
    int       nextToken;
    int       sign;
    int       i;


    nextToken = -1;
    while (nextToken == -1) {
        if (empty) {                    /* Check if input buffer is filled   */ 
            getLine (fp);               /* Read the next line from input     */
        }
        /* Skip white space !!! */

        while (buffer[linePtr] == ' ' || buffer[linePtr] == '\t') {
            linePtr++;
        }
        sign = 1;
        if (buffer[linePtr] == '-' || buffer[linePtr] == '+') {
	    if (buffer[linePtr] == '-') {
		sign = -1;
	    }
	    linePtr++;
        }
        if (isdigit (buffer[linePtr])) {

            /* Get Integer or Float value from stream */

            i = 1;
            yyString[0] = buffer[linePtr++];
            while (isdigit(buffer[linePtr])) {
                yyString[i++] = buffer[linePtr++];
            }

            /* Check if fraction part follows */
            if (buffer[linePtr] == '.') {
                yyString[i++] = buffer[linePtr++];
                while (isdigit (buffer[linePtr])) {
                    yyString[i++] = buffer[linePtr++];
                }
	        if (buffer[linePtr] == 'e' || buffer[linePtr] == 'E') {
		    yyString[i++] = 'E';
		    linePtr++;
		    yyString[i++] = buffer[linePtr++];
		    while (isdigit (buffer[linePtr])) {
                        yyString[i++] = buffer[linePtr++];
		    }
                }
                yyString[i] = '\0';
                yyDouble = (double) sign * (double) atof (yyString);
                nextToken = FLOAT;
            } else {
                yyString[i] = '\0';
                yyInt = sign * atoi (yyString);
                nextToken = INTEGER;
            }
        } else if (buffer[linePtr] == '\"') {

            /* Get text string, delimited by quotes */

            i = 0;
            linePtr++;
            while (buffer[linePtr] != '\"' && buffer[linePtr] != '\0') {
                yyString[i++] = buffer[linePtr++];
            }
            yyString[i] = '\0';
            linePtr++;
            nextToken = STRING;
        } else if (buffer[linePtr] == '\n') {
            empty = GxTrue;
            yyLine++;
        } else if (buffer[linePtr] == EOF) {
            nextToken = END_OF_FILE;
        } else {
            linePtr++;
        }
    }
    return  nextToken;
}
/*
--------------------------------------------------------------------------------
 GxCloseFile - Close an inputfile and flush buffer
--------------------------------------------------------------------------------

 Close the inputfile and flush the inputbuffer

 Arguments
 ---------
 fp       - File pointer to inputfile

 Return value
 ------------

--------------------------------------------------------------------------------
*/

void GxCloseFile (
    FILE   *fp
)
{
    assert (fp);

    fclose (fp);
    empty = GxTrue;
}



/*
--------------------------------------------------------------------------------
 GxGetDouble - Retrieve a double from the input file
--------------------------------------------------------------------------------

 Scan the input file for a double token

 Arguments
 ---------
 fp       - File pointer to inputfile
 val      - Buffer to store data in

 Return value
 ------------
 GXE_NOERROR  token found
 GXE_FILEREAD token not found

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetDouble (
    FILE  *fp,
    double *value,
    char   *filename
)
{
    GxErrorCode rc=GXE_NOERROR;

    assert (fp);
    assert (value);
    assert (filename);

    if (yylex(fp) == FLOAT) {
	*value = yyDouble;
	return rc;
    } 
    rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
    return rc;
}


/*
--------------------------------------------------------------------------------
 GxGetString - Retrieve a string from the input file
--------------------------------------------------------------------------------

 Scan the input file for a string token

 Arguments
 ---------
 fp       - File pointer to inputfile
 val      - Buffer to store data in

 Return value
 ------------
 GXE_NOERROR token found
 GXE_FILEREAD token not found

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetString (
    FILE   *fp,
    char   *value,
    char   *filename
)
{
    GxErrorCode rc=GXE_NOERROR;

    assert (fp);
    assert (value);
    assert (filename);

    if (yylex (fp) == STRING) {
	strcpy (value, yyString);
	return rc;
    } 
    rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
    return rc;
}
   

/*
--------------------------------------------------------------------------------
 GxGetInteger - Retrieve a integer from the input file
--------------------------------------------------------------------------------

 Scan the input file for a integer token

 Arguments
 ---------
 fp       - File pointer to inputfile
 val      - Buffer to store data in

 Return value
 ------------
 GXE_NOERROR token found
 GXE_FILEREAD token not found

--------------------------------------------------------------------------------
*/


GxErrorCode GxGetInteger (
    FILE   *fp,
    int    *value,
    char   *filename
)
{
    GxErrorCode rc=GXE_NOERROR;

    assert (fp);
    assert (value);
    assert (filename);

    if (yylex(fp) == INTEGER) {
        *value = yyInt;
	return rc;
    } 
    rc = GxError (GXE_FILEREAD, GXT_FILEREAD, filename);
    return rc;
}

 
