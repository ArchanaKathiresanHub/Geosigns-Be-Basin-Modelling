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

namespace casa
{  
   // Data digger implementation 
   class DataDiggerImpl : public DataDigger
   {
   public:
      // Constructor / Destructor
      DataDiggerImpl();
      virtual ~DataDiggerImpl();

      /// @brief Insert into project file request for observables values using datadriller cauldron app and data mining interface.\n
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCaseSet object which keeps list of observables
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCaseSet & rcs );

      // Collect observables value from simulation results for given case
      // obs - set of observables 
      // rcs - RunCaseSet object which keeps list of cases and reference to the cauldron model
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCaseSet & rcs );

      // Serialization / Deserialization
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

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
