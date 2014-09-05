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


#include "PrmSourceRockTOC.h"
#include "cmbAPI.h"

#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSourceRockTOC::PrmSourceRockTOC( mbapi::Model & mdl, const char * layerName )
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
PrmSourceRockTOC::PrmSourceRockTOC( double val, const char * layerName ) : m_toc( val ), m_layerName( layerName )
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
         if ( std::fabs( mdlTOC - m_toc ) > 1.e-8 ) oss << "Value of TOC in the model (" << mdlTOC <<
                                                  ") is different from the parameter value (" << m_toc << ")" << std::endl;
      }
   }

   if ( !layerFound ) oss << "There is no such layer in the model: " << m_layerName << std::endl;

   return oss.str();
}

}
