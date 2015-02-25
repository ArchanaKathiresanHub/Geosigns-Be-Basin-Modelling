//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmTopCrustHeatProduction.h
/// @brief This file keeps API implementation for handling Top Crust Heat Production Rate basement parameter. 

// CASA API
#include "PrmTopCrustHeatProduction.h"
#include "VarPrmTopCrustHeatProduction.h"

// CMB API
#include "cmbAPI.h"
#include "ErrorHandler.h"

// utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{

static const char * s_basementTblName      = "BasementIoTbl";
static const char * s_bottomBoundaryModel  = "BottomBoundaryModel";
static const char * s_topCrushHeatProd     = "TopCrustHeatProd";
static const char * s_topCrushHeatProdGrid = "TopCrustHeatProdGrid";

// Constructor
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( const VarPrmTopCrustHeatProduction * parent, double val )
   : m_parent( parent )
   , m_heatProdRateValue( val )
{
}

PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( mbapi::Model & mdl ) : m_parent( 0 )
{
   m_heatProdRateValue = mdl.tableValueAsDouble( s_basementTblName, 0, s_topCrushHeatProd );

   if ( ErrorHandler::NoError != mdl.errorCode() )
   {
      m_heatProdRateValue = UndefinedDoubleValue;
   }
}

// Destructor
PrmTopCrustHeatProduction::~PrmTopCrustHeatProduction() {;}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmTopCrustHeatProduction::setInModel( mbapi::Model & caldModel )
{
   const std::string & modelName = caldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();

   if ( modelName != "Fixed Temperature" ) return caldModel.reportError( ErrorHandler::ValidationError, 
                            "Unsupported bottom boundary model for using top crust heat production. Supported 'Fixed Temperature' model only" );

   const std::string & heatProdMap = caldModel.tableValueAsString( s_basementTblName, 0, s_topCrushHeatProdGrid );
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();

   if ( !heatProdMap.empty() ) return caldModel.reportError( ErrorHandler::ValidationError, 
           "Project has a map defined for top crust heat production rate. Variation of the map doesn't supported yet" );
   
   return caldModel.setTableValue( s_basementTblName, 0, s_topCrushHeatProd, m_heatProdRateValue );
}

// Validate top crust heat production rate value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmTopCrustHeatProduction::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_heatProdRateValue < 0.0 )
   {
      oss << "Top crust heat production rate value  can not be negative: " << m_heatProdRateValue << std::endl;
   }

   const std::string & modelName = caldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( modelName != "Fixed Temperature" )
   {
      oss << "Unsupported bottom boundary model for using top crust heat production rate.";
      oss << " Supported 'Fixed Temperature' model only" << std::endl;
      return oss.str(); // another model, no reason to check further
   }

   const std::string & heatProdMap = caldModel.tableValueAsString( s_basementTblName, 0, s_topCrushHeatProdGrid );
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( !heatProdMap.empty() )
   {
      oss << "Project has a map defined for top crust heat production rate.";
      oss << " Map and one rate value can not be used together" << std::endl;
      return oss.str();
   }
      
   double valInModel = caldModel.tableValueAsDouble( s_basementTblName, 0, s_topCrushHeatProd );

   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( !NumericFunctions::isEqual( valInModel, m_heatProdRateValue, 1e-6 ) )
   {
      oss << "Top crust heat production rate parameter value in the model (" << valInModel; 
      oss << ") is differ from a parameter value (" << m_heatProdRateValue << ")" << std::endl;
   }

   return oss.str();
}


// Are two parameters equal?
bool PrmTopCrustHeatProduction::operator == ( const Parameter & prm ) const
{
   const PrmTopCrustHeatProduction * pp = dynamic_cast<const PrmTopCrustHeatProduction *>( &prm );
   if ( !pp ) return false;
   
   return NumericFunctions::isEqual( m_heatProdRateValue, pp->m_heatProdRateValue, 1.e-6 ) ? true : false;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmTopCrustHeatProduction::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_heatProdRateValue, "heatProdRateValue" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_heatProdRateValue, "heatProdRateValue" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmTopCrustHeatProduction deserialization unknown error";
   }
}
}
