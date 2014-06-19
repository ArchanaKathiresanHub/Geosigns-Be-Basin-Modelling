//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseImpl.C
/// @brief This file keeps API implementation to keep a single run of Cauldron or a single Monte Carlo point

#include "RunCaseImpl.h"

#include <vector>
#include <stdexcept>
#include <cstring>

namespace casa
{

// Constructor
RunCaseImpl::RunCaseImpl() : m_baseCaseModel( NULL ) {;}

// Constructor
RunCaseImpl::RunCaseImpl( mbapi::Model & baseCase ) : m_baseCaseModel( &baseCase )
{
}

// Destructor
RunCaseImpl::~RunCaseImpl()
{
   for ( std::vector<Parameter*>::iterator it = m_prmsSet.begin(); it != m_prmsSet.end(); ++it ) delete *it;
   for ( std::vector<Observable*>::iterator it = m_results.begin(); it != m_results.end(); ++it ) delete *it;
   m_prmsSet.clear(); 
   m_results.clear();
}

// Get i-th parameter
Parameter * RunCaseImpl::parameter( size_t i ) const
{
   return i < m_prmsSet.size() ? m_prmsSet[ i ] : NULL;
}

// Add new parameter to the list
void RunCaseImpl::addParameter( Parameter * prm )
{
   m_prmsSet.push_back( prm );
}

// Get i-th observable
Observable * RunCaseImpl::observable( size_t i ) const
{
   return i < m_results.size( ) ? m_results[ i ] : NULL;
}

// Add new observable to the list
void RunCaseImpl::addObservable( Observable * obs )
{
   m_results.push_back( obs );
}

// Mutate case to given project file
void RunCaseImpl::mutateCaseTo( const char * newProjectName )
{
   if ( !newProjectName || !std::strlen( newProjectName ) ) throw std::runtime_error( "Mutated project file name undefined" );

   // save base case as a new project with given name
   if ( !m_baseCaseModel ) throw std::runtime_error( "Base case wasn't defined" );
   if ( ErrorHandler::NoError != m_baseCaseModel->saveModelToProjectFile( newProjectName ) )
   {
      throw std::runtime_error( std::string( "Can't write mutated project: " ) + newProjectName );
   }

   // create the new one
   m_model.reset( new mbapi::Model() );
   if ( ErrorHandler::NoError != m_model->loadModelFromProjectFile( newProjectName ) )
   {
      throw std::runtime_error( std::string( "Can't read mutated project: " ) + newProjectName );
   }

   // apply mutations
   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      m_prmsSet[ i ]->setInModel( *(m_model.get()) );
   }

   // write mutated project to the file
   if ( ErrorHandler::NoError != m_model->saveModelToProjectFile( newProjectName ) )
   {
      throw std::runtime_error( std::string( "Can't write mutated project: " ) + newProjectName );
   }
}


}
