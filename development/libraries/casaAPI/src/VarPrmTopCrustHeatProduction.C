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


#include "PrmTopCrustHeatProduction.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <cassert>

namespace casa
{

VarPrmTopCrustHeatProduction::VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType )
{
   m_pdf = pdfType;
  
   m_minValue.reset( new PrmTopCrustHeatProduction( minValue ) );
   m_maxValue.reset( new PrmTopCrustHeatProduction( maxValue ) );
   
   assert( minValue <= baseValue && maxValue >= baseValue );

   m_baseValue.reset( new PrmTopCrustHeatProduction( baseValue ) );
}

VarPrmTopCrustHeatProduction::~VarPrmTopCrustHeatProduction()
{
   ;
}

Parameter * VarPrmTopCrustHeatProduction::createNewParameterFromDouble( const std::vector<double> & vals ) const
{
   assert( vals.size() == 1 );

   double minV = dynamic_cast<PrmTopCrustHeatProduction*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmTopCrustHeatProduction*>( m_maxValue.get() )->value();

   return (minV <= vals[0] && vals[0] <= maxV) ? new PrmTopCrustHeatProduction( vals[0] ) : 0;
}

}
