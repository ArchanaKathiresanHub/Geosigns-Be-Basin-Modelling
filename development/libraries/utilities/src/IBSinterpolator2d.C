// Interpolation and Extrapolation in 2-Dimensional Array
// x,y & f as 2-D elements

//________________________________________________________________
// Author: T.K. Olatinwo, Erwin Dufour & Alfred v/d Hoeven; EPT-HM
// Version:
// Date:   July, 1998
// System: Independent
//________________________________________________________________


#include "IBSinterpolator2d.h"

#include <assert.h>
#include <iostream>
#include <stdlib.h>

#include <algorithm>
using namespace std;

#include "array.h"

#ifdef ABS
#undef ABS
#endif
#define ABS(x)	((x) < 0 ? -(x) : (x))

#define ACCURACY	0.0001

// Default constructor
ibs::Interpolator2d::Interpolator2d()
{
   d_hasBeenChecked = 0;
   d_arrayXYF = NULL;
   d_dimX = d_dimY = 0;
   d_indexX = d_indexY = 0;

   d_vectorXYF = new vectorXYF;
}


// Destructor
ibs::Interpolator2d::~Interpolator2d()
{ 
   if (d_arrayXYF) Array<XYF>::delete2d (d_arrayXYF);
   if (d_vectorXYF) delete d_vectorXYF;
}

void ibs::Interpolator2d::clean()
{ 

   d_hasBeenChecked = 0;
   
   if (d_vectorXYF) d_vectorXYF->clear();
}


// Initializer for the 2D array by allocating memory on the
// free store using 'new'

// The method to add a function point to the table.
void ibs::Interpolator2d::addPoint (double x, double y, double f)
{
   XYF point (x, y, f);

   d_vectorXYF -> push_back (point);


   // cerr << "Adding point: " << x << ", " << y << ", " << f;
   // cerr << " to: " << (int) this << "\n";
}

ibs::XYF ibs::Interpolator2d::getPoint (int index)
{
   return (*d_vectorXYF)[index];
}

std::vector<ibs::XYF> ibs::Interpolator2d::getVectorOfValues (void)
{
   return *d_vectorXYF;
}

// Compute function value given input arguments
double ibs::Interpolator2d::compute (double x, double y, Extrapolation type)
{
   int    indexLeft, indexRight, indexBottom, indexTop;
   double fractionX, fractionY;
   double FunctionAtX;

   if (PreProcess (x, y, indexLeft, indexRight, indexBottom, indexTop,
		   fractionX, fractionY, type) < 0) return 0.0;
    
   FunctionAtX = computeValue(indexLeft, indexRight, indexBottom, indexTop, fractionX, fractionY);

   return FunctionAtX;
}

// Compute function derivative in the X direction given input arguments
double ibs::Interpolator2d::computeDerivativeInX (double x, double y, Extrapolation type)
{
   int    indexLeft, indexRight, indexBottom, indexTop;
   double fractionX, fractionY;
   double derivative;

   if (PreProcess (x, y, indexLeft, indexRight, indexBottom, indexTop,
		   fractionX, fractionY, type) < 0) return 0.0;
    
   derivative = computeDerivativeValueInX (indexLeft, indexRight, indexBottom, indexTop, fractionY);

   return derivative;

}

// Compute function derivative in the Y direction given input arguments
double ibs::Interpolator2d::computeDerivativeInY (double x, double y, Extrapolation type)
{
   int    indexLeft, indexRight, indexBottom, indexTop;
   double fractionX, fractionY;
   double derivative;

   if (PreProcess (x, y, indexLeft, indexRight, indexBottom, indexTop,
		   fractionX, fractionY, type) < 0) return 0.0;
    
   derivative = computeDerivativeValueInY (indexLeft, indexRight, indexBottom, indexTop, fractionX);

   return derivative;

}

// return the size of the interpolator table
int ibs::Interpolator2d::getSize(int &sizeX, int &sizeY)
{
   if (CheckAndConvertData () < 0) return -1;

   sizeX = d_dimX;
   sizeY = d_dimY;
   
   return 0;
}


// Get the argument and function values for a certain entry in the interpolator table.
int ibs::Interpolator2d::getValuesAtPoint (int indexX, int indexY,
				      double &x, double &y, double &f)
{
   if (CheckAndConvertData () < 0) return -1;

   assert (indexX >= 0 && indexX < d_dimX);
   assert (indexY >= 0 && indexY < d_dimY);

   x = d_arrayXYF[indexX][indexY].x;
   y = d_arrayXYF[indexX][indexY].y;
   f = d_arrayXYF[indexX][indexY].f;

   return 0;
}


// Check if a point with given arguments exists in the interpolator table and if so,
// return the function value.
int ibs::Interpolator2d::pointExists(double x, double y, double &f)
{
   if (CheckAndConvertData () < 0) return -1;

   for (int i = 0; i < d_dimX; i++)
   {
      for (int j = 0; j < d_dimY; j++)
      {
	 if (ABS (d_arrayXYF[i][j].x - x) <= ABS (ACCURACY * x) &&
	     ABS (d_arrayXYF[i][j].y - y) <= ABS (ACCURACY * y))
	 {
	    f = d_arrayXYF[i][j].f;
	    return 1;
	 }
      }
   }
   return 0;
}

// ***************************** Private methods **************************************

int ibs::Interpolator2d::convertTo2d (void)
{
   size_t k;
   size_t dimX, dimY;

   vectorXYF::iterator iterXYF;

   sort (d_vectorXYF -> begin (), d_vectorXYF -> end ());

   dimY = 1;

   for (k = 1; k < d_vectorXYF -> size (); k++)
   {
      if ((* d_vectorXYF)[k-1].y != (* d_vectorXYF)[k].y)
      {
	 dimY++;
      }
   }

   dimX = d_vectorXYF -> size () / dimY;

   if (dimX * dimY != d_vectorXYF -> size ()) return -1;

   Init2d (static_cast<int>( dimX ), static_cast<int>( dimY ) );

   for (iterXYF = d_vectorXYF -> begin (); iterXYF != d_vectorXYF -> end (); ++iterXYF)
   {
      addPointPrivate (* iterXYF);
   }

   delete d_vectorXYF; d_vectorXYF = NULL;

   return 0;
}

void ibs::Interpolator2d::Init2d(int m, int n)
{
   d_arrayXYF = Array<XYF>::create2d (m, n);
   
   d_dimX   = m;
   d_dimY   = n;
   d_indexX = 0;
   d_indexY = 0;
   
}


// Function to add data on point level.

void ibs::Interpolator2d::addPointPrivate (const XYF& point)
{
   
   assert (d_indexX < d_dimX && d_indexY < d_dimY);
   
   d_arrayXYF[d_indexX][d_indexY] = point;
   
   if (d_indexX == d_dimX-1)
   {
      d_indexX=0;
      d_indexY++;
   }
   else
   {
      d_indexX++;
   }
}

// This function checks(once) the correctness of the array dimensions
// as well as the increment value from the data table. The table should
// be equidistant.
int ibs::Interpolator2d::CheckAndConvertData()
{
   int i, j;
   double errorX;
   double errorY;

   if (d_hasBeenChecked)
   {
      return 0;
   }

   if (convertTo2d () < 0) return -1;
   
   // d_deltaX, d_deltaY used in compute ().
   d_deltaX =  d_arrayXYF[1][0].x - d_arrayXYF[0][0].x;
   d_deltaY =  d_arrayXYF[0][1].y - d_arrayXYF[0][0].y;
   
   // Now check.
   for (i=1; i < d_dimX; i++)
   {
      for (j=0; j <d_dimY; j++)
      {
	 errorX = d_deltaX - (d_arrayXYF[i][j].x - d_arrayXYF[i-1][j].x);
	 
	 if (ABS (errorX) > ABS (ACCURACY * d_deltaX))
	 {
	    cerr << "X[" << i << "]["<< j << "] == "<< d_arrayXYF[i][j].x
		 << " not equidistant\n";
	    return -2;
	 }
      }
   }
   
   for (i=0; i < d_dimX; i++)
   {
      for (j=1; j < d_dimY; j++)
      {
	 errorY = d_deltaY - (d_arrayXYF[i][j].y - d_arrayXYF[i][j-1].y);
	 
	 if (ABS (errorY) > ABS (ACCURACY * d_deltaY))
	 {
	    cerr << "Y[" << i << "]["<< j << "] == "<< d_arrayXYF[i][j].y
		 << " not equidistant\n";
	    return -3;
	 }
      }
   }
   d_hasBeenChecked = 1;
   return 0;
}

// Performs some prerocessing steps before the actual computation
int ibs::Interpolator2d::PreProcess (double x, double y,
                                     int &indexLeft, int &indexRight,
                                     int &indexBottom, int &indexTop,
                                     double &fractionX, double &fractionY,
                                     Extrapolation type)
{
   // One time check whether table is ok!!

   if (CheckAndConvertData () < 0) return -1;
   
   // function calls to other interpolator functions!!
   
   MapInputToGrid(x, y, indexLeft, indexBottom, fractionX, fractionY);
   CheckForExtrapolation(indexLeft, indexRight, indexBottom, indexTop,
			 fractionX, fractionY, type);

   return 0;
}


// Calculation of the four gridpoints that enclose the arguments given and the
// relative distances
// to these gridpoints. Assumes that these gridpoints exist.

void ibs::Interpolator2d::MapInputToGrid (double x, double y,
                                          int &indexLeft, int &indexBottom, double &fractionX, double &fractionY)
{
   
   fractionX = (x - d_arrayXYF[0][0].x)/(d_deltaX);
   
   indexLeft = (int) fractionX;
   
   if (fractionX < 0)
   {
      indexLeft--;
   }
   
   fractionX = (x - (d_arrayXYF[0][0].x + indexLeft*d_deltaX))/(d_deltaX);
   
   fractionY = (y - d_arrayXYF[0][0].y)/(d_deltaY);
   
   indexBottom = int (fractionY);
   
   if (fractionY < 0)
   {
      indexBottom--;
   }

   fractionY = (y - (d_arrayXYF[0][0].y + indexBottom*d_deltaY))/(d_deltaY);
}

// Check if the gridpoints found really exist. If not,
// find the nearest 4 existing gridpoints and adapt the relative distances for extrapolation.
void ibs::Interpolator2d::CheckForExtrapolation (int &indexLeft, int &indexRight,
                                                 int &indexBottom, int &indexTop,
                                                 double &fractionX, double &fractionY,
                                                 Extrapolation type)
{
   
   if (indexLeft < 0)
   {
      fractionX = fractionX + indexLeft;
      indexLeft  = 0;
   }
   else if (indexLeft > d_dimX - 2)
   {
      fractionX = fractionX + (indexLeft - d_dimX + 2);
      indexLeft = d_dimX - 2;
   }
   
   if (indexBottom < 0)
   {
      fractionY = fractionY + indexBottom;
      indexBottom = 0;
   }
   
   else if (indexBottom > d_dimY - 2)
   {
      fractionY = fractionY + (indexBottom - d_dimY + 2);
      indexBottom = d_dimY - 2;
   }

   indexRight = indexLeft + 1;
   indexTop = indexBottom + 1;
   	
   if (type == constant)
   {
      if (fractionX < 0) {
	 fractionX = 0;
	 indexRight = indexLeft;
      }
      if (fractionX > 1) {
	 fractionX = 1;
	 indexLeft = d_dimX - 1;
	 indexRight = indexLeft;
      }
      if (fractionY < 0) {
	 fractionY = 0;
	 indexTop = indexBottom;
      }
      if (fractionY > 1) {
	 fractionY = 1;
	 indexBottom = d_dimY - 1;
	 indexTop = indexBottom;
      }
   }
}


// The actual calculation is done here.

double ibs::Interpolator2d::computeValue (int indexLeft, int indexRight,
                                          int indexBottom, int indexTop,
                                          double fractionX, double fractionY)
{

   /*
    * fprintf (stdout, "indexLeft: %d, indexBottom: %d, fractionX: %lf,"
    * " fractionY: %lf\n",
    * indexLeft,  indexBottom, fractionX,fractionY); 
    */
   double FunctionAtX = 
      (1.0 - fractionX) * (1.0 - fractionY) * d_arrayXYF[indexLeft ][indexBottom].f +
      (1.0 - fractionX) * fractionY         * d_arrayXYF[indexLeft ][indexTop].f +
      fractionX         * (1.0 - fractionY) * d_arrayXYF[indexRight][indexBottom].f +
      fractionX         * fractionY         * d_arrayXYF[indexRight][indexTop].f;
   
   return FunctionAtX;
}

double ibs::Interpolator2d::computeDerivativeValueInX (int indexLeft, int indexRight,
                                                       int indexBottom, int indexTop,
                                                       double fractionY)
{

   double valueLeft, valueRight;
   double derivativeInX;

   valueLeft  = (1 - fractionY) * d_arrayXYF[indexLeft][indexBottom].f +
                     fractionY  * d_arrayXYF[indexLeft][indexTop].f;

   valueRight = (1 - fractionY) * d_arrayXYF[indexRight][indexBottom].f +
                     fractionY  * d_arrayXYF[indexRight][indexTop].f;

   derivativeInX = (valueRight - valueLeft) / d_deltaX;

   return derivativeInX;

}

double ibs::Interpolator2d::computeDerivativeValueInY (int indexLeft, int indexRight,
                                                       int indexBottom, int indexTop,
                                                       double fractionX)
{
   double valueBottom, valueTop;
   double derivativeInY;

   valueBottom = (1 - fractionX) * d_arrayXYF[indexLeft ][indexBottom].f +
                      fractionX  * d_arrayXYF[indexRight][indexBottom].f;

   valueTop    = (1 - fractionX) * d_arrayXYF[indexLeft ][indexTop].f +
                      fractionX  * d_arrayXYF[indexRight][indexTop].f;

   derivativeInY = (valueTop - valueBottom) / d_deltaY;

   return derivativeInY;

}

// XYF functions

namespace ibs {

   bool operator < (const XYF& xyf1, const XYF& xyf2)
   {
      if (xyf1.y < xyf2.y) return true;
      if (xyf1.y > xyf2.y) return false;

      if (xyf1.x < xyf2.x) return true;
      if (xyf1.x > xyf2.x) return false;

      return false;
   }

}

