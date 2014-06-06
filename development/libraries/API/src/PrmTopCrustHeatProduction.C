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


#include "SimpleRange.h"

#include "PrmTopCrustHeatProduction.h"
#include "cmbAPI.h"
#include "ErrorHandler.h"

#include <cassert>
#include <stdexcept>

namespace casa
{

// Constructor
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( double val )  : m_heatProdRateValue( val ) {;}

PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( mbapi::Model & mdl )
{
   m_heatProdRateValue = mdl.tableValueAsDouble( "BasementIoTbl", 0, "TopCrustHeatProd" );

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
   const std::string & modelName = caldModel.tableValueAsString( "BasementIoTbl", 0, "BottomBoundaryModel" );
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();

   if ( modelName != "Fixed Temperature" ) return caldModel.reportError( ErrorHandler::ValidationError, 
                            "Unsupported bottom boundary model for using top crust heat production. Supported 'Fixed Temperature' model only" );

   const std::string & heatProdMap = caldModel.tableValueAsString( "BasementIoTbl", 0, "TopCrustHeatProdGrid" );
   if ( ErrorHandler::NoError != caldModel.errorCode() ) return caldModel.errorCode();

   if ( !heatProdMap.empty() ) return caldModel.reportError( ErrorHandler::ValidationError, 
           "Project has a map defined for top crust heat production rate. Variation of the map doesn't supported yet" );
   
   return caldModel.setTableValue( "BasementIoTbl", 0, "TopCrustHeatProd", m_heatProdRateValue );
}

}
