//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DataDigger.h
/// @brief This file keeps API declaration for data digger.

#ifndef CASA_API_DATA_DIGGER_H
#define CASA_API_DATA_DIGGER_H

#include "CasaSerializer.h"
#include "RunCaseSet.h"
#include "ErrorHandler.h"
#include "Observable.h"
#include "ObsSpace.h"

/// @page CASA_DataDiggerPage Data Digger
///
/// Data digger requests and collects observable values from the simulation results.
/// To full fill this purpose it first adds to each DoE run case project a request for observable values. 
/// This happens just after the base case mutation phase. Each casa::ScenarioAnalysis object keeps a 
/// @link casa::ObsSpace list @endlink of user defined @link CASA_ObservablePage observable definitions objects @endlink.
/// Each observable definition has an information about 
///   - at what age
///   - which property 
///   - at which location  
/// value should be extracted from the simulation results. It also could contains
/// a reference value and a standard deviation value, if this observable is related to any measurement. 
/// From an observable definition object, data digger calls casa::Observable::requestObservableInModel() method, 
/// to add one or several records for this observable in @b DataMiningIoTbl table in project file. 
///
/// casa::DataDigger uses the functionality of datadriller cauldron application to extract 
/// observables values from HDF files with simulation results and store them in @b DataMiningIoTbl 
/// table in project file. casa::RunManager always adds job for datadriller run at the end of the applications 
/// pipeline. 
///
/// After all DoE runs completion, the casa::DataDigger goes over the list of completed cases and
/// for each run case, it loads a project file and converts observable values from @b DataMiningIoTbl table
/// to a list of casa::ObsValue objects. The order of observable values is the same as the order of observable
/// definitions in casa::ObsSpace object. In addition to this, each casa::ObsValue object keep a pointer to
/// its observable definition object. Here is the implemented list of observable values data types:
///
/// -# @subpage CASA_ObsValueDoubleScalar 
/// -# @subpage CASA_ObsValueDoubleArray
/// -# @subpage CASA_ObsValueTransformable


namespace casa
{
   /// @class DataDigger DataDigger.h "DataDigger.h"
   /// @brief Data digger extracts observables value from simulation results for single case
   ///        Also it performs like observables object factory.
   class DataDigger : public ErrorHandler, public CasaSerializable
   {
   public:
      /// @brief Destructor
      virtual ~DataDigger() {}

      /// @brief Insert into project file request for observables values using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCaseSet object which keeps list of run cases
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCaseSet & rcs ) = 0;

      /// @brief Insert into project file request for observables values using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCase object pointer
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCase * rcs ) = 0;

      /// @brief Insert into project file requests for observables value in window (e.g. around a well) using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation. Note: Here it is also checked that the observable matches the observable origin of window.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rc casa::RunCase object
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservablesInWindow( ObsSpace & obSpace, RunCase * rc ) = 0;

      /// @brief Collect observables value from simulation results for the given case set
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will create ObsValue object for each run case
      /// @param rcs casa::RunCaseSet object which keeps a set of casa::RunCase objects 
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCaseSet & rcs ) = 0;

      /// @brief Collect observables value from simulation results for the given case
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will create ObsValue object 
      /// @param rcs casa::RunCase object which keeps list of observables and reference to Cauldron model
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCase * rcs ) = 0;

   protected:
      DataDigger() {}
   };
}

#endif // CASA_API_DATA_DIGGER_H
