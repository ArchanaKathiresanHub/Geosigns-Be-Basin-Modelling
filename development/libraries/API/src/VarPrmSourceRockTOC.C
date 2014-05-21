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


#include "SimpleRange.h"

#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

#include <cassert>

namespace casa
{

VarPrmSourceRockTOC::VarPrmSourceRockTOC( const char * srLithoType, double baseValue, double minValue, double maxValue, PDF pdfType ) : 
   m_srLithoType( srLithoType )
{
   m_pdf = pdfType;
   m_valueRange.reset( new SimpleRange( minValue, maxValue ) );
   
   
   assert( m_valueRange->isValInRange( baseValue ) );

   m_baseValue.reset( new PrmSourceRockTOC( baseValue, srLithoType ) );
}

VarPrmSourceRockTOC::~VarPrmSourceRockTOC()
{
}

double VarPrmSourceRockTOC::baseValueAsDouble() const
{
   return dynamic_cast<PrmSourceRockTOC*>( m_baseValue.get() )->value();
}

Parameter * VarPrmSourceRockTOC::createNewParameterFromDouble( double val ) const
{
   return m_valueRange->isValInRange( val ) ? (new PrmSourceRockTOC( val, m_srLithoType.c_str() )) : 0;
}

}
