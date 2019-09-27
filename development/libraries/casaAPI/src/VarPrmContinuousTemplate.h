//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmContinuousTemplate.h
/// @brief This file keeps API declaration for handling continuous parameters

#ifndef CASA_API_VAR_PRM_CONTINOUS_TEMPLATE_H
#define CASA_API_VAR_PRM_CONTINOUS_TEMPLATE_H

// CASA API
#include "VarPrmContinuous.h"
#include "CasaDeserializer.h"

// STL/C
#include <memory>
#include <set>

/// @page CASA_VarPrmContinuousTemplatePage Continuous influential parameter
///
/// Continuous parameter - a parameter that can take any value between certain bounds
/// (for instance, a fault-sealing factor that varies in the [0, 1] range)
/// The template argument is expected to be of VarParameter type


namespace casa
{
/// @brief Influential parameter with continuous value range.
/// The parameter value can be represented by the one or several doubles values
template<class Prm>
class VarPrmContinuousTemplate : public VarPrmContinuous
{
public:
  /// @brief Constructor base on the base case parameter
  /// @param[in] original  base case parameter, containing intelligence on what to change for derived cases
  /// @param[in] name
  /// @param[in] minValue  minimum of the range for the parameter
  /// @param[in] maxValue  maximum of the range for the parameter
  /// @param[in] rangeShape probability density function distribution of the parameter
  VarPrmContinuousTemplate( const Prm& original,
                            const char* name,
                            double minValue,
                            double maxValue,
                            PDF rangeShape = PDF::Block );

  virtual ~VarPrmContinuousTemplate() = default;

  /// @brief Get name of influential parameter in short form
  /// @return array of names for each subparameter
  std::vector<std::string> name() const override;

  /// @brief Get number of subparameters if it is more than one
  /// @return dimension of influential parameter
  size_t dimension() const override;

  const char * typeName() const;

  /// @brief Create parameter by reading the values stored in the project file
  /// @param[in, out] mdl the model where the parameters values should be read
  /// @param[in] vin an input vector with parameter cpecific values if they needed to extract parameter value from the model
  /// @return the new parameter read from the model
  SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const override;

  /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
  /// @param[in,out] vals iterator which points to the first parameter value.
  /// @return new parameter for given set of values
  virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const override;

  /// @brief Wrapper function to use in C# through Swig due to absence of iterators in Swig
  /// @param vals vector with parameters values.
  /// @param[in,out] off position in array where this parameter values are located
  /// @return new parameter for given set of values
  virtual SharedParameterPtr newParameterFromDoubles(std::vector<double> & vals, int & off) const override;

  /// @brief Convert Cauldron parameter values to SUMlib values for some influential parameters
  /// @param prm cauldron parameter with to this influential parameter corresponded type
  /// @return parameter values suitable for SUMlib
  virtual std::vector<double> asDoubleArray( const SharedParameterPtr prm ) const;

  /// @brief Create a new VarPrmContinuousTemplate instance and deserialize it from the given stream
  /// @param dz input stream
  /// @param objName expected object name
  /// @return new observable instance on susccess, or throw and exception in case of any error
  VarPrmContinuousTemplate(CasaDeserializer & dz, const unsigned int objVer );

  // Available slots
  // called from categorical parameter on which this parameter depends on
  virtual void onCategoryChosen( const Parameter * );
  virtual void onSerialization( CasaSerializer::ObjRefID objSerID );

private:
  /// @brief Defines version of serialized object representation. Must be updated on each change in save()
  /// @return Actual version of serialized object representation
  virtual unsigned int version() const { return 1; }
};

}

#include "VarPrmContinuousTemplate.hpp"

#endif // CASA_API_VAR_PRM_CONTINOUS_TEMPLATE_H
