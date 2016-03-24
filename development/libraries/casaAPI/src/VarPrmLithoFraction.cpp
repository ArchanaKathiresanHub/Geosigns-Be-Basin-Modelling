//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmLithoFraction.h
/// @brief This file keeps API implementation for handling variation of the lithofractions for a given layer

// CASA
#include "VarPrmLithoFraction.h"

// CMB API
#include "cmbAPI.h"

// C lib
#include <cassert>
#include <cstring>
#include <cmath>

namespace casa
{
   VarPrmLithoFraction::VarPrmLithoFraction( const std::string                 & layerName
      , const std::vector<int>                                                 & lithoFractionInds
      , const std::vector<double>                                              & baseLithoFrac
      , const std::vector<double>                                              & minLithoFrac
      , const std::vector<double>                                              & maxLithoFrac
      , const std::vector<casa::VarPrmContinuous::PDF>                         & lithoFractionsPDFs
      , const std::string                                                      & name
      ) : m_layerName( layerName ), m_lithoFractionsInds( lithoFractionInds ), m_parameterPDFs( lithoFractionsPDFs )
   {
      m_name = !name.empty() ? name : std::string( "" );

      m_baseValue.reset( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, baseLithoFrac ) );
      m_minValue.reset( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, minLithoFrac ) );
      m_maxValue.reset( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, maxLithoFrac ) );

   }

   size_t VarPrmLithoFraction::dimension() const
   {
      return m_lithoFractionsInds.size();
   }

   SharedParameterPtr VarPrmLithoFraction::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
   {
      const std::vector<double> & minLithoFractions = dynamic_cast<PrmLithoFraction*>( m_minValue.get() )->asDoubleArray();
      const std::vector<double> & maxLithoFractions = dynamic_cast<PrmLithoFraction*>( m_maxValue.get() )->asDoubleArray();
      std::vector<double>  lithoFractions;

      // check for parameters value are in interval min/max
      for ( size_t i = 0; i != m_lithoFractionsInds.size(); ++i )
      {

         lithoFractions.push_back( *vals++ );

         if ( minLithoFractions[i] > lithoFractions.back() || lithoFractions.back() > maxLithoFractions[i]  )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of the lithofraction parameter " << lithoFractions.back() <<
               " is out of range: [" << minLithoFractions[i] << ":" << maxLithoFractions[i] << "]";
         }
      }

      SharedParameterPtr prm( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, lithoFractions ) );
      return prm;
   }

   std::vector<std::string> VarPrmLithoFraction::name() const
   {
      std::vector<std::string> ret;
      ret.push_back( "LithoFraction( " + m_layerName + ", Percent" + std::to_string( m_lithoFractionsInds[1] ) + ") [rest ratio]" );
      return ret;
   }

   bool VarPrmLithoFraction::save( CasaSerializer & sz, unsigned int version ) const
   {
      // save base class data
      bool ok = VarPrmContinuous::save( sz, version );

      // cast m_parameterPDFs to a vector of integers
      std::vector<int> parameterPDFs( m_parameterPDFs.size() );
      for ( size_t i = 0; i != m_parameterPDFs.size(); ++i )
      {
         parameterPDFs.push_back( static_cast<int>(m_parameterPDFs[i] ));
      }

      ok = ok ? sz.save( m_layerName, "LayerName" ) : ok;
      ok = ok ? sz.save( m_lithoFractionsInds, "LithoFractionsInds" ) : ok;
      ok = ok ? sz.save( parameterPDFs, "parameterPDFs" ) : ok;

      return ok;
   }

   // Constructor from input stream
   VarPrmLithoFraction::VarPrmLithoFraction( CasaDeserializer & dz, unsigned int objVer )
   {

      bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

      std::vector<int> parameterPDFs( m_parameterPDFs.size() );

      ok = ok ? dz.load( m_layerName, "LayerName" ) : ok;
      ok = ok ? dz.load( m_lithoFractionsInds, "LithoFractionsInds" ) : ok;
      ok = ok ? dz.load( parameterPDFs, "parameterPDFs" ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmLithoFraction deserialization unknown error";
      }

      // cast parameterPDFs to a vector of casa::VarPrmContinuous::PDF
      for ( size_t i = 0; i != parameterPDFs.size( ); ++i )
      {
         m_parameterPDFs.push_back( static_cast<casa::VarPrmContinuous::PDF>( parameterPDFs[i] ) );
      }

   }

} // namespace casa

