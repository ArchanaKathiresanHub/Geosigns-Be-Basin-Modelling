//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file RunCaseImpl.cpp
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
#include <regex>

namespace casa
{

// Constructor
RunCaseImpl::RunCaseImpl()
   : m_runState( NotCreated )
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
SharedParameterPtr RunCaseImpl::parameter( const size_t i ) const
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
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Parameter value for influential parameter: " <<
                                                                          prm->parent()->name()[0] << ", already exsit in RunCase";
      }
   }
   m_prmsSet.push_back( prm );
}

// Get i-th observable
const ObsValue * RunCaseImpl::obsValue( size_t i ) const
{
   return i < m_results.size() ? m_results[i] : NULL;
}

// Add new observable to the list
void RunCaseImpl::addObsValue( const ObsValue * obs )
{
   // doe check if we already have such type ObsValue
   for ( size_t i = 0; i < m_results.size(); ++i )
   {
      if ( m_results[i]->parent() == obs->parent() )
      {
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Observable value for obzervable: " <<
                                                                          obs->parent()->name() << ", already exsit in RunCase";
      }
   }
   m_results.push_back( obs );
}

// Mutate case to given project file
void RunCaseImpl::mutateCaseTo( mbapi::Model & baseCase, const char * newProjectName, const bool saveMemberModel )
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
   mbapi::Model* model = new mbapi::Model();
   if ( ErrorHandler::NoError != model->loadModelFromProjectFile( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't read mutated project: " << newProjectName;
   }

   // store the project file name
   m_modelProjectFileName = newProjectName;

   // apply mutations
   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      if ( ErrorHandler::NoError != m_prmsSet[i]->setInModel( *model, id() ) )
      {
         throw ErrorHandler::Exception( model->errorCode() ) << model->errorMessage();
      }
   }

   // clean duplicated lithologies
   if ( m_cleanDupLith )
   {
      model->lithologyManager().cleanDuplicatedLithologies();
   }

   // write mutated project to the file
   if ( ErrorHandler::NoError != model->saveModelToProjectFile( newProjectName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can't write mutated project: " << newProjectName;
   }

   setRunStatus( NotSubmitted );

   if (saveMemberModel)
   {
      m_model.reset(model);
   }
   else
   {
      delete model;
   }

}

// Do checking, are all influential parameters case value in their ranges
std::string RunCaseImpl::validateCase()
{
   if ( !m_model.get() )
   {
      if ( m_modelProjectFileName.empty() ) return "Case can not be validated because cauldron model was not defined for this case";

      // try to load model
      m_model.reset( new mbapi::Model() );
      if ( ErrorHandler::NoError != m_model->loadModelFromProjectFile( m_modelProjectFileName ) )
      {
         return std::string( "Can't read mutated project: " ) + m_modelProjectFileName;
      }
   }

   std::ostringstream oss;
   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      // simple validation for the range
      oss << m_prmsSet[i]->validate( *m_model );
   }

   return oss.str();
}

// Load project file into mbapi::Model object
mbapi::Model & RunCaseImpl::loadProject()
{
   m_model.reset( new mbapi::Model() ); // if already having some model, drop it
   m_model->loadModelFromProjectFile( m_modelProjectFileName );

   return *m_model;
}

std::shared_ptr<RunCase> RunCaseImpl::shallowCopy() const
{
   std::shared_ptr<RunCase> shallowCopy(new RunCaseImpl());
   for ( size_t k = 0; k < parametersNumber(); ++k )
   {
      shallowCopy->addParameter( parameter( k ) );
   }
   return shallowCopy;
}

// compare parameters set for 2 cases
bool RunCaseImpl::operator == ( const RunCase & cs ) const
{
   if ( m_prmsSet.size() != cs.parametersNumber() ) return false;

   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      if ( *m_prmsSet[i] != *cs.parameter(i) ) return false;
   }
   return true;
}

bool RunCaseImpl::isEqual( const RunCase &cs, AppPipelineLevel upTo ) const
{
   if ( m_prmsSet.size() != cs.parametersNumber() ) return false;

   for ( size_t i = 0; i < m_prmsSet.size(); ++i )
   {
      if ( m_prmsSet[i]->appSolverDependencyLevel() > upTo ) continue; // skip parameters which not influence the given dependency level

      if ( *m_prmsSet[i] != *cs.parameter(i) ) return false;
   }
   return true;
}

// Serialize object to the given stream
bool RunCaseImpl::save( CasaSerializer & sz ) const
{
   bool ok = true;

   // register run case with serializer to allow RunManager object keep reference after deserialization
   CasaSerializer::ObjRefID rcID = sz.ptr2id( this );
   ok = sz.save( rcID, "ID" );


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

   //Runcases without project are supported, but loading with empty file name leads to an impropertie initialized m_model with would give trouble.
   if (!m_modelProjectFileName.empty())
   {
      ok = ok ? loadProject().errorCode() == ErrorHandler::ReturnCode::NoError : ok;
   }

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

std::vector<const casa::ObsValue*> RunCaseImpl::getDoubleObsValues() const
{
   std::vector<const casa::ObsValue*> doubleObservables;

   for (size_t j = 0; j < observablesNumber(); ++j)
   {
      const casa::ObsValue* obv = obsValue(j);

      if (!obv->isDouble())
      {
         continue;
      }
      doubleObservables.push_back(obv);
   }
   return doubleObservables;
}

std::vector<double> RunCaseImpl::extractObservableValues() const
{
   std::vector<double> result;
   std::vector<const casa::ObsValue*> doubleObsValues = getDoubleObsValues();
   for (auto obv : doubleObsValues)
   {
      const std::vector<double>& vals = obv->asDoubleArray();
      result.insert(result.end(), vals.begin(), vals.end());
   }
   return result;
}

std::vector<std::string> RunCaseImpl::extractObservableNames() const
{
   std::vector<std::string> obsNames;
   std::vector<const casa::ObsValue*> doubleObsValues = getDoubleObsValues();
   for (auto obv : doubleObsValues)
   {
      const std::vector<std::string>& names = obv->parent()->name();
      obsNames.insert(obsNames.end(),names.begin(),names.end());
   }
   return obsNames;
}

std::vector<std::string> RunCaseImpl::extractParameterNames() const
{
   std::vector<std::string> paramNames;
   for (size_t j = 0; j < parametersNumber(); ++j)
   {
      const casa::Parameter* prm = parameter(j).get();
      std::vector<std::string> names = prm->parent()->name();

      //Remove spaces:
      for (auto& s : names)
      {
         s = std::regex_replace(s, std::regex(" "), "_");
      }

      paramNames.insert(paramNames.end(),names.begin(),names.end());
   }
   return paramNames;
}

std::vector<double> RunCaseImpl::extractParametersValuesAsDouble() const
{
   std::vector<double> result;
   for (size_t j = 0; j < parametersNumber(); ++j)
   {
      const casa::Parameter* prm = parameter(j).get();

      if (!prm || !prm->parent()) continue;

      switch (prm->parent()->variationType())
      {
      case casa::VarParameter::Continuous:
      case casa::VarParameter::Discrete:
      {
         const std::vector<double>& vals = prm->asDoubleArray();
         result.insert(result.end(), vals.begin(), vals.end());
      }
         break;

      case casa::VarParameter::Categorical:
         result.push_back(prm->asInteger());
         break;

      default: assert(false); break;
      }
   }
   return result;
}
}

