//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file cusaAPI.C 
/// This file keeps API definition for creating ScenarioAnalysis

#include "casaAPI.h"
#include "cmbAPI.h"

#include "DataDiggerImpl.h"
#include "DoEGeneratorImpl.h"
#include "MCSolverImpl.h"
#include "RSProxyImpl.h"
#include "RunCaseSetImpl.h"
#include "RunManagerImpl.h"
#include "VarSpaceImpl.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"

#include <stdexcept>
#include <string>
#include <cassert>

namespace casa {

///////////////////////////////////////////////////////////////////////////////
// Set of business logic rules functions to convert one request to set of parameters
namespace BusinessLogicRulesSet
{
// Add a parameter to variate layer thickness value [m] in given range
// theModel a base case Cauldron model
// layerName name of the layer in base case model to variate it thickness
// minVal the minimal range value 
// maxVal the maximal range value 
// rangeShape defines a type of probability function for the parameter. If PDF needs some middle parameter value it will be 
// taken from the base case model
// varPrmsSet VarSpace manager where the new variable parameter will be placed. In case of an error this object will keep an error message
// return ErrorHandler::NoError on success or error code otherwise
ErrorHandler::ReturnCode VariateLayerThickness( const mbapi::Model & theModel
                                              , const char * layerName
                                              , double minVal
                                              , double maxVal
                                              , ContinuousParameter::PDF rangeShape
                                              , VarSpace & varPrmsSet
                                              )
{
   return ErrorHandler::NotImplementedAPI;
}

// Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
// theModel a base case Cauldron model
// minVal the minimal range value 
// maxVal the maximal range value 
// rangeShape defines a type of probability function for the parameter. If PDF needs some middle parameter value it will be 
// taken from the base case model
// varPrmsSet VarSpace manager where the new variable parameter will be placed. In case of an error this object will keep an error message
// @return ErrorHandler::NoError on success or error code otherwise
ErrorHandler::ReturnCode VariateTopCrustHeatProduction( const mbapi::Model & theModel
                                                      , double minVal
                                                      , double maxVal
                                                      , ContinuousParameter::PDF rangeShape
                                                      , VarSpace & varPrmsSet
                                                      )
{
   /// TODO - get base value of parameter from the Model
   return varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( 0.5*( minVal + maxVal ), minVal, maxVal, rangeShape ) );
}

// Add a parameter to variate source rock lithology TOC value @f$ [%%] @f$ in given range
// theModel a base case Cauldron model
// minVal the minimal range value 
// maxVal the maximal range value 
// rangeShape defines a type of probability function for the parameter. If PDF needs some middle parameter value it will be\n
// taken from the base case model
// varPrmsSet VarSpace manager where the new variable parameter will be placed. In case of an error this object will keep\n an error message
// @return ErrorHandler::NoError on success or error code otherwise
ErrorHandler::ReturnCode VariateSourceRockTOC( const mbapi::Model & theModel
                                             , const char * srLithoType
                                             , double minVal
                                             , double maxVal
                                             , ContinuousParameter::PDF rangeShape
                                             , VarSpace & varPrmsSet
                                             )
{
   /// TODO - get base value of parameter from the Model
   return varPrmsSet.addParameter( new VarPrmSourceRockTOC( srLithoType, 0.5*( minVal + maxVal ), minVal, maxVal, rangeShape ) );
}

}





// Class which hides all ScenarioAnalysis implementation
class ScenarioAnalysis::ScenarioAnalysisImpl
{
public:
   // Constructor / destructor
   ScenarioAnalysisImpl();

   ~ScenarioAnalysisImpl();

   // Define a base case for scenario analysis from model, makes deep copy of the model
   void defineBaseCase( const mbapi::Model & bcModel );
   
   // Define a base case for scenario analysis from file.
   void defineBaseCase( const char * projectFileName ); 

   // Get base case model if it was set, empty model otherwise
   mbapi::Model & baseCase();

   // Provide variable parameters set manager
   VarSpace & varSpace() { return *(m_varSpace.get()); }
   
   // Define DoE algorithm. This function should be called before accessing to the instance of DoEGenerator
   // algo Type of DoE algorithm
   // return ErrorHandler::NoError in case of success, or error code otherwise
   void setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo );
   
   // Get reference to instance of DoE generator which will be used in scenario analysis. If algorithm
   // wasn't set before by ScenarioAnalysis::setDoEAlgorithm(), the DoEGenerator::Tornado algorithm will be chosen.
   // return reference to the DoEGenerator instance.
   DoEGenerator & doeGenerator();

   // Get all cases for this scenario. The list will include cases generated by DoE only
   // return reference to casa::RunCaseSet object
   RunCaseSet & doeCaseSet() { return *( m_doeCases.get() ); }

   // Get all cases for this scenario. The list will include cases generated by MC/MCMC only
   // return array of casa::Case objects
   RunCaseSet & mcCaseSet() { return *( m_mcCases.get() ); }

   // Create copy of the base case and set all variable parameters value defined for this case
   // cs casa::RunCase object
   // return ErrorHandler::NoError
   ErrorHandler::ReturnCode applyMutation( RunCase & cs );

   // Validate Cauldron model for consistency and valid parameters range. This function should be 
   //  called after ScenarioAnalysis::applyMutation()
   // cs casa::RunCase object to be validated
   // return ErrorHandler::NoError on success or error code otherwise
   ErrorHandler::ReturnCode validateCase( RunCase & cs );

   // Get run manager associated with this scenario analysis
   // return reference to the instance of run manager
   RunManager & runManager() { return *(m_runManager.get()); }

   // Get data digger associated with this scenario analysis
   // return reference to the instance of data digger
   DataDigger & dataDigger() { return *(m_dataDigger.get()); }
   
   // Define which order of response surface polynomial approximation of  will be used in this scenario analysis
   // order order of polynomial approximation
   // krType do we need Kriging interpolation, and which one?
   void setRSAlgorithm( int order, RSProxy::RSKrigingType krType ) { throw std::runtime_error( "setRSAlgorithm() not implemented yet" ); }
   
   // Get response surface proxy 
   // return reference to proxy object
   RSProxy & responseSurfaceProxy() { return *(m_rsProxy.get()); }

   // Define type of Monte Carlo algorithm which will be used in this scenario analysis
   // algo Monte Carlo algorithm
   // interp do we need Kriging interpolation? If yes, the response surface proxy must also use it.
   // return ErrorHandler::NoError on success or error code otherwise
   ErrorHandler::ReturnCode setMCAlgorithm( MCSolver::MCAlgorithm algo, MCSolver::MCKrigingType interp = MCSolver::NoKriging )
   { throw std::runtime_error( "setMCAlgorithm() not implemented yet" ); }
   
   // Get Monte Carlo solver
   // return reference to Monte Carlo solver. If MC solver algorithm wasn't defined befor by ScenarioAnalysis::setMCAlgorithm(), it
   //         will be set up to MC with no Kriging by default.
   MCSolver & mcSolver()  { return *(m_mcSolver.get()); }
      
private:
   std::auto_ptr<mbapi::Model>     m_baseCase;
   std::auto_ptr<VarSpaceImpl>     m_varSpace;
   std::auto_ptr<DoEGeneratorImpl> m_doe;
   
   std::auto_ptr<RunCaseSetImpl>   m_doeCases;
   std::auto_ptr<RunCaseSetImpl>   m_mcCases;

   std::auto_ptr<RunManager>       m_runManager;
   std::auto_ptr<DataDigger>       m_dataDigger;
   std::auto_ptr<RSProxy>          m_rsProxy;
   std::auto_ptr<MCSolver>         m_mcSolver;
};



///////////////////////////////////////////////////////////////////////////////
// Set of ScenarioAnalysis wrapper functions to hide the actual implementation from .h

ScenarioAnalysis::ScenarioAnalysis()  { m_pimpl.reset( new ScenarioAnalysisImpl() ); }
ScenarioAnalysis::~ScenarioAnalysis() { m_pimpl.reset( 0 ); }

ErrorHandler::ReturnCode ScenarioAnalysis::defineBaseCase( const mbapi::Model & bcModel )
{
   try { m_pimpl->defineBaseCase( bcModel ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

ErrorHandler::ReturnCode ScenarioAnalysis::defineBaseCase( const char * projectFileName )
{
   try { m_pimpl->defineBaseCase( projectFileName ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

mbapi::Model & ScenarioAnalysis::baseCase() { return m_pimpl->baseCase(); }
VarSpace   & ScenarioAnalysis::varSpace()   { return m_pimpl->varSpace(); }
RunCaseSet & ScenarioAnalysis::doeCaseSet() { return m_pimpl->doeCaseSet(); }
RunCaseSet & ScenarioAnalysis::mcCaseSet()  { return m_pimpl->mcCaseSet( ); }

ErrorHandler::ReturnCode ScenarioAnalysis::setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo )
{
   try { m_pimpl->setDoEAlgorithm( algo ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( AlreadyDefined, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

DoEGenerator & ScenarioAnalysis::doeGenerator() { return m_pimpl->doeGenerator(); }

ErrorHandler::ReturnCode ScenarioAnalysis::applyMutation( RunCase & cs )
{
   try { m_pimpl->applyMutation( cs ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( MutationError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

ErrorHandler::ReturnCode ScenarioAnalysis::validateCase( RunCase & cs )
{
   try { m_pimpl->validateCase( cs ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( ValidationError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

RunManager & ScenarioAnalysis::runManager() { return m_pimpl->runManager(); }
DataDigger & ScenarioAnalysis::dataDigger() { return m_pimpl->dataDigger(); }

RSProxy & ScenarioAnalysis::responseSurfaceProxy() { return m_pimpl->responseSurfaceProxy(); }

ErrorHandler::ReturnCode ScenarioAnalysis::setRSAlgorithm( int order, RSProxy::RSKrigingType krType )
{
   try { m_pimpl->setRSAlgorithm( order, krType ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( MCSolverError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

MCSolver & ScenarioAnalysis::mcSolver()  { return m_pimpl->mcSolver(); }

ErrorHandler::ReturnCode ScenarioAnalysis::setMCAlgorithm( MCSolver::MCAlgorithm algo, MCSolver::MCKrigingType interp )
{
   try { m_pimpl->setMCAlgorithm( algo, interp ); }
   catch( std::exception & ex ) { return this->ErrorHandler::ReportError( RSProxyError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::ReportError( UnknownError, "Unknown error" ); }

   return NoError;
}

///////////////////////////////////////////////////////////////////////////////
// The actual implementation of CASA API

ScenarioAnalysis::ScenarioAnalysisImpl::ScenarioAnalysisImpl()
{
   m_varSpace.reset(   new VarSpaceImpl()   );

   m_doeCases.reset(   new RunCaseSetImpl() );
   m_mcCases.reset(    new RunCaseSetImpl() );

   m_runManager.reset( new RunManagerImpl() );
   m_dataDigger.reset( new DataDiggerImpl() );
   m_rsProxy.reset(    new RSProxyImpl()    );
   m_mcSolver.reset(   new MCSolverImpl()   );
}

ScenarioAnalysis::ScenarioAnalysisImpl::~ScenarioAnalysisImpl()
{
}

void ScenarioAnalysis::ScenarioAnalysisImpl::defineBaseCase( const mbapi::Model & bcModel )
{
   throw std::runtime_error( "defineBaseCase() is not implemented yet" );
}

void ScenarioAnalysis::ScenarioAnalysisImpl::defineBaseCase( const char * projectFileName )
{
   if ( m_baseCase.get() ) { throw std::runtime_error( "defineBaseCase(): Base case is already defined" ); }
   
   m_baseCase.reset( new mbapi::Model() );
   if ( NoError != m_baseCase->loadModelFromProjectFile( projectFileName ) )
   {
      throw std::runtime_error( std::string( "defineBaseCase() can not load model from " ) + projectFileName );
   }
}

mbapi::Model & ScenarioAnalysis::ScenarioAnalysisImpl::baseCase()
{
   if ( !m_baseCase.get() ) m_baseCase.reset( new mbapi::Model() );
   return *( m_baseCase.get() );
}

void ScenarioAnalysis::ScenarioAnalysisImpl::setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo )
{
   if ( m_doe.get() ) throw std::runtime_error( "Algorithm is already set" );

   m_doe.reset( new DoEGeneratorImpl( *(m_baseCase.get()), algo ) );
}

DoEGenerator & ScenarioAnalysis::ScenarioAnalysisImpl::doeGenerator()
{
   assert( m_baseCase.get() );

   if ( !m_doe.get() )
   {
      m_doe.reset( new DoEGeneratorImpl( *(m_baseCase.get()), DoEGenerator::Tornado ) );
   }

   return *(m_doe.get());
}

ErrorHandler::ReturnCode ScenarioAnalysis::ScenarioAnalysisImpl::applyMutation( RunCase & cs )
{
   throw std::runtime_error( "applyMutation() is not implemented yet" );
}

ErrorHandler::ReturnCode ScenarioAnalysis::ScenarioAnalysisImpl::validateCase( RunCase & cs )
{
   throw std::runtime_error( "validateCase() is not implemented yet" );
}

}
