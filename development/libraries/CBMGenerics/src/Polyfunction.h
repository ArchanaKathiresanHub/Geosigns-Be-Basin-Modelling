//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CBMGENERICS_POLYFUNCTION_H_
#define _CBMGENERICS_POLYFUNCTION_H_

#define NEWPOLYF

#include <vector>

namespace CBMGenerics
{

class Polyfunction
{
public:

   class Point
   {
   public:
      double getX () const;
      double getY () const;
      static void printStatus ();
      void setX (double x);
      void setY (double y);
      void raiseY (double r);
      void scaleY (double factor);
      Point (double x, double y);
      explicit Point (const Point & p);
      Point ();
      ~Point ();
      Point const& operator=(const Point&& rhs) = delete;
      Point(const Point&& rhs) = delete;

   private:
      double m_x = 0.0;
      double m_y = 0.0;
      static int s_instanceCount;
      static int s_maxInstanceCount;

   };

  typedef std::vector < Point * > Polypoint;
  typedef Polypoint::iterator iterator;
  typedef Polypoint::const_iterator const_iterator;
  typedef Polypoint::const_reverse_iterator const_reverse_iterator;
  typedef Polypoint::reverse_iterator reverse_iterator;

  Polyfunction ();
  Polyfunction (const Polyfunction & original);

  ~Polyfunction ();
  Polyfunction & operator= (const Polyfunction & rhs);
  bool AddPoint (double x, double y);
  double GetPoint (double x);
  bool descending (double x) const;
  double F (double x) const;

  iterator getBegin ();
  iterator begin ();
  iterator getEnd ();
  iterator end ();
  const_reverse_iterator getRBegin () const;
  reverse_iterator rbegin ();
  const_reverse_iterator getREnd () const;
  reverse_iterator rend ();

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

inline double Polyfunction::Point::getX () const
{
   return m_x;
}

inline double Polyfunction::Point::getY () const
{
   return m_y;
}

inline void Polyfunction::Point::setX (double x)
{
   m_x = x;
}

inline void Polyfunction::Point::setY (double y)
{
   m_y = y;
}

inline void Polyfunction::Point::raiseY (double r)
{
   m_y += r;
}

inline void Polyfunction::Point::scaleY (double factor)
{
   m_y *= factor;
}

inline Polyfunction::Point::Point (double x, double y) : m_x (x), m_y (y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

inline Polyfunction::Point::Point (const Point & p) : m_x (p.m_x), m_y (p.m_y)
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

inline Polyfunction::Point::Point ()
{
   s_instanceCount++;
   s_maxInstanceCount++;
}

inline Polyfunction::Point::~Point ()
{
   s_instanceCount--;
}

inline Polyfunction::iterator Polyfunction::getBegin ()
{
   return m_points.begin ();
}

inline Polyfunction::iterator Polyfunction::begin ()
{
   return m_points.begin ();
}

inline Polyfunction::iterator Polyfunction::getEnd ()
{
   return m_points.end ();
}

inline Polyfunction::iterator Polyfunction::end ()
{
   return m_points.end ();
}

inline Polyfunction::const_reverse_iterator Polyfunction::getRBegin () const
{
   return m_points.rbegin ();
}

inline Polyfunction::reverse_iterator Polyfunction::rbegin ()
{
   return m_points.rbegin ();
}

inline Polyfunction::const_reverse_iterator Polyfunction::getREnd () const
{
   return m_points.rend ();
}

inline Polyfunction::reverse_iterator Polyfunction::rend ()
{
   return m_points.rend ();
}

}
#endif                          // ifndef _CBMGENERICS_POLYFUNCTION_H_
