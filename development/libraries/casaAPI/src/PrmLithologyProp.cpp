//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmLithologyProp.C
/// @brief This file keeps API implementation of common part of any Lithology property parameter implemented as variable parameter

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
PrmLithologyProp::PrmLithologyProp() : m_parent( 0 ), m_val( UndefinedDoubleValue ) {;}

PrmLithologyProp::PrmLithologyProp( const VarParameter * parent
                                  , const std::vector<std::string> & lithosName
                                  , double val 
                                  )
                                  : m_parent( parent )
                                  , m_lithosName( lithosName.begin(), lithosName.end() )
                                  , m_val( val )
                                  {;}

// Are two parameters equal?
bool PrmLithologyProp::operator == ( const Parameter & prm ) const
{
   const PrmLithologyProp * pp = dynamic_cast<const PrmLithologyProp *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_lithosName.size() != pp->m_lithosName.size() ) return false;
   for ( size_t i = 0; i < m_lithosName.size(); ++i ) { if ( m_lithosName[i] != pp->m_lithosName[i] ) { return false; } }
   
   if ( m_propName  != pp->m_propName  ) return false;
   if ( !NumericFunctions::isEqual( m_val,  pp->m_val,  eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmLithologyProp::serializeCommonPart( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,       "propName"        ) : ok;
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
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,       "propName"        ) : ok;
   ok = ok ? dz.load( m_lithosName, "LithologiesName" ) : ok;
   ok = ok ? dz.load( m_val,        "LithoPropValue"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmLithologyProp deserialization unknown error";
   }
   return ok;
}

} // namespace casa
