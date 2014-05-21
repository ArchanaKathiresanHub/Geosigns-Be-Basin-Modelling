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

// Constructor
RunCaseImpl::RunCaseImpl() {;}

// Constructor
RunCaseImpl::RunCaseImpl( const mbapi::Model & baseCase )
{
   m_model.reset( new mbapi::Model( ) );

   *( m_model.get( ) ) = baseCase; // create a deep copy of given model
}

// Destructor
RunCaseImpl::~RunCaseImpl()
{
   m_prmsSet.clear(); 
   m_results.clear();
}

// Get i-th parameter
Parameter * RunCaseImpl::parameter( size_t i ) const
{
   return i < m_prmsSet.size() ? m_prmsSet[ i ].get() : NULL;
}

// Add new parameter to the list
void RunCaseImpl::addParameter( Parameter * prm )
{
   m_prmsSet.resize( m_prmsSet.size() + 1 );
   m_prmsSet.back().reset( prm );
}

// Get i-th observable
Observable * RunCaseImpl::observable( size_t i ) const
{
   return i < m_results.size( ) ? m_results[ i ].get() : NULL;
}

// Add new observable to the list
void RunCaseImpl::addObservable( Observable * obs )
{
   m_results.resize( m_results.size() + 1 );
   m_results.back().reset( obs );
}

}
