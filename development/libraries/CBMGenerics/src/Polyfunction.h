#ifndef _CBMGENERICS_POLYFUNCTION_H_
#define _CBMGENERICS_POLYFUNCTION_H_

#include "DllExport.h"

#define NEWPOLYF

#include <vector>
using namespace std;

namespace CBMGenerics
{

class CBMGENERICS_DLL_EXPORT Polyfunction
{
public:

   class Point
   {
   public:
      inline double getX () const;
      inline double getY () const;
      static void printStatus ();
      inline void setX (double x);
      inline void setY (double y);
      inline void raiseY (double r);
      inline void scaleY (double factor);
      inline Point (double x, double y);
      inline Point (const Point & p);
      inline Point ();
      inline ~Point ();

    private:
      double m_x;
      double m_y;
      static int s_instanceCount;
      static int s_maxInstanceCount;

   };

public:

  typedef vector < Point * > Polypoint;
  typedef Polypoint::iterator iterator;
  typedef Polypoint::const_iterator const_iterator;
  typedef Polypoint::const_reverse_iterator const_reverse_iterator;
  typedef Polypoint::reverse_iterator reverse_iterator;

public:

  Polyfunction ();
  Polyfunction (const Polyfunction & original);

  ~Polyfunction ();
  Polyfunction & operator= (const Polyfunction & rhs);
  bool AddPoint (double x, double y);
  double GetPoint (double x);
  bool descending (double x) const;
  double F (double x) const;

  inline iterator getBegin ();
  inline iterator begin ();
  inline iterator getEnd ();
  inline iterator end ();
  inline const_reverse_iterator getRBegin () const;
  inline reverse_iterator rbegin ();
  inline const_reverse_iterator getREnd () const;
  inline reverse_iterator rend ();

  double MinX (double &y) const;
  double MaxX (double &y) const;

  double MinY(double &x) const;
  double MaxY (double &x) const;
  double MinY (double beginX, double endX);
  double MaxY (double beginX, double endX);
  static void printGlobalStatus ();
  void printPoints ();
  void printStatus ();
  void RaiseBy (double r);
  void ScaleBy (double factor);
  void swap (Polyfunction & pf);
  bool seedPoints (double x, const Point * & low, const Point * & high) const;
  bool findIntervalForValueY(double y, const Point * & low, const Point * & high) const;

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

double Polyfunction::Point::getX () const
{
   return m_x;
}

double Polyfunction::Point::getY () const
{
   return m_y;
}

void Polyfunction::Point::setX (double x)
{
   m_x = x;
}

void Polyfunction::Point::setY (double y)
{
   m_y = y;
}

void Polyfunction::Point::raiseY (double r)
{
   m_y += r;
}

void Polyfunction::Point::scaleY (double factor)
{
   m_y *= factor;
}

Polyfunction::Point::Point (double x, double y) : m_x (x), m_y (y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

Polyfunction::Point::Point (const Point & p) : m_x (p.m_x), m_y (p.m_y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

Polyfunction::Point::Point () : m_x (0), m_y (0)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

Polyfunction::Point::~Point ()
{
   s_instanceCount--;
}

Polyfunction::iterator Polyfunction::getBegin ()
{
   return m_points.begin ();
}

Polyfunction::iterator Polyfunction::begin ()
{
   return m_points.begin ();
}

Polyfunction::iterator Polyfunction::getEnd ()
{
   return m_points.end ();
}

Polyfunction::iterator Polyfunction::end ()
{
   return m_points.end ();
}

Polyfunction::const_reverse_iterator Polyfunction::getRBegin () const
{
   return m_points.rbegin ();
}

Polyfunction::reverse_iterator Polyfunction::rbegin ()
{
   return m_points.rbegin ();
}

Polyfunction::const_reverse_iterator Polyfunction::getREnd () const
{
   return m_points.rend ();
}

Polyfunction::reverse_iterator Polyfunction::rend ()
{
   return m_points.rend ();
}

}
#endif                          // ifndef _CBMGENERICS_POLYFUNCTION_H_
