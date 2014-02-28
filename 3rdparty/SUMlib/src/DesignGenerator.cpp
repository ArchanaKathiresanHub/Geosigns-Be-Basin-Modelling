// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "DataStructureUtils.h"
#include "DesignGenerator.h"
#include "Exception.h"
#include "FactDesign.h"
#include "SUMlib.h"

namespace SUMlib {

DesignGenerator::DesignGenerator( )
   : m_nbPar( 0 ), m_nbRuns( 0 ), m_resolution( 0 )
{
   check();
}

DesignGenerator::DesignGenerator( unsigned int nbPar, unsigned int nbRuns, unsigned int resolution )
   : m_nbPar( nbPar ), m_nbRuns( nbRuns ), m_resolution( resolution )
{
   check();
}

DesignGenerator::DesignGenerator( unsigned int nbPar, unsigned int nbRuns, unsigned int resolution, intMatrix const& columns )
: m_nbPar( nbPar ), m_nbRuns( nbRuns ), m_resolution( resolution ), m_columns( columns )
{
   check();
}

DesignGenerator::DesignGenerator( DesignGenerator const& dg )
: m_nbPar( dg.m_nbPar ), m_nbRuns( dg.m_nbRuns ), m_resolution( dg.m_resolution ), m_columns( dg.m_columns )
{
   check();
}

DesignGenerator& DesignGenerator::operator=( DesignGenerator const& dg )
{
   if ( this != &dg )
   {
      m_nbPar = dg.m_nbPar;
      m_nbRuns = dg.m_nbRuns;
      m_resolution = dg.m_resolution;
      m_columns = dg.m_columns;
      check();
   }
   return *this;
}

DesignGenerator::~DesignGenerator()
{
}

void DesignGenerator::check() const
{
   if ( m_resolution > FactDesign::FullFactorialResolution )
   {
      THROW2 ( ValueOutOfBounds, "resolution larger than FullFactorialResolution" );
   }
}

void DesignGenerator::set( intMatrix gen  )
{
   m_columns = gen;
}

void DesignGenerator::add( intVector gen )
{
   m_columns.push_back( gen );
}

bool DesignGenerator::operator==( DesignGenerator const& rhs ) const
{
   bool result = ( m_nbPar == rhs.m_nbPar ) && ( m_nbRuns == rhs.m_nbRuns ) && ( m_resolution == rhs.m_resolution );
   return result;
}

bool DesignGenerator::operator!=( DesignGenerator const& rhs ) const
{
   return ! ( *this == rhs );
}

bool DesignGenerator::operator<( DesignGenerator const& rhs ) const
{
   bool result;
   if ( m_nbPar != rhs.m_nbPar )
   {
      result = ( m_nbPar < rhs.m_nbPar );
   }
   else if ( m_nbRuns != rhs.m_nbRuns )
   {
      result = ( m_nbRuns < rhs.m_nbRuns );
   }
   else
   {
      result = ( m_resolution < rhs.m_resolution );
   }
   return result;
}


bool DesignGenerator::operator>=( DesignGenerator const& rhs ) const
{
   return ! ( *this < rhs );
}

bool DesignGenerator::operator>( DesignGenerator const& rhs ) const
{
   return (*this != rhs ) && ! (*this < rhs );
}

bool DesignGenerator::operator<=( DesignGenerator const& rhs ) const
{
   return ( *this < rhs ) || ( *this == rhs );
}

void DesignGeneratorMap(
      std::vector<DesignGenerator> const& dgvec,
      std::vector<unsigned int>& ids,
      std::vector<unsigned int>& nbRuns,
      std::vector<unsigned int>& resolution )
{
   ids.clear();
   nbRuns.clear();
   resolution.clear();

   std::vector<DesignGenerator>::const_iterator dgit;
   for ( dgit = dgvec.begin(); dgit != dgvec.end(); ++dgit )
   {
      ids.push_back( dgit->numParameters() );
      nbRuns.push_back( dgit->numRuns() );
      resolution.push_back( dgit->resolution() );
   }
}

} // namespace SUMlib
