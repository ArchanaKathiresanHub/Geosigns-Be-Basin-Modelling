//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file cmbAPI.C
/// @brief This file keeps API definition for creating Cauldron data model

#include "cmbAPI.h"

// CMB API
#include "LithologyManagerImpl.h"
#include "StratigraphyManagerImpl.h"
#include "FluidManagerImpl.h"
#include "SourceRockManagerImpl.h"

// DataAccess library
#include "Interface/ProjectHandle.h"

#include <stdexcept>
#include <string>
#include <sstream>

namespace mbapi {

///////////////////////////////////////////////////////////////////////////////
// Class which hides all CMB API implementation
class Model::ModelImpl
{
public:
   // constructor / destructor
   ModelImpl();
   ~ModelImpl(); 

   ModelImpl & operator = ( const ModelImpl & otherModel );

   // methods
   // IO methods
   void loadModelFromProjectFile( const char * projectFileName );
   void saveModelToProjectFile(   const char * projectFileName );

   // Lithology
   LithologyManager    & getLithologyManager() { return m_lythMgr; }
   // Stratigraphy
   StratigraphyManager & getStratigraphyManager() { return m_stratMgr; }
   // Fluid
   FluidManager        & getFluidManager() { return m_fluidMgr; }
   // Source Rock
   SourceRockManager   & getSourceRockManager() { return m_srkMgr; }

   // data members
   LithologyManagerImpl    m_lythMgr;
   StratigraphyManagerImpl m_stratMgr;
   FluidManagerImpl        m_fluidMgr;
   SourceRockManagerImpl   m_srkMgr;

   DataAccess::Interface::ProjectHandle * m_projHandle;
   std::string                            m_projFileName;
};


///////////////////////////////////////////////////////////////////////////////
// Set of Model wrapper functions to hide the actual implementation from .h
Model::Model() { m_pimpl.reset( new ModelImpl() ); }
Model::Model( const Model & otherModel ) {}
Model::~Model() { m_pimpl.reset( 0 ); }

Model & Model::operator = ( const Model & otherModel )
{
   *(m_pimpl.get()) = *(otherModel.m_pimpl.get());
   return *this;
}

Model::ReturnCode Model::loadModelFromProjectFile( const char * projectFileName )
{
   try { m_pimpl->loadModelFromProjectFile( projectFileName ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::saveModelToProjectFile( const char * projectFileName )
{
   try { m_pimpl->saveModelToProjectFile( projectFileName ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

LithologyManager    & Model::getLithologyManager(   ) { return m_pimpl->getLithologyManager(   ); }
StratigraphyManager & Model::getStratigraphyManager() { return m_pimpl->getStratigraphyManager(); }
FluidManager        & Model::getFluidManager(       ) { return m_pimpl->getFluidManager(       ); }
SourceRockManager   & Model::getSourceRockManager(  ) { return m_pimpl->getSourceRockManager(  ); }

///////////////////////////////////////////////////////////////////////////////
// Actual implementation of CMB API

Model::ModelImpl::ModelImpl()
{
   m_projHandle = NULL;
}

Model::ModelImpl::~ModelImpl( )
{
   if ( m_projHandle )
   {
      DataAccess::Interface::CloseCauldronProject( m_projHandle );
   }
}

// Load model from the project file
Model::ModelImpl & Model::ModelImpl::operator = ( const Model::ModelImpl & otherModel )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

void Model::ModelImpl::loadModelFromProjectFile( const char * projectFileName )
{
   m_projHandle = DataAccess::Interface::OpenCauldronProject( projectFileName, "r" );
   if ( !m_projHandle )
   {
      std::ostringstream oss;
      oss << "Model::loadModelFromProjectFile() failed to load " << projectFileName;
      throw std::runtime_error( oss.str() );
   }
   m_projFileName = projectFileName;
}

// Save model to the project file
void Model::ModelImpl::saveModelToProjectFile( const char * projectFileName )
{
   if ( m_projHandle )
   {
      if ( !m_projHandle->saveToFile( projectFileName ) )
      {
         std::ostringstream oss;
         oss << "Model::saveModelToProjectFile() failed to save to " << projectFileName << " project file";
         throw std::runtime_error( oss.str() );
      }
   }
   else
   {
      throw std::runtime_error( "Model::saveModelToProjectFile(): no project to save" );
   }
}

}

