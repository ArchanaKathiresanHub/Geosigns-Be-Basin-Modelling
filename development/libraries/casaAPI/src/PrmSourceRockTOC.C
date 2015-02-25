//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockTOC.C
/// @brief This file keeps API implementation for Source Rock TOC parameter handling 

// CASA API
#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

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

// Constructor
PrmSourceRockTOC::PrmSourceRockTOC( mbapi::Model & mdl, const char * layerName ) : m_parent( 0 )
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
         m_toc = mgr.tocIni( srIDs[i] );
         if ( ErrorHandler::NoError != mgr.errorCode() ) mdl.moveError( mgr );
         isFound = true;
         break;
      }
   }
   if ( !isFound ) mdl.reportError( ErrorHandler::NonexistingID, std::string( "Can't find layer with name " ) +
                                    layerName + " in source rock lithology table" );
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockTOC::PrmSourceRockTOC( const VarPrmSourceRockTOC * parent, double val, const char * layerName ) :
     m_parent( parent )
   , m_toc( val )
   , m_layerName( layerName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel )
{
   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();
   
   // go over all source rock lithologies and check do we have TOC map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( size_t i = 0; i < srIDs.size(); ++i )
   {
      if ( mgr.layerName( srIDs[i] ) == m_layerName )
      {
         const std::string & mapName = mgr.tocInitMapName( srIDs[i] );
         if ( !mapName.empty() )
         {
            std::ostringstream oss;
            oss << "Source rock lithology with ID " << srIDs[i] << " has TOC already defined as a map";
            return caldModel.reportError( ErrorHandler::AlreadyDefined, oss.str() );
         }
         else if ( ErrorHandler::NoError != mgr.errorCode() ) return caldModel.moveError( mgr );
      }
   }
   
   ErrorHandler::ReturnCode ret = mgr.setTOCIni( m_layerName, m_toc );

   if ( ErrorHandler::NoError != ret ) return caldModel.moveError( mgr );
   
   return ErrorHandler::NoError;
}

// Validate TOC value if it is in [0:100] range
std::string PrmSourceRockTOC::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_toc < 0   ) oss << "TOC value for the layer " << m_layerName << ", can not be negative: " << m_toc << std::endl;
   else if ( m_toc > 100 ) oss << "TOC value for the layer " << m_layerName << ", can not be more than 100%: " << m_toc << std::endl;

   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();

   bool layerFound = false;

   // go over all source rock lithologies and check do we have TOC map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( size_t i = 0; i < srIDs.size(); ++i )
   {
      if ( mgr.layerName( srIDs[i] ) == m_layerName )
      {
         layerFound = true;

         const std::string & mapName = mgr.tocInitMapName( srIDs[i] );
         if ( !mapName.empty() )
         {
            oss << "Source rock lithology with ID " << srIDs[i] << "for the layer " << m_layerName <<
                   " has TOC already defined as a map" << std::endl;
         }
         else if ( ErrorHandler::NoError != mgr.errorCode() ) oss << mgr.errorCode() << std::endl;
         
         double mdlTOC = mgr.tocIni( srIDs[i] );
         if ( !NumericFunctions::isEqual( mdlTOC, m_toc, 1.e-4 ) )
         {
            oss << "Value of TOC in the model (" << mdlTOC << ") is different from the parameter value (" << m_toc << ")" << std::endl;
         }
      }
   }

   if ( !layerFound ) oss << "There is no such layer in the model: " << m_layerName << std::endl;

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockTOC::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockTOC * pp = dynamic_cast<const PrmSourceRockTOC *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_layerName != pp->m_layerName ) return false;

   if ( !NumericFunctions::isEqual( m_toc, pp->m_toc, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockTOC::save( CasaSerializer & sz, unsigned int version ) const
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
   ok = ok ? sz.save( m_toc,       "toc"       ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockTOC::PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name, "name" ) : ok;
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_toc, "toc" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockTOC deserialization unknown error";
   }
}

} // namespace casa
