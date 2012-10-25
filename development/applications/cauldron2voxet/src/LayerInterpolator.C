#include "LayerInterpolator.h"

#include <cmath>
#include <assert.h>

const float LayerInterpolator::IBSNULLVALUE = -9999;


LayerInterpolator::LayerInterpolator () {
   clear ();
}

LayerInterpolator::~LayerInterpolator () {
   clear ();
}


void LayerInterpolator::setMaximumNumberOfSamples ( const int size ) {

   m_depths.clear ();
   m_propertyValues.clear ();

   m_depths.reserve ( size );
   m_propertyValues.reserve ( size );
}


void LayerInterpolator::setInterval ( const float top,
                                      const float bottom ) {

   assert ( top <= bottom );

   m_top = top;
   m_bottom = bottom;
}


void LayerInterpolator::addSample ( const float z,
                                    const float value ) {

   m_depths.push_back ( z );
   m_propertyValues.push_back ( value );
}


void LayerInterpolator::freeze () {

   //
   const double tolerance = 1.0e-3;

   // 
   // 
   //             ( x - a )            ( b - x )
   //     P(x) =  --------- * f(b)  +  --------- * f (a)
   //             ( b - a )            ( b - a )
   // 
   // 
   //             x * ( f(b) - f(a))    b * f (a)    a * f (b)
   //          =  ------------------ +  --------  -  ---------
   //                  ( b - a )        ( b - a )    ( b - a )
   // 
   //          = A + B x
   // 
   // 
   //        b * f (a)     a * f (b)
   //   A =  ---------  -  ---------
   //        ( b - a )     ( b - a )
   // 
   // 
   //        ( f(b) - f(a))    
   //   B  =  --------------
   //           ( b - a )
   // 
   // 
   // 
   //  Rearranging the calculation like this reduces the flop count
   //  considerably, from 6 (including a division) to 2, per evaluation.
   // 
   // 

   assert ( m_depths.size () > 1 );

   int i;
   float divisor;

   m_coeffsA.reserve ( m_depths.size ());
   m_coeffsB.reserve ( m_depths.size ());

   for ( i = 0; i < m_depths.size () - 1; ++i ) {

//       if (  m_depths [ i + 1 ] > m_depths [ i ] + tolerance ) {
         divisor = 1.0 / ( m_depths [ i + 1 ] - m_depths [ i ]);
//       } else {
//          divisor = 0.0;
//       }

      m_coeffsA [ i ] = divisor * ( m_depths [ i + 1 ] * m_propertyValues [ i ] - m_depths [ i ] * m_propertyValues [ i + 1 ]);
      m_coeffsB [ i ] = divisor * ( m_propertyValues [ i + 1 ] - m_propertyValues [ i ]);
   }

   m_lastPanel = 0;
}


void LayerInterpolator::clear () {

   m_top = IBSNULLVALUE;
   m_bottom = IBSNULLVALUE;

   m_depths.clear ();
   m_propertyValues.clear ();
   m_coeffsA.clear ();
   m_coeffsB.clear ();

   m_lastPanel = 0;
}

float LayerInterpolator::topOfInterval () const {
   return m_top;
}

float LayerInterpolator::bottomOfInterval () const {
   return m_bottom;
}

bool LayerInterpolator::containsDepth ( const float z ) const {
   return m_top <= z and z <= m_bottom;
}

float LayerInterpolator::operator ()( const float z ) const {

   int panel = findPanel ( z );

   return m_coeffsA [ panel ] + m_coeffsB [ panel ] * z;
}

int LayerInterpolator::findPanel ( const float z ) const {

   if ( m_depths [ m_lastPanel ] <= z and z <= m_depths [ m_lastPanel + 1 ]) {
      return m_lastPanel;
   }

   int i;

   for ( i = 0; i < m_depths.size () - 1; ++i ) {

      if ( m_depths [ i ] <= z and z <= m_depths [ i + 1 ] ) {
         m_lastPanel = i;
         return i;
      }
      
   }

   // Should this be an error?
   if ( z < m_depths [ 0 ]) {
      m_lastPanel = 0;
   } else {
      // z > m_depths [ m_depths.size () - 1 ]
      m_lastPanel = m_depths.size () - 1;
   }

   return m_lastPanel;
}


void LayerInterpolator::print ( std::ostream& o ) const {

   int i;

   o << " interval: [ " << m_top << ", " << m_bottom << " ]" << std::endl;
   o << " number of samples: " << m_depths.size () << std::endl;

   for ( i = 0; i < m_depths.size (); ++i ) {
      o << " sample data: " << m_depths [ i ] << "  " << m_propertyValues [ i ] << std::endl;
   }

}

