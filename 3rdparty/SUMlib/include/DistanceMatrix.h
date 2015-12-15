// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_DISTANCE_MATRIX_H
#define SUMLIB_DISTANCE_MATRIX_H

#include "BaseTypes.h"

namespace SUMlib {

/**
 * @class DistanceMatrix
 * @brief A symmetric matrix filled with the (euclidian) distances between the points in a dataset. This class is a helper
 * class for constructing clusters. @see ChainClusterAlg.
 */
class DistanceMatrix
{
   public:
      /**
       * Constructor. Calculates the distances between every point of dataSet and stores them in a symmetric matrix.
       * @param [in] dataSet: the data set for which the distances are calculated
       */
      DistanceMatrix(const std::vector<RealVector>& dataSet);

      /**
       * Destructor
       */
      virtual ~DistanceMatrix() {}

      /**
       * Get distance between points. This function is symmetric in its input arguments. If rowIndex is equal to colIndex
       * the static value s_maxDistance is returned.
       * @param [in] rowIndex: the index of the first point
       * @param [in] colIndex: the index of the second point
       */
      double getDistance(Index rowIndex, Index colIndex) const;
      /**
       * Get the number of rows in the matrix.
       */
      Index getNumRows() const;
      /**
       * Get the number of cols in the matrix.
       */
      Index getNumCols() const;

      /**
       * Find the minimum value in a row. Returns a pair of a minimum value and the column index.
       * @param [in] rowIndex: the index of the row
       */
      std::pair<double, Index> getMinOfRow(Index rowIndex) const;
      /**
       * Find the minimum value in a column. Returns a pair of a miniumum value and the row index.
       * @param [in] colIndex: the index of the column
       */
      std::pair<double, Index> getMinOfCol(Index colIndex) const;

      /**
       * Find the minimum value in a row only considering the indices contained in an IndexSubset. Returns the minimum value
       * and the iterator to minimum element from the IndexSubset.
       * @param [in] rowIndex   : the index of the row
       * @param [in] indexSubset: the subset of column indices
       */
      std::pair<double, IndexSubset::const_iterator> getMinOfRow(Index rowIndex, const IndexSubset& indexSubset) const;
      /**
       * Find the minimum value in a column only considering the indices contained in an IndexSubset. Returns the minimum value
       * and the iterator to minimum element from the IndexSubset.
       * @param [in] colIndex   : the index of the column
       * @param [in] indexSubset: the subset of row indices
       */
      std::pair<double, IndexSubset::const_iterator> getMinOfCol(Index colIndex, const IndexSubset& indexSubset) const;

      /**
       * Element access operator. See description of getDistance()
       */
      double operator()(Index rowIndex, Index colIndex) const;

      /**
       * Returns the maximum distance in the matrix, which is a static constant.
       */
      static double getMaxDistance();

      /**
       * Calculate the distance between points @param point1 and @param point2.
       */
      static double calcDistance(const RealVector& point1, const RealVector& point2);

   private:
      RealMatrix                        m_distances;         //! the matrix
      static const double               s_maxDistance;       //! maximum distance

};

} /// namespace SUMlib

#endif
