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

   m_minValue.reset( new PrmSourceRockTOC( minValue, layerName ) );
   m_maxValue.reset( new PrmSourceRockTOC( maxValue, layerName ) );
   
   m_baseValue.reset( new PrmSourceRockTOC( baseValue, layerName ) );
}

VarPrmSourceRockTOC::~VarPrmSourceRockTOC()
{
   ;
}

Parameter * VarPrmSourceRockTOC::createNewParameterFromDouble( const std::vector<double> & vals ) const
{
   assert( vals.size() == 1 );

   double minV = dynamic_cast<PrmSourceRockTOC*>( m_minValue.get() )->value();
   double maxV = dynamic_cast<PrmSourceRockTOC*>( m_maxValue.get() )->value();

   if ( minV > vals[0] || maxV < vals[0] ) return 0;
   return new PrmSourceRockTOC( vals[0], m_layerName.c_str() );
}

}
