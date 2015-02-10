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

// CASA
#include "ScenarioAnalysis.h"

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
/// - @subpage CASA_SensitivityCalculatorPage
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
///   - casa::SensitivityCalculator - for calculating data for Tornado/Pareto diagrams
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
///     -# casa::PrmSourceRockHI  - @link CASA_SourceRockHIPage  Source rock Hydrogen Index (HI) initial ratio parameter @endlink
///     -# casa::PrmTopCrustHeatProduction - @link CASA_TopCrustHeatProductionPage Top crust heat production rate parameter @endlink
///     -# casa::PrmOneCrustThinningEvent - @link CASA_OneCrustThinningEventPage Crust thinning parameter @endlink
///     -# casa::PrmPorosityModel - @link CASA_PorosityModelPage lithology porosity model parameters @endlink
///   - casa::Observable - base class which keeps a describtion of target value from simulation results. It also could include reference 
///                        value from measurements. casa::ScenarioAnalysis keeps one set of Observables in casa::ObsSpace container.
///                        The following set of implemented targets is implemented now:
///      -# casa::ObsGridPropertyXYZ - @link CASA_ObservableGridPropXYZPage  Any Cauldron grid property at specified XYZ point. @endlink
///      -# casa::ObsGridPropertyWell - @link CASA_ObservableGridPropWellPage Cauldron grid property along a well trajectory. @endlink
///      -# casa::ObsTrapProp - @link CASA_ObservableTrapPropPage Any trap property which could be extracted by datadriller after migration calculation. @endlink
///      -# casa::ObsSourceRockMapProp - @link CASA_ObservableSourceRockMapPropPage Any Genex map property for source rock layer at specified XY point. @endlink
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

      /// @brief Add a parameter to variate source rock lithology initial TOC value @f$ [\%] @f$ in given range
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VarySourceRockTOC(
            ScenarioAnalysis    & sa          ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
          , const char          * layerName   ///< [in] layer name. If layer has mixing of source rocks, for all of them TOC will be changed
          , double                minVal      ///< [in] the minimal range value 
          , double                maxVal      ///< [in] the maximal range value 
          , VarPrmContinuous::PDF rangeShape  /**< [in] defines a type of probability function for the parameter. If PDF needs some middle 
                                                        parameter value it will be taken from the base case model */
          );

      /// @brief Add a parameter to variate source rock lithology HI initial ratio value @f$ [kg/tonne] @f$ in given range
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VarySourceRockHI(
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

      /// @brief Add porosity model parameters variation
      /// @return ErrorHandler::NoError on success or error code otherwise
      ErrorHandler::ReturnCode VaryPorosityModelParameters( 
            ScenarioAnalysis    & sa            ///< [in,out] casa::ScenarioAnalysis object reference, if any error, this object will keep an error message
          , const char *          litName       ///< [in] lithology name
          , const char *          modelName     ///< [in] porosity model name, supported now: Exponential, Soil_Mechanics, Double_Exponential
          , double                minSurfPor    ///< [in] min range value for the surface porosity 
          , double                maxSurfPor    ///< [in] max range value for the surface porosity
          , double                minCompCoef   ///< [in] min range value for the compaction coefficient
          , double                maxCompCoef   ///< [in] max range value for the compaction coefficient
          , double                minMinPor     ///< [in] min range value for the minimal porosity value (Double_Exponential model only)
          , double                maxMinPor     ///< [in] max range value for the minimal porosity value (Double_Exponential model only)
          , double                minCompCoef1  ///< [in] min range value for the compaction coefficient for the second exponent (Double_Exponential model only)
          , double                maxCompCoef1  ///< [in] max range value for the compaction coefficient for the second exponent (Double_Exponential model only)
          , VarPrmContinuous::PDF pdfType          /**< [in] probability function type for the variable parameter. If PDF needs 
                                                        some middle parameter value it will be taken from the base case model */

          );
   };
}

#endif // CASA_API
