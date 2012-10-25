#include "stdafx.h"

#include "Polyfunction.h"

#include <iostream>
#include <assert.h>

using namespace std;

//#include "globaldefs.h"
//#include "utils.h"

namespace CBMGenerics
{

const double NULLVALUE           = -9999;

int Polyfunction::s_instanceCount = 0;
int Polyfunction::s_maxInstanceCount = 0;
int Polyfunction::Point::s_instanceCount = 0;
int Polyfunction::Point::s_maxInstanceCount = 0;

void Polyfunction::Point::printStatus()
{
   cout << "Point        : " << s_maxInstanceCount;
   cout << " created " << s_instanceCount << " still allocated" << endl;
}

Polyfunction::Polyfunction()
{
   m_points.reserve (2);
   s_instanceCount++;
   s_maxInstanceCount++;
   m_max = 0;
   m_min = 0;
}

Polyfunction::Polyfunction(const Polyfunction& original)
{
   m_points.reserve (2);
   s_instanceCount++;
   s_maxInstanceCount++;
   m_max = 0;
   m_min = 0;

   deepCopyAllPoints (original);
}

Polyfunction& Polyfunction::operator= (const Polyfunction &rhs)
{
  DeleteAllPoints ();
  deepCopyAllPoints (rhs);     

  return *this;
}

bool Polyfunction::AddPoint(double x, double y)
{
   iterator it;
   for (it = m_points.begin (); it != m_points.end (); ++it)
   {
      if ((*it)->getX () == x /* && (*it)->getY () == y */) 
      {
	 // cout << " failed" << endl;
	 return false; 
      }

      if ((*it)->getX () > x) break;
   }

   // cout << " succeeded" << endl;


   Point *point = new Point (x, y);

   m_points.insert (it, point);

   if (m_max == 0 || m_max->getY () <= point->getY ())
   {
      m_max = point;
   }

   if (m_min == 0 || m_min->getY () >= point->getY ())
   {
      m_min = point;
   }
#if 0
   if (m_points.capacity () > 3 * m_points.size ())
   {
      cerr << "m_points.capacity () = " << m_points.capacity () << "!!" << endl;
      cerr << "m_points.size () = " << m_points.size () << "!!" << endl;
   }
#endif
   return true;
}

double Polyfunction::GetPoint (double x)
{
   iterator it;
   for (it = m_points.begin (); it != m_points.end (); ++it)
   {
      if ((*it)->getX () == x)
      {
	 return (*it)->getY (); 
      }
   }
   if (!m_points.empty() && x <= (* m_points.begin ())->getX ())
   {
      return (* m_points.begin ())->getY ();
   }
   else if (!m_points.empty() && x >= (* m_points.rbegin ())->getX ())
   {
      return (* m_points.rbegin ())->getY ();
   }
   return 0.0;
}


void Polyfunction::deepCopyAllPoints (const Polyfunction & src)
{
   //PETSC_ASSERT (m_points.size () == 0);
   assert(m_points.size () == 0);
   // deep copy of all points
   const_iterator srcIter;

   for (srcIter = src.m_points.begin ();
	 srcIter != src.m_points.end (); ++srcIter)
   {
      const Point *srcP = (*srcIter);
      Point * destP = new Point (* srcP);

      m_points.push_back (destP);
      
      if (src.m_max == srcP) m_max = destP;
      if (src.m_min == srcP) m_min = destP;
   }
}

void Polyfunction::DeleteAllPoints()
{
   // Reset all
   iterator it;
   for (it = m_points.begin (); it != m_points.end (); ++it)
   {
      delete (*it);
   }
   m_points.clear ();
   m_max = 0;
   m_min = 0;
}

bool Polyfunction::descending(double x) const
{
   const Point * low = 0;
   const Point * high = 0;

   if (!seedPoints (x, low, high))
   {
      return false;
   }
   return (low->getY () < high->getY ());
}

double Polyfunction::F(double x) const
{
   // No points in this polyfunction at all -> return NULLVALUE
   if (m_points.size () == 0)
      return NULLVALUE;

   Point *firstPoint = *m_points.begin ();
   Point *lastPoint = *m_points.rbegin ();


   // Check if in range, use constant extrapolation if not
   if (x <= firstPoint->getX ())
   {
      return firstPoint->getY ();
   }
   else if (x >= lastPoint->getX ())
   {
      return lastPoint->getY ();
   }

#define USESEEDPOINTS
#ifdef USESEEDPOINTS
   const Point *low = 0;
   const Point *high = 0;

   if (seedPoints (x, low, high))
   {
      // Calculate & return linearly interpolated value

      //PETSC_ASSERT (low);
      assert(low);
      //PETSC_ASSERT (high);
      assert(high);
      double f = low->getY () +
	 (high->getY () - low->getY ()) * (x - low->getX ()) /
	 (high->getX () - low->getX ());

      /*
      cout << "F ((" << low->getX () << ", " << low->getY () << ")";
      cout << ", (" << high->getX () << ", " << high->getY () << ")) = ";
      cout << f << endl;
      */

      return f;
   }
   else
   {
      return NULLVALUE;
   }
#else
   const_iterator it, prev;

  // Find the right place in the polypoint assuming ascending 
  // order of polypoint
   for (it = m_points.begin (); it != m_points.end (); ++it)
   {
      if ((*it)->getX () > x)
         break;
      else
         prev = it;
   }
   // Calculate & return linearly interpolated value
   return ((*prev)->getY () +
           ((*it)->getY () - (*prev)->getY ()) * (x - (*prev)->getX ()) /
	   ((*it)->getX () - (*prev)->getX ()));
#endif
}

double Polyfunction::MinX(double &y) const
{
   // Mind that this polyfunction is sorted on point->x !
   Point *firstPoint = *m_points.begin ();

   y = firstPoint->getY ();
   return firstPoint->getX ();
}

double Polyfunction::MaxX(double &y) const
{
   // Mind that this polyfunction is sorted on point->x !
   Point *lastPoint = *m_points.rbegin ();

   y = lastPoint->getY ();
   return lastPoint->getX ();
}

double Polyfunction::MaxY(double &x) const
{
   if (m_max)
   {
      x = m_max->getX ();
      return m_max->getY ();
   }
   else
   {
      x = NULLVALUE;
      return NULLVALUE;
   }
}

double Polyfunction::MinY(double &x) const
{
   if (m_min)
   {
      x = m_min->getX ();
      return m_min->getY ();
   }
   else
   {
      x = NULLVALUE;
      return NULLVALUE;
   }
}

double Polyfunction::MinY (double beginX, double endX)
{
   if (beginX < endX)
   {
      double tmpX = beginX;
      beginX = endX;
      endX = tmpX;
   }

   double xMax, yMin;

   if ((yMin = MaxY (xMax)) == NULLVALUE)
      return NULLVALUE;

   double y;

   y = F(beginX);
   if (y < yMin) yMin = y;

   y = F(endX);
   if (y < yMin) yMin = y;

   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      if ((*it)->getX () <= beginX && (*it)->getX () >= endX)
      {
	 y = (*it)->getY ();
	 if (y < yMin) yMin = y;
      }
   }

   return yMin;
}

double Polyfunction::MaxY (double beginX, double endX)
{
   if (beginX < endX)
   {
      double tmpX = beginX;
      beginX = endX;
      endX = tmpX;
   }

   double xMin, yMax;

   if ((yMax = MinY (xMin)) == NULLVALUE)
      return NULLVALUE;

   double y;

   y = F(beginX);
   if (y > yMax) yMax = y;

   y = F(endX);
   if (y > yMax) yMax = y;

   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      Point * point = (*it);
      if (point->getX () <= beginX && point->getX () >= endX)
      {
	 y = point->getY ();
	 if (y > yMax) yMax = y;
      }
   }

   return yMax;
}

void Polyfunction::printGlobalStatus() {
  cout << "Polyfunction : " << s_maxInstanceCount;
  cout << " created " << s_instanceCount << " still allocated" << endl;
  Point::printStatus();
}

void Polyfunction::printPoints()
{
   if (m_points.begin () == m_points.end ())
   {
      cout << "No points in this polyfunction" << endl;
      return;
   }

   for (reverse_iterator it = m_points.rbegin (); it != m_points.rend (); it++)
   {
      cout << "(" << (*it)->getX () << "," << (*it)->getY () << ") ";
   }
   cout << endl;
}

void Polyfunction::printStatus()
{
   cout << "Function contains " << m_points.size () << " points" << endl;
   bool first = true;
   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      if (!first) cout << ", ";
      first = false;
      cout << "(" << (*it)->getX () << "," << (*it)->getY () << ")";
   }
   cout << endl;
}

void Polyfunction::RaiseBy(double r)
{
   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      (*it)->raiseY (r);
   }
}

void Polyfunction::ScaleBy(double factor)
{
   //PETSC_ASSERT (factor > 0);
   assert(factor > 0);
   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      (*it)->scaleY (factor);
   }
}
bool Polyfunction::findIntervalForValueY(double y, const Point * & low, const Point * & high) const
{
   bool minfound = false;
   bool maxfound = false;

   for (const_iterator it = m_points.begin (),itEnd = m_points.end () ; it != itEnd; ++it)
   {
  
      const Point * point = * it;
      //PETSC_ASSERT (point);
      assert(point);
      if (point->getY () <= y)
      {
         low = point;
         minfound = true;
      }
      if ((maxfound == false) && (point->getY () >= y))
      {
         high = point;
         maxfound = true;
	 break;
      }
   }

   return (minfound && maxfound);
}
bool Polyfunction::seedPoints(double x, const Point * & low, const Point * & high) const
{
   bool minfound = false;
   bool maxfound = false;

   for (const_iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      const Point * point = * it;
      //PETSC_ASSERT (point);
      assert(point);
      if (point->getX () <= x)
      {
         low = point;
         minfound = true;
      }
      if ((maxfound == false) && (point->getX () > x))
      {
         high = point;
         maxfound = true;
	 break;
      }
   }

   return (minfound && maxfound);
}

void Polyfunction::swap(Polyfunction &pf)
{
  Point * temp;
      
  temp = m_max;
  m_max = pf.m_max;
  pf.m_max = temp; 

  temp = m_min;
  m_min = pf.m_min;
  pf.m_min = temp; 

  m_points.swap(pf.m_points);
}

Polyfunction::~Polyfunction()
{
  DeleteAllPoints();
  s_instanceCount--;
}

}
