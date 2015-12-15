#ifndef _IBSINTERPOLATOR_H
#define _IBSINTERPOLATOR_H

// Header for Interpolation and Extrapolation in 1-D

//________________________________________________________________
// Author: Alfred v/d Hoeven; EPT-AE
// Version:
// Date:   September, 1999
// System: Independent
//________________________________________________________________

#include <vector>

using namespace std;

namespace ibs {

   // Helper class for the interpolator and other classes.
   class XF {

   public:

      XF () {}

      ~XF () {}

      XF ( const double xx, const double ff ): x(xx), f(ff) {}

      /// Return the x-value.
      double getX () const;

      /// Return the f-value.
      double getF () const;

      /// Set the x-value.
      void setX ( const double newX );

      /// Set the f-value.
      void setF ( const double newF );

      /// Set both the x- and f-values.
      void set ( const double newX,
                 const double newF );

      /// Print the XF-object to cerr.
      void print (void) const;

   private:

      double x;
      double f;

      friend bool operator < (const XF& xf1, const XF& xyf2);
      friend bool UnOrderedSorter (const XF& xf1, const XF& xyf2);
      friend bool AscendingSorter (const XF& xf1, const XF& xyf2);
      friend bool DescendingSorter (const XF& xf1, const XF& xyf2);

   };


   /// \brief Used for sorting algorithms.
   class XYAscendingOrderFunctor {
   public :
      bool operator ()( const XF& xf1, const XF& xf2 ) const;
   };

   /// \brief Used for sorting algorithms.
   class XYDecendingOrderFunctor {
   public :
      bool operator ()( const XF& xf1, const XF& xf2 ) const;
   };

   class Interpolator
   {

      typedef bool (* Sorter) (const XF & xf1, const XF & xf2);

      Sorter d_sorterFunc;

      typedef vector<XF>	    vectorXF;
   
   public:

      typedef enum { UnOrdered, Ascending, Descending } SorterType;
   
      Interpolator (SorterType sorterType = UnOrdered);
      Interpolator ( const Interpolator& interp );
      ~Interpolator (void); 
   
      enum Extrapolation { linear, constant, none };
   
      // input function elements (x, f)
      void    addPoint(double x, double f);
   
      // compute f given (x)
      double  compute(double x, Extrapolation type = linear);

      /// \brief Sort the data if required.
      void freeze ();

      void print (void);
   
   private:
   
      // whether the table was sorted.
      int d_sorted;
      // Tables that store the function values.
      vectorXF d_vectorXF;
   };

} // end ibs


inline double ibs::XF::getX () const {
   return x;
}

inline double ibs::XF::getF () const {
   return f;
}

inline bool ibs::XYAscendingOrderFunctor::operator ()( const XF& xf1, const XF& xf2 ) const {

   if ( xf1.getX () < xf2.getX ()) {
      return true;
   } else if ( xf1.getX () == xf2.getX () and xf1.getF () < xf2.getF ()) {
      return true;
   } else {
      return false;
   }

}

inline bool ibs::XYDecendingOrderFunctor::operator ()( const XF& xf1, const XF& xf2 ) const {

   if ( xf1.getX () > xf2.getX ()) {
      return true;
   } else if ( xf1.getX () == xf2.getX () and xf1.getF () > xf2.getF ()) {
      return true;
   } else {
      return false;
   }

}


#endif
