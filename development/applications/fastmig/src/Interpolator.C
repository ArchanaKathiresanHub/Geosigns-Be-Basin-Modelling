// Interpolation and Extrapolation in 1-D

#include "Interpolator.h"

#include <assert.h>
#include <iostream>
#include <stdlib.h>

#include <algorithm>
using namespace std;

#include "array.h"

void XF::print (void)
{
   cerr << "(" << x << ", " << f << ")";
}

bool UnOrderedSorter (const XF& xf1, const XF& xf2)
{
   return (xf1.x < xf2.x);
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

Interpolator::Interpolator (Interpolator::SorterType sorterType) : m_sorted (false), m_vectorXF ()
{
   switch (sorterType)
   {
      case UnOrdered:
	 m_sorterFunc = UnOrderedSorter;
	 break;
      case Ascending:
	 m_sorterFunc = AscendingSorter;
	 break;
      case Descending:
	 m_sorterFunc = DescendingSorter;
	 break;
      default:
	 assert (false);
   }
}

// Destructor
Interpolator::~Interpolator(void)
{ 
}

// The method to add a function point to the table.
void Interpolator::addPoint (double x, double f)
{
   m_sorted = false;
   XF point (x, f);

   m_vectorXF.push_back (point);

}

// Compute function value given input arguments
double Interpolator::compute (double x, Extrapolation type)
{
   if (!m_sorted)
   {
      sort (m_vectorXF.begin (), m_vectorXF.end (), m_sorterFunc);
      m_sorted = true;
   }

   XF * pointLeft = NULL;
   XF * pointRight = NULL;

   assert (m_vectorXF.size () >= 2);

   // find the two adjacent points
   for (unsigned int i = 0; i < m_vectorXF.size (); i++)
   {
      XF * point = & m_vectorXF[i];

      if (point->x <= x)
      {
	 pointLeft = point;
      }

      if (point->x >= x)
      {
	 pointRight = point;
	 break;
      }
   }

   assert (pointLeft || pointRight);
   
   if (pointLeft == pointRight)
   {
      return pointLeft->f;
   }
   else if (!pointLeft)
   {
      // left extrapolation
      if (type == constant)
      {
	 return pointRight->f;
      }
      else
      {
	 pointLeft = &m_vectorXF[0];
	 pointRight = &m_vectorXF[1];
      }
   }
   else if (!pointRight)
   {
      // right extrapolation
      if (type == constant)
      {
	 return pointLeft->f;
      }
      else
      {
	 int size = m_vectorXF.size ();

	 pointLeft = &m_vectorXF[size - 2];
	 pointRight = &m_vectorXF[size - 1];
      }
   }

   // inter- or extrapolate
   double fraction = (x - pointLeft->x) / (pointRight->x - pointLeft->x);

   double f = pointLeft->f + fraction * (pointRight->f - pointLeft->f);

   return f;
}

void Interpolator::print (void)
{
   for (unsigned int i = 0; i < m_vectorXF.size (); i++)
   {
      XF * point = & m_vectorXF[i];
      point->print ();
      cerr << endl;
   }
   cerr << endl;
}

bool operator < (const XF& xf1, const XF& xf2)
{
   if (xf1.x < xf2.x) return true;
   else if (xf1.f < xf2.f) return true;
   else return false;
}

