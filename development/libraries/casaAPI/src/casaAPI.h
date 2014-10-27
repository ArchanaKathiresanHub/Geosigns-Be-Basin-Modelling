//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file casaAPI.h
/// @brief This file keeps API declaration for performing UA/SA using DoE and response surface methodology

#ifndef CASA_API
#define CASA_API

#include "RunCaseSet.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "DataDigger.h"
#include "DoEGenerator.h"
#include "ErrorHandler.h"
#include "MonteCarloSolver.h"
#include "RSProxy.h"
#include "RSProxySet.h"
#include "ObsSpace.h"
#include "RunManager.h"
#include "VarSpace.h"

#include "cmbAPI.h"

#include <memory>
#include <vector>

/// @page CASA_API_Descr Computer Aided Scenario Analysis API
/// API for Computer Aided Scenario Analysis (CASA) provides set of classes for performing
/// Uncertainty/Sensitivity analysis using Design of Experiments (DoE) and response surface methodology (RSM)
///
/// An important application area, related to the quantitative assessment of model uncertainties,
/// involves Experimental Design (also known as Design of Experiments), Response Surface Modeling
/// (also known as Proxy modeling) and - if historical data is available - Markov Chain Monte Carlo
/// (MCMC). Here, the behavior of certain observables in a simulation model (for instance, the produced
/// cumulative oil) is approximated by the response surface as a function of the input parameters,
/// which is calculated from a number of simulation cases. Here, the parameters have been varied
/// according to an appropriately chosen scheme (the Experimental Design). The response surface for
/// an observable is also called a proxy for this observable and allows a cheap evaluation (estimation)
/// of the observable. Once these proxies are available, algorithms like Monte Carlo (MC) and MCMC
/// utilize them to rapidly infer stochastic properties of the model (like P10, P50, P90 cases)
///
/// This manual is divided in the following sections:
/// - @subpage CASA_ScenarioAnalysisPage
/// - @subpage CASA_ClassHierachyPage
/// - @subpage CASA_VarParametersAndObservablesPage
/// - @subpage CASA_DoEGeneratorPage
/// - @subpage CASA_RunManagerPage
/// - @subpage CASA_DataDiggerPage
/// - @subpage CASA_RSProxyPage
/// - @subpage CASA_MonteCarloSolverPage
///
/// The block diagram which describes an interaction between different parts of CASA API could be seen here:
/// @image html TopView.png "High level design diagram for Cauldron  CASA API library"
///
/// @page CASA_ClassHierachyPage CASA interface classes hierarchy description.
/// The top level class is the casa::ScenarioAnalysis. It includes and provides access to the following set of classes:
///   - casa::BusinessLogicRulesSet - namespace which keeps set of high level function to define variable parameters in scenario analysis
///   - casa::DoEGenerator - for generating design of experiments cases
///   - casa::RunManager - for submitting jobs to the HPC cluster
///   - casa::DataDigger - for extracting observables (targets) value from the simulation results
///   - casa::RSProxy - for calculating coefficients for polynomial approximation of the response surface 
///   - casa::MonteCarloSolver - for performing Monte Carlo/Markov Chain Monte Carlo calculations
///
///  The set of data classes includes:
///   - casa::VarParameter - @link CASA_VarParameterPage Base class for variable parameter description.@endlink It is inhereted by:
///      -# casa::VarPrmContinuous  - @link CASA_VarPrmContinuousPage Continuous variable parameters @endlink
///      -# casa::VarPrmCategorical - @link  CASA_VarPrmCategoricalPage Variable parameters for categorical values @endlink
///   - casa::VarSpace - @link CASA_VarSpacePage Container which keeps and allows to manipulate variable parameters description.@endlink
///   - casa::RunCase - main class for keeping variable parameters value and targets value for a single Cauldron project run or for
///                     a single point of Monte Carlo simulation results
///   - casa::RunCaseSet - container which keeps a collection of casa::RunCase objects
///   - casa::Parameter - base class for keeping a single parameter value. The following set of parameters is implemented now:
///     -# casa::PrmSourceRockTOC - @link CASA_SourceRockTOCPage Source rock initial Total Organic Contents (TOC) parameter @endlink
///     -# casa::PrmTopCrustHeatProduction - @link CASA_TopCrustHeatProductionPage Top crust heat production rate parameter @endlink
///     -# casa::PrmOneCrustThinningEvent - @link CASA_OneCrustThinningEventPage Crust thinning parameter @endlink
///   - casa::Observable - base class which keeps a describtion of target value from simulation results. It also could include reference 
///                        value from measurements. casa::ScenarioAnalysis keeps one set of Observables in casa::ObsSpace container.
///                        The following set of implemented targets is implemented now:
///      -# casa::ObsGridPropertyXYZ - @link CASA_ObservableGridPropXYZPage  Any Cauldron grid property at specified XYZ point.@endlink
///      -# casa::ObsGridPropertyWell - @link CASA_ObservableGridPropWellPage Cauldron grid property along a well trajectory. @endlink
///   - casa::ObsValue - base class which keeps target value itself. Each casa::RunCase keeps a unique set of casa::ObsValue objects with
///                      the simulation results from this case. 
/// 
///   The UML class diagram of CASA API is here:
///   @image html CASA-class-dagram.png "UML class diagram Cauldron CASA API library"
///   The green blocks show API user interface classes, red blocks show the implementation of API.


/// @page CASA_VarParametersAndObservablesPage Variable Parameters and Observables
/// @subpage CASA_ParameterPage
///
/// @subpage CASA_VarParameterPage - a parameter in Cauldron project file which exact value is unknown.
/// There are only some estimations on it value range. For example - source rock TOC - [5:20]%.
///
/// Variable parameters are kept by @subpage CASA_VarSpacePage container. Types of implemented in API
/// variable parameters are described here: @link CASA_VarParameterPage @b Variable @b Parameter @endlink
///
/// @link CASA_ObservablePage @b Observable @endlink - a description of simulator output value. Could be any data from 
/// the simulation results.
/// Usually, modeler have some measurements for the basin from wells. Some of observables could correspond those measurements.
/// For example temperature or VRe at some position and depth for "@i present @i day" time. 
/// Some observables could have integrated over space or/and time values and be used for risk assessment, 
/// for example - a total amount of HC in a trap.
/// 
/// Observables are kept by @subpage CASA_ObsSpacePage
/// casa::DataDigger, after the simulation stage, goes over all observables
/// and extracts their values from simulation results. For each case, casa::DataDigger creates 
/// a list of casa::ObsValue objects which keep the observables values for this case.
///
/// Types of observables implemented in API are described here: \subpage CASA_ObservablePage


/// @page CASA_ScenarioAnalysisPage Scenario analysis workflows
/// Scenario analysis API allows to reproduce the following workflows to perform a scenario analysis
/// -# <b> First guess </b> 
/// 
/// -# <b> Parameters screening and sensitivity analysis. </b> Allows to screen a wide range of input parameters 
/// (maps, boundary condition tables or scalar values) on their importance of influencing a target model output and secondary calibration 
/// responses. Target outputs are generally
///      - trap properties( e.g.volumes, GOR, API ),
///      - property predictions at hypothetical well / prospect locations( e.g.pore pressure, temperatures ), or
///      - aggregate property predictions over an area / volume( e.g.cumulative expulsion over a kitchen, total adsorbed hydrocarbons, average   
///      - direction of migration, etc. )
/// Secondary calibration responses represent simulated values of T, VR and pressures at the position and depth of available measured data.
///
/// -# <b> Semi-automated calibration. </b> Calibration tries to achieve the best match of model
/// outputs and measurement data by use of minimization algorithms.The modeler has however a
/// choice on what "best" means via the comparison criteria. This is how project objectives 
/// can be included and why it is semi-automated on purpose.
///
/// -# <b> Uncertainty and risk analysis.</b> Many parameter combinations may yield the comparably good model fits.
///   - Some parameter influencing prediction quantities may hardly be constraint by available data
///   - Consider a large number of such models and combine predictions in empirical probability distributions
///

/// @brief Namespace which keeps API for performing uncertainty/sensitivity scenario analysis
namespace casa
{
   class ScenarioAnalysis;
   
   /// @brief Name space which keeps set of high level functions for variable parameters definitions.
   ///        This set of functions provides an easy way to add variable parameters to casa::ScenarioAnalysis
   namespace BusinessLogicRulesSet
   {
      /// @brief Add a parameter to variate layer thickness value [m] in given range
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VaryLayerThickness(
              ScenarioAnalysis    & sa          ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
            , const char          * layerName   ///< [in] name of the layer in base case model to variate it thickness
            , double                minVal      ///< [in] the minimal range value 
            , double                maxVal      ///< [in] the maximal range value 
            , VarPrmContinuous::PDF rangeShape  /**< [in] defines a type of probability function for the parameter. If PDF needs some middle
                                                     parameter value it will be taken from the base case model */
            );

      /// @brief Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VaryTopCrustHeatProduction(
              ScenarioAnalysis    & sa          ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
            , double                minVal      ///< [in] the minimal range value 
            , double                maxVal      ///< [in] the maximal range value 
            , VarPrmContinuous::PDF rangeShape  /**< [in] defines a type of probability function for the parameter. If PDF needs some middle
                                                     parameter value it will be taken from the base case model */
            );

      /// @brief Add a parameter to variate source rock lithology TOC value @f$ [\%] @f$ in given range
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VarySourceRockTOC(
              ScenarioAnalysis    & sa          ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
            , const char          * layerName   ///< [in] layer name. If layer has mixing of source rocks, for all of them TOC will be changed
            , double                minVal      ///< [in] the minimal range value 
            , double                maxVal      ///< [in] the maximal range value 
            , VarPrmContinuous::PDF rangeShape  /**< [in] defines a type of probability function for the parameter. If PDF needs some middle 
                                                          parameter value it will be taken from the base case model */
            );

      /// @brief Add 4 variable parameters to one crust thinning event.
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VaryOneCrustThinningEvent(
            ScenarioAnalysis    & sa             ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
          , double                minThickIni    ///< [in] minimal range value for the initial crust thickness
          , double                maxThickIni    ///< [in] maximal range value for the initial crust thickness
          , double                minT0          ///< [in] minimal range value for the start time of crust thinning
          , double                maxT0          ///< [in] maximal range value for the start time of crust thinning
          , double                minDeltaT      ///< [in] minimal range value for the duration of crust thinning
          , double                maxDeltaT      ///< [in] maximal range value for the duration of crust thinning
          , double                minThinningFct /**< [in] minimal range value for the crust thickness factor (final crust 
                                                           thickness is equal the initial thickness multiplied by this factor) */
          , double                maxThinningFct ///< [in] maximal range value for the crust thickness factor 
          , VarPrmContinuous::PDF pdfType        /**< [in] probability function type for the variable parameter. If PDF needs 
                                                      some middle parameter value it will be taken from the base case model */
          );                                   
   };

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

      /// @brief Get base case model
      /// @return pointer to the base case model if it was set before, empty model otherwise
      mbapi::Model & baseCase();

      /// @brief Set path where SA will generate a bunch of cases. By default it is the current folder
      /// @param pathToCaseSet
      /// @return ErrorHandler::NoError on success, or ErrorHandler::WrongPath if SA can't create folders/files using this path
      ErrorHandler::ReturnCode setScenarioLocation( const char * pathToCaseSet );

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

      /// @brief Add new response surface polynomial approximation to scenario analysis
      /// @param name proxy name
      /// @param order order of polynomial approximation. 
      /// @param krType do we need Kriging interpolation, and which one?
      ErrorHandler::ReturnCode addRSAlgorithm( const char * name, int order, RSProxy::RSKrigingType krType );
      
      /// @brief Get response surface proxies set which were defined for the scenario.
      /// @return set of proxies
      RSProxySet & rsProxySet();

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

#endif // CASA_API
