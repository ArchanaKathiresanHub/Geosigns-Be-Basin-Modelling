// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_KRIGINGDATA_H
#define SUMLIB_KRIGINGDATA_H

#include "BaseTypes.h"
#include "SUMlib.h"

#include "ISerializer.h"

namespace SUMlib {

class INTERFACE_SUMLIB KrigingData : public ISerializable
{
   public:

      KrigingData();
      KrigingData( ParameterSet const& parSet, unsigned int nbOfOrdPars );
      virtual ~KrigingData();

      /// Initialises all internal data structures from parSet.
      /// @param [in] parSet       the scaled parameter vectors
      /// @param [in] nbOfOrdPars  the number of prepared ordinal parameters
      void initialise( ParameterSet const& parSet, unsigned int nbOfOrdPars );

      /// Calculates a single covariance as a function of distance and correlation length.
      /// Returns 1 if distance = 0, else 0 <= return value < 1.
      /// @param [in] distance   a distance between 2 points (i.e. between 2 parameters)
      /// @param [in] corLength  correlation length (local or global)
      /// @returns the covariance
      double calcCovariance( double distance, double corLength ) const;

      double               globalCorLength() const { return m_globalCorLength; }
      double               localCorLength() const { return m_localCorLength; }
      unsigned int         sizeOrd() const { return m_nbOfOrdPars; }
      RealMatrix const&    distances() const { return m_distances; }
      double               minDistance() const { return m_minDistance; }
      unsigned int         refIndex() const { return m_refIndex; }
      RealMatrix const&    globalInvCov() const { return m_globalInvCov; }
      RealMatrix const&    localInvCov() const { return m_localInvCov; }

      bool empty() const;

      // made deliberately private so that calling load/save directly on this class is more difficult
      // the preferred way is to call save/load on the ISerializer.
   private:
      // ISerializable
      virtual bool load( IDeserializer*, unsigned int version );
      virtual bool save( ISerializer*, unsigned int version ) const;


   private:

      /// global correlation length beyond which no correlation exists between 2 points
      double   m_globalCorLength;

      /// local correlation length beyond which no correlation exists between 2 points
      double   m_localCorLength;

      /// number of prepared ordinal parameters
      unsigned int   m_nbOfOrdPars;

      /// matrix of distances between base points (from parSet)
      RealMatrix  m_distances;

      /// minimum of all positive distances between base points
      double m_minDistance;

      /// index that refers to a suitable reference base point (i.e. to parSet[m_refIndex])
      unsigned int   m_refIndex;

      /// (pseudo) inverse of global covariance matrix
      RealMatrix  m_globalInvCov;

      /// (pseudo) inverse of local covariance matrix
      RealMatrix  m_localInvCov;

      /// Defines the global and local correlation length.
      /// @param [in] dim  the dimension of the parameter space
      void defineCorrelationLengths( unsigned int dim );

      /// Calculates the matrix m_distances of size parSet.size() by parSet.size().
      /// m_distances[i][j] is the distance between parSet[i] and parSet[j].
      /// It also defines m_refIndex that refers to parSet[m_refIndex].
      /// @param [in] parSet  the parameter vectors
      void calcDistances( ParameterSet const& parSet );

      /// Calculates the inverses of the global and local covariance matrices.
      /// A covariance matrix cov is of size parSet.size() by parSet.size().
      /// cov[i][j] is a function of the distance between parSet[i] and parSet[j].
      /// cov[i][j] = 1 if i = j, else 0 <= cov[i][j] < 1.
      void calcInvCov();
};

} // namespace SUMlib

#endif // SUMLIB_KRIGINGDATA_H
