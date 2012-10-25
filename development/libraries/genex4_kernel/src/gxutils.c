/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxutils.c,v 25.0 2005/07/05 08:03:51 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxutils.c,v $
 * Revision 25.0  2005/07/05 08:03:51  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:03  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:37  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:15  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:41  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:32  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:50  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:09  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:26  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:32  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:27  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:05  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:04  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:16  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:25  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:46  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:43  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxUtils.c    Genex utility functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxUtils.c contains the Genex utility functions. The following
              functions are available:

                GxAllocArray - allocate memory for an array
                GxFreeArray  - free memory allocated for an array
		GxLocalPow   - Local version of power function

 Usage notes: - In order to use the functions in this source file the header
                file gxutils.h should be included.
              - Some functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 24-JUN-1992  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <stdarg.h>
#include  <stdlib.h>
#include  <math.h>
#include  "gxerror.h"
#include  "gxutils.h"

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocSubArray (void *, size_t, int, va_list);
static void         GxFreeSubArray (void *, int, va_list);



/*
--------------------------------------------------------------------------------
 GxAllocArray - allocate memory for a multi-dimensional array
--------------------------------------------------------------------------------

 GxAllocArray allocates memory for a multi dimensional array and initializes
 the array with pointers to the actual memory.

 Arguments
 ---------
 arrayptr - pointer for the array
 size     - size of the elements in the array
 ndimens  - number of array dimensions
 ...      - dimensions of the array

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxAllocArray (void *arrayptr, size_t size, int ndimens, ...)
{
   GxErrorCode  rc=GXE_NOERROR;
   va_list      argptr;

   assert (arrayptr);
   assert (size > 0);
   assert (ndimens > 0);

/* Start accessing variable length argument list */
   va_start (argptr, ndimens);

/* Call the function that does the actual work */
   rc = GxAllocSubArray (arrayptr, size, ndimens, argptr);
   if (rc) {

/* -- Free what was already allocated when the error occured */
      GxFreeSubArray (*(void **) arrayptr, ndimens, argptr);
      *(void **) arrayptr = NULL;
   }

/* Stop accessing variable length argument list */
   va_end (argptr);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocSubArray - allocate memory for a subarray of a multi-dimensional array
--------------------------------------------------------------------------------

 GxAllocSubArray allocates memory for a subarray of a multi-dimensional array
 and initializes the array with pointers to the actual memory.

 Arguments
 ---------
 arrayptr - pointer for the subarray
 size     - size of the elements in the subarray
 ndimens  - number of subarray dimensions
 argptr   - dimensions of the subarray

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocSubArray (void *arrayptr, size_t size, int ndimens,
                                    va_list argptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   void         *arraydata, **array;
   int          i, dimension1, dimension2;

   assert (size > 0);
   assert (arrayptr);
   assert (ndimens > 0);

/* Get the first array dimension */
   dimension1 = va_arg (argptr, int);
   if (ndimens == 1) {

/* -- One dimensional array: allocate memory for it */
      array = (void *) malloc (dimension1 * size);
      if (!array) rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- More dimensional array: allocate memory for an array of pointers */
      array = (void **) malloc (dimension1 * sizeof (void *));
      if (!array) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {
         if (ndimens == 2) {

/* -------- Array is two-dimensional: allocate memory for the array data */
            dimension2 = va_arg (argptr, int);
            arraydata = malloc (dimension1 * dimension2 * size);
            if (!arraydata) {
               rc = GxError (GXE_MEMORY, GXT_MEMORY);
               free (array);
               array = NULL;
            } else {

/* ----------- Fill the pointer array with pointers into data array */
               for (i=0; i<dimension1; i++)
                  array[i] = (char *) arraydata + i * dimension2 * size;
            }
         } else {

/* -------- Array has more than two dimensions: allocate a series of
 * -------- subarrays through recursive calls of GxAllocSubArray
 */
            for (i=0; i<dimension1; i++) {
               ((void **) array)[i] = NULL;
               if (!rc) rc = GxAllocSubArray (&((void **) array)[i], size,
                                              ndimens-1, argptr);
            }
         }
      }
   }

/* Return the pointer to the allocated array */
   *(void **) arrayptr = array;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeArray - free the memory allocated for an array
--------------------------------------------------------------------------------

 GxFreeArray frees the memory that was allocated for a multi-dimensional
 array. It is not required to specify all of the array dimensions when
 freeing an array. The last two dimensions may be omitted from the call.
 However, it is recommended to specify them for documentation purposes.

 Arguments
 ---------
 array    - the array to be freed
 ndimens  - number of array dimensions
 ...      - dimensions of the array

--------------------------------------------------------------------------------
*/

void GxFreeArray (void *array, int ndimens, ...)
{
   va_list  argptr;

   assert (ndimens > 0);

/* Start accessing variable length argument list */
   va_start (argptr, ndimens);

/* Call the function that does the actual work */
   GxFreeSubArray (array, ndimens, argptr);

/* Stop accessing variable length argument list */
   va_end (argptr);
}



/*
--------------------------------------------------------------------------------
 GxFreeSubArray - free the memory allocated for a subarray
--------------------------------------------------------------------------------

 GxFreeArray frees the memory that was allocated for a subarray of a
 multi-dimensional array.

 Arguments
 ---------
 array    - the subarray to be freed
 ndimens  - number of array dimensions in subarray
 argptr   - dimensions of the subarray

--------------------------------------------------------------------------------
*/

static void GxFreeSubArray (void *array, int ndimens, va_list argptr)
{
   int  i, dimension;

   assert (ndimens > 0);

   if (array) {
      if (ndimens == 2) {

/* ----- Two dimensional array: free the memory allocated for array data */
         if (((void **) array)[0]) free (((void **) array)[0]);
      } else if (ndimens > 2) {

/* ----- Array has more than two dimensions: free a series of
 * ----- subarrays through recursive calls of GxFreeSubArray
 */
         dimension = va_arg (argptr, int);
         for (i=0; i<dimension; i++)
            GxFreeSubArray (((void **) array)[i], ndimens-1, argptr);
      }

/* -- Free the base array */
      free (array);
   }
}


/*
--------------------------------------------------------------------------------
 GxLocalPow - new version for calculating x^y
--------------------------------------------------------------------------------

 Power function with check for negative values

 Arguments
 ---------
 x        - base number
 y        - power

 RETURN VALUE
 ------------
 x^y
--------------------------------------------------------------------------------
*/

double GxLocalPow (double x, double y)
{
    double  power;

    if (x < 0.0) {
	power = pow(x, (int)y);	
    } else {
	power = pow(x, y);
    }
    return (power);
}
