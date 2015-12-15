#ifndef _INTERPOLATOR2D_H
#define _INTERPOLATOR2D_H

// Header for Interpolation and Extrapolation in 2-Dimensional Array
// x,y & f as 2-D elements

//________________________________________________________________
// Author: T.K. Olatinwo, Erwin Dufour & Alfred v/d Hoeven; EPT-HM
// Version:
// Date:   July, 1998
// System: Independent
//________________________________________________________________

// Class to represent a 2-D function X x Y -> F that allows interpolations between
// supplied values (x, y).
// The supplied values (x, y) need to form an equi-distant, rectangular array of points.
// Functions are supplied to
//    - input function elements (x, y, f): addPoint ()
//    - compute f given (x, y): compute ()
//    - compute partial derivatives given (x, y): computeDerivativeIn[XY] ()
//    - get the size of the equi-distant, rectangular array of points: getSize ()
//    - get values (x, y, f) at a certain index in the equi-distant,
//      rectangular array of points: getValuesAtPoint ()
//    - determine if an input point (x, y) was supplied via addPoint (): pointExists ()
//
// These functions(except addPoint ()) may only be called after all triples (x, y,f)
// have been entered via addPoint ().
// When any of the functions except addPoint () has been called, the function addPoint ()
// may no longer be called.

#include <vector>

using namespace std;

namespace ibs {

   // Helper class for the interpolator
   class XYF {
   public:
      XYF () {}
      ~XYF () {}
      XYF (double xx, double yy, double ff): x(xx), y(yy), f(ff) {}

   private:

      double x;
      double y;
      double f;

      friend bool operator < (const XYF& xyf1, const XYF& xyf2);
      friend class Interpolator2d;
   };


   class Interpolator2d
   {

      typedef vector<XYF>	    vectorXYF;

   public:

      Interpolator2d ();
      ~Interpolator2d (); 

      enum Extrapolation { linear, constant, none };

      // input function elements (x, y, f)
      void addPoint(double x, double y, double f);

      // compute f given (x, y)
      double compute(double x, double y, Extrapolation type = linear);
    
      // compute partial derivatives given (x, y)
      double  computeDerivativeInX (double x, double y, Extrapolation type = linear);
      double  computeDerivativeInY (double x, double y, Extrapolation type = linear);

      // get the size of the equi-distant, rectangular array of points
      int     getSize (int &sizeX, int &sizeY);

      // get values (x, y, f) at a certain index in the equi-distant,
      // rectangular array of points
      int     getValuesAtPoint (int indexX, int indexY,
                                double &x, double &y, double &f);

      // determine if an input point (x, y) was supplied via addPoint ()
      int     pointExists (double x, double y, double &f);
    
   private:

      // static int compareXYF (const void * arg1, const void * arg2);

      // Tables that store the function values.
      XYF ** d_arrayXYF;

      // size of the 2D array of function points.
      int      d_dimX, d_dimY;

      // distance between function points, used by compute(Derivative).
      double   d_deltaX, d_deltaY;

      // makes sure that function is checked only once.
      int     d_hasBeenChecked;

      // Used for input purposes, to compute dim1 and dim2.
      // I am sure we can do better, but .....
      vectorXYF * d_vectorXYF;

      // Used by addPointPrivate during conversion from 1D to 2D table.
      int      d_indexX, d_indexY;

      void   Init2d (int m, int n);
      int    CheckAndConvertData();
      int    convertTo2d ();
      void   addPointPrivate (const XYF& point);

      int PreProcess (double x, double y,
                      int &indexLeft, int &indexRight,
                      int &indexBottom, int &indexTop,
                      double &fractionX, double &fractionY,
                      Extrapolation type);
    
      void   MapInputToGrid (double x, double y,  int &indexLeft, int &indexBottom,
                             double &fractionX, double &fractionY);

      void CheckForExtrapolation (int &indexLeft, int &indexRight,
                                  int &indexBottom, int &indexTop,
                                  double &fractionX, double &fractionY,
                                  Extrapolation type);

      double computeValue (int indexLeft, int indexRight,
                           int indexBottom, int indexTop,
                           double fractionX, double fractionY);

      double  computeDerivativeValueInX (int indexLeft, int indexRight,
                                         int indexBottom, int indexTop, double fraction);

      double  computeDerivativeValueInY (int indexLeft, int indexRight,
                                         int indexBottom, int indexTop, double fraction);
   };

} // end ibs

#endif
