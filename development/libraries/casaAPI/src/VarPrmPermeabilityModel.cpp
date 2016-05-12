//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmPermeabilityModel.h
/// @brief This file keeps API implementation for handling variation of permeability model parameters for the given layer 

// CASA
#include "VarPrmPermeabilityModel.h"

// CMB API
#include "cmbAPI.h"

// utilities lib
#include "PiecewiseInterpolator.h"

// STL/C lib
#include <cassert>
#include <cstring>
#include <memory>
#include <cmath>

#include <set>

namespace casa
{
// reinterpolate permeability vs porosity profile to common for base/min/max porosity grid
static std::vector<double> ReinterpolatePermProf( const std::set<double> & commonGrid, const std::vector<double> & vals )
{
   ibs::PiecewiseInterpolator pwInterp;

   size_t numPoints = (vals.size() - (PrmPermeabilityModel::MPProfileNumPoints + 1) )/2;
   double * por_perm = new double[ numPoints * 2 ];
     
   for ( size_t i = 0; i < numPoints; ++i )
   {
      por_perm[i]             = vals[PrmPermeabilityModel::MPProfileNumPoints + 1 + i*2];
      por_perm[numPoints + i] = vals[PrmPermeabilityModel::MPProfileNumPoints + 1 + i*2 + 1];
   }
   pwInterp.setInterpolation( ibs::PiecewiseInterpolator::PIECEWISE_LINEAR, static_cast<int>( numPoints ), por_perm, por_perm + numPoints ); 
   pwInterp.computeCoefficients();

   std::vector<double> reintVals;
   for ( std::set<double>::const_iterator it = commonGrid.begin(); it != commonGrid.end(); ++it )
   {
      reintVals.push_back( *it );
      reintVals.push_back( pwInterp.evaluate( *it ) ); 
   }

   delete [] por_perm;

   return reintVals;
}

// Calculate base case permeability with porosity profile as average between minmial and maximal curves
std::vector<double> VarPrmPermeabilityModel::createBaseCaseMPModelPrms( const std::vector<double> & minModelPrms, const std::vector<double> & maxModelPrms )
{
   // create unique sorted porosity set values
   std::set<double> sortedPorosityValsSet;

   for ( size_t i = PrmPermeabilityModel::MPProfileNumPoints + 1; i < minModelPrms.size();  i += 2 ) sortedPorosityValsSet.insert( minModelPrms[i] );
   for ( size_t i = PrmPermeabilityModel::MPProfileNumPoints + 1; i < minModelPrms.size();  i += 2 ) sortedPorosityValsSet.insert( maxModelPrms[i] );

   // interpolate min values to the common grid
   const std::vector<double> & minReinterp = ReinterpolatePermProf( sortedPorosityValsSet, minModelPrms );
   std::vector<double> mnmp( minModelPrms.begin(), ( minModelPrms.begin() + PrmPermeabilityModel::MPProfileNumPoints + 1 ) );
   mnmp[PrmPermeabilityModel::MPProfileNumPoints] = minReinterp.size() / 2.0;
   mnmp.insert( mnmp.end(), minReinterp.begin(), minReinterp.end() );

   // interpolate max values to the common grid
   const std::vector<double> & maxReinterp = ReinterpolatePermProf( sortedPorosityValsSet, maxModelPrms );
   std::vector<double> mxmp( maxModelPrms.begin(), ( maxModelPrms.begin() + PrmPermeabilityModel::MPProfileNumPoints + 1 ) );
   mxmp[PrmPermeabilityModel::MPProfileNumPoints] = maxReinterp.size() / 2.0;
   mxmp.insert( mxmp.end(), maxReinterp.begin(), maxReinterp.end() );

   // create base case for common porosity grid
   std::vector<double> baseCasePrms;

   for ( size_t i = 0; i < PrmPermeabilityModel::MPProfileNumPoints; ++i )
   {
      baseCasePrms.push_back( ( minModelPrms[i] + maxModelPrms[i] ) * 0.5 );
   }
   // put number of por/perm points in curve
   baseCasePrms.push_back( static_cast<double>( sortedPorosityValsSet.size() ) );
   for ( size_t i = 0; i < minReinterp.size(); i += 2 )
   {
      baseCasePrms.push_back( minReinterp[i] );
      baseCasePrms.push_back( ( minReinterp[i+1] + maxReinterp[i+1] ) * 0.5 );
   }

   return baseCasePrms;
}



VarPrmPermeabilityModel::VarPrmPermeabilityModel( const char                                 *  lithoName
                                                , PrmPermeabilityModel::PermeabilityModelType   mdlType 
                                                , const std::vector<double>                   & baseModelPrms
                                                , const std::vector<double>                   & minModelPrms 
                                                , const std::vector<double>                   & maxModelPrms  
                                                , PDF                                           prmPDF
                                                , const char                                  * name
                                                ) 
{
   m_pdf       = prmPDF;
   m_mdlType   = mdlType;
   m_lithoName = lithoName;
   m_name      = name && strlen( name ) > 0 ? std::string( name ) : std::string( "" );
      
   if ( PrmPermeabilityModel::None == mdlType || PrmPermeabilityModel::Impermeable == mdlType )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "It is not possible to variate permeability model parameters for impermeable lithology";
   }

   std::unique_ptr<PrmPermeabilityModel> minPrm;
   std::unique_ptr<PrmPermeabilityModel> maxPrm;
   std::unique_ptr<PrmPermeabilityModel> basPrm;

   // for Multipoint create profiles with the same porosity points set
   if ( PrmPermeabilityModel::Multipoint == mdlType )
   {
      // create unique sorted porosity set values
      std::set<double> sortedPorosityValsSet;
      
      for ( size_t i = PrmPermeabilityModel::MPProfileNumPoints + 1; i < baseModelPrms.size(); i += 2 ) sortedPorosityValsSet.insert( baseModelPrms[i] );
      for ( size_t i = PrmPermeabilityModel::MPProfileNumPoints + 1; i < minModelPrms.size();  i += 2 ) sortedPorosityValsSet.insert( minModelPrms[i]  );
      for ( size_t i = PrmPermeabilityModel::MPProfileNumPoints + 1; i < maxModelPrms.size();  i += 2 ) sortedPorosityValsSet.insert( maxModelPrms[i]  );
      
      // interpolate base values to the common grid
      const std::vector<double> & bsReinterp = ReinterpolatePermProf( sortedPorosityValsSet, baseModelPrms );
      std::vector<double> bsmp( baseModelPrms.begin(), ( baseModelPrms.begin() + PrmPermeabilityModel::MPProfileNumPoints + 1 ) );
      bsmp[ PrmPermeabilityModel::MPProfileNumPoints] = bsReinterp.size() / 2.0;
      bsmp.insert( bsmp.end(), bsReinterp.begin(), bsReinterp.end() );

      // interpolate min values to the common grid
      const std::vector<double> & minReinterp = ReinterpolatePermProf( sortedPorosityValsSet, minModelPrms );
      std::vector<double> mnmp( minModelPrms.begin(), ( minModelPrms.begin() + PrmPermeabilityModel::MPProfileNumPoints + 1 ) );
      mnmp[PrmPermeabilityModel::MPProfileNumPoints] = minReinterp.size() / 2.0;
      mnmp.insert( mnmp.end(), minReinterp.begin(), minReinterp.end() );

      // interpolate max values to the common grid
      const std::vector<double> & maxReinterp = ReinterpolatePermProf( sortedPorosityValsSet, maxModelPrms );
      std::vector<double> mxmp( maxModelPrms.begin(), ( maxModelPrms.begin() + PrmPermeabilityModel::MPProfileNumPoints + 1 ) );
      mxmp[PrmPermeabilityModel::MPProfileNumPoints] = maxReinterp.size() / 2.0;
      mxmp.insert( mxmp.end(), maxReinterp.begin(), maxReinterp.end() );

      minPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, mnmp ) );
      maxPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, mxmp ) );
      basPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, bsmp ) );
   }
   else
   {
      minPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, minModelPrms ) );
      maxPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, maxModelPrms ) );
      basPrm.reset( new PrmPermeabilityModel( this, lithoName, mdlType, baseModelPrms ) );
   }

   if ( PrmPermeabilityModel::Multipoint == mdlType )
   {
      minPrm->setVariationPosition( 0.0 );
      basPrm->setVariationPosition( 0.5 );
      maxPrm->setVariationPosition( 1.0 );
   }

   m_minValue.reset( minPrm.release() );
   m_maxValue.reset( maxPrm.release() );
   m_baseValue.reset( basPrm.release() ); 
}

VarPrmPermeabilityModel::~VarPrmPermeabilityModel()
{
}

std::vector<std::string> VarPrmPermeabilityModel::name() const
{
   std::vector<std::string> ret;
   
   if ( m_name.empty() )
   {
      switch ( m_mdlType )
      {
         case PrmPermeabilityModel::None:
         case PrmPermeabilityModel::Impermeable:
            break;

         case PrmPermeabilityModel::Sandstone:
            ret.resize( PrmPermeabilityModel::ClayPercentage + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_lithoName + ". Anisotropic coeff. [kv/kh]";
            ret[PrmPermeabilityModel::DepositionalPerm] = m_lithoName + ". Depositional permeability [mD]";
            ret[PrmPermeabilityModel::ClayPercentage]   = m_lithoName + ". Sandstone clay percentage [%]";
            break;

         case PrmPermeabilityModel::Mudstone:
            ret.resize( PrmPermeabilityModel::RecoverCoeff + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_lithoName + ". Anisotropic coeff. [kv/kh]";
            ret[PrmPermeabilityModel::DepositionalPerm] = m_lithoName + ". Depositional permeability [mD]";
            ret[PrmPermeabilityModel::SensitivityCoeff] = m_lithoName + ". Sensitivity coeff. []";
            ret[PrmPermeabilityModel::RecoverCoeff]     = m_lithoName + ". Recovery coeff. []";
            break;
            
         case PrmPermeabilityModel::Multipoint:
            ret.resize( PrmPermeabilityModel::AnisotropicCoeff + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_lithoName + ". Anisotropic coeff. [kv/kh]";
            ret.push_back( m_lithoName + ". Profile variation parameter []" );
            break;

         default: throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknown permeability model ID: " << m_mdlType;
      }
   }
   else
   {
      switch ( m_mdlType )
      {
         case PrmPermeabilityModel::None:
         case PrmPermeabilityModel::Impermeable:
            break;

         case PrmPermeabilityModel::Sandstone:
            ret.resize( PrmPermeabilityModel::ClayPercentage + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_name + "_ansCoef";
            ret[PrmPermeabilityModel::DepositionalPerm] = m_name + "_depPerm";
            ret[PrmPermeabilityModel::ClayPercentage]   = m_name + "_clayPrc";
            break;

         case PrmPermeabilityModel::Mudstone:
            ret.resize( PrmPermeabilityModel::RecoverCoeff + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_name + "_ansCoef";
            ret[PrmPermeabilityModel::DepositionalPerm] = m_name + "_depPerm";
            ret[PrmPermeabilityModel::SensitivityCoeff] = m_name + "_sensCof";
            ret[PrmPermeabilityModel::RecoverCoeff]     = m_name + "_recvCof";
            break;
            
         case PrmPermeabilityModel::Multipoint:
            ret.resize( PrmPermeabilityModel::AnisotropicCoeff + 1 );
            ret[PrmPermeabilityModel::AnisotropicCoeff] = m_name + "_ansCoef";
            ret.push_back( m_name + "_intCoef" );
            break;
      
         default: throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknown permeability model ID: " << m_mdlType;
      }
      if ( ret.size() > 0 ) ret[0] = m_name;
      else ret.push_back( m_name );
   }
	return ret;
}

size_t VarPrmPermeabilityModel::dimension() const 
{
   switch ( m_mdlType )
   {
      case PrmPermeabilityModel::None:
      case PrmPermeabilityModel::Impermeable:       return 0;
      case PrmPermeabilityModel::Sandstone:         return 3;
      case PrmPermeabilityModel::Mudstone:          return 4;
      case PrmPermeabilityModel::Multipoint:        return 2;
      default: return 0;
   }
}

SharedParameterPtr VarPrmPermeabilityModel::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmPermeabilityModel*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmPermeabilityModel*>( m_maxValue.get() )->asDoubleArray();

   std::vector<double> valsP;

   // check for parameters value are in interval min/max
   for ( size_t i = 0; i < minV.size(); ++i )
   {
      valsP.push_back( *vals++ );

      if ( (minV[i] - std::fabs(minV[i]) * 1e-10) > valsP.back() || valsP.back() > (maxV[i] + std::fabs(maxV[i]) * 1.e-10) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of PermeabilityModel parameter " << valsP.back() << 
               " is out of range: [" << minV[i] << ":" << maxV[i] << "]";
      }
   }

   // create profile interpolation
   if ( PrmPermeabilityModel::Multipoint == m_mdlType )
   {
      double interpPos = valsP.back(); // last one parameter is in [0:1] range
      
      // adjust interpPos according to base 0.5 value
      const PrmPermeabilityModel * minPrm = dynamic_cast< const PrmPermeabilityModel *>( interpPos > 0.5 ? m_baseValue.get() : m_minValue.get()  );
      const PrmPermeabilityModel * maxPrm = dynamic_cast< const PrmPermeabilityModel *>( interpPos > 0.5 ? m_maxValue.get()  : m_baseValue.get() );
      interpPos = (interpPos > 0.5 ? (interpPos - 0.5) : interpPos ) * 2.0;

      // create new profile by interpolating between min/max values
      valsP.resize( PrmPermeabilityModel::MPProfileNumPoints + 1 );
      valsP[PrmPermeabilityModel::MPProfileNumPoints] = static_cast<double>( minPrm->multipointPorosity().size() );
      for ( size_t i = 0; i < minPrm->multipointPorosity().size(); ++i )
      {
         valsP.push_back( minPrm->multipointPorosity()[i] );
         valsP.push_back( minPrm->multipointPermeability()[i] + (maxPrm->multipointPermeability()[i] - minPrm->multipointPermeability()[i] ) * interpPos );
      }
   }

   SharedParameterPtr prm( new PrmPermeabilityModel( this, m_lithoName.c_str(), m_mdlType, valsP ) );

   return prm;
}

SharedParameterPtr VarPrmPermeabilityModel::newParameterFromModel( mbapi::Model & mdl ) const
{
   SharedParameterPtr prm( new PrmPermeabilityModel( mdl, m_lithoName.c_str( ) ) );
   return prm;
}

SharedParameterPtr VarPrmPermeabilityModel::makeThreeDFromOneD( mbapi::Model & mdl, const std::vector<double>& xin, const std::vector<double>& yin, const std::vector<SharedParameterPtr>& prmVec ) const
{
   // Not yet implemented
   return nullptr;
}


bool VarPrmPermeabilityModel::save( CasaSerializer & sz, unsigned int version ) const 
{ 
   // save base class data
   bool ok = VarPrmContinuous::save( sz, version );

   ok = ok ? sz.save( static_cast<int>( m_mdlType ), "PorModelType"   ) : ok;
   ok = ok ? sz.save( m_lithoName,                   "LithologyName"  ) : ok;

   return ok;
}

// Constructor from input stream
VarPrmPermeabilityModel::VarPrmPermeabilityModel( CasaDeserializer & dz, unsigned int objVer ) 
{
   bool ok = VarPrmContinuous::deserializeCommonPart( dz, objVer );
   
   int mdlTypeSaved;
   ok = ok ? dz.load( mdlTypeSaved, "PorModelType" ) : ok;
   
   if ( ok ) m_mdlType = static_cast<PrmPermeabilityModel::PermeabilityModelType>( mdlTypeSaved );

   ok = ok ? dz.load( m_lithoName,  "LithologyName"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmPermeabilityModel deserialization unknown error";
   }
}

}
