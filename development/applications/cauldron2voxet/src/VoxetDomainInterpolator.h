#ifndef _VoxetDomainInterpolator_H_
#define _VoxetDomainInterpolator_H_

#include "DepthInterpolator.h"

/// Just a simple 2d-array of depth-interpolators so that construction and deletion is simplified.
class VoxetDomainInterpolator {

public :

   VoxetDomainInterpolator ( const unsigned int nodesX,
                             const unsigned int nodesY );

   ~VoxetDomainInterpolator ();

   /// Compute the interpolator at the position defined.
   double operator ()( const unsigned int i,
                      const unsigned int j,
                      const double        depth ) const;

   /// Return the depth-interpolator at the map-location.
   const DepthInterpolator& operator ()( const unsigned int i,
                                         const unsigned int j ) const;

   /// Return the depth-interpolator at the map-location.
   DepthInterpolator& operator ()( const unsigned int i,
                                   const unsigned int j );

#if 0
   const LayerInterpolator& operator ()( const unsigned int nodesX,
                                         const unsigned int nodesY,
                                         const std::string& formationName ) const;

   LayerInterpolator& operator ()( const unsigned int nodesX,
                                   const unsigned int nodesY,
                                   const std::string& formationName );
#endif

   /// Set the null value for the interpolators.
   void setNullValue ( const float nullValue );

private :

   const int m_nodesX;
   const int m_nodesY;
   DepthInterpolator** m_interpolators;

};

#endif // _VoxetDomainInterpolator_H_
