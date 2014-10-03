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


#include "RSProxySetImpl.h"

#include "RSProxy.h"

casa::RSProxySetImpl::~RSProxySetImpl()
{
   for ( std::map<std::string, casa::RSProxy *>::iterator it = m_proxySet.begin(); it != m_proxySet.end(); ++it ) delete it->second;
}

std::vector< std::string > casa::RSProxySetImpl::names() const
{ 
   std::vector< std::string> ret; 
   for ( std::map< std::string, casa::RSProxy *>::const_iterator it = m_proxySet.begin(); it != m_proxySet.end(); ++it )
   {
      ret.push_back( it->first );
   }
   return ret;
}

