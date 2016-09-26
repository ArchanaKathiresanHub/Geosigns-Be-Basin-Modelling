//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERPOLATOR_H
#define _INTERPOLATOR_H

// Header for Interpolation and Extrapolation in 1-D

#include <vector>

using namespace std;

namespace migration
{

   // Helper class for the interpolator
   class XF
   {
   public:
      XF () {}
      ~XF () {}
      XF (double xx, double ff) : x (xx), f (ff) {}

      double getX () const
      {
         return x;
      }

      double getF () const
      {
         return f;
      }

   private:
      double x;
      double f;

      void print (void);

      friend class Interpolator;

   };

   bool operator < (const XF& xf1, const XF& xyf2);
   bool UnOrderedSorter (const XF& xf1, const XF& xyf2);
   bool AscendingSorter (const XF& xf1, const XF& xyf2);
   bool DescendingSorter (const XF& xf1, const XF& xyf2);

   class Interpolator
   {
      typedef bool (*Sorter) (const XF & xf1, const XF & xf2);

      Sorter m_sorterFunc;

      typedef vector<XF>	    vectorXF;

   public:
      typedef enum { UnOrdered, Ascending, Descending } SorterType;

      Interpolator (SorterType sorterType = UnOrdered);
      ~Interpolator (void);

      enum Extrapolation { linear, constant, none };

      // input function elements (x, f)
      void    addPoint (double x, double f);

      // compute f given (x)
      double  compute (double x, Extrapolation type = linear);

      void print (void);

   private:

      // whether the table was sorted.
      bool m_sorted;
      // Tables that store the function values.
      vectorXF m_vectorXF;
   };

}
#endif
