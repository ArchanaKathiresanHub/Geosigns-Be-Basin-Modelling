//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockTOC.C
/// @brief This file keeps API implementation for Source Rock TOC parameter handling 


#include "SimpleRange.h"

#include "PrmSourceRockTOC.h"
#include "cmbAPI.h"

#include <cassert>

namespace casa
{

// Constructor
PrmSourceRockTOC::PrmSourceRockTOC( double val, const char * srLithoType ) : m_toc( val ), m_srLithoType( srLithoType ) {;}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel )
{
   return caldModel.ReportError( ErrorHandler::NotImplementedAPI, "PrmSourcRockTOC::setInModel() not implemented yet" );
}

}
