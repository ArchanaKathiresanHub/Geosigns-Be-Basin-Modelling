//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmPorosityModel.h
/// @brief This file keeps API implementation for handling variation of porosity model parameters for the given lithology

// CASA
#include "VarPrmPorosityModel.h"

// CMB API
#include "cmbAPI.h"

// C lib
#include <cassert>
#include <cstring>
#include <cmath>

namespace casa
{

VarPrmPorosityModel::VarPrmPorosityModel( const char * lithoName
                                        , PrmPorosityModel::PorosityModelType mdlType 
                                        , double                              baseSurfPor    
                                        , double                              minSurfPor    
                                        , double                              maxSurfPor    
                                        , double                              baseMinPor     
                                        , double                              minMinPor     
                                        , double                              maxMinPor     
                                        , double                              baseCompCoeff  
                                        , double                              minCompCoeff  
                                        , double                              maxCompCoeff  
                                        , double                              baseCompCoeffB 
                                        , double                              minCompCoeffB 
                                        , double                              maxCompCoeffB 
                                        , PDF                                 prmPDF
                                        , const char                        * name
                                        )
{
   m_pdf       = prmPDF;
   m_mdlType   = mdlType;
   m_lithoName = lithoName;
   m_name      = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );

   std::vector<double> minPorModelPrms; // minimal range parameters 
   std::vector<double> maxPorModelPrms; // maximal range parameters
   std::vector<double> basPorModelPrms; // base parameters

   switch ( mdlType )
   {
      case PrmPorosityModel::Exponential:
         minPorModelPrms.push_back( minSurfPor );
         minPorModelPrms.push_back( minCompCoeff );

         maxPorModelPrms.push_back( maxSurfPor );
         maxPorModelPrms.push_back( maxCompCoeff );
         
         basPorModelPrms.push_back( baseSurfPor );
         basPorModelPrms.push_back( baseCompCoeff );
         break;

      case PrmPorosityModel::SoilMechanics:
         minPorModelPrms.push_back( minSurfPor );
         minPorModelPrms.push_back( minCompCoeff );
         
         maxPorModelPrms.push_back( maxSurfPor );
         maxPorModelPrms.push_back( maxCompCoeff );

         basPorModelPrms.push_back( baseSurfPor );
         basPorModelPrms.push_back( baseCompCoeff );
         break;

      case PrmPorosityModel::DoubleExponential:
         minPorModelPrms.push_back( minSurfPor );
         minPorModelPrms.push_back( minMinPor );
         minPorModelPrms.push_back( minCompCoeff );
         minPorModelPrms.push_back( minCompCoeffB );

         maxPorModelPrms.push_back( maxSurfPor );
         maxPorModelPrms.push_back( maxMinPor );
         maxPorModelPrms.push_back( maxCompCoeff );
         maxPorModelPrms.push_back( maxCompCoeffB );

         basPorModelPrms.push_back( baseSurfPor );
         basPorModelPrms.push_back( baseMinPor );
         basPorModelPrms.push_back( baseCompCoeff );
         basPorModelPrms.push_back( baseCompCoeffB );
         break;
   }

   m_minValue.reset( new PrmPorosityModel( this, lithoName, mdlType, minPorModelPrms ) );
   m_maxValue.reset( new PrmPorosityModel( this, lithoName, mdlType, maxPorModelPrms ) );
   
   m_baseValue.reset( new PrmPorosityModel( this, lithoName, mdlType, basPorModelPrms ) );
}

VarPrmPorosityModel::~VarPrmPorosityModel()
{
}

std::vector<std::string> VarPrmPorosityModel::name() const
{
	std::vector<std::string> ret;

   if ( m_name.empty() )
   {
      switch ( m_mdlType )
      {
         case PrmPorosityModel::Exponential:
            ret.push_back( m_lithoName + ". SurfacePorosity [%]" );
            ret.push_back( m_lithoName + ". Compaction Coefficient [10e-8 Pa-1]" );
            break;

         case PrmPorosityModel::SoilMechanics:
            ret.push_back( m_lithoName + ". Clay fraction [%]" );
            break;

         case PrmPorosityModel::DoubleExponential:
            ret.push_back( m_lithoName + ". SurfacePorosity [%]" );
            ret.push_back( m_lithoName + ". MinimalPorosity [%]" );
            ret.push_back( m_lithoName + ". Compaction Coefficient A [10e-8 Pa-1]" );
            ret.push_back( m_lithoName + ". Compaction Coefficient B [10e-8 Pa-1]" );
            break;
      }
   }
   else
   {
      ret.push_back( m_name );
      switch ( m_mdlType )
      {
         case PrmPorosityModel::Exponential:
            ret.push_back( m_name + "_CC" );
            break;

         case PrmPorosityModel::SoilMechanics:
            break;

         case PrmPorosityModel::DoubleExponential:
            ret.push_back( m_name + "_MP" );
            ret.push_back( m_name + "_CCA" );
            ret.push_back( m_name + "_CCB" );
            break;
      }
   }
	return ret;
}

size_t VarPrmPorosityModel::dimension() const 
{
   switch ( m_mdlType )
   {
      case PrmPorosityModel::Exponential:       return 2;
      case PrmPorosityModel::SoilMechanics:     return 1;
      case PrmPorosityModel::DoubleExponential: return 4;
      default: return 0;
   }
}


SharedParameterPtr VarPrmPorosityModel::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmPorosityModel*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmPorosityModel*>( m_maxValue.get() )->asDoubleArray();

   std::vector<double> valsP;

   for ( size_t i = 0; i < minV.size(); ++i )
   {
      valsP.push_back( *vals++ );

      if ( (minV[i] - std::fabs(minV[i]) * 1e-10) > valsP.back() || valsP.back() > (maxV[i] + std::fabs(maxV[i]) * 1.e-10) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of PorosityModel parameter " << valsP.back() << 
               " is out of range: [" << minV[i] << ":" << maxV[i] << "]";
      }
   }
  
   SharedParameterPtr prm( new PrmPorosityModel( this, m_lithoName.c_str(), m_mdlType, valsP ) );

   return prm;
}

bool VarPrmPorosityModel::save( CasaSerializer & sz, unsigned int version ) const 
{ 
   // save base class data
   bool ok = VarPrmContinuous::save( sz, version );

   ok = ok ? sz.save( static_cast<int>( m_mdlType ), "PorModelType"   ) : ok;
   ok = ok ? sz.save( m_lithoName,                   "LithologyName"  ) : ok;

   return ok;
}

// Constructor from input stream
VarPrmPorosityModel::VarPrmPorosityModel( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );

   int mdlTypeSaved;
   ok = ok ? dz.load( mdlTypeSaved, "PorModelType" ) : ok;
   
   if ( ok ) m_mdlType = static_cast<PrmPorosityModel::PorosityModelType>( mdlTypeSaved );

   ok = ok ? dz.load( m_lithoName, "LithologyName"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmPorosityModel deserialization unknown error";
   }
}

} // namespace casa