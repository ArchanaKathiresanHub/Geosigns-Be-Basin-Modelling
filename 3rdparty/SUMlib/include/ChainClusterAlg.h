// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_CHAIN_CLUSTER_ALG_H
#define SUMLIB_CHAIN_CLUSTER_ALG_H

#include "SUMlib.h"
#include "BaseTypes.h"

#include <set>

namespace SUMlib {

class Cluster;

/**
 * @class ChainClusterAlg
 * @brief Run a recursive cluster search on a data set
 */
class INTERFACE_SUMLIB ChainClusterAlg
{

   /**
    * Constructor/destructor
    */
   public:
      /**
       * Constructor.
       * Creates an instance of the algorithm
       */
      ChainClusterAlg( const std::vector<RealVector>& dataSet, unsigned int numSeedPoints, double linkingStrenght );
      /**
       * Destructor.
       */
      virtual ~ChainClusterAlg();
      /**
       * Run cluster finding algorithm.
       */
      virtual void generate();
      /**
       * Retrieves the cluster objects. The clusters are owned by this class
       */
      std::vector<const Cluster*> getClusters() const;

      /**
       * Set the number points, @param numSeedPoints, to be used to obtain a density estimate for the seed.
       */
      void setNumSeedPoints(unsigned int numSeedPoints);
      /**
       * Set the linking strength (determines the size of the clusters)
       */
      void setLinkingStrength(double linkingStrength);

   private:
      /**
       * Blocked copy-constructor and assignment operator
       */
      ChainClusterAlg(const ChainClusterAlg& other);
      ChainClusterAlg& operator=(const ChainClusterAlg& other);

      /**
       * Compare size of clusters
       */
      struct ClusterCompare
      {
         bool operator()(const Cluster* cluster1, const Cluster* cluster2 );
      };

   protected:
      std::multiset<const Cluster*, ClusterCompare>   m_clusters;             //! Clusters found by the algorithm, sorted by size
      const std::vector<RealVector>&                  m_dataSet;              //! Reference to the data set
      unsigned int                                    m_numSeedPoints;        //! Number of points used to spawn the seeds
      double                                          m_linkingStrength;      //! Linking strength

};

} /// namespace SUMlib

#endif
