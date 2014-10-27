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
#include "MonteCarloSolverImpl.h"
#include "ObsSpaceImpl.h"
#include "PrmOneCrustThinningEvent.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmSourceRockTOC.h"
#include "RSProxyImpl.h"
#include "RSProxySetImpl.h"
#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "RunManagerImpl.h"
#include "VarSpaceImpl.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"

#include "FolderPath.h"
#include "FilePath.h"

#include <sstream>
#include <string>
#include <cassert>
#include <cmath>
#include <map>

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

   const std::vector<double> & baseValue = prm.asDoubleArray();
   assert( baseValue.size() == 1 );

   if ( baseValue[0] < minVal || baseValue[0] > maxVal )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
   }

   return varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( baseValue[0], minVal, maxVal, rangeShape ) );
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

   const std::vector<double> & baseValue = prm.asDoubleArray();
   assert( baseValue.size() == 1 );

   if ( baseValue[0] < minVal || baseValue[0] > maxVal )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
   }

   return varPrmsSet.addParameter( new VarPrmSourceRockTOC( layerName, baseValue[0], minVal, maxVal, rangeShape ) );
}

// Add 4 parameters to variate one crust thinning event.
ErrorHandler::ReturnCode VaryOneCrustThinningEvent( casa::ScenarioAnalysis & sa, double minThickIni,    double maxThickIni,
                                                                                 double minT0,          double maxT0,       
                                                                                 double minDeltaT,      double maxDeltaT,   
                                                                                 double minThinningFct, double maxThinningFct, VarPrmContinuous::PDF thingFctPDF )
{
   VarSpace & varPrmsSet = sa.varSpace();

   // Get base value of parameter from the Model
   mbapi::Model & mdl = sa.baseCase();

   casa::PrmOneCrustThinningEvent prm( mdl );
   if ( mdl.errorCode() != ErrorHandler::NoError ) return mdl.errorCode();

   std::vector<double> baseValues = prm.asDoubleArray();

   for ( size_t i = 0; i < 4; ++i ) // replace undefined base value with middle of value range
   // crust thickness profile shape in base project file could not match what we need : 
   // *--------*
   //          \
   //           *-----------*
   // in this case, constructor of parameter could pick up some of base values from the base project file
   // for others - we will use avarage from min/max
   {
      if ( std::abs(UndefinedDoubleValue - baseValues[i]) < 1.e-10 )
      {
         switch ( i )
         {
            case 0: baseValues[i] = 0.5 * ( minThickIni    + maxThickIni ); break;
            case 1: baseValues[i] = 0.5 * ( minT0          + maxT0       ); break;
            case 2: baseValues[i] = 0.5 * ( minDeltaT      + maxDeltaT   ); break;
            case 3: baseValues[i] = 0.5 * ( minThinningFct + maxThinningFct ); break;
         }
      }
   }

   if ( baseValues[0] < minThickIni || baseValues[0] > maxThickIni )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of initial thickness parameter in base case is outside of the given range" );
   }

   if ( baseValues[1] < minT0 || baseValues[1] > maxT0 )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of start time for crust thinning parameter in base case is outside of the given range" );
   }

   if ( baseValues[2] < minDeltaT || baseValues[2] > maxDeltaT )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of duration of crust thinning parameter in base case is outside of the given range" );
   }

   if ( baseValues[3] < minThinningFct || baseValues[3] > maxThinningFct )
   {
      return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of crust thinning factor parameter in base case is outside of the given range" );
   }
   
   return varPrmsSet.addParameter( new VarPrmOneCrustThinningEvent( baseValues[0], minThickIni,    maxThickIni,
                                                                    baseValues[1], minT0,          maxT0,
                                                                    baseValues[2], minDeltaT,      maxDeltaT,
                                                                    baseValues[3], minThinningFct, maxThinningFct,
                                                                    thingFctPDF ) );
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
   void addRSAlgorithm( const std::string & name, size_t order, RSProxy::RSKrigingType krType )
   {
      if ( order < 0 || order > 3 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "addRSAlgorithm(): wrong value for the order: " << order << 
                                                                              ", must be in range: [0:3]";
      }
      if ( name.empty() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "addRSAlgorithm(): empty proxy name";

      if ( m_rsProxySet->rsProxy( name ) ) // already has response surface with the same name
      {
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "addRSAlgorithm(): proxy with name: " << name << ", already exist in the scenario";
      }
      else
      { 
         if ( order < 3 )
         {
            m_rsProxySet->addNewRSProxy( new RSProxyImpl( name, varSpace(), obsSpace(), order, krType ), name );
         }
         else
         {
            m_rsProxySet->addNewRSProxy( new RSProxyImpl( name, varSpace(), obsSpace(), 2, krType, true, 1.0 ), name );
         }
      }
   }
   
   // Get response surface proxies set
   RSProxySet & rsProxySet() { return *(m_rsProxySet.get() ); }

   // Get all cases for this scenario. The list will include cases generated by MC/MCMC only
   RunCaseSet & mcCaseSet() { return *( m_mcCases.get() ); }

   // Define type of Monte Carlo algorithm which will be used in this scenario analysis
   // algo Monte Carlo algorithm
   // interp do we need Kriging interpolation? If yes, the response surface proxy must also use it.
   void setMCAlgorithm( MonteCarloSolver::Algorithm algo
                      , MonteCarloSolver::KrigingType kr
                      , MonteCarloSolver::PriorDistribution priorDist
                      , MonteCarloSolver::MeasurementDistribution measureDist
                      )
   {
      m_mcSolver.reset( new MonteCarloSolverImpl( algo, kr, priorDist, measureDist ) );
   }
   
   // Get Monte Carlo solver
   // return reference to Monte Carlo solver. If MC solver algorithm wasn't defined befor by ScenarioAnalysis::setMCAlgorithm(), it
   //         will be set up to MC with no Kriging by default.
   MonteCarloSolver & mcSolver()  { return *(m_mcSolver.get()); }
      
private:
   std::string                      m_caseSetPath;         // path to folder which will be the root folder for all scenario cases
   std::string                      m_baseCaseProjectFile; // path to the base case project file
   int                              m_iterationNum;        // Scenario analysis iteration number
   int                              m_caseNum;             // counter for the cases, used in folder name of the case

   std::auto_ptr<mbapi::Model>      m_baseCase;
   std::auto_ptr<ObsSpaceImpl>      m_obsSpace;           // observables manager
   std::auto_ptr<VarSpaceImpl>      m_varSpace;           // variable parameters manager
   std::auto_ptr<DoEGeneratorImpl>  m_doe;
   
   std::auto_ptr<RunCaseSetImpl>    m_doeCases;
   std::auto_ptr<RunCaseSetImpl>    m_mcCases;

   std::auto_ptr<RunManager>        m_runManager;
   std::auto_ptr<DataDigger>        m_dataDigger;
   std::auto_ptr<RSProxySetImpl>    m_rsProxySet;
   std::auto_ptr<MonteCarloSolver>          m_mcSolver;
};



///////////////////////////////////////////////////////////////////////////////
// Set of ScenarioAnalysis wrapper functions to hide the actual implementation from .h

// Constructor / destructor
ScenarioAnalysis::ScenarioAnalysis()  { m_pimpl.reset( new ScenarioAnalysisImpl() ); }
ScenarioAnalysis::~ScenarioAnalysis() { m_pimpl.reset( 0 ); }

// One line methods
mbapi::Model     & ScenarioAnalysis::baseCase()   { return m_pimpl->baseCase();   } // get scenario base case
VarSpace         & ScenarioAnalysis::varSpace()   { return m_pimpl->varSpace();   } // Get set of variable parameters for the scenario
ObsSpace         & ScenarioAnalysis::obsSpace()   { return m_pimpl->obsSpace();   } // Get set of observables for the scenario
RunCaseSet       & ScenarioAnalysis::doeCaseSet() { return m_pimpl->doeCaseSet(); } // Get set of cases generated by DoE
RunManager       & ScenarioAnalysis::runManager() { return m_pimpl->runManager(); }
DataDigger       & ScenarioAnalysis::dataDigger() { return m_pimpl->dataDigger(); }
RSProxySet       & ScenarioAnalysis::rsProxySet() { return m_pimpl->rsProxySet(); }
RunCaseSet       & ScenarioAnalysis::mcCaseSet()  { return m_pimpl->mcCaseSet();  }
MonteCarloSolver & ScenarioAnalysis::mcSolver()   { return m_pimpl->mcSolver();   }


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

// set path where scenario will generate projects
ErrorHandler::ReturnCode ScenarioAnalysis::setScenarioLocation( const char * pathToCaseSet )
{
   try { m_pimpl->setScenarioLocation( pathToCaseSet ); }
   catch ( Exception & ex           ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ibs::PathException & pex ) { return reportError( IoError,        pex.what() ); }
   catch ( ...                      ) { return reportError( UnknownError,   "Unknown error" ); }

   return NoError;
}

// Define DoE algorithm
ErrorHandler::ReturnCode ScenarioAnalysis::setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo )
{
   try { m_pimpl->setDoEAlgorithm( algo ); }
   catch ( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


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


ErrorHandler::ReturnCode ScenarioAnalysis::addRSAlgorithm( const char * name, int order, RSProxy::RSKrigingType krType )
{
   try { m_pimpl->addRSAlgorithm( name, static_cast<size_t>( order ), krType ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


ErrorHandler::ReturnCode ScenarioAnalysis::setMCAlgorithm( MonteCarloSolver::Algorithm               algo
                                                         , MonteCarloSolver::KrigingType             interp
                                                         , MonteCarloSolver::PriorDistribution       priorDist
                                                         , MonteCarloSolver::MeasurementDistribution measureDist
                                                         )
{
   try { m_pimpl->setMCAlgorithm( algo, interp, priorDist, measureDist ); }
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

   m_rsProxySet.reset( new RSProxySetImpl() );

   m_mcSolver.reset(   new MonteCarloSolverImpl()   );
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

      RunCaseImpl * cs = dynamic_cast<RunCaseImpl*>( rcs[ i ] );

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
      RunCaseImpl * cs = dynamic_cast<RunCaseImpl*>( rcs[ i ] );

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
