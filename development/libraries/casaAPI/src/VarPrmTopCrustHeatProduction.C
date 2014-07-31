//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmTopCrustHeatProduction.h
/// @brief This file keeps API implementation for handling variation of Top Crust Heat Production Rate parameter. 


#include "SimpleRange.h"

#include "PrmTopCrustHeatProduction.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <cassert>

namespace casa
{

VarPrmTopCrustHeatProduction::VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType )
{
   m_pdf = pdfType;
   m_valueRange.reset( new SimpleRange( minValue, maxValue ) );
   
   assert( m_valueRange->isValInRange( baseValue ) );

   m_baseValue.reset( new PrmTopCrustHeatProduction( baseValue ) );
}

VarPrmTopCrustHeatProduction::~VarPrmTopCrustHeatProduction()
{
}

double VarPrmTopCrustHeatProduction::baseValueAsDouble() const
{
   return dynamic_cast<PrmTopCrustHeatProduction*>( m_baseValue.get() )->value( );
}

Parameter * VarPrmTopCrustHeatProduction::createNewParameterFromDouble( double val ) const
{
   return m_valueRange->isValInRange( val ) ? new PrmTopCrustHeatProduction( val ) : 0;
}

}
