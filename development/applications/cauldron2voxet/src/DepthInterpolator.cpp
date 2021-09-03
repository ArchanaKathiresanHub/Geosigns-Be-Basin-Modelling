#include "DepthInterpolator.h"
#include <assert.h>

DepthInterpolator::DepthInterpolator () {
   m_lastLayer = 0;
   m_isNull = true;
}

DepthInterpolator::~DepthInterpolator () {
}


void DepthInterpolator::setNullInterpolator ( const bool isNull ) {
   m_isNull = isNull;
}

bool DepthInterpolator::isNullInterpolator () const {
   return m_isNull;
}

void DepthInterpolator::setNullValue ( const float nullValue ) {
   m_nullValue = nullValue;
}

void DepthInterpolator::setNumberOfLayers ( const int numberOfLayers ) {
   m_interpolators.clear ();
   m_layerInterpolators.clear ();
   m_interpolators.reserve ( numberOfLayers );
   m_lastLayer = 0;
}

LayerInterpolator& DepthInterpolator::addLayer ( const std::string& layerName ) {
   m_interpolators.push_back ( LayerInterpolator ());
   m_layerInterpolators [ layerName ] = &m_interpolators [ m_interpolators.size () - 1 ];
   return m_interpolators [ m_interpolators.size () - 1 ];
}

LayerInterpolator& DepthInterpolator::operator ()( const std::string& layerName ) {

   LayerInterpolatorMapping::iterator layer = m_layerInterpolators.find ( layerName );

   if ( layer == m_layerInterpolators.end ()) {
      std::cout << " NOT found layer " << layerName << std::endl;
      assert ( false );
   }

   return *(layer->second);

}

double DepthInterpolator::topOfInterval () const {
   return m_interpolators [ 0 ].topOfInterval ();
}

double DepthInterpolator::bottomOfInterval () const {
   return m_interpolators [ m_interpolators.size () - 1 ].bottomOfInterval ();
}


#if 0
const LayerInterpolator* DepthInterpolator::getInterpolator ( const float z ) const {

   int layerNumber = findLayer ( z );

   if ( layerNumber != -1 ) {
      return &m_interpolators [ layerNumber ];
   } else {
      return 0;
   }

}
#endif

double DepthInterpolator::operator ()( const double z ) const {

   if ( m_isNull ) {
      return m_nullValue;
   } else {

      int layerNumber = findLayer ( z );

      if ( layerNumber != -1 ) {
         return m_interpolators [ layerNumber ]( z );
      } else {
         return m_nullValue;
      }

   }

}

int DepthInterpolator::findLayer ( const double z ) const {

   if ( m_isNull ) {
      return -1;
   }

   if ( m_lastLayer != -1 and m_interpolators [ m_lastLayer ].containsDepth ( z )) {
      return m_lastLayer;
   }

   int i;

   for ( i = 0; i < m_interpolators.size (); ++i ) {

      if ( m_interpolators [ i ].containsDepth ( z )) {
         m_lastLayer = i;
         return i;
      }

   }

   m_lastLayer = -1;
   return m_lastLayer;
}

void DepthInterpolator::print ( std::ostream& o ) const {

   int i;

   for ( i = 0; i < m_interpolators.size (); ++i ) {
      o << std::endl << std::endl << " interpolator " << i << std::endl;
      m_interpolators [ i ].print ( o );
   }

   o << std::endl << std::endl;
}
