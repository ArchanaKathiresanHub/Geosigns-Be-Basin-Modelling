// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "DistanceMatrix.h"

#include <math.h>

namespace SUMlib {

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix constructor
///////////////////////////////////////////////////////////////////////////////
DistanceMatrix::DistanceMatrix(const std::vector<RealVector>& dataSet) :
   m_distances(dataSet.size(), RealVector(dataSet.size()))
{
   /// Fill matrix
   for ( Index i = 0; i < dataSet.size(); i++ )
   {
      for ( Index j = i + 1; j < dataSet.size(); j++ )
      {
         double distance = calcDistance(dataSet[i], dataSet[j]);
         m_distances[i][j] = distance;
         m_distances[j][i] = distance;
      }
      m_distances[i][i] = s_maxDistance;
   }
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getNumRows
///////////////////////////////////////////////////////////////////////////////
Index DistanceMatrix::getNumRows() const
{
   return m_distances.size();
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getNumCols
///////////////////////////////////////////////////////////////////////////////
Index DistanceMatrix::getNumCols() const
{
   return m_distances.size();
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getMinOfRow
///////////////////////////////////////////////////////////////////////////////
std::pair<double, Index> DistanceMatrix::getMinOfRow(Index rowIndex) const
{
   Index minIndex = m_distances.size();
   double    minVal   = getMaxDistance();
   for ( Index colIndex = 0; colIndex < m_distances.size(); colIndex++ )
   {
      double val = (*this)(rowIndex, colIndex);
      if ( val < minVal )
      {
         minVal = val;
         minIndex = colIndex;
      }
   }
   return std::pair<double, Index>(minVal, minIndex);
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getMinOfCol
///////////////////////////////////////////////////////////////////////////////
std::pair<double, Index> DistanceMatrix::getMinOfCol(Index colIndex) const
{
   /// We use that the distance matrix is symmetric
   return getMinOfRow(colIndex);
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getMinOfRow
///////////////////////////////////////////////////////////////////////////////
std::pair<double, IndexSubset::const_iterator> DistanceMatrix::getMinOfRow(Index rowIndex, const IndexSubset& indexSubset) const
{
   std::list<Index>::const_iterator minIt  = indexSubset.end();
   double                               minVal = getMaxDistance();
   for ( std::list<Index>::const_iterator it = indexSubset.begin(); it != indexSubset.end(); it++ )
   {
      Index colIndex = *it;
      double val = (*this)(rowIndex, colIndex);
      if ( val < minVal )
      {
         minVal = val;
         minIt = it;
      }
   }
   return std::pair<double, IndexSubset::const_iterator>(minVal, minIt);
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getMinOfCol
///////////////////////////////////////////////////////////////////////////////
std::pair<double, IndexSubset::const_iterator> DistanceMatrix::getMinOfCol(Index colIndex, const IndexSubset& indexSubset) const
{
   return getMinOfRow(colIndex, indexSubset);
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix access operator
///////////////////////////////////////////////////////////////////////////////
double DistanceMatrix::operator()(Index rowIndex, Index colIndex) const
{
   return m_distances[rowIndex][colIndex];
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getDistance
///////////////////////////////////////////////////////////////////////////////
double DistanceMatrix::getDistance(Index rowIndex, Index colIndex) const
{
   return (*this)(rowIndex, colIndex);
}

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::calcDistance
///////////////////////////////////////////////////////////////////////////////
double DistanceMatrix::calcDistance(const RealVector& point1, const RealVector& point2)
{
   double distSquare = 0;
   for ( Index iCoord = 0; iCoord < point1.size(); iCoord++ )
   {
      double deltaX = point1[iCoord] - point2[iCoord];
      distSquare += deltaX*deltaX;
   }
   return sqrt(distSquare);
}

/// A random large number (this number should be bigger than sqrt(n_dimensions)
const double DistanceMatrix::s_maxDistance = 1e9;

///////////////////////////////////////////////////////////////////////////////
/// DistanceMatrix::getMaxDistance
///////////////////////////////////////////////////////////////////////////////
double DistanceMatrix::getMaxDistance()
{
   return s_maxDistance;
}

} /// end of namespace SUMlib

