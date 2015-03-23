//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCategorical.h
/// @brief This file keeps API declaration for handling categorical parameters. 

#ifndef CASA_API_VAR_PRM_CATEGORICAL_H
#define CASA_API_VAR_PRM_CATEGORICAL_H

#include "Parameter.h"
#include "VarParameter.h"
#include "CasaDeserializer.h"

#include <memory>
#include <vector>

/// @page CASA_VarPrmCategoricalPage Variable parameter for categorical values
///
/// Categorical parameter - a parameter that can take some unordered discrete values each of which referring to a
/// different category (for instance, a PVT parameter that can distinguish between a few, non related PVT models)
/// There is no ordering in this labeling (you cannot say that one scenario is ‘larger’ than the other)
///
/// The following list of variable parameters is implemented in CASA API
/// - @subpage CASA_SourceRockTypePage 


namespace casa
{

   /// @brief Class to manage categorical variable parameter type
   class VarPrmCategorical : public VarParameter
   {
   public:
      /// @brief Destructor
      virtual ~VarPrmCategorical() {;}

      /// @brief Define this variable parameter as a categorical
      /// @return VarParameter::Categorical
      virtual Type variationType() const { return Categorical; }

     /// @brief A parameter which corresponds the base value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr baseValue() const { return m_variation[m_baseVal]; }

      /// @brief A parameter which corresponds the minimal range value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr minValue() const { assert( !m_variation.empty() ); return m_variation.front(); }

      /// @brief A parameter which corresponds the maximal range value of the variable parameter 
      /// @return the parameter object should be deleted by a caller
      virtual const SharedParameterPtr maxValue() const { assert( !m_variation.empty() ); return m_variation.back(); }

      /// @brief Check does this parameter was selected for DoE
      /// @return true if min/max values are different, false otherwise
      virtual bool selected() const { return !(*(minValue().get()) == *(maxValue().get())); }

      /// @brief Get user specified weights for each parameter value. If user didn't specify weights,
      ///        empty array will be returned
      /// @return array with parameter values weights
      virtual std::vector<double> weights() const { return m_weights; }

      /// @brief Get list of categorical parameter values as sorted list of unsigned integers.
      ///        Enumeration of categorical values must start from zero
      virtual std::vector< unsigned int> valuesAsUnsignedIntSortedSet() const;

      /// @brief Create a copy of the parameter and assign to the given value. If value is not in var. parameter values set,
      ///        the method will return a zero pointer
      /// @param val new value for parameter
      /// @return the new parameter object which should be deleted by the caller itself
      virtual SharedParameterPtr createNewParameterFromUnsignedInt( unsigned int val ) const { return m_variation[val]; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new VarPrmCategorical instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objName expected object name
      /// @return new observable instance on susccess, or throw and exception in case of any error
      static VarPrmCategorical * load( CasaDeserializer & dz, const char * objName );

   protected:
      VarPrmCategorical() {;}

      /// @brief  Constructor from input stream, implements common part of deserialization for continuous variable parameters
      /// @param dz input stream
      /// @param objVer The object version.
      VarPrmCategorical( CasaDeserializer & dz, unsigned int objVer );


      std::vector<SharedParameterPtr> m_variation;
      size_t                          m_baseVal;

      std::vector<double>             m_weights;
   };
}

#endif // CASA_API_VAR_PRM_CATEGORICAL_H
