// Interpolation and Extrapolation in 1-D

#include "IBSinterpolator.h"

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

void ibs::XF::print (void) const
{
   cerr << "(" << x << ", " << f << ")";
}

void ibs::XF::setX ( const double newX ) {
   x = newX;
}

void ibs::XF::setF ( const double newF ) {
   f = newF;
}

void ibs::XF::set ( const double newX,
                    const double newF ) {
   x = newX;
   f = newF;
}




namespace ibs {

bool UnOrderedSorter (const XF& xf1, const XF& xf2)
{
   if (xf1.x < xf2.x) return true;
   else return false;
}

bool AscendingSorter (const XF& xf1, const XF& xf2)
{
   if (xf1.x < xf2.x) return true;
   else if (xf1.f < xf2.f) return true;
   else return false;
}

bool DescendingSorter (const XF& xf1, const XF& xf2)
{
   if (xf1.x < xf2.x) return true;
   else if (xf1.f > xf2.f) return true;
   else return false;
}


bool operator < (const XF& xf1, const XF& xf2)
{
   if (xf1.x < xf2.x) return true;
   else if (xf1.f < xf2.f) return true;
   else return false;
}

}

ibs::Interpolator::Interpolator (ibs::Interpolator::SorterType sorterType) : d_sorted (0), d_vectorXF ()
{
   switch (sorterType)
   {
      case UnOrdered:
	 d_sorterFunc = UnOrderedSorter;
	 break;
      case Ascending:
	 d_sorterFunc = AscendingSorter;
	 break;
      case Descending:
	 d_sorterFunc = DescendingSorter;
	 break;
      default:
	 assert (false);
   }
}

ibs::Interpolator::Interpolator ( const Interpolator& interp ) :
   d_sorted ( interp.d_sorted ),
   d_vectorXF ( interp.d_vectorXF )

{
}

// Destructor
ibs::Interpolator::~Interpolator(void)
{ 
}

// The method to add a function point to the table.
void ibs::Interpolator::addPoint (double x, double f)
{
   d_sorted = 0;
   XF point (x, f);

   d_vectorXF.push_back (point);

}

// Compute function value given input arguments
double ibs::Interpolator::compute (double x, Extrapolation type)
{
   if (!d_sorted)
   {
      sort (d_vectorXF.begin (), d_vectorXF.end (), d_sorterFunc);
      d_sorted = 1;
   }

   XF * pointLeft = NULL;
   XF * pointRight = NULL;

   assert (d_vectorXF.size () >= 2);

   // find the two adjacent points
   for (int i = 0; i < d_vectorXF.size (); i++)
   {
      XF * point = & d_vectorXF[i];

      if (point->getX () <= x)
      {
	 pointLeft = point;
      }

      if (point->getX () >= x)
      {
	 pointRight = point;
	 break;
      }
   }

   assert (pointLeft || pointRight);
   
   if (pointLeft == pointRight)
   {
      return pointLeft->getF ();
   }
   else if (!pointLeft)
   {
      // left extrapolation
      if (type == constant)
      {
	 return pointRight->getF ();
      }
      else
      {
	 pointLeft = &d_vectorXF[0];
	 pointRight = &d_vectorXF[1];
      }
   }
   else if (!pointRight)
   {
      // right extrapolation
      if (type == constant)
      {
	 return pointLeft->getF ();
      }
      else
      {
	 size_t size = d_vectorXF.size ();

	 pointLeft = &d_vectorXF[size - 2];
	 pointRight = &d_vectorXF[size - 1];
      }
   }

   // inter- or extrapolate
   double fraction = (x - pointLeft->getX ()) / (pointRight->getX () - pointLeft->getX ());

   double f = pointLeft->getF () + fraction * (pointRight->getF () - pointLeft->getF ());

   return f;
}

void ibs::Interpolator::freeze () {

   if ( d_sorted == 0 ) {
      sort (d_vectorXF.begin (), d_vectorXF.end (), d_sorterFunc);
      d_sorted = 1;
   }

}


void ibs::Interpolator::print (void)
{
   for (int i = 0; i < d_vectorXF.size (); i++)
   {
      XF * point = & d_vectorXF[i];
      point->print ();
      cerr << endl;
   }
   cerr << endl;
}

