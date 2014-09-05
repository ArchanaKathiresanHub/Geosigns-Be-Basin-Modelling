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


#include "PrmTopCrustHeatProduction.h"
#include "cmbAPI.h"
#include "ErrorHandler.h"

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
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( double val )  : m_heatProdRateValue( val )
{
   ;
}

PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( mbapi::Model & mdl )
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

   if ( std::fabs( valInModel - m_heatProdRateValue ) > 1e-8 )
   {
      oss << "Top crust heat production rate parameter value in the model (" << valInModel; 
      oss << ") is differ from a parameter value (" << m_heatProdRateValue << ")" << std::endl;
   }

   return oss.str();
}

}
