//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxySetImpl.C
/// @brief This file keeps definitions for PRoxySet API implementation

// CASA
#include "CasaDeserializer.h"
#include "RSProxySetImpl.h"
#include "RSProxyImpl.h"

casa::RSProxySetImpl::~RSProxySetImpl()
{
   for ( ProxySet::iterator it = m_proxySet.begin(); it != m_proxySet.end(); ++it ) delete it->second;
}

std::vector< std::string > casa::RSProxySetImpl::names() const
{ 
   std::vector< std::string> ret; 
   for ( ProxySet::const_iterator it = m_proxySet.begin(); it != m_proxySet.end(); ++it )
   {
      ret.push_back( it->first );
   }
   return ret;
}


// Serialize object to the given stream
bool casa::RSProxySetImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
{
   bool ok = true;

   if ( fileVersion >= 0 ) // initial implementation
   {
      bool ok = sz.save( m_proxySet.size(), "ProxiesSetSize" );
      for ( ProxySet::const_iterator it = m_proxySet.begin(); it != m_proxySet.end() && ok; ++it )
      {
         ok = sz.save( it->first, "ProxyName" );
         ok = sz.save( *(it->second), "ProxyObj" );
      }
   }
   return ok;
}

// Create a new instance and deserialize it from the given stream
casa::RSProxySetImpl::RSProxySetImpl( CasaDeserializer & dz, const char * objName )
{
   // read from file object name and version
   std::string  objNameInFile;
   std::string  objType;
   unsigned int objVer;

   bool ok = dz.loadObjectDescription( objType, objNameInFile, objVer );
   if ( objType.compare( typeid(*this).name() ) || objNameInFile.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Deserialization error. Can not load object: " << objName;
   }

   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Version of object in file is newer. No forward compatibility!";
   }

   size_t setSize;
   ok = ok ? dz.load( setSize, "ProxiesSetSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      std::string proxyName;
      ok = dz.load( proxyName, "ProxyName" );
      
      RSProxyImpl * pr = ok ? new RSProxyImpl( dz, "ProxyObj" ) : 0;
      ok = pr ? true : false;
      if ( ok ) m_proxySet[proxyName] = pr;
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "RSProxySetImpl deserialization error";
   }
}

