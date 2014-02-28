// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_DESIGNGENERATOR_H
#define SUMLIB_DESIGNGENERATOR_H

#include <vector>

#include "SUMlib.h"

namespace SUMlib {

   /**
    * Holds data for generating a factorial design
    */
class INTERFACE_SUMLIB_DEBUG DesignGenerator
{
   public:
      /**
       * Default constructor
       */
      DesignGenerator( );
      /**
       * Constructor for a specific factorial design
       * @param [in] nbPar Number of parameters
       * @param [in] nbRuns Number of runs
       * @param [in] resolution Resolution of the design
       */
      DesignGenerator( unsigned int nbPar, unsigned int nbRuns, unsigned int resolution );
      /**
       * Constructor for a specific factorial design including the design data
       * @param [in] nbPar Number of parameters
       * @param [in] nbRuns Number of runs
       * @param [in] resolution Resolution of the design
       * @param [in] columns aliasing data
       */
      DesignGenerator( unsigned int nbPar, unsigned int nbRuns, unsigned int resolution, std::vector< std::vector< int > > const& columns );

      /**
       * Copy constructor
       * @param [in] dg Design generator instance to copy data from
       */
      DesignGenerator( DesignGenerator const& dg );
      /**
       * Copy assignment
       * @param [in] dg Design generator instance to copy data from
       */
      DesignGenerator& operator=( DesignGenerator const& dg );

      /**
       * Destructor
       */
      virtual ~DesignGenerator();

      /**
       * Set the aliasing data
       * @param [in] gen a vector of vectors containing sign, source
       *                 columns, target column
       */
      void set( std::vector< std::vector< int > > gen  );

      /**
       * Add a vector of aliasing data
       * @param [in] gen a vector containing sign, source columns, target column
       */
      void add( std::vector< int > gen );

      /**
       * Get the number of parameters
       * @return the number of parameters
       */
      inline unsigned int numParameters() const;
      /**
       * Get the number of runs
       * @return the number of runs
       */
      inline unsigned int numRuns() const;
      /**
       * Get the resolution
       * @return the resolution
       */
      inline unsigned int resolution() const;
      /**
       * Get the aliasing data
       * @return a vector of vectors containing sign, source columns, target
       *         column
       */
      inline std::vector< std::vector< int > > data() const;

      /**
       * @brief Boolean comparison operators use nr. of parameters, nr. of
       * runs and resolution.
       */
      bool operator ==( DesignGenerator const& rhs ) const;
      bool operator !=( DesignGenerator const& rhs ) const;
      bool operator <( DesignGenerator const& rhs ) const;
      bool operator <=( DesignGenerator const& rhs ) const;
      bool operator >=( DesignGenerator const& rhs ) const;
      bool operator >( DesignGenerator const& rhs ) const;


   private:
      void check() const;

      unsigned int m_nbPar;
      unsigned int m_nbRuns;
      unsigned int m_resolution;

      std::vector< std::vector< int > > m_columns;
};

/**
 * Maps the design generator data to vectors
 * @param [in] dgvec          the design generators
 * @param [out] factDesignIds the ids of the factorial designs
 * @param [out] nbRuns        the number of runs in the designs
 * @param [out] resolution    the resolution of the designs
 */

INTERFACE_SUMLIB_DEBUG void DesignGeneratorMap(
      std::vector<DesignGenerator> const& dgvec,
      std::vector<unsigned int>& ids,
      std::vector<unsigned int>& nbRuns,
      std::vector<unsigned int>& resolution );


///
/// Implementation of inline methods
///
unsigned int DesignGenerator::numParameters() const
{
   return m_nbPar;
}

unsigned int DesignGenerator::numRuns() const
{
   return m_nbRuns;
}

unsigned int DesignGenerator::resolution() const
{
   return m_resolution;
}

std::vector< std::vector< int > > DesignGenerator::data() const
{
   return m_columns;
}

} // namespace SUMlib

#endif // SUMLIB_DESIGNGENERATOR_H
