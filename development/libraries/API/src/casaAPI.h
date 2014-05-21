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
#include "ContinuousParameter.h"
#include "CategoricalParameter.h"
#include "DataDigger.h"
#include "DoEGenerator.h"
#include "ErrorHandler.h"
#include "MCSolver.h"
#include "RSProxy.h"
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
/// - \subpage CASA_ScenarioAnalysisPage
/// - \subpage CASA_ClassHierachyPage
/// - \subpage CASA_VarParametersAndObservablesPage
/// - \subpage CASA_DoEGeneratorPage
/// - \subpage CASA_DataDiggerPage
/// - \subpage CASA_RSProxyPage
/// - \subpage CASA_RunManagerPage
/// - \subpage CASA_MCSolverPage
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
///   - casa::MCSolver - for performing Monte Carlo/Markov Chain Monte Carlo calculations
///
///  The set of data classes includes:
///   - casa::VarSpace - variable parameters manager. Keep and allows to manipulate variable parameters with range. VarSpace keeps:
///     -# casa::ContinuousParameter continuous variable parameters set
///     -# casa::CategoricalParameter categorical variable parameters set
///   - casa::RunCase - main class for keeping a single set of variable parameters value and observables from the simulation results
///   - casa::RunCaseSet - container to keep a collection of casa::RunCases objects
///   - casa::Parameter - base class for keeping a single parameter value. The following set of parameters is implemented now:
///     -# case::PrmSourceRockTOC - \subpage CASA_SourceRockTOCPage
///     -# case::PrmTopCrustHeatProduction - \subpage CASA_TopCrustHeatProductionPage
///   - CASAPI::Observable - base class for keeping a target value from simulation results. It also could include reference value from measurements
///      -# casa::PropertyValueIJK
///      -# casa::PropertyValueXYZ
/// 
///   The UML class diagram of CASA API is here:
///   @image html CASA-class-dagram.png "UML class diagram Cauldron CASA API library"
///   The green blocks show API user interface classes, red blocks show the implementation of API.


/// @page CASA_VarParametersAndObservablesPage Variable Parameters and Observables
///
/// @b Variable @b Parameter - a parameter in Cauldron project file which exact value is unknown. 
/// There are only some estimations on it value range. For example - source rock TOC - [5:20]%.
///
/// Variable parameters are kept by \subpage CASA_VarSpacePage
/// Types of variable parameters implemented in API are described here: \subpage CASA_VarParameterPage
///
/// @link CASA_ObservablePage @b Observable @endlink - a simulator output value. Could be any data value from the simulation results. Usually, modelers\n
/// have some measurements for the basin from wells. Some of observables could correspond those measurements.\n
/// For example temperature or VRe at some position and depth for current time. Some observables could be used\n
/// for risk assessment - for example total amount of HC in a trap.
/// 
/// Each RunCase object keeps a copy of the same set of observables for one ScenarioAnalysis object
/// Types of observables implemented in API are described here: \subpage CASA_ObservablePage


/// @page CASA_ScenarioAnalysisPage Scenario analysis workflows
/// Scenario analysis API allows to reproduce the following workflows to perform a scenario analysis
/// -# First guess
/// -# Parameters sensitivity estimation
/// -# Calibration
/// -# Uncertainty and risk analysis

/// @brief Namespace which keeps API for performing uncertainty/sensitivity scenario analysis
namespace casa
{
   /// @brief Namespace which keeps set of high level functions for variable parameters definitions
   namespace BusinessLogicRulesSet
   {
      /// @brief Add a parameter to variate layer thickness value [m] in given range
      /// @param[in] theModel a base case Cauldron model
      /// @param[in] layerName name of the layer in base case model to variate it thickness
      /// @param[in] minVal the minimal range value 
      /// @param[in] maxVal the maximal range value 
      /// @param[in] rangeShape defines a type of probability function for the parameter. If PDF needs some middle parameter value it will be\n
      ///            taken from the base case model
      /// @param[in,out] varPrmsSet VarSpace manager where the new variable parameter will be placed. In case of an error this object will keep\n
      ///                an error message
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VariateLayerThickness( const mbapi::Model & theModel, const char * layerName, double minVal, double maxVal,
                                                      ContinuousParameter::PDF rangeShape, VarSpace & varPrmsSet );

      /// @brief Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
      /// @param[in] theModel a base case Cauldron model
      /// @param[in] minVal the minimal range value 
      /// @param[in] maxVal the maximal range value 
      /// @param[in] rangeShape defines a type of probability function for the parameter. If PDF needs some middle parameter value it will be\n
      ///            taken from the base case model
      /// @param[in,out] varPrmsSet VarSpace manager where the new variable parameter will be placed. In case of an error this object will keep\n
      ///                an error message
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VariateTopCrustHeatProduction( const mbapi::Model & theModel, double minVal, double maxVal, 
                                                              ContinuousParameter::PDF rangeShape, VarSpace & varPrmsSet );
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

      /// @brief Provide variable parameters set manager
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
      
      /// @brief Get all cases for this scenario. The list will include cases generated by MC/MCMC only
      /// @return array of casa::Case objects
      RunCaseSet & mcCaseSet();

      /// @brief Create copy of the base case and set all variable parameters value defined for this case
      /// @param cs casa::RunCase object
      /// @return ErrorHandler::NoError
      ErrorHandler::ReturnCode applyMutation( RunCase & cs );

      /// @brief Validate Cauldron model for consistency and valid parameters range. This function should be 
      ///         called after ScenarioAnalysis::applyMutation()
      /// @param cs casa::RunCase object to be validated
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode validateCase( RunCase & cs );

      /// @brief Get run manager associated with this scenario analysis
      /// @return reference to the instance of run manager
      RunManager & runManager();

      /// @brief Get data digger associated with this scenario analysis
      /// @return reference to the instance of data digger
      DataDigger & dataDigger();
      
      /// @brief Define which order of response surface polynomial approximation of  will be used in this scenario analysis
      /// @param order order of polynomial approximation
      /// @param krType do we need Kriging interpolation, and which one?
      ErrorHandler::ReturnCode setRSAlgorithm( int order, RSProxy::RSKrigingType krType );
      
      /// @brief Get response surface proxy 
      /// @return reference to proxy object
      RSProxy & responseSurfaceProxy();

      /// @brief Define type of Monte Carlo algorithm which will be used in this scenario analysis
      /// @param algo Monte Carlo algorithm
      /// @param interp do we need Kriging interpolation? If yes, the response surface proxy must also use it.
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode setMCAlgorithm( MCSolver::MCAlgorithm algo, MCSolver::MCKrigingType interp = MCSolver::NoKriging );
      
      /// @brief Get Monte Carlo solver
      /// @return reference to Monte Carlo solver. If MC solver algorithm wasn't defined befor by ScenarioAnalysis::setMCAlgorithm(), it
      ///         will be set up to MC with no Kriging by default.
      MCSolver & mcSolver();
      
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