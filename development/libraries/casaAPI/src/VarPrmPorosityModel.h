//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmPorosityModel.h
/// @brief This file keeps API declaration for handling variation of casa::PrmPorosityModel parameter

#ifndef CASA_API_VAR_PARAMETER_POROSITY_MODEL_H
#define CASA_API_VAR_PARAMETER_POROSITY_MODEL_H

#include "VarPrmContinuous.h"
#include "PrmPorosityModel.h"

namespace casa
{
   class MapInterpolator;

   /// @brief Variation for casa::PrmPorosityModel parameter
   class VarPrmPorosityModel: public VarPrmContinuous
   {
   public:
      /// @brief Construct influential parameter for variation of porosity model parameters in lithology
      ///
      /// Porosity model must be defined for each lithology type in project. Each model has own model parameters
      /// not all of them can be independent. CASA API supports now 3 type of porosity model:
      /// -# Exponential
      /// -# Soil mechanics
      /// -# Double exponential
      /// These models are described @link CASA_PorosityModelPage here. @endlink
      VarPrmPorosityModel(const char * lithoName      ///< name of the lithology type
                         , PrmPorosityModel::PorosityModelType mdlType ///< type of porosity model, supported: Exponential/Soil_Mechanics/Double_Exponential
                         , double       baseSurfPor    ///< base value for surface porosity
                         , double       minSurfPor     ///< minimal value for surface porosity
                         , double       maxSurfPor     ///< maximal value for surface porosity
                         , double       baseMinPor     ///< base value for minimal porosity (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       minMinPor      ///< minimal value for minimal porosity (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       maxMinPor      ///< maximal value for minimal porosity (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       baseCompCoeff  ///< base value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       minCompCoeff   ///< minimal value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       maxCompCoeff   ///< maximal value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       baseCompCoeffB ///< base value for compaction coefficient "B" (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       minCompCoeffB  ///< minimal value for compaction coefficient "B" (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       maxCompCoeffB  ///< maximal value for compaction coefficient "B" (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       baseCompRatio  ///< base value for compaction ratio (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       minCompRatio   ///< minimal value for compaction ratio (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , double       maxCompRatio   ///< maximal value for compaction ratio (Double_Exponential model only, for others must be Utilities::Numerical::IbsNoDataValue)
                         , PDF          prmPDF         ///< probability density function for all 4 sub-parameters
                         , const char * name = 0       ///< user specified parameter name
                         );

      virtual ~VarPrmPorosityModel();

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
                                                   , const InterpolationParams& interpolationParams ///< set of interpolation parameters
                                                   , const MapInterpolator& interpolator) const;
      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "VarPrmPorosityModel"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmPorosityModel( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      PrmPorosityModel::PorosityModelType   m_mdlType;   ///< porosity model type
      std::string                           m_lithoName; ///< lithology name
   };
}

#endif // CASA_API_VAR_PARAMETER_POROSITY_MODEL_H

