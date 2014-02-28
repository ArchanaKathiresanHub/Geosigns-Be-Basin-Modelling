// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_CHAIN_CLUSTER_H
#define SUMLIB_CHAIN_CLUSTER_H

#include "Cluster.h"
#include "DistanceMatrix.h"

namespace SUMlib {

class ChainCluster : public Cluster
{
   public:
      /**
       * Create cluster by density linking
       * @param seedIndex: the index of the seed point initiating the cluster
       * @param freePoints: a list of indices that are available for building the cluster. WARNING: This list
       *    will be updated, removing the points that are associated to the current cluster.
       * @param searchRadius: the typical distance for looking for new neighbours
       * @param densityCutoff: quantity related to the linking depth
       * @param distanceMatrix: a matrix containing the distances between the points in @param dataSet
       * @param dataSet: reference to the actual data set. The cluster object assumes that the data set is left untouched
       */
      ChainCluster( Index seedIndex, IndexSubset& freePoints, double searchRadius, double densityCutoff,
               const DistanceMatrix& distanceMatrix, const std::vector<RealVector>& dataSet );
      /**
       * Destructor
       */
      virtual ~ChainCluster();

      /**
       * For documentation of these public methods @see Cluster base class.
       */
      const IndexList& getContent() const;
      const RealVector& getCentreOfMass() const;
      Index getMedoidIndex() const;
      unsigned int getNumPoints() const;
      const RealVector& getPoint(Index memberIndex) const;

   private:
      /**
       * Recursive cluster builder function. Called by constructor
       * @param localSeed: the index of the seed
       * @param probability: the current probability/cost
       * @param recursionDepth: counts the number of recursive calls
       */
      void localSearch(Index localSeed, double localDensity, const DistanceMatrix& distanceMatrix);
      /**
       * Calculate current probability
       */
      double calcDensity(double distance) const;

      /**
       * Calculate the centre of mass, is called during construction
       */
      void calcCentreOfMass();
      /**
       * Find the index to the medoid, is called during construction
       */
      void findMedoid();

   private:
      IndexSubset&                     m_freePoints;        //! Reference to the freePoints array
      IndexList                        m_clusterContent;    //! Array of indices pointing to the data set entries in this cluster
      const double                     m_searchRadius;      //! Search radius (used in cost function evaluation)
      const double                     m_searchRadiusSqr;   //! m_searchRadius*m_searchRadius (cached for optimisation)
      const std::vector<RealVector>&   m_dataSet;           //! Reference to data set
      const double                     m_densityCutoff;     //! Density Cut-off for cluster association
      Index                            m_medoidIndex;       //! Index to the medoid
      RealVector                       m_centreOfMass;      //! Centre of mass
};

} /// namespace SUMlib

#endif
