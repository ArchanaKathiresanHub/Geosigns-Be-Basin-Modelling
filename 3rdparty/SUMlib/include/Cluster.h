// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_CLUSTER_H
#define SUMLIB_CLUSTER_H

#include "SUMlib.h"
#include "BaseTypes.h"

namespace SUMlib {

/**
  * @class Cluster
  * @brief A collection of points that represents a cluster.
  */
class INTERFACE_SUMLIB Cluster
{
   public:
      virtual ~Cluster() {}

      /**
       * Returns the number of points in the cluster
       */
      virtual unsigned int getNumPoints() const = 0;
      /**
       * Returns point with cluster index. @param memberIndex is the cluster member index and runs from 0 to getNumPoints()
       */
      virtual const RealVector& getPoint(Index memberIndex) const = 0;
      /**
       * Obtain a list of indices in the data set that belong to this cluster
       */
      virtual const IndexList& getContent() const = 0;

      /**
       * Returns the centre of mass of the cluster
       */
      virtual const RealVector& getCentreOfMass() const = 0;
      /**
       * Returns the index to the point in the data set that is closest to the centre-of-mass
       */
      virtual Index getMedoidIndex() const = 0;
};

} /// namespace SUMlib

#endif
