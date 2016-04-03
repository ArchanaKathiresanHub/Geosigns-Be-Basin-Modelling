//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseImpl.C
/// @brief This file keeps API implementation to keep a single run of Cauldron or a single Monte Carlo point

// CASA
#include "CasaDeserializer.h"
#include "RunCaseImpl.h"
#include "ObsValue.h"
#include "Parameter.h"
#include "VarParameter.h"

// STL
#include <cstring>
#include <sstream>
#include <vector>

namespace casa
{

   // Constructor
   RunCaseImpl::RunCaseImpl()
      : m_runState( NotSubmitted )
      , m_id( 0 )
      , m_cleanDupLith( false )
   { ; }

   // Destructor
   RunCaseImpl::~RunCaseImpl()
   {
      for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[i];
      m_prmsSet.clear();
      m_results.clear();
   }

   // Get i-th parameter
   SharedParameterPtr RunCaseImpl::parameter( size_t i ) const
   {
      SharedParameterPtr nulPtr( static_cast<Parameter*>(0) );

      return i < m_prmsSet.size() ? m_prmsSet[i] : nulPtr;
   }

   // Add new parameter to the list
   void RunCaseImpl::addParameter( SharedParameterPtr prm )
   {
      for ( size_t i = 0; i < m_prmsSet.size(); ++i )
      {
         if ( m_prmsSet[i]->parent() == prm->parent() )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Parameter value for variable parameter: " <<
               prm->parent()->name()[0] << ", already exsit in RunCase";
         }
      }
      m_prmsSet.push_back( prm );
   }

   // Get i-th observable
   ObsValue * RunCaseImpl::obsValue( size_t i ) const
   {
      return i < m_results.size() ? m_results[i] : NULL;
   }

   // Add new observable to the list
   void RunCaseImpl::addObsValue( ObsValue * obs )
   {
      // doe check if we already have such type ObsValue
      for ( size_t i = 0; i < m_results.size(); ++i )
      {
         if ( m_results[i]->observable() == obs->observable() )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Observable value for obzervable: " <<
               obs->observable()->name() << ", already exsit in RunCase";
         }
      }
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
         if ( ErrorHandler::NoError != m_prmsSet[i]->setInModel( *(m_model.get()), id() ) ) 
         {
            throw ErrorHandler::Exception( m_model->errorCode() ) << m_model->errorMessage();
         }
      }

      // clean duplicated lithologies
      if ( m_cleanDupLith )
      {
         m_model->lithologyManager().cleanDuplicatedLithologies();
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
      if ( !m_model.get() ) 
      {
         if ( m_modelProjectFileName.empty() ) return "Case can not be validated because cauldron model was not defined for this case";

         // try to load model
         m_model.reset( new mbapi::Model() );
         if ( ErrorHandler::NoError != m_model->loadModelFromProjectFile( m_modelProjectFileName.c_str() ) )
         {
            return std::string( "Can't read mutated project: " ) + m_modelProjectFileName;
         }
      }

      std::ostringstream oss;
      for ( size_t i = 0; i < m_prmsSet.size(); ++i )
      {
         // simple validation for the range
         oss << m_prmsSet[i]->validate( *(m_model.get()) );
      }

      return oss.str();
   }

   // Load project file into mbapi::Model object
   mbapi::Model & RunCaseImpl::loadProject()
   {
      m_model.reset( new mbapi::Model() ); // if already having some model, drop it
      m_model->loadModelFromProjectFile( m_modelProjectFileName.c_str() );

      return *(m_model.get());
   }

   // compare parameters set for 2 cases
   bool RunCaseImpl::operator == ( const RunCase & cs ) const
   {
      const RunCaseImpl & rci = dynamic_cast<const RunCaseImpl &>( cs );

      if ( m_prmsSet.size() != rci.m_prmsSet.size() ) return false;

      for ( size_t i = 0; i < m_prmsSet.size(); ++i )
      {
         if ( *(m_prmsSet[i].get()) != *(rci.m_prmsSet[i].get()) ) return false;
      }
      return true;
   }

   bool RunCaseImpl::isEqual( const RunCase &cs, AppPipelineLevel upTo ) const
   {
      const RunCaseImpl & rci = dynamic_cast<const RunCaseImpl &>( cs );

      if ( m_prmsSet.size() != rci.m_prmsSet.size() ) return false;

      for ( size_t i = 0; i < m_prmsSet.size(); ++i )
      {
         if ( m_prmsSet[i]->appSolverDependencyLevel() > upTo ) continue; // skip parameters which not influence the given dependency level

         if ( *(m_prmsSet[i].get()) != *(rci.m_prmsSet[i].get()) ) return false;
      }
      return true;
   }

   // Serialize object to the given stream
   bool RunCaseImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization

      if ( fileVersion >= 3 )
      {
         // register run case with serializer to allow RunManager object keep reference after deserialization
         CasaSerializer::ObjRefID rcID = sz.ptr2id( this );
         ok = sz.save( rcID, "ID" );
      }

      // std::unique_ptr<mbapi::Model> m_model;
      ok = ok ? sz.save( m_modelProjectFileName, "PathToModel" ) : ok;

      // save parameters value for this case
      ok = ok ? sz.save( m_prmsSet.size(), "PrmsSetSize" ) : ok;
      for ( size_t i = 0; i < m_prmsSet.size() && ok; ++i )
      {
         ok = sz.save( *(m_prmsSet[i].get()), "CasePrm" );
      }

      // save observables value for this case
      ok = ok ? sz.save( m_results.size(), "ObsSetSize" ) : ok;
      for ( size_t i = 0; i < m_results.size() && ok; ++i )
      {
         ok = sz.save( *m_results[i], "CaseObsVal" );
      }

      ok = ok ? sz.save( static_cast<int>( m_runState ), "RunCaseState"   ) : ok;
      ok = ok ? sz.save( m_id,                           "RunCaseID"      ) : ok;

      ok = ok ? sz.save( m_cleanDupLith, "cleanDupLith" ) : ok;

      return ok;
   }

   RunCaseImpl::RunCaseImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      CasaDeserializer::ObjRefID rcID;

      // load data necessary to create an object
      ok = ok ? dz.load( rcID, "ID" ) : ok;

      // register runcase with deserializer under read ID to allow RunManager object keep reference after deserializtion
      if ( ok ) dz.registerObjPtrUnderID( this, rcID );

      // std::unique_ptr<mbapi::Model> m_model;
      ok = ok ? dz.load( m_modelProjectFileName, "PathToModel" ) : ok;

      // load parameters value for this case
      size_t setSize;
      ok = ok ? dz.load( setSize, "PrmsSetSize" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         SharedParameterPtr pptr( Parameter::load( dz, "CasePrm" ) );
         ok = pptr.get() ? true : false;
         m_prmsSet.push_back( pptr );
      }

      // load observables value for this case
      ok = ok ? dz.load( setSize, "ObsSetSize" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         ObsValue * ov = ObsValue::load( dz, "CaseObsVal" );
         ok = ov ? true : false;
         m_results.push_back( ov );
      }

      int st;
      ok = ok ? dz.load( st, "RunCaseState" ) : ok;
      m_runState = ok ? static_cast<CaseStatus>(st) : NotSubmitted;
      
      ok = ok ? dz.load( m_id, "RunCaseID" ) : ok;
      
      if ( objVer >= 1 )
      {
         ok = ok ? dz.load( m_cleanDupLith, "cleanDupLith" ) : ok;
      }
      else { m_cleanDupLith = false; }

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "RunCaseImpl deserialization error";
      }
   }
}

