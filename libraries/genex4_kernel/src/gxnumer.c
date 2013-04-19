/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxnumer.c,v 25.0 2005/07/05 08:03:48 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxnumer.c,v $
 * Revision 25.0  2005/07/05 08:03:48  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:59  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:29  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:11  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:43  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:27  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:24  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:38  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:48  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:15  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:08  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:17  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:49  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:30  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:29  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:09  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:26  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:54:03  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxNumer.c    Genex numerical functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        27-MAR-1992

 Description: GxNumer.c contains the Genex functions for numerical
              methods and algorithms. The following functions are available:

                GxLinearInterpolate    - interpolate linear between values
                GxSearchDouble         - search a double in a sorted array
                GxSearchDoubleA        - search a double in ascending array
                GxSearchDoubleD        - search a double in descending array
                GxSolveLinearEquations - solve a set of linear equations
                GxSolveTriDiagonal     - solve a tri-diagonal matrix
                GxSolveQuadratic       - solve a quadratic equation

 Usage notes: - In order to use the functions in this source file the header
                file gxnumer.h should be included.
              - Some functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 27-MAR-1992  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <math.h>
#include  "gxerror.h"
#include  "gxnumer.h"



/*
--------------------------------------------------------------------------------
 GxLinearInterpolate - interpolate linear between two values
--------------------------------------------------------------------------------

 GxLinearInterpolate uses linear interpolation to determine a value in
 an interval.

 Arguments
 ---------
 x  - value to interpolate
 x1 - first x value of interval
 x2 - second x value of interval
 y1 - y value corresponding to first x value
 y2 - y value corresponding to second x value

 Return value
 ------------
 the interpolated y value

--------------------------------------------------------------------------------
*/

double GxLinearInterpolate (double x, double x1, double x2, double y1,
                            double y2)
{
   assert (x1 != x2);

   return (y1 + (x - x1) / (x2 - x1) * (y2 - y1));
}



/*
--------------------------------------------------------------------------------
 GxSearchDouble - search a double in a sorted array
--------------------------------------------------------------------------------

 GxSearchDouble searches for a double value in a sorted array of doubles
 using binary search. The sorting can be in either ascending or descending
 order.

 Arguments
 ---------
 n - size of the sorted array
 x - array in which the double should be searched
 t - double value to be searched for

 Return value
 ------------
 The return value is the index of the array element where the double was
 found or the index where it would be if it were present. If the value of the
 double falls outside of the range of the array -1 or <narray> is returned
 depending on which side of the array the value lies.

--------------------------------------------------------------------------------
*/

int GxSearchDouble (int n, const double *x, double t)
{
   int  index=-1;

   assert (n >= 0);
   assert (x);

   if (n > 0) {
      if (x[0] <= x[n-1]) {
         index = GxSearchDoubleA (n, x, t);
      } else {
         index = GxSearchDoubleD (n, x, t);
      }
   }
   return (index);
}



/*
--------------------------------------------------------------------------------
 GxSearchDoubleA - search a double in an ascending array
--------------------------------------------------------------------------------

 GxSearchDoubleA searches for a double value in an ascending array of doubles
 using binary search.

 Arguments
 ---------
 n - size of the ascending array
 x - array in which the double should be searched
 t - double value to be searched for

 Return value
 ------------
 The return value is the index of the array element where the double was
 found or the index where it would be if it were present. If the value of the
 double falls outside of the range of the array -1 or <narray> is returned
 depending on which side of the array the value lies.

--------------------------------------------------------------------------------
*/

int GxSearchDoubleA (int n, const double *x, double t)
{
   int  l, m, u;

   assert (n >= 0);
   assert (x && ((n == 0) || (x[0] <= x[n-1])));

   if ((n == 0) || (t < x[0])) {
      u = -1;
   } else if (t > x[n-1]) {
      u = n;
   } else if (t == x[0]) {
      u = 0;
   } else {
      l = 0;
      u = n - 1;
      while (l < u-1) {
         m = (l + u) / 2;
         if (x[m] <= t) {
            l = m;
         } else {
            u = m;
         }
      }
   }
   return (u);
}



/*
--------------------------------------------------------------------------------
 GxSearchDoubleD - search a double in a descending array
--------------------------------------------------------------------------------

 GxSearchDoubleD searches for a double value in a descending array of doubles
 using binary search.

 Arguments
 ---------
 n - size of the descending array
 x - array in which the double should be searched
 t - double value to be searched for

 Return value
 ------------
 The return value is the index of the array element where the double was
 found or the index where it would be if it were present. If the value of the
 double falls outside of the range of the array -1 or <narray> is returned
 depending on which side of the array the value lies.

--------------------------------------------------------------------------------
*/

int GxSearchDoubleD (int n, const double *x, double t)
{
   int  l, m, u;

   assert (n >= 0);
   assert (x && ((n == 0) || (x[0] >= x[n-1])));

   if ((n == 0) || (t > x[0])) {
      u = -1;
   } else if (t < x[n-1]) {
      u = n;
   } else if (t == x[0]) {
      u = 0;
   } else {
      l = 0;
      u = n - 1;
      while (l < u-1) {
         m = (l + u) / 2;
         if (x[m] >= t) {
            l = m;
         } else {
            u = m;
         }
      }
   }
   return (u);
}



/*
--------------------------------------------------------------------------------
 GxSolveLinearEquations - solve a set of linear equations
--------------------------------------------------------------------------------

 GxSolveLinearEquations solves a set of linear equations Ax = b with Gaussian
 eliminition using partial pivotal condensation.

 Arguments
 ---------
 n - number of equations to be solved
 a - two dimensional array for matrix A and vector b of the set of equations
 x - solution x of the set of equations

--------------------------------------------------------------------------------
*/

void GxSolveLinearEquations (int n, double **a, double *x)
{
   int     i, j, k, l;
   double  sum, fac, tmp;

   assert (n > 0);
   assert (a);
   assert (x);

   for (k=0; k<n-1; k++) {

/* -- Find row # l of largest element in column k, on or below diagonal */
      l = k;
      for (i=k+1; i<n; i++)
         if (fabs (a[i][k]) > fabs (a[l][k])) l = i;

/* -- If the largest element in column k is not already on the diagonal */
      if (l != k)
         for (j=k; j<=n; j++) {
            tmp     = a[k][j];
            a[k][j] = a[l][j];
            a[l][j] = tmp;
         }

/* -- Gaussian "elimination" */
      for (i=k+1; i<n; i++) {
         fac = a[i][k] / a[k][k];
         for (j=k+1; j<=n; j++) a[i][j] -= fac * a[k][j];
      }
   }

/* Back substitution */
   x[n-1] = a[n-1][n] / a[n-1][n-1];
   for (i=n-2; i>=0; i--) {
      sum = 0.0;
      for (j=i+1; j<n; j++) sum += a[i][j] * x[j];
      x[i] = (a[i][n] - sum) / a[i][i];
   }
}



/*
--------------------------------------------------------------------------------
 GxSolveTriDiagonal - solve a tri-diagonal matrix
--------------------------------------------------------------------------------

 GxSolveTriDiagonal solves a set of linear equations Ax = b where the
 matrix A is a tri-diagonal matrix. The method used for solving is known
 as the Thomas algorithm (Anderson et al, pages 127, 128, and 549).

 Arguments
 ---------
 n - number of equations to be solved
 a - elements of matrix A above diagonal
 b - elements of matrix A below diagonal
 c - elements of vector b of the set of equations
 d - elements of matrix A on diagonal
 x - solution x of the set of equations

--------------------------------------------------------------------------------
*/

void GxSolveTriDiagonal (int n, double *a, double *b, double *c, double *d,
                         double *x)
{
   int  i;

   assert (n > 0);
   assert (a);
   assert (b);
   assert (c);
   assert (d);
   assert (x);

/* Form upper triangular matrix */
   for (i=1; i<n; i++) {
      d[i] -= b[i] / d[i-1] * a[i-1];
      c[i] -= b[i] / d[i-1] * c[i-1];
   }

/* Back substitution */
   x[n-1] = c[n-1] / d[n-1];
   for (i=n-2; i>=0; i--)
      x[i] = (c[i] - a[i] * x[i+1]) / d[i];
}



/*
--------------------------------------------------------------------------------
 GxSolveQuadratic - find the largest real root of a quadratic equaition
--------------------------------------------------------------------------------

 GxSolveQuadratic finds the largest real root of the quadratic equation

    a * x ** 2 + b * x + c = 0

 Arguments
 ---------
 a - coefficient of the equation
 b - coefficient of the equation
 c - coefficient of the equation

 Return value
 ------------
 the largest real root of the equation

--------------------------------------------------------------------------------
*/

double GxSolveQuadratic (double a, double b, double c)
{
   double  root;

   assert (a != 0.0);

   if (4.0 * a * c < b * b) {
      root = (-b + sqrt (b * b - 4.0 * a * c)) / (2.0 * a);
   } else {
      root = -b / (2.0 * a);
   }
   return (root);
}



