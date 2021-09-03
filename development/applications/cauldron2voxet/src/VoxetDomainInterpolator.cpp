#include "VoxetDomainInterpolator.h"

#include "array.h"

using namespace ibs;

VoxetDomainInterpolator::VoxetDomainInterpolator ( const unsigned int nodesX,
                                                   const unsigned int nodesY ) :
   m_nodesX ( nodesX ),
   m_nodesY ( nodesY ) {

   m_interpolators = Array<DepthInterpolator>::create2d ( m_nodesX, m_nodesY );

}


VoxetDomainInterpolator::~VoxetDomainInterpolator () {
   Array<DepthInterpolator>::delete2d ( m_interpolators );
}

const DepthInterpolator& VoxetDomainInterpolator::operator ()( const unsigned int i,
                                                               const unsigned int j ) const {
   return m_interpolators [ i ][ j ];
}

DepthInterpolator& VoxetDomainInterpolator::operator ()( const unsigned int i,
                                                         const unsigned int j ) {
   return m_interpolators [ i ][ j ];
}


double VoxetDomainInterpolator::operator ()( const unsigned int i,
                                            const unsigned int j,
                                            const double        depth ) const {
   return m_interpolators [ i ][ j ]( depth );
}


void VoxetDomainInterpolator::setNullValue ( const float nullValue ) {

   int i;
   int j;

   for ( i = 0; i < m_nodesX; ++i ) {

      for ( j = 0; j < m_nodesY; ++j ) {
         m_interpolators [ i ][ j ].setNullValue ( nullValue );
      }

   }

}
