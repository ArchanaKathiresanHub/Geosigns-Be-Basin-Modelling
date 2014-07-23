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
#include "ObsSpaceImpl.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmSourceRockTOC.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "RunManagerImpl.h"
#include "VarSpaceImpl.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"

#include "FolderPath.h"
#include "FilePath.h"

#include <sstream>
#include <string>
#include <cassert>

namespace casa {

///////////////////////////////////////////////////////////////////////////////
// Set of business logic rules functions to convert one request to set of parameters
namespace BusinessLogicRulesSet
{
// Add a parameter to variate layer thickness value [m] in given range
ErrorHandler::ReturnCode VaryLayerThickness( ScenarioAnalysis & sa
                                           , const char * layerName
                                           , double minVal
                                           , double maxVal
                                           , VarPrmContinuous::PDF rangeShape
                                           )
{
   return ErrorHandler::NotImplementedAPI;
}

// Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
ErrorHandler::ReturnCode VaryTopCrustHeatProduction( ScenarioAnalysis & sa
                                                   , double minVal
                                                   , double maxVal
                                                   , VarPrmContinuous::PDF rangeShape
                                                   )
{
   VarSpace & varPrmsSet = sa.varSpace();

   // Get base value of parameter from the Model
   mbapi::Model & mdl = sa.baseCase();
   
   casa::PrmTopCrustHeatProduction prm( mdl );
   if ( mdl.errorCode() != ErrorHandler::NoError ) return mdl.errorCode();

   double baseValue = prm.doubleValue( );

   if ( baseValue < minVal || baseValue > maxVal )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
   }

   return varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( prm.doubleValue(), minVal, maxVal, rangeShape ) );
}

// Add a parameter to variate source rock lithology TOC value @f$ [%%] @f$ in given range
ErrorHandler::ReturnCode VarySourceRockTOC( ScenarioAnalysis & sa
                                          , const char * layerName
                                          , double minVal
                                          , double maxVal
                                          , VarPrmContinuous::PDF rangeShape
                                          )
{
   VarSpace & varPrmsSet = sa.varSpace();

   // Get base value of parameter from the Model
   mbapi::Model & mdl = sa.baseCase();

   casa::PrmSourceRockTOC prm( mdl, layerName );
   if ( mdl.errorCode() != ErrorHandler::NoError ) return mdl.errorCode();

   double baseValue = prm.doubleValue();

   if ( baseValue < minVal || baseValue > maxVal )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
   }

   return varPrmsSet.addParameter( new VarPrmSourceRockTOC( layerName, baseValue, minVal, maxVal, rangeShape ) );
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

   // Set path where SA will generate a bunch of cases
   void setScenarioLocation( const char * pathToCaseSet );

   // Provide variable parameters set manager
   VarSpace & varSpace() { return *(m_varSpace.get()); }
   
   // Define DoE algorithm. This function should be called before accessing to the instance of DoEGenerator
   // algo Type of DoE algorithm
   // return ErrorHandler::NoError in case of success, or error code otherwise
   void setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo );
   
   // Get reference to instance of DoE generator which will be used in scenario analysis. If algorithm
   // wasn't set before by ScenarioAnalysis::setDoEAlgorithm(), the DoEGenerator::Tornado algorithm will be chosen.
   // return reference to the DoEGenerator instance.
   DoEGenerator * doeGenerator();

   // Get all cases for this scenario. The list will include cases generated by DoE only
   // return reference to casa::RunCaseSet object
   RunCaseSet & doeCaseSet() { return *( m_doeCases.get() ); }

   // Create copy of the base case model and set all variable parameters value defined for each case
   void applyMutations( RunCaseSet & cs );

   // Validate Cauldron model for consistency and valid parameters range. This function should be 
   // called after ScenarioAnalysis::applyMutation()
   void validateCaseSet( RunCaseSet & cs );

   // Get run manager associated with this scenario analysis
   // return reference to the instance of run manager
   RunManager & runManager() { return *( m_runManager.get() ); }

   // Get data digger associated with this scenario analysis
   // return reference to the instance of data digger
   DataDigger & dataDigger() { return *( m_dataDigger.get() ); }
   
   // Get list of observables for this scenario
   // return Observables set manager
   ObsSpace & obsSpace() { return *( m_obsSpace.get() ); }

   // Define which order of response surface polynomial approximation of  will be used in this scenario analysis
   // order order of polynomial approximation
   // krType do we need Kriging interpolation, and which one?
   void setRSAlgorithm( int order, RSProxy::RSKrigingType krType )
   {
      throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "setRSAlgorithm() not implemented yet";
   }
   
   // Get response surface proxy 
   // return reference to proxy object
   RSProxy & responseSurfaceProxy() { return *(m_rsProxy.get()); }

   // Get all cases for this scenario. The list will include cases generated by MC/MCMC only
   // return array of casa::Case objects
   RunCaseSet & mcCaseSet() { return *( m_mcCases.get() ); }

   // Define type of Monte Carlo algorithm which will be used in this scenario analysis
   // algo Monte Carlo algorithm
   // interp do we need Kriging interpolation? If yes, the response surface proxy must also use it.
   void setMCAlgorithm( MCSolver::MCAlgorithm algo, MCSolver::MCKrigingType interp = MCSolver::NoKriging )
   {
      throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "setMCAlgorithm() not implemented yet";
   }
   
   // Get Monte Carlo solver
   // return reference to Monte Carlo solver. If MC solver algorithm wasn't defined befor by ScenarioAnalysis::setMCAlgorithm(), it
   //         will be set up to MC with no Kriging by default.
   MCSolver & mcSolver()  { return *(m_mcSolver.get()); }
      
private:
   std::string                     m_caseSetPath;         // path to folder which will be the root folder for all scenario cases
   std::string                     m_baseCaseProjectFile; // path to the base case project file
   int                             m_iterationNum;        // Scenario analysis iteration number
   int                             m_caseNum;             // counter for the cases, used in folder name of the case

   std::auto_ptr<mbapi::Model>     m_baseCase;
   std::auto_ptr<ObsSpaceImpl>     m_obsSpace;           // observables manager
   std::auto_ptr<VarSpaceImpl>     m_varSpace;           // variable parameters manager
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

// Constructor / destructor
ScenarioAnalysis::ScenarioAnalysis()  { m_pimpl.reset( new ScenarioAnalysisImpl() ); }
ScenarioAnalysis::~ScenarioAnalysis() { m_pimpl.reset( 0 ); }

// Define base case for scenario from cauldron model in memory
ErrorHandler::ReturnCode ScenarioAnalysis::defineBaseCase( const mbapi::Model & bcModel )
{
   try { m_pimpl->defineBaseCase( bcModel ); }
   catch( Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Define base case form cauldron project file
ErrorHandler::ReturnCode ScenarioAnalysis::defineBaseCase( const char * projectFileName )
{
   try { m_pimpl->defineBaseCase( projectFileName ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// get scenario base case
mbapi::Model & ScenarioAnalysis::baseCase() { return m_pimpl->baseCase(); }

// set path where scenario will generate projects
ErrorHandler::ReturnCode ScenarioAnalysis::setScenarioLocation( const char * pathToCaseSet )
{
   try { m_pimpl->setScenarioLocation( pathToCaseSet ); }
   catch ( Exception & ex           ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ibs::PathException & pex ) { return reportError( IoError,        pex.what() ); }
   catch ( ...                      ) { return reportError( UnknownError,   "Unknown error" ); }

   return NoError;
}

// Get set of variable parameters for the scenario 
VarSpace   & ScenarioAnalysis::varSpace() { return m_pimpl->varSpace(); }

// Get set of observables for the scenario
ObsSpace   & ScenarioAnalysis::obsSpace() { return m_pimpl->obsSpace(); }

// Define DoE algorithm
ErrorHandler::ReturnCode ScenarioAnalysis::setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo )
{
   try { m_pimpl->setDoEAlgorithm( algo ); }
   catch ( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Get set of cases generated by DoE
RunCaseSet & ScenarioAnalysis::doeCaseSet() { return m_pimpl->doeCaseSet(); }

// Get DoE generator
DoEGenerator & ScenarioAnalysis::doeGenerator()
{
   if ( !m_pimpl->doeGenerator() )
   {
      m_pimpl->setDoEAlgorithm( DoEGenerator::Tornado );
   }
   return *( m_pimpl->doeGenerator() );
}

// Apply mutations and generate project files for the set of cases from DoE
ErrorHandler::ReturnCode ScenarioAnalysis::applyMutations( RunCaseSet & cs )
{
   try { m_pimpl->applyMutations( cs ); }
   catch ( Exception & ex           ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ibs::PathException & pex ) { return reportError( IoError,        pex.what() ); }
   catch ( ...                      ) { return reportError( UnknownError,   "Unknown error" ); }

   return NoError;
}

ErrorHandler::ReturnCode ScenarioAnalysis::validateCaseSet( RunCaseSet & cs )
{
   try { m_pimpl->validateCaseSet( cs ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

RunManager & ScenarioAnalysis::runManager() { return m_pimpl->runManager(); }
DataDigger & ScenarioAnalysis::dataDigger() { return m_pimpl->dataDigger(); }

RSProxy & ScenarioAnalysis::responseSurfaceProxy() { return m_pimpl->responseSurfaceProxy(); }

RunCaseSet & ScenarioAnalysis::mcCaseSet()  { return m_pimpl->mcCaseSet(); }

ErrorHandler::ReturnCode ScenarioAnalysis::setRSAlgorithm( int order, RSProxy::RSKrigingType krType )
{
   try { m_pimpl->setRSAlgorithm( order, krType ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

MCSolver & ScenarioAnalysis::mcSolver()  { return m_pimpl->mcSolver(); }

ErrorHandler::ReturnCode ScenarioAnalysis::setMCAlgorithm( MCSolver::MCAlgorithm algo, MCSolver::MCKrigingType interp )
{
   try { m_pimpl->setMCAlgorithm( algo, interp ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

///////////////////////////////////////////////////////////////////////////////
// The actual implementation of CASA API

ScenarioAnalysis::ScenarioAnalysisImpl::ScenarioAnalysisImpl()
{
   m_iterationNum = 1;
   m_caseNum      = 1;
   m_caseSetPath = ".";

   m_varSpace.reset(   new VarSpaceImpl()   );
   m_obsSpace.reset(   new ObsSpaceImpl()   );

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
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) <<  "defineBaseCase() is not implemented yet";
}

void ScenarioAnalysis::ScenarioAnalysisImpl::defineBaseCase( const char * projectFileName )
{
   if ( m_baseCase.get() ) { throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "defineBaseCase(): Base case is already defined"; }
   
   m_baseCase.reset( new mbapi::Model() );
   if ( NoError != m_baseCase->loadModelFromProjectFile( projectFileName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "defineBaseCase() can not load model from " << projectFileName;
   }
   
   m_baseCaseProjectFile = projectFileName;
}

mbapi::Model & ScenarioAnalysis::ScenarioAnalysisImpl::baseCase()
{
   if ( !m_baseCase.get() ) m_baseCase.reset( new mbapi::Model() );
   return *( m_baseCase.get() );
}

void ScenarioAnalysis::ScenarioAnalysisImpl::setScenarioLocation( const char * pathToCaseSet )
{
   try
   {
      if ( pathToCaseSet )
      {
         ibs::FolderPath saFolder = ibs::FolderPath( pathToCaseSet );

         if ( !saFolder.exists() )
         {
            saFolder.create();
         }
         else if ( !saFolder.empty( ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::IoError ) << " folder " << pathToCaseSet << " is not empty";
         }

         m_caseSetPath = pathToCaseSet;
      }
   }
   catch ( const ibs::PathException & ex )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << ex.what();
   }      
}

void ScenarioAnalysis::ScenarioAnalysisImpl::setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo )
{
   m_doe.reset( new DoEGeneratorImpl( algo ) );
}

DoEGenerator * ScenarioAnalysis::ScenarioAnalysisImpl::doeGenerator()
{
   if ( !m_doe.get() )
   {
      setDoEAlgorithm( DoEGenerator::Tornado );
   }

   return m_doe.get();
}

void ScenarioAnalysis::ScenarioAnalysisImpl::applyMutations( RunCaseSet & cs )
{
   if ( !m_baseCase.get() ) throw Exception( ErrorHandler::UndefinedValue ) << "Base case was not defined for the scenario. Mutations failed";
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( cs );

   // construct case set path like pathToScenario/Iteration_XX
   ibs::FolderPath caseSetPath( m_caseSetPath );

   std::string expFld = std::string( "Iteration_" ) + ibs::to_string( m_iterationNum );   
   // also use experiment name (if it was set) in folder name pathToScenario/Iteration_XX_ExperimentName
   if ( !rcs.filter().empty() ) { expFld += std::string( "_" ) + rcs.filter(); }
   
   caseSetPath << expFld;

   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      // construct case project path: pathToScenario/Iteration_XX_ExperimentName/Case_XX/ProjectName.project3d
      ibs::FolderPath casePath = caseSetPath;
      casePath << ( std::string( "Case_" ) + ibs::to_string( m_caseNum ) );
      
      // extract project file name:
      std::string projectFileName = "Project.project3d";
      if ( !m_baseCaseProjectFile.empty() )
      {
         ibs::FilePath pf( m_baseCaseProjectFile );
         const std::string & fn = pf.fileName();
         if ( !fn.empty() )
         {
            projectFileName = fn;
         }
      }

      RunCaseImpl * cs = dynamic_cast<RunCaseImpl*>( rcs.at( i ) );

      if ( cs )
      {
         // create folder
         casePath.create();
         casePath << projectFileName;
         // do mutation
         cs->mutateCaseTo( *(m_baseCase.get()), casePath.path().c_str() );

         ++m_caseNum;
      }
   }
   ++m_iterationNum;
}


void ScenarioAnalysis::ScenarioAnalysisImpl::validateCaseSet( RunCaseSet & cs )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( cs );
   
   // exception object which will keep list of validation error
   ErrorHandler::Exception ex( ErrorHandler::ValidationError );
   bool allValid = true;

   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      RunCaseImpl * cs = dynamic_cast<RunCaseImpl*>( rcs.at( i ) );

      if ( cs )
      {
         const std::string errList = cs->validateCase();
         if ( !errList.empty() )
         {
            ex << "Case: " << i + 1 << " validation failed with message: " << errList;
            allValid = false;
         }
      }
   }

   if ( !allValid ) throw ex;
}

}
