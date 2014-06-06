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


#include "SimpleRange.h"

#include "PrmSourceRockTOC.h"
#include "cmbAPI.h"

#include <cassert>
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
    for ( std::vector<mbapi::SourceRockManager::SourceRockID>::const_iterator it = srIDs.begin(); it != srIDs.end(); ++it )
    {
       if ( mgr.layerName( *it ) == m_layerName )
       {
          m_toc = mgr.tocIni( *it );
          if ( ErrorHandler::NoError != mgr.errorCode() ) mdl.moveError( mgr );
          isFound = true;
          break;
       }
    }
    if ( !isFound ) mdl.reportError( ErrorHandler::NonexistingID, std::string( "Can't find layer with name " ) +
                                     layerName + " in source rock lithology table" );
 }

 // Constructor
PrmSourceRockTOC::PrmSourceRockTOC( double val, const char * layerName ) : m_toc( val ), m_layerName( layerName ) {;}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel )
{
   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();
   
   // go over all source rock lithologies and check do we have TOC map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( std::vector<mbapi::SourceRockManager::SourceRockID>::const_iterator it = srIDs.begin(); it != srIDs.end(); ++it )
   {
      if ( mgr.layerName( *it ) == m_layerName )
      {
         const std::string & mapName = mgr.tocInitMapName( *it );
         if ( !mapName.empty() )
         {
            std::ostringstream oss;
            oss << "Source rock lithology with ID " << *it << " has TOC already defined as a map";
            return caldModel.reportError( ErrorHandler::AlreadyDefined, oss.str() );
         }
         else if ( ErrorHandler::NoError != mgr.errorCode() ) return caldModel.moveError( mgr );
      }
   }
   
   ErrorHandler::ReturnCode ret = mgr.setTOCIni( m_layerName, m_toc );

   if ( ErrorHandler::NoError != ret ) return caldModel.moveError( mgr );
   
   return ErrorHandler::NoError;
}

}
