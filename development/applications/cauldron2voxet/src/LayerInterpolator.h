#ifndef _LAYER_INTERPOALTOR_H_
#define _LAYER_INTERPOALTOR_H_

#include <iostream>
#include <vector>

/// A simple piecewise linear interpolator.
class LayerInterpolator {

   typedef std::vector<float> FloatArray;

public :

   LayerInterpolator ();

   ~LayerInterpolator ();

   /// Set the maximum number of samples the interpolator will contain.
   ///
   /// This is to reduce the number of memory allocations that may occur.
   void setMaximumNumberOfSamples ( const int size );

   /// Set the limits of the oned-domain.
   void setInterval ( const double top,
                      const double bottom );

   /// Add a sample to the interplator.
   void addSample ( const double z,
                    const double value );

   /// Freeze the interpolator, i.e. compute the interpolator polynomials.
   void freeze ();

   /// Clear the interpoaltor.
   void clear ();

   /// The top end of the interval.
   double topOfInterval () const;

   /// The bottom end of the interval.
   double bottomOfInterval () const;

   /// Is the depth within this interval.
   bool containsDepth ( const double z ) const;

   /// Interpolator for the depth.
   double operator ()( const double z ) const;

   /// Print the interpolator to the stream.
   void print ( std::ostream& o ) const;


private :

   static const float IBSNULLVALUE;

   int findPanel ( const double depth ) const;

   mutable int m_lastPanel;

   double m_top;
   double m_bottom;

   FloatArray m_depths;
   FloatArray m_propertyValues;

   FloatArray m_coeffsA;
   FloatArray m_coeffsB;

};


#endif // _LAYER_INTERPOALTOR_H_
