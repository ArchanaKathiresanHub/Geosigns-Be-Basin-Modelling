//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmDiscrete.h
/// @brief This file keeps API declaration for handling continuous parameters. 

#ifndef CASA_API_VAR_PRM_DISCRETE_H
#define CASA_API_VAR_PRM_DISCRETE_H

#include "Parameter.h"
#include "VarParameter.h"
#include "CasaDeserializer.h"


#include <memory>

/// @page CASA_VarPrmDiscretePage Discrete variable parameter
///
/// Discrete parameter - a parameter that can have only a (numerical) value out of a given set of numbers. 
///
/// The following list of variable discrete parameters is implemented in CASA API

namespace casa
{
   /// @brief Variable parameter with continuous value range.
   /// The parameter value can be represented by the one or several doubles values
   class VarPrmDiscrete : public VarParameter
   {
   public:
      /// @brief Destructor
      virtual ~VarPrmDiscrete() {;}

      /// @brief Define this variable parameter as a continuous
      /// @return VarParameter::Discrete
      virtual Type variationType() const { return Discrete; }

      /// @brief A parameter which corresponds the minimal range value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr minValue() const { return m_values[m_minValue]; }

      /// @brief A parameter which corresponds the maximal range value of the variable parameter 
      /// @return the parameter object should be deleted by a caller
      virtual const SharedParameterPtr maxValue() const { return m_values[m_maxValue]; }

      /// @brief A parameter which corresponds the base value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr baseValue() const { return m_values[m_baseValue]; }

      /// @brief Get all values as a set
      /// @return set of possible parameter values
      virtual const std::vector<SharedParameterPtr> valuesSet() const { return m_values; }

      /// @brief Returns user specified weights for each parameter value. If user didn't specify weights, empty array will be returned
      /// @return array with parameter values weights
      virtual std::vector<double> weights() const { return m_weights; }
 
      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const = 0;

      /// @brief Wrapper function to use in C# through Swig due to absence of iterators in Swig
      /// @param vals vector with parameters values.
      /// @param[in,out] off position in array where this parameter values are located
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double> & vals, int & off ) const
      {
         std::vector<double>::const_iterator it = vals.begin() + off;
         std::vector<double>::const_iterator sit = it;
         SharedParameterPtr ret = newParameterFromDoubles( it );
         off += static_cast<unsigned int>(it - sit);
         return ret;
      }

      /// @brief Create a new VarPrmDiscrete instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objName expected object name
      /// @return new observable instance on susccess, or throw and exception in case of any error
      static VarPrmDiscrete * load( CasaDeserializer & dz, const char * objName );

   protected:
      VarPrmDiscrete() {;}

      std::vector<SharedParameterPtr> m_values;
      std::vector<double>             m_weights;

      size_t  m_baseValue;   ///< Base parameter value, used also as object factory for concrete parameter value
      size_t  m_minValue;    ///< Base parameter value, used also as object factory for concrete parameter value
      size_t  m_maxValue;    ///< Base parameter value, used also as object factory for concrete parameter value

   private:
   };

}

#endif // CASA_API_VAR_PRM_DISCRETE_H
