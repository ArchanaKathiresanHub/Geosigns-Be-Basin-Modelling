#ifndef _POLYFUNCTION_H_
#define _POLYFUNCTION_H_

#define NEWPOLYF

#include <vector>
using namespace std;


template <typename T> class Polyfunction
{
public:

   class Point
   {
   public:
      inline T getX () const;
      inline T getY () const;
      static void printStatus ();
      inline void setX (T x);
      inline void setY (T y);
      inline void raiseY (T r);
      inline void scaleY (T factor);
      inline Point (T x, T y);
      inline Point (const Point & p);
      inline Point ();
      inline ~Point ();

    private:
      T m_x;
      T m_y;
      static int s_instanceCount;
      static int s_maxInstanceCount;

   };

private:

  typedef vector < Point * > Polypoint;
  typedef typename Polypoint::iterator iterator;
  typedef typename Polypoint::const_iterator const_iterator;
  typedef typename Polypoint::const_reverse_iterator const_reverse_iterator;
  typedef typename Polypoint::reverse_iterator reverse_iterator;



public:

  Polyfunction ();
  Polyfunction (const Polyfunction & original);

  ~Polyfunction ();
  Polyfunction & operator= (const Polyfunction & rhs);
  bool AddPoint (T x, T y);
  bool descending (T x) const;
  T F (T x) const;

  inline iterator getBegin ();
  inline iterator begin ();
  inline iterator getEnd ();
  inline iterator end ();
  inline const_reverse_iterator getRBegin () const;
  inline reverse_iterator rbegin ();
  inline const_reverse_iterator getREnd () const;
  inline reverse_iterator rend ();

  T MinX (T &y) const;
  T MaxX (T &y) const;

  T MinY(T &x) const;
  T MaxY (T &x) const;
  static void printGlobalStatus ();
  void printPoints ();
  void printStatus ();
  void RaiseBy (T r);
  void ScaleBy (T factor);
  void swap (Polyfunction & pf);
  bool seedPoints (T x, const Point * & low, const Point * & high) const;

  // this should be private

private:

  Polypoint m_points;

  void DeleteAllPoints ();
  void deepCopyAllPoints (const Polyfunction & src);

  static int s_instanceCount;
  static int s_maxInstanceCount;

  Point * m_max;
  Point * m_min;

};


//implementation



template <typename T> T Polyfunction<T>::Point::getX () const
{
   return m_x;
}

template <typename T> T Polyfunction<T>::Point::getY () const
{
   return m_y;
}

template <typename T> void Polyfunction<T>::Point::setX (T x)
{
   m_x = x;
}

template <typename T> void Polyfunction<T>::Point::setY (T y)
{
   m_y = y;
}

template <typename T> void Polyfunction<T>::Point::raiseY (T r)
{
   m_y += r;
}

template <typename T> void Polyfunction<T>::Point::scaleY (T factor)
{
   m_y *= factor;
}

template <typename T> Polyfunction<T>::Point::Point (T x, T y) : m_x (x), m_y (y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

template <typename T> Polyfunction<T>::Point::Point (const Point & p) : m_x (p.m_x), m_y (p.m_y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

template <typename T> Polyfunction<T>::Point::Point () : m_x (0), m_y (0)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

template <typename T> Polyfunction<T>::Point::~Point ()
{
   s_instanceCount--;
}

template <typename T> typename Polyfunction<T>::iterator Polyfunction<T>::getBegin ()
{
   return m_points.begin ();
}

template <typename T> typename Polyfunction<T>::iterator Polyfunction<T>::begin ()
{
   return m_points.begin ();
}

template <typename T> typename Polyfunction<T>::iterator Polyfunction<T>::getEnd ()
{
   return m_points.end ();
}

template <typename T> typename Polyfunction<T>::iterator Polyfunction<T>::end ()
{
   return m_points.end ();
}

template <typename T> typename Polyfunction<T>::const_reverse_iterator Polyfunction<T>::getRBegin () const
{
   return m_points.rbegin ();
}

template <typename T> typename Polyfunction<T>::reverse_iterator Polyfunction<T>::rbegin ()
{
   return m_points.rbegin ();
}

template <typename T> typename Polyfunction<T>::const_reverse_iterator Polyfunction<T>::getREnd () const
{
   return m_points.rend ();
}

template <typename T> typename Polyfunction<T>::reverse_iterator Polyfunction<T>::rend ()
{
   return m_points.rend ();
}


#include <iostream>
#include <assert.h>

using namespace std;

#include "globaldefs.h"
#include "utils.h"

const double NULLVALUE           = -9999;

template <typename T> int Polyfunction<T>::s_instanceCount = 0;
template <typename T> int Polyfunction<T>::s_maxInstanceCount = 0;
template <typename T> int Polyfunction<T>::Point::s_instanceCount = 0;
template <typename T> int Polyfunction<T>::Point::s_maxInstanceCount = 0;

template <typename T> void Polyfunction<T>::Point::printStatus()
{
   cout << "Point        : " << s_maxInstanceCount;
   cout << " created " << s_instanceCount << " still allocated" << endl;
}

template <typename T> Polyfunction<T>::Polyfunction()
{
   m_points.reserve (2);
   s_instanceCount++;
   s_maxInstanceCount++;
   m_max = 0;
   m_min = 0;
}

template <typename T> Polyfunction<T>::Polyfunction(const Polyfunction& original)
{
   m_points.reserve (2);
   s_instanceCount++;
   s_maxInstanceCount++;
   m_max = 0;
   m_min = 0;

   deepCopyAllPoints (original);
}

template <typename T> Polyfunction<T>& Polyfunction<T>::operator= (const Polyfunction<T> &rhs)
{
  DeleteAllPoints ();
  deepCopyAllPoints (rhs);     

  return *this;
}

template <typename T> bool Polyfunction<T>::AddPoint(T x, T y)
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

   if (m_points.capacity () > 3 * m_points.size ())
   {
      cerr << "m_points.capacity () = " << m_points.capacity () << "!!" << endl;
      cerr << "m_points.size () = " << m_points.size () << "!!" << endl;
   }
   return true;
}

template <typename T> void Polyfunction<T>::deepCopyAllPoints (const Polyfunction & src)
{
   PETSC_ASSERT (m_points.size () == 0);
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

template <typename T> void Polyfunction<T>::DeleteAllPoints()
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

template <typename T> bool Polyfunction<T>::descending(T x) const
{
   const Point * low = 0;
   const Point * high = 0;

   if (!seedPoints (x, low, high))
   {
      return false;
   }
   return (low->getY () < high->getY ());
}

template <typename T> T Polyfunction<T>::F(T x) const
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

      PETSC_ASSERT (low);
      PETSC_ASSERT (high);
      T f = low->getY () +
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

template <typename T> T Polyfunction<T>::MinX(T &y) const
{
   // Mind that this polyfunction is sorted on point->x !
   Point *firstPoint = *m_points.begin ();

   y = firstPoint->getY ();
   return firstPoint->getX ();
}

template <typename T> T Polyfunction<T>::MaxX(T &y) const
{
   // Mind that this polyfunction is sorted on point->x !
   Point *lastPoint = *m_points.rbegin ();

   y = lastPoint->getY ();
   return lastPoint->getX ();
}

template <typename T> T Polyfunction<T>::MaxY(T &x) const
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

template <typename T> T Polyfunction<T>::MinY(T &x) const
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

template <typename T> void Polyfunction<T>::printGlobalStatus() {
  cout << "Polyfunction : " << s_maxInstanceCount;
  cout << " created " << s_instanceCount << " still allocated" << endl;
  Point::printStatus();
}

template <typename T> void Polyfunction<T>::printPoints()
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

template <typename T> void Polyfunction<T>::printStatus()
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

template <typename T> void Polyfunction<T>::RaiseBy(T r)
{
   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      (*it)->raiseY (r);
   }
}

template <typename T> void Polyfunction<T>::ScaleBy(T factor)
{
   PETSC_ASSERT (factor > 0);
   for (iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      (*it)->scaleY (factor);
   }
}

template <typename T> bool Polyfunction<T>::seedPoints(T x, const Point * & low, const Point * & high) const
{
   bool minfound = false;
   bool maxfound = false;

   for (const_iterator it = m_points.begin (); it != m_points.end (); ++it)
   {
      const Point * point = * it;
      PETSC_ASSERT (point);
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

template <typename T> void Polyfunction<T>::swap(Polyfunction &pf)
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

template <typename T> Polyfunction<T>::~Polyfunction()
{
  DeleteAllPoints();
  s_instanceCount--;
}


#endif                          // ifndef _POLYFUNCTION_H_
