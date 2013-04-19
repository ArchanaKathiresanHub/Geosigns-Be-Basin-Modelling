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
   void setInterval ( const float top,
                      const float bottom );

   /// Add a sample to the interplator.
   void addSample ( const float z,
                    const float value );

   /// Freeze the interpolator, i.e. compute the interpolator polynomials.
   void freeze ();

   /// Clear the interpoaltor.
   void clear ();

   /// The top end of the interval.
   float topOfInterval () const;

   /// The bottom end of the interval.
   float bottomOfInterval () const;

   /// Is the depth within this interval.
   bool containsDepth ( const float z ) const;

   /// Interpolator for the depth.
   float operator ()( const float z ) const;

   /// Print the interpolator to the stream.
   void print ( std::ostream& o ) const;
   

private :

   static const float IBSNULLVALUE;

   int findPanel ( const float depth ) const;

   mutable int m_lastPanel;

   float m_top;
   float m_bottom;

   FloatArray m_depths;
   FloatArray m_propertyValues;

   FloatArray m_coeffsA;
   FloatArray m_coeffsB;

};


#endif // _LAYER_INTERPOALTOR_H_
