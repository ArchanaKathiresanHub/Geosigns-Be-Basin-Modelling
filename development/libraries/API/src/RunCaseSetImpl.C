//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseSetImpl.C
/// @brief This file keeps definitions for RunCaseSet API implementation

#include "RunCaseSetImpl.h"
#include "RunCaseImpl.h"

#include <utility>

namespace casa
{

// Destructor
RunCaseSetImpl::~RunCaseSetImpl()
{
   for ( size_t i = 0; i < m_caseSet.size(); ++i ) delete m_caseSet[i];
   m_caseSet.clear();
}

// Get number of cases
size_t RunCaseSetImpl::size() const
{ 
   return m_filter.empty() ? m_caseSet.size() : m_expIndSet.size();
}


// Read only access to i-th element
const RunCase * RunCaseSetImpl::operator[] ( size_t i ) const
{
   if ( !m_filter.empty() )
   {
      return i < m_expIndSet.size() ? m_caseSet[ m_expIndSet[ i ] ] : NULL;
   }

   return i < m_caseSet.size() ? m_caseSet[ i ] : NULL;
}
      

// Set filter for experiments by experiment name
void RunCaseSetImpl::filterByExperimentName( const std::string expName )
{
   // reset previous filter any case
   m_expIndSet.clear();
   m_filter = expName;

   std::map< std::string, std::vector<size_t> >::iterator ret;

   // check first do we have such experiment?
   ret = m_expSet.find( expName );
   if ( ret != m_expSet.end() )
   {
      m_expIndSet.assign( ret->second.begin(), ret->second.end() );
   }
}

// Get all experiment names for this case set as an array
std::vector< std::string > RunCaseSetImpl::experimentNames( )
{
   std::vector<std::string> expSet;

   // loop over all experiments and collect names
   for ( std::map<std::string, std::vector<size_t> >::iterator it = m_expSet.begin(); it != m_expSet.end(); ++it )
   {
      expSet.push_back( it->first );
   }

   return expSet;
}

// Access to i-th element
RunCase * RunCaseSetImpl::at( size_t i )
{
   if ( !m_filter.empty( ) )
   {
      return i < m_expIndSet.size( ) ? m_caseSet[ m_expIndSet[ i ] ] : NULL;
   }

   return i < m_caseSet.size( ) ? m_caseSet[ i ] : NULL;
}
 
// Move a new Cases to the collection and clear array 
void RunCaseSetImpl::addNewCases( std::vector<RunCase*> & newCases, const std::string & expLabel )
{

   if ( m_expSet.count( expLabel ) > 0 )
   {
      throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Experiment with name:" << expLabel << "already exist in the set";
   }
   else
   {
      std::vector<size_t> newIndSet( newCases.size() );
      
      size_t pos = m_caseSet.size(); // position of new inserted elements
      m_caseSet.insert( m_caseSet.end(), newCases.begin(), newCases.end() );

      // add experiment indexes to experiments set
      for ( size_t i = 0; i < newCases.size(); ++i )
      {
         newIndSet[ i ] = i + pos;
      }
      m_expSet[ expLabel ] = newIndSet;

      newCases.clear( ); // clean container
   }
}

}
