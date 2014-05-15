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

#include <cassert>

namespace casa
{

// Constructor
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( double val )  : m_heatProdRateValue( val ) {;}

// Destructor
PrmTopCrustHeatProduction::~PrmTopCrustHeatProduction() {;}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmTopCrustHeatProduction::setInModel( mbapi::Model & caldModel )
{
   return caldModel.ReportError( ErrorHandler::NotImplementedAPI, "PrmTopCrustHeatProduction::setInModel() not implemented yet" );
}

}
