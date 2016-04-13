//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockProp.C
/// @brief This file keeps API implementation for common part of all source rock  lithology parameter handling 

// CASA API
#include "PrmSourceRockProp.h"
#include "VarPrmSourceRockProp.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>
#include "Path.h" // for to_string

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSourceRockProp::PrmSourceRockProp()
{
   m_val = 0.0;
   m_parent = 0;
   m_mixID = 1;
}

// Constructor
PrmSourceRockProp::PrmSourceRockProp( mbapi::Model & mdl
                                  , const char * layerName
                                  , const char * srType
                                  , int          mixingID
                                  )
                                  : m_parent(0)
                                  , m_layerName( layerName )
                                  , m_srTypeName( srType ? srType : "" )
                                  , m_mixID( mixingID )
{    
   mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

   // get check is this layer has a mix of source rocks
   mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
   if (stMgr.errorCode() != ErrorHandler::NoError) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

   // check if layer set as active source rock
   if ( !stMgr.isSourceRockActive( lid ) )
   {
      throw ErrorHandler::Exception(ErrorHandler::ValidationError) << m_propName << 
         " setting error: source rock is not active for the layer:" << m_layerName;
   }

   const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
   if ( srtNames.empty() || m_mixID < 1 || static_cast<size_t>( m_mixID ) > srtNames.size() )
   {
      throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
         " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
   }
   
   mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (m_srTypeName.empty() ? srtNames[m_mixID-1] : m_srTypeName) );
   if ( IsValueUndefined( sid ) )
   {
      throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Can not find source rock lithology for layer "
         << m_layerName << " and SR type " << (srType ? std::string( srType ) : srtNames[m_mixID-1]);
   }
}

// Constructor
PrmSourceRockProp::PrmSourceRockProp( const VarPrmSourceRockProp * parent
                                    , double                       val
                                    , const char                 * lrName
                                    , const char                 * srType
                                    , int                          mixingID 
                                 )
                                  : m_parent( parent )
                                  , m_layerName( lrName )
                                  , m_srTypeName( srType ? srType : "" )
                                  , m_mixID( mixingID )
                                  , m_val( val ) { ; }


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockProp::serializeCommonPart( CasaSerializer & sz, unsigned int /* version  */) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,       "name"       ) : ok;
   ok = ok ? sz.save( m_layerName,  "layerName"  ) : ok;
   ok = ok ? sz.save( m_val,        "val"        ) : ok;
   ok = ok ? sz.save( m_mixID,      "mixID"      ) : ok;
   ok = ok ? sz.save( m_srTypeName, "srTypeName" ) : ok;
   ok = ok ? sz.save( m_propName,   "propName"   ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
bool PrmSourceRockProp::deserializeCommonPart( CasaDeserializer & dz, unsigned int /* objVer */ )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,       "name"       ) : ok;
   ok = ok ? dz.load( m_layerName,  "layerName"  ) : ok;
   ok = ok ? dz.load( m_val,        "val"        ) : ok;
   ok = ok ? dz.load( m_mixID,      "mixID"      ) : ok;
   ok = ok ? dz.load( m_srTypeName, "srTypeName" ) : ok;
   ok = ok ? dz.load( m_propName,   "propName"   ) : ok;

   return ok;
}

} // namespace cas
