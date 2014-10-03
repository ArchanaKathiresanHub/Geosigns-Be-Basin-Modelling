//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockTOC.h
/// @brief This file keeps API implementation for handling variation of initial source rock TOC parameter. 

#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

#include <cassert>

namespace casa
{

VarPrmSourceRockTOC::VarPrmSourceRockTOC( const char * layerName, double baseValue, double minValue, double maxValue, PDF pdfType ) : 
   m_layerName( layerName )
{
   m_pdf = pdfType;

   assert( minValue <= baseValue && maxValue >= baseValue );

   m_minValue.reset( new PrmSourceRockTOC( this, minValue, layerName ) );
   m_maxValue.reset( new PrmSourceRockTOC( this, maxValue, layerName ) );
   
   m_baseValue.reset( new PrmSourceRockTOC( this, baseValue, layerName ) );
}

VarPrmSourceRockTOC::~VarPrmSourceRockTOC()
{
   ;
}

SharedParameterPtr VarPrmSourceRockTOC::createNewParameterFromDouble( std::vector<double>::const_iterator & vals ) const
{
   double minV = dynamic_cast<PrmSourceRockTOC*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmSourceRockTOC*>( m_maxValue.get() )->value();
   double prmV = *vals++;

   if ( minV > prmV || prmV > maxV )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Variation of source rock TOC parameter for layer " << m_layerName << ": " << prmV << 
                                                                        " falls out of range: [" << minV << ":" << maxV << "]";
   }
   SharedParameterPtr prm( new PrmSourceRockTOC( this, prmV, m_layerName.c_str() ) );

   return prm;
}

}
