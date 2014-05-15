//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCase.C
/// @brief This file keeps API implementation to keep a single run of Cauldron or a single Monte Carlo point

#include "RunCaseImpl.h"

#include <vector>

namespace casa
{

RunCaseImpl::RunCaseImpl()
{
   ;
}

RunCaseImpl::RunCaseImpl( const mbapi::Model & baseCase )
{
   m_model.reset( new mbapi::Model( ) );

   *( m_model.get( ) ) = baseCase; // create a deep copy of given model
}

RunCaseImpl::~RunCaseImpl()
{
   for ( size_t i = 0; i < m_prmsSet.size( ); ++i ) delete m_prmsSet[i];
   m_prmsSet.clear();
  
   for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[ i ];
   m_results.clear();
}


}
