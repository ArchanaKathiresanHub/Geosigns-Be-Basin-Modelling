//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DataDiggerImpl.h
/// @brief This file keeps API implementation declaration for data digger.

#ifndef CASA_API_DATA_DIGGER_IMPL_H
#define CASA_API_DATA_DIGGER_IMPL_H

#include "CasaDeserializer.h"
#include "DataDigger.h"

#include <set>

namespace casa
{
   // Data digger implementation
   class DataDiggerImpl : public DataDigger
   {
   public:
      // Constructor / Destructor
      DataDiggerImpl();
      virtual ~DataDiggerImpl();

      /// @brief Insert into project file requests for observables value using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCaseSet object which keeps list of cases
      /// @param excludeSet The indices of runcases which are skipped
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables(ObsSpace & obs, RunCaseSet & rcs , const std::set<int>& excludeSet);

      /// @brief Insert into project file requests for observables value using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rc casa::RunCase object
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCase * rc );

      /// @brief Insert into project file requests for observables value in window (e.g. around a well) using datadriller cauldron app and data mining interface.
      /// Observables must be requested after case mutation. Note: Here it is also checked that the observable matches the observable origin of window.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rc casa::RunCase object
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservablesInWindow( ObsSpace & obSpace, RunCase * rc );

      // Collect observables value from simulation results for given case
      // obs - set of observables
      // rcs - RunCaseSet object which keeps a set of cases
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults(ObsSpace & obs, RunCaseSet & rcs , const std::set<int>& excludedSet);

      // Collect observables value from simulation results for the given case
      // obs - casaObsSpace object which keeps list of observables. For each observable, data digger will create ObsValue object
      // rcs - casa::RunCase object which keeps list of observables and reference to Cauldron model
      // excludeSet - The indices of runcases which are skipped
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCase * rcs );


      // Serialization / Deserialization
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "DataDiggerImpl"; }

      /// @brief Create a new DataDiggerImpl instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new observable instance on susccess, or throw and exception in case of any error
      DataDiggerImpl( CasaDeserializer & inStream, const char * objName );

      /// @}

   protected:
   };
}

#endif // CASA_API_DATA_DIGGER_IMPL_H
