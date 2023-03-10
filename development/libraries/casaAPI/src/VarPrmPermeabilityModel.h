//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmPermeabilityModel.h
/// @brief This file keeps API declaration for handling variation of casa::PrmPermeabilityModel parameter

#ifndef CASA_API_VAR_PARAMETER_PERMEABILITY_MODEL_H
#define CASA_API_VAR_PARAMETER_PERMEABILITY_MODEL_H

#include "VarPrmContinuous.h"
#include "PrmPermeabilityModel.h"

namespace casa
{
   /// @brief Variation for casa::PrmPermeabilityModel parameter
   class VarPrmPermeabilityModel: public VarPrmContinuous
   {
   public:
      /// @brief Calculate base case permeability with porosity profile as average between minmial and maximal curves
      /// @return averaged model parameters
      static std::vector<double> createBaseCaseMPModelPrms( const std::vector<double> & minModelPrms ///< list of model parameters for minimal range values
                                                          , const std::vector<double> & maxModelPrms ///< list of model parameters for maximal range values
                                                          );

      /// @brief Construct influential parameter for variation of permeability model parameters for the layer with specific lithology
      /// Permeability models and their parameters are described @link CASA_PermeabilityModelPage here @endlink
      VarPrmPermeabilityModel( const char                                  * lithoName     ///< name of the lithology
                             , PrmPermeabilityModel::PermeabilityModelType   mdlType       ///< perm. model type: None/Impermeable/Sanstone/Mudstone/Multipoint
                             , const std::vector<double>                   & baseModelPrms ///< base case model parameters
                             , const std::vector<double>                   & minModelPrms  ///< min case model parameters
                             , const std::vector<double>                   & maxModelPrms  ///< max case model parameters
                             , PDF                                           prmPDF        ///< probability density function for all 4 sub-parameters
                             , const char                                  * name = 0      ///< user specified parameter name
                         );

      virtual ~VarPrmPermeabilityModel();

      /// @brief Get name of influential parameter in short form
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of influential parameter
      virtual size_t dimension() const;

      using VarPrmContinuous::newParameterFromDoubles;
      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in] mdl the model where the parameters values should be read
      /// @param[in] vin an input vector with parameter specific values. Not used in this parameter
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const;

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model              & mdl ///< [in,out] the model where to set the new averaged parameter
                                                   , const std::vector<double> & xin ///< the x coordinates of each 1D project
                                                   , const std::vector<double> & yin ///< the y coordinates of each 1D project
                                                   , const std::vector<SharedParameterPtr> & prmVec /// the optimal parameter value of each 1D project
                                                   , const InterpolationParams& interpolationParams ///< set of smoothing parameters
                                                   , const MapInterpolator& interpolator) const;
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "VarPrmPermeabilityModel"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmPermeabilityModel( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      PrmPermeabilityModel::PermeabilityModelType m_mdlType;   ///< permeability model type
      std::string                                 m_lithoName; ///< lithology name
   };
}

#endif // CASA_API_VAR_PARAMETER_PERMEABILITY_MODEL_H

