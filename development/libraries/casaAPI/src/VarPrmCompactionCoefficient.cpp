//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmCompactionCoefficient.cpp
/// @brief This file keeps API implementation for handling variation of compaction coefficient for the given list of lithologies

// CASA
#include "VarPrmCompactionCoefficient.h"

// CMB API
#include "cmbAPI.h"

// C lib
#include <cassert>
#include <cstring>
#include <cmath>

namespace casa
{
   VarPrmCompactionCoefficient::VarPrmCompactionCoefficient(
                                          const std::vector<std::string> & lithosName
                                        , double                           baseCompCoef ///< base value for compaction coefficient
                                        , double                           minCompCoef  ///< minimal value for compaction coefficient
                                        , double                           maxCompCoef ///< maximal value for compaction coefficient
                                        , VarPrmContinuous::PDF            prmPDF
                                        , const std::string              & name
                                        )
                                        : VarPrmLithologyProp( lithosName, prmPDF, name )
{
   m_propName = "CompactionCoefficient";
   m_minValue.reset(  new PrmCompactionCoefficient( this, m_lithosName, minCompCoef  ) );
   m_maxValue.reset(  new PrmCompactionCoefficient( this, m_lithosName, maxCompCoef  ) );
   m_baseValue.reset( new PrmCompactionCoefficient( this, m_lithosName, baseCompCoef ) );
}


PrmLithologyProp * VarPrmCompactionCoefficient::createNewPrm( double val ) const
{
   return new PrmCompactionCoefficient( this, m_lithosName, val );
}

PrmLithologyProp * VarPrmCompactionCoefficient::createNewPrmFromModel( mbapi::Model & mdl ) const
{
   PrmCompactionCoefficient mdlPrm( mdl, m_lithosName[0] );
   return new PrmCompactionCoefficient( this, m_lithosName, mdlPrm.value( ) );
}

bool VarPrmCompactionCoefficient::save( CasaSerializer & sz ) const
{
   bool ok = VarPrmLithologyProp::serializeCommonPart( sz );
   return ok;
}

// Constructor from input stream
VarPrmCompactionCoefficient::VarPrmCompactionCoefficient( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = VarPrmLithologyProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "CompactionCoefficient"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "VarPrmCompactionCoefficient deserialization unknown error";
   }
}

} // namespace casa
