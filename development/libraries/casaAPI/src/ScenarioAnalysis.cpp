//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ScenarioAnalysis.C
/// This file keeps methods definition for ScenarioAnalysis class

// CASA
#include "casaAPI.h"
#include "cmbAPI.h"

#include "DataDiggerImpl.h"
#include "DoEGeneratorImpl.h"
#include "MonteCarloSolverImpl.h"
#include "ObsSpaceImpl.h"
#include "ObsGridPropertyXYZ.h"
#include "ObsGridPropertyWell.h"
#include "ObsValueDoubleArray.h"
#include "ObsValueDoubleScalar.h"
#include "PrmOneCrustThinningEvent.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmSourceRockTOC.h"
#include "RSProxyImpl.h"
#include "RSProxySetImpl.h"
#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "RunManagerImpl.h"
#include "SensitivityCalculatorImpl.h"
#include "SimpleSerializer.h"
#include "SimpleDeserializer.h"
#include "VarSpaceImpl.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"

#include "FolderPath.h"
#include "FilePath.h"

// STL
#include <fstream>
#include <sstream>
#include <string>

// Standard C lib
#include <cassert>
#include <cmath>
#include <map>

namespace casa
{

///////////////////////////////////////////////////////////////////////////////////////////////
// Class which hides all ScenarioAnalysis implementation
class ScenarioAnalysis::ScenarioAnalysisImpl
{
public:
   // Constructor / destructor
   ScenarioAnalysisImpl();

   ~ScenarioAnalysisImpl();

   // Define scenario ID
   void defineScenarioID( const char * scID ) { m_scenarioID = std::string( scID ); }

   // Get scenario ID 
   const char * scenarioID() { return m_scenarioID.c_str(); }

   // Define a base case for scenario analysis from model, makes deep copy of the model
   void defineBaseCase( const mbapi::Model & bcModel );

   // Define a base case for scenario analysis from file.
   void defineBaseCase( const char * projectFileName );

   // get base case project file name
   const char * baseCaseProjectFileName() const { return m_baseCaseProjectFile.c_str(); }

   // Get base case model if it was set, empty model otherwise
   mbapi::Model & baseCase();

   // Get RunCase for the base case project
   RunCase * baseCaseRunCase() { return m_baseCaseRunCase.get(); }

   // Set path where SA will generate a bunch of cases
   void setScenarioLocation( const char * pathToCaseSet );

   // Restore path where SA generated a bunch of cases
   void restoreScenarioLocation( const char * pathToCaseSet );

   // Get path to top level folder where the set of generated cases are located
   const char *scenarioLocation() const { return m_caseSetPath.c_str(); }

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
   void addRSAlgorithm( const std::string & name, int order, RSProxy::RSKrigingType krType, const std::vector<std::string> & doeList, double targetR2 );

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

   // Save best matched case from Monte Carlo solver as calibrated scenario
   void saveCalibratedCase( const char * projFileName, size_t mcSampleNum );

   // Get SensitivityCalculator
   SensitivityCalculator & sensitivityCalculator() { return *(m_sensCalc.get()); }

   // Dump ScenarioAnalysis object to file
   void serialize( CasaSerializer & outStream );

   // Load ScenarioAnalysis object from file
   void deserialize( CasaDeserializer & inStream );

private:
   std::string                              m_scenarioID;          // scenario ID, some id which will be mentioned in all generated files
   std::string                              m_caseSetPath;         // path to folder which will be the root folder for all scenario cases
   std::string                              m_baseCaseProjectFile; // path to the base case project file
   int                                      m_iterationNum;        // Scenario analysis iteration number
   int                                      m_caseNum;             // counter for the cases, used in folder name of the case

   std::auto_ptr<mbapi::Model>              m_baseCase;
   std::auto_ptr<RunCaseImpl>               m_baseCaseRunCase;    // run case for the base case project
   std::auto_ptr<ObsSpaceImpl>              m_obsSpace;           // observables manager
   std::auto_ptr<VarSpaceImpl>              m_varSpace;           // variable parameters manager
   std::auto_ptr<DoEGeneratorImpl>          m_doe;

   std::auto_ptr<RunCaseSetImpl>            m_doeCases;
   std::auto_ptr<RunCaseSetImpl>            m_mcCases;

   std::auto_ptr<RunManagerImpl>            m_runManager;
   std::auto_ptr<DataDiggerImpl>            m_dataDigger;
   std::auto_ptr<RSProxySetImpl>            m_rsProxySet;
   std::auto_ptr<SensitivityCalculatorImpl> m_sensCalc;
   std::auto_ptr<MonteCarloSolver>          m_mcSolver;
};

///////////////////////////////////////////////////////////////////////////////
// Set of ScenarioAnalysis wrapper functions to hide the actual implementation from .h

// Constructor / destructor
ScenarioAnalysis::ScenarioAnalysis()  { m_pimpl.reset( new ScenarioAnalysisImpl() ); }
ScenarioAnalysis::~ScenarioAnalysis() { m_pimpl.reset( 0 ); }

// One line methods
mbapi::Model          & ScenarioAnalysis::baseCase()              { return m_pimpl->baseCase();              } // get scenario base case
RunCase               * ScenarioAnalysis::baseCaseRunCase()       { return m_pimpl->baseCaseRunCase();       } // get base case project run case
VarSpace              & ScenarioAnalysis::varSpace()              { return m_pimpl->varSpace();              } // Get set of variable parameters for the scenario
ObsSpace              & ScenarioAnalysis::obsSpace()              { return m_pimpl->obsSpace();              } // Get set of observables for the scenario
RunCaseSet            & ScenarioAnalysis::doeCaseSet()            { return m_pimpl->doeCaseSet();            } // Get set of cases generated by DoE
RunManager            & ScenarioAnalysis::runManager()            { return m_pimpl->runManager();            }
DataDigger            & ScenarioAnalysis::dataDigger()            { return m_pimpl->dataDigger();            }
RSProxySet            & ScenarioAnalysis::rsProxySet()            { return m_pimpl->rsProxySet();            }
RunCaseSet            & ScenarioAnalysis::mcCaseSet()             { return m_pimpl->mcCaseSet();             }
MonteCarloSolver      & ScenarioAnalysis::mcSolver()              { return m_pimpl->mcSolver();              }
SensitivityCalculator & ScenarioAnalysis::sensitivityCalculator() { return m_pimpl->sensitivityCalculator(); }
const char            * ScenarioAnalysis::scenarioID()            { return m_pimpl->scenarioID();            }

// Define Scenario ID
ErrorHandler::ReturnCode ScenarioAnalysis::defineScenarioID( const char * scID )
{
   if ( !scID ) { return this->ErrorHandler::reportError( OutOfRangeValue, "Empty scenario ID" ); }
   m_pimpl->defineScenarioID( scID );

   return NoError;
}

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

const char * ScenarioAnalysis::baseCaseProjectFileName() const
{
   return m_pimpl->baseCaseProjectFileName();
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

// restroe path where scenario generated projects
ErrorHandler::ReturnCode ScenarioAnalysis::restoreScenarioLocation( const char * pathToCaseSet )
{
   try { m_pimpl->restoreScenarioLocation( pathToCaseSet ); }
   catch ( Exception & ex           ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ibs::PathException & pex ) { return reportError( IoError,        pex.what() ); }
   catch ( ...                      ) { return reportError( UnknownError,   "Unknown error" ); }

   return NoError;
}


const char * ScenarioAnalysis::scenarioLocation() const
{
   return m_pimpl->scenarioLocation();
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


ErrorHandler::ReturnCode ScenarioAnalysis::addRSAlgorithm( const char * name
                                                         , int order
                                                         , RSProxy::RSKrigingType krType
                                                         , const std::vector<std::string> & doeList
                                                         , double targetR2
                                                         )
{
   try { m_pimpl->addRSAlgorithm( name, order, krType, doeList, targetR2 ); }
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


ErrorHandler::ReturnCode ScenarioAnalysis::saveCalibratedCase( const char * projFileName, size_t mcSampleNum )
{
   try { m_pimpl->saveCalibratedCase( projFileName, mcSampleNum ); }
   catch( Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch( ...            ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


// Save scenario to the file
ErrorHandler::ReturnCode ScenarioAnalysis::saveScenario( const char * fileName, const char * fileType )
{
   try
   {
      SimpleSerializer outStream( fileName, fileType, version() );
      m_pimpl->serialize( outStream );
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      return reportError( ex.errorCode(), ex.what() );
   }
   return NoError;
}

// Create new ScenarioAnaylysis object and read all data from the given file
ScenarioAnalysis * ScenarioAnalysis::loadScenario( const char * fileName, const char * fileType )
{
   std::auto_ptr<ScenarioAnalysis> sc( new ScenarioAnalysis() );
   try
   {
      std::ifstream fid;

      if (      !strcmp( "bin", fileType ) ) { fid.open( fileName, std::ofstream::binary ); }
      else if ( !strcmp( "txt", fileType ) ) { fid.open( fileName ); }
      else
      {
         throw Exception( NonexistingID ) << "Unknown type of input file for loading ScenarioAnalysis object: " << fileType;
      }

      if ( !fid.good() ) throw Exception(DeserializationError) << "Can not open file: " << fileName << " for reading";
      
      SimpleDeserializer inStream( fid, sc->version(), !strcmp( "bin", fileType ) );
      sc->m_pimpl->deserialize( inStream );
      if ( sc->errorCode() != ErrorHandler::NoError )
      {
         throw ErrorHandler::Exception( sc->errorCode() ) << sc->errorMessage();
      }
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      sc->reportError( ex.errorCode(), ex.what() );
   }
   return sc.release();
}

// Create new ScenarioAnaylysis object and read all data from the given file
ScenarioAnalysis * ScenarioAnalysis::loadScenario( const char * stateFileBuf, size_t bufSize, const char * fileType )
{
   std::auto_ptr<ScenarioAnalysis> sc( new ScenarioAnalysis() );
   try
   {
      std::string inpStr( stateFileBuf, bufSize );
      std::istringstream fid( inpStr );

      if ( !fid.good() ) throw Exception( DeserializationError ) << "Can not read from the given memory buffer";

      if ( !strcmp( "bin", fileType ) )
      {
         throw Exception( NotImplementedAPI ) << "Binary deserialization not implemented yet";
      }
      else if ( !strcmp( "txt", fileType ) )
      {
         SimpleDeserializer txtInStream( fid, sc->version() );
         sc->m_pimpl->deserialize( txtInStream );
         if ( sc->errorCode() != ErrorHandler::NoError )
         {
            throw ErrorHandler::Exception( sc->errorCode() ) << sc->errorMessage();
         }
      }
      else
      {
         throw Exception( NonexistingID ) << "Unknown type of input stream for loading ScenarioAnalysis object";
      }
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      sc->reportError( ex.errorCode(), ex.what() );
   }
   return sc.release();
}



///////////////////////////////////////////////////////////////////////////////
// The actual implementation of CASA API
ScenarioAnalysis::ScenarioAnalysisImpl::ScenarioAnalysisImpl()
{
   m_iterationNum = 1;
   m_caseNum      = 1;
   m_caseSetPath = ".";
   m_scenarioID = "Undefined";

   m_varSpace.reset(   new VarSpaceImpl()   );
   m_obsSpace.reset(   new ObsSpaceImpl()   );

   m_doeCases.reset(   new RunCaseSetImpl() );
   m_mcCases.reset(    new RunCaseSetImpl() );

   m_runManager.reset( new RunManagerImpl() );
   m_dataDigger.reset( new DataDiggerImpl() );

   m_rsProxySet.reset( new RSProxySetImpl() );

   m_mcSolver.reset(   new MonteCarloSolverImpl()   );
   m_sensCalc.reset(   new SensitivityCalculatorImpl( m_varSpace.get(), m_obsSpace.get() ) );
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


   m_baseCaseProjectFile = projectFileName;
}

mbapi::Model & ScenarioAnalysis::ScenarioAnalysisImpl::baseCase()
{
   if ( !m_baseCase.get() )
   {
      if ( m_baseCaseProjectFile.empty() ) throw Exception( ErrorHandler::UndefinedValue ) << "Base case was not defined for the scenario";

      m_baseCase.reset( new mbapi::Model() );
      if ( NoError != m_baseCase->loadModelFromProjectFile( m_baseCaseProjectFile.c_str() ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::IoError ) << "defineBaseCase() can not load model from " << m_baseCaseProjectFile;
      }
   }

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
            bool found = false;
            for ( int i = 1; i < 99 && !found; ++i )
            {
               ibs::FolderPath tmpPath( saFolder );
               tmpPath << "Iteration_" + ibs::to_string( i );
               if ( !tmpPath.exists() )
               {
                  found = true;
                  m_iterationNum = i;
               }
            }
            if ( !found ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << " folder " << pathToCaseSet << " is not empty"; }
         }

         m_caseSetPath = saFolder.fullPath().path();
      }
   }
   catch ( const ibs::PathException & ex )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << ex.what();
   }
}

void ScenarioAnalysis::ScenarioAnalysisImpl::restoreScenarioLocation( const char * pathToCaseSet )
{
   try
   {
      if ( !pathToCaseSet ) throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Empty path to completed cases";

      ibs::FolderPath saFolder = ibs::FolderPath( pathToCaseSet );

      if ( !saFolder.exists() || saFolder.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::IoError ) << " folder " << pathToCaseSet << " does not exist or empty";
      }

      m_caseSetPath = pathToCaseSet;
      const std::vector<std::string> & expNames = m_doeCases->experimentNames();

      // construct case set path like pathToScenario/Iteration_XX
      bool found = false;

      ibs::FolderPath caseSetPath( m_caseSetPath );

      for ( m_iterationNum = 1; m_iterationNum < 100 && !found; ++m_iterationNum )
      {
         caseSetPath = m_caseSetPath;

         caseSetPath << std::string( "Iteration_" ) + ibs::to_string( m_iterationNum );
         if ( caseSetPath.exists() ) { found = true; }
         else // try to find folders with experiment names
         {
            for ( size_t i = 0; i < expNames.size() && !found; ++i )
            {
               ibs::FolderPath caseSetPathDoE( caseSetPath );

               caseSetPathDoE << std::string( "Iteration_" ) + ibs::to_string( m_iterationNum ) + "_" + expNames[i];
               if ( caseSetPathDoE.exists() )
               { 
                  found = true;
                  caseSetPath = caseSetPathDoE;
                  m_doeCases->filterByExperimentName( expNames[i] );
               }
            }
         }
         if ( found ) // do checking if folder has Case_N subfolders and what is the starting number for case numbering
         {
            for ( m_caseNum = 1; m_caseNum < 100; ++m_caseNum )
            {
               ibs::FolderPath casePath = caseSetPath;
               casePath << ( std::string( "Case_" ) + ibs::to_string( m_caseNum ) );
               if ( casePath.exists() ) break;
            }
            found = m_caseNum == 100 ? false : true;
         }
      }

      if ( !found ) throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Given folder: " << caseSetPath.path() << "  does not have cases set";

      for ( size_t i = 0; i < m_doeCases->size(); ++i, ++m_caseNum )
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

         casePath << projectFileName;

         // point case to the corresponded location and validate it
         RunCaseImpl * rc = dynamic_cast<RunCaseImpl *>( m_doeCases->runCase( i ) );
         rc->setProjectPath( casePath.path().c_str() );
         const std::string & errMsgs = rc->validateCase();
         if ( !errMsgs.empty() )
         {
            throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Mismatched case variable parameters and completed case project: " <<
               casePath.path() << ", " << errMsgs;
         }

         // get case run status
         m_runManager->restoreCaseStatus( rc );
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
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( cs );

   // construct case set path like pathToScenario/Iteration_XX
   ibs::FolderPath caseSetPath( m_caseSetPath );

   std::string expFld = std::string( "Iteration_" ) + ibs::to_string( m_iterationNum );
   // also use experiment name (if it was set) in folder name pathToScenario/Iteration_XX_ExperimentName
   if ( !rcs.filter().empty() ) { expFld += std::string( "_" ) + rcs.filter(); }

   caseSetPath << expFld;

   for ( size_t i = 0; i <= rcs.size(); ++i )
   {
      // construct case project path: pathToScenario/Iteration_XX_ExperimentName/Case_XX/ProjectName.project3d
      ibs::FolderPath casePath = caseSetPath;

      casePath <<  ( i < rcs.size() ? ( std::string( "Case_" ) + ibs::to_string( m_caseNum ) ) : "BaseCase" );

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

      RunCaseImpl * cs = NULL;
      if ( i < rcs.size() ) { cs = dynamic_cast<RunCaseImpl*>( rcs[ i ] ); }
      else
      {
         // generate base case run case and save it as project file
         if ( !m_baseCaseRunCase.get() ) // this wasn't done before
         {
            // create new CASA case
            m_baseCaseRunCase.reset( new RunCaseImpl() );

            // add base parameters value to base case run case
            for ( size_t i = 0; i < m_varSpace->size(); ++i )
            {
               m_baseCaseRunCase->addParameter( m_varSpace->parameter( i )->baseValue() );
            }
         }
         cs = m_baseCaseRunCase.get();
      }

      if ( cs )
      {
         // create folder
         casePath.create();
         casePath << projectFileName;
         // do mutation
         cs->mutateCaseTo( baseCase(), casePath.path().c_str() );

         m_caseNum += i < rcs.size() ? 1 : 0; // do not count base case
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

void ScenarioAnalysis::ScenarioAnalysisImpl::saveCalibratedCase( const char * projFileName, size_t mcSampleNum )
{
   if ( mcSampleNum < 1 )
   {
      throw Exception( MonteCarloSolverError ) << "Monte Carlo sampling numbering starts with 1, can not get sample point:" << mcSampleNum;
   }
   if ( m_mcSolver->samplingsNumber() < mcSampleNum )
   {
      throw Exception( MonteCarloSolverError ) << "Requested Monte Carlo sampling point number bigger than total number of sampling points";
   }

   RunCase * bmCase = const_cast<RunCase*>( m_mcSolver->samplingPoint( mcSampleNum-1 ) ); // by default samples are sorted according to RMSE
   if ( !bmCase )
   {
      throw Exception( MonteCarloSolverError ) << "Can not generate calibrated case: " << projFileName << ". Monte Carlo simulation should be done first";
   }

   RunCaseImpl * bmCaseImpl = dynamic_cast<RunCaseImpl*>( bmCase );

   // construct best matched case set path like pathToScenario/BestMatch_projFileName
   ibs::FilePath fp( projFileName );

   ibs::FolderPath bmCasePath( m_caseSetPath );
   bmCasePath << std::string( "Calibrated_" ) + fp.fileNameNoExtension();

   if ( bmCasePath.exists() ) throw ErrorHandler::Exception( ErrorHandler::IoError ) << " folder " << bmCasePath.path() << " is not empty";

   bmCasePath.create();
   bmCasePath << projFileName;

   // do mutation
   bmCaseImpl->mutateCaseTo( baseCase(), bmCasePath.path().c_str() );
   // add observables
   m_dataDigger->requestObservables( *m_obsSpace.get(), bmCaseImpl );
   // generate scripts
   m_runManager->scheduleCase( *bmCaseImpl, scenarioID() );
}

void ScenarioAnalysis::ScenarioAnalysisImpl::addRSAlgorithm( const std::string              & name
                                                           , int                              order
                                                           , RSProxy::RSKrigingType           krType
                                                           , const std::vector<std::string> & doeList
                                                           , double                           targetR2
                                                           )
{

   if ( order < -1 || order > 3 )
   {
      throw Exception( OutOfRangeValue ) << "addRSAlgorithm(): wrong value for the order: " << order << 
                                            ", must be in range: [0:3]";
   }
   if ( name.empty() ) throw Exception( OutOfRangeValue ) << "addRSAlgorithm(): empty proxy name";

   RSProxy * proxy = m_rsProxySet->rsProxy( name );
   if ( proxy ) // already has response surface with the same name
   {
      throw Exception( AlreadyDefined ) << "addRSAlgorithm(): proxy with name: " << name << ", already exists in the scenario";
   }
   else
   {
      switch ( order )
      {
         case -1: proxy = new RSProxyImpl( name, varSpace(), obsSpace(), 0,     krType, true, targetR2 ); break;
         case  0:
         case  1:
         case  2: proxy = new RSProxyImpl( name, varSpace(), obsSpace(), order, krType                 ); break;
         case  3: proxy = new RSProxyImpl( name, varSpace(), obsSpace(), 0,     krType, true, 1.0      ); break;
      }

      m_rsProxySet->addNewRSProxy( proxy, name );
   }

   // if DoEs name list is not empty - calculate response surface
   if ( doeList.size() )
   {
      const std::vector<const casa::RunCase *> & rcs = m_doeCases->collectCompletedCases( doeList );

      if ( rcs.empty() ) throw Exception( RSProxyError ) << "addRSAlgorithm(): empty completed cases list for given DoEs";

      if ( !obsSpace().size() ) throw Exception( RSProxyError ) << "No observable is defined for proxy calculation";

      if ( NoError != proxy->calculateRSProxy( rcs ) ) { throw Exception( proxy->errorCode() ) << proxy->errorMessage(); }
   }
}

void ScenarioAnalysis::ScenarioAnalysisImpl::serialize( CasaSerializer & outStream )
{
   bool ok = outStream.save( m_caseSetPath,         "caseSetPath"  );
   ok = ok ? outStream.save( m_baseCaseProjectFile, "baseCaseProjectFile" ) : ok;
   ok = ok ? outStream.save( m_iterationNum,        "iterationNum" ) : ok;
   ok = ok ? outStream.save( m_caseNum,             "caseNum"      ) : ok;

   ok = ok ? outStream.save( obsSpace(),            "ObsSpace"     ) : ok; // serialize observables manager
   ok = ok ? outStream.save( varSpace(),            "VarSpace"     ) : ok; // serialize variable parameters set

   ok = ok ? outStream.save( *m_doeCases.get(),     "DoECasesSet"  ) : ok;
   ok = ok ? outStream.save( *m_mcCases.get(),      "MCCasesSet"   ) : ok;
   ok = ok ? outStream.save( *m_rsProxySet.get(),   "RSProxySet"   ) : ok;

   ok = ok ? outStream.save( *(doeGenerator()),     "DoE"          ) : ok; // serialize doe generator
   ok = ok ? outStream.save( dataDigger(),          "DataDigger"   ) : ok; // data digger
   ok = ok ? outStream.save( runManager(),          "RunManager"   ) : ok; // run manager

   ok = ok ? outStream.save( mcSolver(),            "MCSolver"     ) : ok;
   ok = ok ? outStream.save( *m_sensCalc.get(),     "SensitivityCalculator" ) : ok;
   ok = ok ? outStream.save( m_scenarioID,          "scenarioID"   ) : ok;

   if ( !ok ) throw ErrorHandler::Exception( SerializationError ) << "Serialization error in ScenarioAnalysis";
}

// Load ScenarioAnalysis object from file
void ScenarioAnalysis::ScenarioAnalysisImpl::deserialize( CasaDeserializer & inStream )
{
   if ( inStream.version() < 9 )
   {
      throw Exception( DeserializationError ) << "Incompatible casa state file version. Versions 9 and later are incompatible with " <<
         " the given state file version: " <<  inStream.version();
   }

   bool ok = inStream.load( m_caseSetPath,         "caseSetPath" );

   // read base case name and load it as a model
   std::string baseCaseName;
   ok = ok ? inStream.load( baseCaseName, "baseCaseProjectFile" ) : ok;

   if ( ok ) defineBaseCase( baseCaseName.c_str() );

   ok = ok ? inStream.load( m_iterationNum,        "iterationNum" ) : ok;
   ok = ok ? inStream.load( m_caseNum,             "caseNum" ) : ok;

   if ( !ok ) throw Exception( DeserializationError ) << "Deserialization error in ScenarioAnalysis";

   m_obsSpace.reset(   new ObsSpaceImpl(              inStream, "ObsSpace"              ) );
   m_varSpace.reset(   new VarSpaceImpl(              inStream, "VarSpace"              ) );

   m_doeCases.reset(   new RunCaseSetImpl(            inStream, "DoECasesSet"           ) );
   m_mcCases.reset(    new RunCaseSetImpl(            inStream, "MCCasesSet"            ) );
   m_rsProxySet.reset( new RSProxySetImpl(            inStream, "RSProxySet"            ) );

   m_doe.reset(        new DoEGeneratorImpl(          inStream, "DoE"                   ) );
   m_dataDigger.reset( new DataDiggerImpl(            inStream, "DataDigger"            ) );
   m_runManager.reset( new RunManagerImpl(            inStream, "RunManager"            ) );
   m_mcSolver.reset(   new MonteCarloSolverImpl(      inStream, "MCSolver"              ) );
   m_sensCalc.reset(  new SensitivityCalculatorImpl(  inStream, "SensitivityCalculator" ) );
   
   inStream.load( m_scenarioID, "scenarioID" );
}

}

