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
#include "ObsValue.h"

#include <cstring>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
RunCaseImpl::RunCaseImpl() {;}

// Destructor
RunCaseImpl::~RunCaseImpl()
{
   for ( size_t i = 0; i < m_prmsSet.size(); ++i ) delete m_prmsSet[i];
   for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[i];
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
ObsValue * RunCaseImpl::observableValue( size_t i ) const
{
   return i < m_results.size( ) ? m_results[ i ] : NULL;
}

// Add new observable to the list
void RunCaseImpl::addObservableValue( ObsValue * obs )
{
   m_results.push_back( obs );
}

// Mutate case to given project file
void RunCaseImpl::mutateCaseTo( mbapi::Model & baseCase, const char * newProjectName )
{
   if ( !newProjectName || !strlen( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::MutationError ) << "Mutated project file name is undefined";
   }

   // save base case as a new project with given name
   if ( ErrorHandler::NoError != baseCase.saveModelToProjectFile( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't write mutated project: " << newProjectName;
   }

   // create the new one
   m_model.reset( new mbapi::Model() );
   if ( ErrorHandler::NoError != m_model->loadModelFromProjectFile( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't read mutated project: " << newProjectName;
   }

   // store the project file name
   m_modelProjectFileName = newProjectName;

   // apply mutations
   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      m_prmsSet[ i ]->setInModel( *( m_model.get() ) );
   }

   // write mutated project to the file
   if ( ErrorHandler::NoError != m_model->saveModelToProjectFile( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't write mutated project: " << newProjectName;
   }
}

// Do checking, are all variable parameters case value in their ranges
std::string RunCaseImpl::validateCase()
{
   if ( !m_model.get() ) return "Case can not be validated because cauldron model was not defined for this case";

   std::ostringstream oss;
   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      // simple validation for the range
      oss << m_prmsSet[ i ]->validate( *( m_model.get() ) );
   }

   return oss.str();
}

// Load project file into mbapi::Model object
mbapi::Model * RunCaseImpl::loadProject()
{
   m_model.reset( new mbapi::Model( ) ); // if already having some model, drop it
   m_model->loadModelFromProjectFile( m_modelProjectFileName.c_str() );

   return m_model.get();
}

}
