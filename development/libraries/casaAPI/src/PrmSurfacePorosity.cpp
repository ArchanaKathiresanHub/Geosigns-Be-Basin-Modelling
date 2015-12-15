//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSurfacePorosity.C
/// @brief This file keeps API implementation for handling Porosity model parameter Surface porosity

// CASA API
#include "PrmSurfacePorosity.h"
#include "VarPrmSurfacePorosity.h"
#include "PrmPorosityModel.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSurfacePorosity::PrmSurfacePorosity( mbapi::Model & mdl, const std::string & lithoName )
                                      : PrmLithologyProp( 0, std::vector<std::string>( 1, lithoName ), UndefinedDoubleValue )
{ 
   m_propName = "SurfacePorosity";

   // construct parameter name
   std::ostringstream oss;
   oss << m_propName << "(" << lithoName << ")";
   m_name = oss.str();

   mbapi::LithologyManager & mgr = mdl.lithologyManager();

   // go over all lithologies and look for the first lithology with the same name as given
   mbapi::LithologyManager::LithologyID lid = mgr.findID( lithoName );
   if ( lid == UndefinedIDValue )
   {
      throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage();
   }

   mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorUnknown;
   std::vector<double> porModelPrms;

   if ( ErrorHandler::NoError != mgr.porosityModel( lid, porModel, porModelPrms ) )
   {
      throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage();
   }

   switch ( porModel )
   {
      case mbapi::LithologyManager::PorExponential:
      case mbapi::LithologyManager::PorSoilMechanics:
      case mbapi::LithologyManager::PorDoubleExponential:
         m_val = porModelPrms[0];
         break;
      default: throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknow porosity model for the lithology: " << lithoName;
   }
}

 // Constructor
PrmSurfacePorosity::PrmSurfacePorosity( const VarPrmSurfacePorosity * parent, const std::vector<std::string> & lithosName, double surfPor )
                                      : PrmLithologyProp( parent, lithosName, surfPor )
{
   m_propName = "SurfacePorosity";

   // construct parameter name
   std::ostringstream oss;
   oss << m_propName << "(";
   for ( size_t i  = 0; i < m_lithosName.size(); ++i ) oss << (i == 0 ? "" : ", ") << m_lithosName[i];
   oss << ")";
   m_name = oss.str();
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSurfacePorosity::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
{
   mbapi::LithologyManager            & mgr = caldModel.lithologyManager();

   for ( size_t i = 0; i < m_lithosName.size(); ++i )
   {
      mbapi::LithologyManager::LithologyID lid = mgr.findID( m_lithosName[i] );
      if ( lid == UndefinedIDValue )
      {
         return caldModel.moveError( mgr );
      }

      mbapi::LithologyManager::PorosityModel mdlType = mbapi::LithologyManager::PorUnknown;
      std::vector<double> porModelPrms;

      if ( ErrorHandler::NoError != mgr.porosityModel( lid, mdlType, porModelPrms ) )
      {
         return caldModel.moveError( mgr );
      }

      switch ( mdlType )
      {
         case mbapi::LithologyManager::PorExponential:
         case mbapi::LithologyManager::PorDoubleExponential:
            porModelPrms[0] = m_val;
            break;

         case mbapi::LithologyManager::PorSoilMechanics:
            porModelPrms[0] = m_val;
            porModelPrms[1] = SMsp2cc( m_val ); // fill dependent parameter also
            break;

         default: return caldModel.reportError( ErrorHandler::OutOfRangeValue, "Unsupported porosity model" );
      }

      if ( ErrorHandler::NoError != mgr.setPorosityModel( lid, mdlType, porModelPrms ) )
      {
         return caldModel.moveError( mgr );
      }
   }
   return ErrorHandler::NoError;
}

// Validate all porosity model parameters
std::string PrmSurfacePorosity::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;
   mbapi::LithologyManager            & mgr = caldModel.lithologyManager();

   for ( size_t i = 0; i < m_lithosName.size(); ++i )
   {
      if ( m_val < 0   ) oss << "Surface porosity for lithology " << m_lithosName[i] << " can not be negative: "       << m_val  << std::endl;
      if ( m_val > 100 ) oss << "Surface porosity for lithology " << m_lithosName[i] << " can not be more than 100%: " << m_val  << std::endl;
 
      mbapi::LithologyManager::LithologyID lid = mgr.findID( m_lithosName[i] );

      if ( lid == UndefinedIDValue )
      {
         oss << "Lithology " << m_lithosName[i] << " is not defined in the project" << std::endl;
         return oss.str();
      }

      mbapi::LithologyManager::PorosityModel mdlType = mbapi::LithologyManager::PorUnknown;
      std::vector<double> porModelPrms;

      if ( ErrorHandler::NoError != mgr.porosityModel( lid, mdlType, porModelPrms ) )
      {
         oss << "Can not get porosity model parameters for the lithology: " << m_lithosName[i] << ". " << mgr.errorMessage() << std::endl;
         return oss.str();
      }

      switch ( mdlType )
      {
         case mbapi::LithologyManager::PorSoilMechanics:
            if ( ! NumericFunctions::isEqual( SMsp2cc( m_val ), porModelPrms[1], 1e-3 ) )
            {
               oss << "Compaction coefficient for Soil Mechanics model for the lithology " << m_lithosName[i] << " in project: " << porModelPrms[1] << 
                      " is different from the parameter value: "  << SMsp2cc( m_val ) << ", they are related through a clay fraction and can't be defined" 
                      << " independently." << std::endl;
            }

         case mbapi::LithologyManager::PorExponential:
         case mbapi::LithologyManager::PorDoubleExponential:
            if ( ! NumericFunctions::isEqual( m_val, porModelPrms[0], 1.e-4 ) )
            {
               oss << "Surface porosity for lithology " << m_lithosName[i] << " in project: " << porModelPrms[0] << 
                      " is different from the parameter value: "  << m_val << std::endl;
            }
            break;

         default: oss << "Unsupported porosity model for the lithology " << m_lithosName[i] << std::endl;
      }
   }
   return oss.str();
}

// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSurfacePorosity::save( CasaSerializer & sz, unsigned int version ) const
{
   return PrmLithologyProp::serializeCommonPart( sz, version );
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSurfacePorosity::PrmSurfacePorosity( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = PrmLithologyProp::deserializeCommonPart( dz, objVer );
   
   if ( m_propName.empty() ) { m_propName = "SurfacePororsity"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmSurfacePorosity deserialization unknown error";
   }
}

} // namespace casa
