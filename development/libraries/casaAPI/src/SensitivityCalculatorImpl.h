//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SensitivityCalculatorImpl.h
/// @brief This file keeps declaration of the implementation part of variable parameters sensitivity calculator

#ifndef CASA_API_SENSITIVITY_CALCULATOR_IMPL_H
#define CASA_API_SENSITIVITY_CALCULATOR_IMPL_H

// CASA
#include "CasaDeserializer.h"
#include "SensitivityCalculator.h"

// SUMlib
#include "CompoundProxy.h"

// STL
#include <memory>

namespace casa
{
   class RSProxyImpl;

   /// @brief Allows to find all variable parameters sensitivity with respect to each observable and
   /// build Tornado and Pareto diagrams
   class SensitivityCalculatorImpl : public SensitivityCalculator
   {
   public:
      /// @brief Constructor
      SensitivityCalculatorImpl( const VarSpace * varPrmsSet, const ObsSpace * obsSet );

      /// @brief Destructor
      virtual ~SensitivityCalculatorImpl() {;}

      /// @brief Calculate data for Pareto diagram construction
      /// @param proxy[in]     pointer to the proxy object which is used to calculate parameters sensitivities
      /// @param sensInfo[out] data set which contains all parameters cumulative sensitivities which can be used to create Pareto diagram
      /// @return ErrorHandler::NoError in case of success, or error code otherwise
      virtual ErrorHandler::ReturnCode calculatePareto(  const RSProxy * proxy, ParetoSensitivityInfo  & sensInfo );

      /// @brief Construct 1st order proxy for given set of cases and calculate Tornado variable parameters sensitivities
      /// @param cs[in] case set manager which keeps run cases for DoE experiments
      /// @param expName[in] list of DoE names which will be used to create proxy for parameters sensitivity calculation
      /// @param tornSens[out] array which contains for each observable, a set of variable parameters sensitivities which could be used for creation Tornado diagram
      /// @return ErrorHandler::NoError in case of success, or error code otherwise
      virtual ErrorHandler::ReturnCode calculateTornado( RunCaseSet & cs, const std::vector<std::string> & expNames, std::vector<TornadoSensitivityInfo> & tornSens );


      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "SensitivityCalculatorImpl"; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Create a new instance and deserialize it from the given stream
      SensitivityCalculatorImpl( CasaDeserializer & inStream, const char * objName );

   protected:
      const ObsSpace * m_obsSpace; // set of observable definitions
      const VarSpace * m_varSpace; // set of variable parameters descriptions

   private: // not copyable
      SensitivityCalculatorImpl( const SensitivityCalculatorImpl & sc );
      SensitivityCalculatorImpl & operator = ( const SensitivityCalculatorImpl & sc );
   };

}

#endif // CASA_API_SENSITIVITY_CALCULATOR_IMPL_H

