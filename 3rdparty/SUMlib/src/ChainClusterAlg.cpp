// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "ChainClusterAlg.h"

#include <sstream>
#include <math.h>
#include <cassert>

#include "ChainCluster.h"
#include "DataStructureUtils.h"
#include "DistanceMatrix.h"
#include "SUMlib.h"

namespace SUMlib {

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg constructor
///////////////////////////////////////////////////////////////////////////////
ChainClusterAlg::ChainClusterAlg( const std::vector<SUMlib::RealVector>& dataSet, unsigned int numSeedPoints, double linkingStrength ) :
   m_dataSet(dataSet),
   m_numSeedPoints(numSeedPoints),
   m_linkingStrength(linkingStrength)
{
   generate();
}

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg destructor
///////////////////////////////////////////////////////////////////////////////
ChainClusterAlg::~ChainClusterAlg()
{
   /// Clean up dynamically created clusters
   for ( std::multiset< const Cluster*, ClusterCompare >::iterator it = m_clusters.begin(); it != m_clusters.end(); ++it )
   {
      delete *it;
   }
}

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg generate
///////////////////////////////////////////////////////////////////////////////
void ChainClusterAlg::generate()
{
   /// Define parameters
   const unsigned int nMaxCluster = static_cast<unsigned int>( m_dataSet.size() ) / 2;
   const unsigned int minPointsPerCluster = m_numSeedPoints;
   const double linkDensityCutoff = exp(-m_linkingStrength);

   /// Compute distances between all points
   DistanceMatrix distanceMatrix(m_dataSet);

   /// Initialize an array of indices to points that are not yet assigned to a cluster
   IndexSubset freePoints;
   for ( SUMlib::Index index = 0; index < m_dataSet.size(); ++index ) {
      freePoints.push_back(index);
   }

   /// Create default seed matrix column
   double* initSeedCol = new double[minPointsPerCluster];
   for ( unsigned int i = 0; i < minPointsPerCluster; ++i )
   {
      initSeedCol[i] = DistanceMatrix::getMaxDistance();
   }
   std::multiset<double> nnDistances_init(initSeedCol, initSeedCol + minPointsPerCluster);

   /// Cluster finding algorithm
   while ( m_clusters.size() < nMaxCluster && freePoints.size() > minPointsPerCluster )
   {

      /// Initialize with an invalid seedIndex
      Index seedIndex = static_cast<unsigned int>( m_dataSet.size() );
      double    seedAvgNnDist = DistanceMatrix::getMaxDistance();

      /// The searchRadius is defined as the maximum distance in the list of 'minPointsPerCluster' nearest neighbour distances
      double    seedSearchRadius = -1;

      /// Seed finding
      for ( IndexSubset::const_iterator itRow = freePoints.begin(); itRow != freePoints.end(); ++itRow )
      {
         std::multiset<double> nnDistances = std::multiset<double>(nnDistances_init);
         for ( IndexSubset::const_iterator itCol = freePoints.begin(); itCol != freePoints.end(); ++itCol )
         {
            double dist = distanceMatrix(*itRow, *itCol);
            double maxDist = *(--nnDistances.end());
            if ( dist < maxDist )
            {
               nnDistances.erase( (--nnDistances.end()) );
               nnDistances.insert(dist);
            }
         }
         /// Calculate average of closest neighbours
         double avgNnDist = 0;
         for ( std::multiset<double>::const_iterator it = nnDistances.begin(); it !=nnDistances.end(); ++it )
         {
            avgNnDist += *it;
         }
         avgNnDist /= minPointsPerCluster;
         if ( avgNnDist < seedAvgNnDist )
         {
            seedIndex = *itRow;
            seedSearchRadius = *(--nnDistances.end());
            seedAvgNnDist = avgNnDist;
         }
      }

      /// Check if seed finding succeeded
      assert( seedIndex != m_dataSet.size() );

      /// Build cluster
      ChainCluster* cluster = new ChainCluster( seedIndex, freePoints, seedSearchRadius, linkDensityCutoff, distanceMatrix, m_dataSet );
      m_clusters.insert( cluster );
   }

   /// Cleanup
   delete[] initSeedCol;

}

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg getClusters
///////////////////////////////////////////////////////////////////////////////
std::vector<const Cluster*> ChainClusterAlg::getClusters() const
{
   std::vector<const Cluster*> clusVec;

   for ( std::multiset< const Cluster*, ClusterCompare >::const_reverse_iterator rit = m_clusters.rbegin();
         rit != m_clusters.rend(); ++rit )
   {
      clusVec.push_back( *rit );
   }
   return clusVec;
}

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg setNumSeedPoints
///////////////////////////////////////////////////////////////////////////////
void ChainClusterAlg::setNumSeedPoints( unsigned int numSeedPoints )
{
   m_numSeedPoints = numSeedPoints;
}

///////////////////////////////////////////////////////////////////////////////
/// ChainClusterAlg setLinkingStrength
///////////////////////////////////////////////////////////////////////////////
void ChainClusterAlg::setLinkingStrength( double linkingStrength )
{
   m_linkingStrength = linkingStrength;
}

bool ChainClusterAlg::ClusterCompare::operator()( const Cluster* cluster1, const Cluster* cluster2 ) const
{
   return ( cluster1->getNumPoints() < cluster2->getNumPoints() );
}

} /// namespace SUMlib
