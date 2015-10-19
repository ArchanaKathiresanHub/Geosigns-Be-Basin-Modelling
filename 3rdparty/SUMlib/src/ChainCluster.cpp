// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "ChainCluster.h"

#include <iostream>
#include <math.h>
#include <cassert>
#include <set>

namespace SUMlib {

///////////////////////////////////////////////////////////////////////////////
/// Cluster constructor
///////////////////////////////////////////////////////////////////////////////
ChainCluster::ChainCluster( Index seedIndex, IndexSubset& freePoints, double searchRadius, double densityCutoff,
                  const DistanceMatrix& distanceMatrix, const std::vector<RealVector>& dataSet ) :
   m_freePoints(freePoints),
   m_clusterContent(0),
   m_searchRadius(searchRadius),
   m_searchRadiusSqr(searchRadius*searchRadius),
   m_dataSet(dataSet),
   m_densityCutoff(densityCutoff),
   m_medoidIndex(),
   m_centreOfMass()
{
   localSearch(seedIndex, 1.0, distanceMatrix);
   calcCentreOfMass();
   findMedoid();
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster destructor
///////////////////////////////////////////////////////////////////////////////
ChainCluster::~ChainCluster()
{
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::localSearch
///////////////////////////////////////////////////////////////////////////////
void ChainCluster::localSearch(Index localSeed, double localDensity, const DistanceMatrix& distanceMatrix)
{
   /// Add local seed to the cluster content
   m_clusterContent.push_back(localSeed);

   /// Create empty IndexSubset and density vector for new neighbours
   IndexSubset newContent;
   std::vector<double> newContentDens;
   int nPointsAdded = 0;

   /// Loop over all unassigned points
   IndexSubset::iterator it = m_freePoints.begin();
   while ( it != m_freePoints.end() )
   {
      /// Get index to the candidate neighbour
      Index candidateIndex = *it;
      /// Remove only localSeed from freePoints if neighbour is the localSeed itself
      if ( localSeed == candidateIndex )
      {
         it = m_freePoints.erase(it);
      }
      else
      {
         /// Get distance to neighbour
         double dist = distanceMatrix(localSeed, candidateIndex);
         double newDensity = localDensity*calcDensity(dist);

         /// Case 1: distance smaller than search radius (reward linking)
         if ( dist < m_searchRadius && newDensity > m_densityCutoff )
         {
            /// Add neighbour
            newContent.push_back(candidateIndex);
            it = m_freePoints.erase(it);
            nPointsAdded++;
            /// Reward current point because distance smaller than searchRadius
            newDensity = (1 + newDensity)/2;
            newContentDens.push_back(newDensity);
         }

         /// Case 2: distance outside search radius (penalise linking)
         else if ( newDensity > m_densityCutoff )
         {
            /// Add neighbour
            newContent.push_back(candidateIndex);
            it = m_freePoints.erase(it);
            nPointsAdded++;
            newContentDens.push_back(newDensity);
         }

         /// Case 3: point is too far away
         else
         {
            it++;
         }
      }
   }

   /// Loop over neighbours and call a recursive local search
   std::vector<double>::const_iterator densIt = newContentDens.begin();
   for ( it = newContent.begin(); it != newContent.end(); it++, densIt++ )
   {
      localSearch( *it, *densIt, distanceMatrix );
   }

}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::calcDensity
///////////////////////////////////////////////////////////////////////////////
double ChainCluster::calcDensity(double distance) const
{
   /// Density based on Gaussian probability function
   return exp(-distance*distance/m_searchRadiusSqr);
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::calcCentreOfMass
///////////////////////////////////////////////////////////////////////////////
void ChainCluster::calcCentreOfMass()
{
   unsigned int nDimensions = static_cast<unsigned int>( m_dataSet[0].size() );
   for ( Index iCoord = 0; iCoord < nDimensions; iCoord++ )
   {
      double xCm = 0;
      for ( IndexList::const_iterator it = m_clusterContent.begin(); it != m_clusterContent.end(); it++ )
      {
         xCm += m_dataSet[*it][iCoord];
      }
      xCm /= getNumPoints();
      m_centreOfMass.push_back(xCm);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Cluster::findMedoid
////////////////////////////////////////////////////////////////////////////////
void ChainCluster::findMedoid()
{
   m_medoidIndex = 0;
   RealVector centreOfMass = getCentreOfMass();
   double minDistToCentreOfMass = DistanceMatrix::getMaxDistance();
   for ( Index i = 0; i < m_clusterContent.size(); ++i )
   {
      double distToCentreOfMass = DistanceMatrix::calcDistance( centreOfMass, getPoint(i) );
      if ( distToCentreOfMass < minDistToCentreOfMass )
      {
         minDistToCentreOfMass = distToCentreOfMass;
         m_medoidIndex = m_clusterContent[i];
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::getContent
///////////////////////////////////////////////////////////////////////////////
const IndexList& ChainCluster::getContent() const
{
   return m_clusterContent;
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::getNumPoints
///////////////////////////////////////////////////////////////////////////////
unsigned int ChainCluster::getNumPoints() const
{
   return static_cast<unsigned int>( m_clusterContent.size() );
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::getPoint
///////////////////////////////////////////////////////////////////////////////
const RealVector& ChainCluster::getPoint(Index memberIndex) const
{
   return m_dataSet[m_clusterContent[memberIndex]];
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::getCentreOfMass
///////////////////////////////////////////////////////////////////////////////
const RealVector& ChainCluster::getCentreOfMass() const
{
   return m_centreOfMass;
}

///////////////////////////////////////////////////////////////////////////////
/// Cluster::getMedoidIndex
///////////////////////////////////////////////////////////////////////////////
Index ChainCluster::getMedoidIndex() const
{
   return m_medoidIndex;
}

} /// namespace SUMlib

