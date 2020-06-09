//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmLithologyProp.cpp
/// @brief This file keeps API implementation of common part of any Lithology property parameter implemented as influential parameter

// CASA API
#include "PrmLithologyProp.h"
#include "VarPrmLithologyProp.h"
#include "PrmPorosityModel.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructors
PrmLithologyProp::PrmLithologyProp() :
  Parameter(),
  m_val( Utilities::Numerical::IbsNoDataValue )
{
}

PrmLithologyProp::PrmLithologyProp( const VarParameter * parent
                                  , const std::vector<std::string> & lithosName
                                  , double val
                                  )
                                  : Parameter( parent )
                                  , m_lithosName( lithosName.begin(), lithosName.end() )
                                  , m_val( val )
                                  {;}

// Are two parameters equal?
bool PrmLithologyProp::operator == ( const Parameter & prm ) const
{
   const PrmLithologyProp * pp = dynamic_cast<const PrmLithologyProp *>( &prm );
   if ( !pp ) return false;

   const double eps = 1.e-6;

   if ( m_propName != pp->m_propName ||
        !NumericFunctions::isEqual( m_val,  pp->m_val,  eps ) ||
        m_lithosName.size() != pp->m_lithosName.size() )
   {
     return false;
   }

   for ( size_t i = 0; i < m_lithosName.size(); ++i )
   {
     if ( m_lithosName[i] != pp->m_lithosName[i] )
     {
       return false;
     }
   }

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmLithologyProp::serializeCommonPart( CasaSerializer & sz ) const
{
   bool ok = saveCommonPart(sz);
   ok = ok ? sz.save( m_lithosName, "LithologiesName" ) : ok;
   ok = ok ? sz.save( m_val,        "LithoPropValue"  ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
bool PrmLithologyProp::deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      setParent( ok ? dz.id2ptr<VarParameter>( parentID ) : 0 );
   }

   if (objVer<1)
   {
     std::string name;
     ok = ok && dz.load( name, "propName" );
   }

   ok = ok && dz.load( m_lithosName, "LithologiesName" );
   ok = ok && dz.load( m_val,        "LithoPropValue"  );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmLithologyProp deserialization unknown error";
   }
   return ok;
}

} // namespace casa
