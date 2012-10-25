#ifndef _INTERPOLATOR_H
#define _INTERPOLATOR_H

// Header for Interpolation and Extrapolation in 1-D

//________________________________________________________________
// Author: Alfred v/d Hoeven; EPT-AE
// Version:
// Date:   September, 1999
// System: Independent
//________________________________________________________________

#include <vector>

using namespace std;

// Helper class for the interpolator
class XF {
 public:
   XF () {}
   ~XF () {}
   XF (double xx, double ff): x(xx), f(ff) {}

 private:
   double x;
   double f;

   void print (void);

   friend bool operator < (const XF& xf1, const XF& xyf2);
   friend class Interpolator;
   friend bool UnOrderedSorter (const XF& xf1, const XF& xyf2);
   friend bool AscendingSorter (const XF& xf1, const XF& xyf2);
   friend bool DescendingSorter (const XF& xf1, const XF& xyf2);
};


class Interpolator
{
   typedef bool (* Sorter) (const XF & xf1, const XF & xf2);

   Sorter m_sorterFunc;

   typedef vector<XF>	    vectorXF;
   
public:
   typedef enum { UnOrdered, Ascending, Descending } SorterType;
   
   Interpolator (SorterType sorterType = UnOrdered);
   ~Interpolator (void); 
   
   enum Extrapolation { linear, constant, none };
   
   // input function elements (x, f)
   void    addPoint(double x, double f);
   
   // compute f given (x)
   double  compute(double x, Extrapolation type = linear);

   void print (void);
   
private:
   
   // whether the table was sorted.
   bool m_sorted;
   // Tables that store the function values.
   vectorXF m_vectorXF;
};


#endif
