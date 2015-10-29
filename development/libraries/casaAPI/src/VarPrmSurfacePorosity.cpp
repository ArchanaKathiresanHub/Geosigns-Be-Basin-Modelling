//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSurfacePorosity.h
/// @brief This file keeps API implementation for handling variation of surface porosity the given list of lithologies

// CASA
#include "VarPrmSurfacePorosity.h"

// CMB API
#include "cmbAPI.h"

// C lib
#include <cassert>
#include <cstring>
#include <cmath>

namespace casa
{

VarPrmSurfacePorosity::VarPrmSurfacePorosity( const std::vector<std::string> & lithosName
                                        , double                               baseSurfPor    
                                        , double                               minSurfPor    
                                        , double                               maxSurfPor    
                                        , PDF                                  prmPDF
                                        , const std::string                  & name
                                        )
                                        : VarPrmLithologyProp( lithosName, prmPDF, name )
{
   m_propName = "SurfacePorosity";
   m_minValue.reset(  new PrmSurfacePorosity( this, m_lithosName, minSurfPor  ) );
   m_maxValue.reset(  new PrmSurfacePorosity( this, m_lithosName, maxSurfPor  ) );
   m_baseValue.reset( new PrmSurfacePorosity( this, m_lithosName, baseSurfPor ) );
}


PrmLithologyProp * VarPrmSurfacePorosity::createNewPrm( double val ) const
{
   return new PrmSurfacePorosity( this, m_lithosName, val );
}

bool VarPrmSurfacePorosity::save( CasaSerializer & sz, unsigned int version ) const 
{ 
   bool ok = VarPrmLithologyProp::serializeCommonPart( sz, version );
   return ok;
}

// Constructor from input stream
VarPrmSurfacePorosity::VarPrmSurfacePorosity( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmLithologyProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "SurfacePorosity"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmSurfacePorosity deserialization unknown error";
   }
}

} // namespace casa
