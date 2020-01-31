//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
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
      , casa::VarPrmContinuous::PDF                                              pdfType
      , const std::string                                                      & name
      ) : m_layerName( layerName ), m_lithoFractionsInds( lithoFractionInds )
   {
      m_name = !name.empty() ? name : std::string( "" );
      m_pdf = pdfType ;
      m_baseValue.reset( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, baseLithoFrac ) );
      m_minValue.reset(  new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, minLithoFrac ) );
      m_maxValue.reset(  new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, maxLithoFrac ) );
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

   SharedParameterPtr VarPrmLithoFraction::newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const
   {
      SharedParameterPtr prm( 0 );

      if ( vin.empty() )
      {
         prm.reset( new PrmLithoFraction( mdl, m_layerName, m_lithoFractionsInds ) );
      }
      else if ( vin.size() == 2 )
      {
         std::vector<size_t> ij( 2, static_cast<size_t>( floor( vin[ 0 ] + 0.5 ) ) );
         ij[1] = static_cast<size_t>( floor( vin[1] + 0.5 ) );
         prm.reset( new PrmLithoFraction( mdl, m_layerName, m_lithoFractionsInds, ij ) );
      }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for PrmLithofraction constructor";
      }

      prm->setParent( this );
      return prm;
   }

   SharedParameterPtr VarPrmLithoFraction::makeThreeDFromOneD( mbapi::Model                          & mdl
                                                             , const std::vector<double>             & xin
                                                             , const std::vector<double>             & yin
                                                             , const std::vector<SharedParameterPtr> & prmVec
                                                             , const InterpolationParams             & interpolationParams
                                                             ) const
   {
      // get the lithofractions calculate the lithopercentages
      std::vector<double> lf1;
      std::vector<double> lf2;
      std::vector<double> lf3;

      for ( size_t i = 0; i != prmVec.size(); ++i )
      {
         const std::vector<double> lithoFractions = prmVec[i].get()->asDoubleArray();
         const std::vector<double> lithoPercentages = PrmLithoFraction::createLithoPercentages( lithoFractions, m_lithoFractionsInds );
         if ( lithoPercentages.size() != 3 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The number of lithopercentages is incorrect: " << lithoPercentages.size();
         }
         lf1.push_back( lithoPercentages[0] );
         lf2.push_back( lithoPercentages[1] );
         lf3.push_back( lithoPercentages[2] );
      }

      // interpolate
      std::vector<double> xout;
      std::vector<double> yout;
      std::vector<double> rpInt;
      std::vector<double> r13Int;

      ErrorHandler::ReturnCode returnCode = ErrorHandler::UnknownError;
      if ( interpolationParams.interpolationMethod == 1 )
      {
         returnCode = mdl.interpolateLithoFractionsNN( xin, yin, lf1, lf2, lf3, xout, yout, rpInt, r13Int );
      }
      else
      {
         returnCode = mdl.interpolateLithoFractionsIDW( interpolationParams.IDWpower, xin, yin, lf1, lf2, lf3, xout, yout, rpInt, r13Int );
      }

      if ( ErrorHandler::NoError != returnCode )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The interpolation of the lithopercentages failed for the layer : " << m_layerName;
      }

      if ( ErrorHandler::NoError != mdl.smoothenVector( rpInt, interpolationParams.smoothingMethod, interpolationParams.radius, interpolationParams.nrOfThreads ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The smoothing of the rp intermediate state failed for the layer : " << m_layerName;
      }

      if ( ErrorHandler::NoError != mdl.smoothenVector( r13Int, interpolationParams.smoothingMethod, interpolationParams.radius, interpolationParams.nrOfThreads ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The smoothing of the r13 intermediate state failed for the layer : " << m_layerName;
      }

      // back transform the rpInt and r13Int
      std::vector<double> lf1CorrInt;
      std::vector<double> lf2CorrInt;
      std::vector<double> lf3CorrInt;

      if ( ErrorHandler::NoError != mdl.backTransformLithoFractions( rpInt, r13Int, lf1CorrInt, lf2CorrInt, lf3CorrInt ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The back transformation of the lithopercentages failed for the layer : "
                                                                     << m_layerName;
      }

      // get the maps manager
      mbapi::MapsManager & mapsMgr = mdl.mapsManager();

      // get the stratigraphy manager
      mbapi::StratigraphyManager & strMgr = mdl.stratigraphyManager();

      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = strMgr.layerID( m_layerName );
      if ( strMgr.errorCode() != ErrorHandler::NoError )
      {
         throw ErrorHandler::Exception( strMgr.errorCode() ) << strMgr.errorMessage();
      }

      // generate the maps
      std::string  mapNameFirstLithoPercentage  = std::to_string( lid ) + "_percent_1";
      std::string  mapNameSecondLithoPercentage = std::to_string( lid ) + "_percent_2";
      size_t mapSeqNbr = Utilities::Numerical::NoDataIDValue;

      mbapi::MapsManager::MapID id = mapsMgr.generateMap( "StratIoTbl", mapNameFirstLithoPercentage, lf1CorrInt, mapSeqNbr,"" ); // use the default filename for the file storing the maps
      if ( IsValueUndefined( id ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Generation of the " << mapNameFirstLithoPercentage
                                                                        << " lithofraction map failed";
      }
      id = mapsMgr.generateMap( "StratIoTbl", mapNameSecondLithoPercentage, lf2CorrInt, mapSeqNbr,"" );
      if ( IsValueUndefined( id ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Generation of the " << mapNameSecondLithoPercentage
                                                                        << " lithofraction map failed";
      }

      strMgr.setLayerLithologiesPercentageMaps( lid, mapNameFirstLithoPercentage, mapNameSecondLithoPercentage);

      // both maps are provided, no scalar values is needed
      return SharedParameterPtr( new PrmLithoFraction( this, m_name, m_layerName, m_lithoFractionsInds, std::vector<double>(),
                                                       mapNameFirstLithoPercentage, mapNameSecondLithoPercentage ) );
   }

   std::vector<std::string> VarPrmLithoFraction::name() const
   {
      std::vector<std::string> ret;
      if ( m_name.empty() )
      {
         ret.push_back( "LithoFraction( " + m_layerName + ", Percent" + std::to_string( m_lithoFractionsInds[1] ) + ") [rest ratio]" );
      }
      else { ret.push_back( m_name ); }

      return ret;
   }

   bool VarPrmLithoFraction::save( CasaSerializer & sz ) const
   {
      // save base class data
      bool ok = VarPrmContinuous::save( sz );

      ok = ok ? sz.save( m_layerName, "LayerName" ) : ok;
      ok = ok ? sz.save( m_lithoFractionsInds, "LithoFractionsInds" ) : ok;

      return ok;
   }

   // Constructor from input stream
   VarPrmLithoFraction::VarPrmLithoFraction( CasaDeserializer & dz, unsigned int objVer )
   {
      bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

      ok = ok ? dz.load( m_layerName, "LayerName" ) : ok;
      ok = ok ? dz.load( m_lithoFractionsInds, "LithoFractionsInds" ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmLithoFraction deserialization unknown error";
      }
   }

} // namespace casa

