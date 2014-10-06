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
  
   m_minValue.reset( new PrmTopCrustHeatProduction( this, minValue ) );
   m_maxValue.reset( new PrmTopCrustHeatProduction( this, maxValue ) );
   
   assert( minValue <= baseValue && maxValue >= baseValue );

   m_baseValue.reset( new PrmTopCrustHeatProduction( this, baseValue ) );
}

VarPrmTopCrustHeatProduction::~VarPrmTopCrustHeatProduction()
{
   ;
}

std::vector<std::string> VarPrmTopCrustHeatProduction::name()
{
	std::vector<std::string> ret;
	ret.push_back("TopCrustHeatProductionRate");

	return ret;
}

SharedParameterPtr VarPrmTopCrustHeatProduction::createNewParameterFromDouble( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmTopCrustHeatProduction*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmTopCrustHeatProduction*>( m_maxValue.get() )->value();
   double valP = *vals++;

   if ( minV > valP || valP > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of Top crust heat production rate parameter " << valP << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   
   SharedParameterPtr prm( new PrmTopCrustHeatProduction( this, valP ) );

   return prm;
}

}
