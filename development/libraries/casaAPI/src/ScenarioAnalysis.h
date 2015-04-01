//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ScenarioAnalysis.h
/// @brief This file keeps declaration of ScenarioAnalysis class - base CASA API class which defines most of interfaces

#ifndef CASA_SCENARIO_ANALYSIS
#define CASA_SCENARIO_ANALYSIS

// CASA
#include "RunCaseSet.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "DataDigger.h"
#include "DoEGenerator.h"
#include "ErrorHandler.h"
#include "MonteCarloSolver.h"
#include "ObsSpace.h"
#include "RSProxy.h"
#include "RSProxySet.h"
#include "RunManager.h"
#include "SensitivityCalculator.h"
#include "VarSpace.h"

#include "cmbAPI.h"

// STL
#include <memory>
#include <vector>


/// @brief Namespace which keeps API for performing uncertainty/sensitivity scenario analysis
namespace casa
{
   /// @brief ScenarioAnalysis keeps all objects which are necessary for UA/SA of single workflow
   class ScenarioAnalysis : public ErrorHandler
   {
   public:
      /// @brief Constructor. Create new empty scenario analysis.
      ScenarioAnalysis();

      /// @brief Destructor
      ~ScenarioAnalysis();

      /// @brief Define a base case for scenario analysis
      /// @param bcModel Cauldron model loaded into memory
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode defineBaseCase( const mbapi::Model & bcModel );
      
      /// @brief Define a base case for scenario analysis.
      /// @param projectFileName name of the Cauldron project file. File will be loaded to memory as mbapi::Model
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode defineBaseCase( const char * projectFileName );

      /// @brief Return a base case project name
      /// @return project file name for the base case of scenario analysis
      const char * baseCaseProjectFileName() const;

      /// @brief Get base case model
      /// @return pointer to the base case model if it was set before, empty model otherwise
      mbapi::Model & baseCase();

      /// @brief Set path where SA will generate a bunch of cases. By default it is the current folder
      /// @param pathToCaseSet
      /// @return ErrorHandler::NoError on success, or ErrorHandler::WrongPath if SA can't create folders/files using this path
      ErrorHandler::ReturnCode setScenarioLocation( const char * pathToCaseSet );

      /// @brief Restore path where SA generated a bunch of cases
      /// @param pathToCaseSet
      /// @return ErrorHandler::NoError on success, or ErrorHandler::WrongPath if SA can't create folders/files using this path
      ErrorHandler::ReturnCode restoreScenarioLocation( const char * pathToCaseSet );

      /// @brief Get path where SA generats a bunch of cases
      /// @return path to the top folder where generated cases are located
      const char * scenarioLocation() const;

      /// @brief Provides variable parameters set manager
      /// @return reference to the instance of VarSpace class
      VarSpace & varSpace();
      
      /// @brief Define DoE algorithm. This function should be called before accessing to the instance of DoEGenerator
      /// @param algo Type of DoE algorithm
      /// @return ErrorHandler::NoError in case of success, or error code otherwise
      ErrorHandler::ReturnCode setDoEAlgorithm( DoEGenerator::DoEAlgorithm algo );
      
      /// @brief Get reference to instance of DoE generator which will be used in scenario analysis. If algorithm
      ///        wasn't set before by ScenarioAnalysis::setDoEAlgorithm(), the DoEGenerator::Tornado algorithm will be chosen.
      /// @return reference to the DoEGenerator instance.
      DoEGenerator & doeGenerator();

      /// @brief Get all cases for this scenario. The list will include cases generated by DoE only
      /// @return reference to casa::RunCaseSet object
      RunCaseSet & doeCaseSet();
      
      /// @brief Create copy of the base case model and set all variable parameters value defined for each case
      /// @param cs casa::RunCaseSet object
      /// @return ErrorHandler::NoError
      ErrorHandler::ReturnCode applyMutations( RunCaseSet & cs );

      /// @brief Validate Cauldron models for consistency and valid parameters range. This function should be 
      ///         called after ScenarioAnalysis::applyMutations()
      /// @param cs casa::RunCaseSet object to be validated
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode validateCaseSet( RunCaseSet & cs );

      /// @brief Get run manager associated with this scenario analysis
      /// @return reference to the instance of run manager
      RunManager & runManager();

      /// @brief Get data digger associated with this scenario analysis
      /// @return reference to the instance of data digger
      DataDigger & dataDigger();

      /// @brief Get list of observables for this scenario
      /// @return Observables set manager
      ObsSpace & obsSpace();

      /// @brief Add the new response surface polynomial approximation to scenario analysis. If
      ///        list of DoE experiments is not empty - calculate proxy for corresponded cases set
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode addRSAlgorithm( const char                     * name            ///< proxy name
                                             , int                              order           /*! order of polynomial approximation. Possible values are -1, 0, 1, 2, 3.
                                                                                                If parameter value is set to -1 it switch on the automatic search for the order
                                                                                                of polynomial approximation. In this mode the proxy tries to find the optimal 
                                                                                                polynomial up to third order. Internally,a polynomial representation
                                                                                                is set to 0 order first. Next, this representation is improved by carefully 
                                                                                                adding or removing polynomial terms one by one, until no (significant) improvement 
                                                                                                can be found. To avoid over-fitting, 75% of the added cases are randomly selected 
                                                                                                and used for proxy building. The other 25% are used as internal blind tests. As a
                                                                                                further improvement, the autosearch method repeats the random case selection 10 
                                                                                                times, to guarantee that sufficiently many, different blind tests are used. */
                                             , RSProxy::RSKrigingType           krType          ///< do we need Kriging interpolation, and which one?
                                             , const std::vector<std::string> & doeList         ///< list of DoE experiments name to calculate polynomial coefficients
                                             , double                           targetR2 = 0.95 /*! A target value can be set for the so-called adjusted R2 that is an (adjusted) 
                                                                                                indicator for the quality of the polynomial fit. This value must range between 0 
                                                                                                (very poor target) and 1 (highest target). Note that a high regression quality 
                                                                                                does not automatically imply a good predictability.
                                                                                                Note: this parameter is taking in account only when automatic search for polynomial 
                                                                                                order is set. */
                                             );
      
      /// @brief Get response surface proxies list of this scenario.
      /// @return set of proxies
      RSProxySet & rsProxySet();

      /// @brief Get sensitivity calculator
      /// @return reference to sensitivity calculator object
      SensitivityCalculator & sensitivityCalculator();

      /// @brief Get all cases for this scenario. The list will include cases generated by MC/MCMC only
      /// @return array of casa::Case objects
      RunCaseSet & mcCaseSet();
      
      /// @brief Define type of Monte Carlo algorithm which will be used in this scenario analysis      
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode setMCAlgorithm( MonteCarloSolver::Algorithm               algo                                   /**< Monte Carlo algorithm type */
                                             , MonteCarloSolver::KrigingType             interp = MonteCarloSolver::NoKriging   /**< Do we need Kriging interpolation? If yes, the 
                                                                                                                                     response surface proxy must also use it. */
                                             , MonteCarloSolver::PriorDistribution       priorDist = MonteCarloSolver::NoPrior  /**< How to use variable parameter PDF. If it is set
                                                                                                                                     to NoPrior, uniform block PDF is assumed. */
                                             , MonteCarloSolver::MeasurementDistribution measureDist = MonteCarloSolver::Normal /**< How measurements are distributed */
                                             );
      
      /// @brief Get Monte Carlo solver
      /// @return reference to Monte Carlo solver. If MC solver algorithm wasn't defined befor by ScenarioAnalysis::setMCAlgorithm(), it
      ///         will be set up to MC with no Kriging by default.
      MonteCarloSolver & mcSolver();

      /// @brief After the Monte Carlo simulation it creates project file with parameters set which corresponds to the
      ///        Monte Carlo sampling point with minimal RMSE
      /// @param projFileName name of the project file for best matched case project file
      /// @param mcSampleNum Monte Carlo sample number to be exported as calibrated project
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode saveCalibratedCase( const char * projFileName, size_t mcSampleNum );

      /// @brief Get serialization version number
      // History:
      // version 0: initial implementation
      // version 1: 
      // version 2: Added SensitivityCalculator
      // version 3: Added RunManager cases state
      // version 4: SUMlib updated to the latest version
      // version 5: Changed the way how variable parameters set is serialized to keep the original order of variable parameters
      int version() { return 5; }

      /// @brief Save scenario to the file
      /// @param fileName - name of the file for scenario to be saved in
      /// @param fileType - "bin"/"txt" - how to save scenario - in binary or in text
      ErrorHandler::ReturnCode saveScenario( const char * fileName, const char * fileType );

      /// @brief  Create new ScenarioAnaylysis object and read all data from the given file
      /// @param  fileName name of the file.
      /// @param  fileType "bin"/"txt" file type
      /// @return null if it fails, else the new casa::ScenarioAnalysis object.
      static ScenarioAnalysis * loadScenario( const char * fileName, const char * fileType );

   private:
      /// @brief Copy constructor is disabled
      ScenarioAnalysis( const ScenarioAnalysis & );

      /// @brief Copy operator is disabled
      ScenarioAnalysis & operator = ( const ScenarioAnalysis & );

      /// @{
      /// Implementation part
      /// @brief Class which hides all interface implementation
      class ScenarioAnalysisImpl;
      
      std::auto_ptr<ScenarioAnalysisImpl> m_pimpl; ///< The actual object which will do all the job
      /// @}
   };
}

#endif // CASA_SCENARIO_ANALYSIS
