//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockHI.C
/// @brief This file keeps API implementation for Source Rock HI parameter handling 


#include "PrmSourceRockHI.h"
#include "VarPrmSourceRockHI.h"

// CMB API
#include "cmbAPI.h"

#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSourceRockHI::PrmSourceRockHI( mbapi::Model & mdl, const char * layerName ) : m_parent( 0 )
{ 
   m_layerName = layerName;
   bool isFound = false;

   mbapi::SourceRockManager & mgr = mdl.sourceRockManager();

   // go over all source rock lithologies and look for the first lithology with the same layer name as given
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( size_t i = 0; i < srIDs.size(); ++i )
   {
      if ( mgr.layerName( srIDs[i] ) == m_layerName )
      {
         m_hi = mgr.hiIni( srIDs[i] );
         if ( ErrorHandler::NoError != mgr.errorCode() ) mdl.moveError( mgr );
         isFound = true;
         break;
      }
   }
   if ( !isFound ) mdl.reportError( ErrorHandler::NonexistingID, std::string( "Can't find layer with name " ) +
                                    layerName + " in source rock lithology table" );
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHI(" << m_layerName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockHI::PrmSourceRockHI( const VarPrmSourceRockHI * parent, double val, const char * layerName ) :
     m_parent( parent )
   , m_hi( val )
   , m_layerName( layerName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHI(" << m_layerName << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockHI::~PrmSourceRockHI() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockHI::setInModel( mbapi::Model & caldModel )
{
   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();
   
   // go over all source rock lithologies and check do we have HI map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   
   if ( ErrorHandler::NoError != mgr.setHIIni( m_layerName, m_hi )  ) return caldModel.moveError( mgr );
   
   return ErrorHandler::NoError;
}

// Validate HI value if it is in [0:100] range
std::string PrmSourceRockHI::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_hi < 0    ) oss << "HI value for the layer " << m_layerName << ", can not be negative: " << m_hi << std::endl;
   else if ( m_hi > 1000 ) oss << "HI value for the layer " << m_layerName << ", can not be more than 1000 kg/tonne: " << m_hi << std::endl;

   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();

   bool layerFound = false;

   // go over all source rock lithologies and check do we have HI map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( size_t i = 0; i < srIDs.size(); ++i )
   {
      if ( mgr.layerName( srIDs[i] ) == m_layerName )
      {
         layerFound = true;

         double mdlHI = mgr.hiIni( srIDs[i] );
         if ( std::fabs( mdlHI - m_hi ) > 1.e-8 ) oss << "Value of HI in the model (" << mdlHI <<
                                                  ") is different from the parameter value (" << m_hi << ")" << std::endl;
      }
   }

   if ( !layerFound ) oss << "There is no such layer in the model: " << m_layerName << std::endl;

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockHI::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockHI * pp = dynamic_cast<const PrmSourceRockHI *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-5;

   if ( m_layerName != pp->m_layerName       ) return false;
   if ( std::fabs( m_hi - pp->m_hi ) > eps ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockHI::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,      "name"      ) : ok;
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;
   ok = ok ? sz.save( m_hi,        "HI"       ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockHI::PrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,      "name" ) : ok;
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_hi,        "HI" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockHI deserialization unknown error";
   }
}

} // namespace casa
