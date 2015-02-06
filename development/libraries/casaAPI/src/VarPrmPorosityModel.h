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
   /// @brief Variation for casa::PrmPorosityModel parameter
   class VarPrmPorosityModel: public VarPrmContinuous
   {
   public:
      /// @brief Construct variable parameter for variation of porosity model parameters in lithology 
      ///
      /// Porosity model must be defined for each lithology type in project. Each model has own model parameters
      /// not all of them can be independent. CASA API supports now 3 type of porosity model:
      /// -# Exponential
      /// -# Soil mechanics
      /// -# Double exponential
      /// These models are described @link CASA_PorosityModelPage here.
      VarPrmPorosityModel( const char * lithoName      ///< name of the lithology type
                         , PrmPorosityModel::PorosityModelType mdlType ///< type of porosity model, supported: Exponential/Soil_Mechanics/Double_Exponential
                         , double       baseSurfPor    ///< base value for surface porosity
                         , double       minSurfPor     ///< minimal value for surface porosity
                         , double       maxSurfPor     ///< maximal value for surface porosity
                         , double       baseMinPor     ///< base value for minimal porosity (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , double       minMinPor      ///< minimal value for minimal porosity (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , double       maxMinPor      ///< maximal value for minimal porosity (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , double       baseCompCoeff  ///< base value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       minCompCoeff   ///< minimal value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       maxCompCoeff   ///< maximal value for compaction coefficient (CompCoefA for Double_Exponential model)
                         , double       baseCompCoeffB ///< base value for compaction coefficient "B" (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , double       minCompCoeffB  ///< minimal value for compaction coefficient "B" (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , double       maxCompCoeffB  ///< maximal value for compaction coefficient "B" (Double_Exponential model only, for others must be UndefinedDoubleValue)
                         , PDF prmPDF                  ///< probability density function for all 4 sub-parameters
                         );

      virtual ~VarPrmPorosityModel();

      /// @brief Get name of variable parameter in short form     
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const;

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmPorosityModel"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmPorosityModel( CasaDeserializer & dz, unsigned int objVer );
      /// @}
   
   protected:
      PrmPorosityModel::PorosityModelType   m_mdlType;
      std::string                           m_lithoName; 
   };
}

#endif // CASA_API_VAR_PARAMETER_POROSITY_MODEL_H

