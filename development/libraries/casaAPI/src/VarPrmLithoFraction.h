//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmLithoFraction.h
/// @brief This file keeps API implementation for handling variation of the lithofractions for a given layer

#ifndef CASA_API_VAR_PARAMETER_LITHO_FRAC_H
#define CASA_API_VAR_PARAMETER_LITHO_FRAC_H

#include "VarPrmContinuous.h"
#include "PrmLithoFraction.h"

namespace casa
{
   /// @brief Variation for casa::VarPrmLithoFraction parameter
   class VarPrmLithoFraction : public  VarPrmContinuous
   {
   public:

      /// @brief Construct influential parameter for variation of the lithofraction parameter
      VarPrmLithoFraction( const std::string                 & layerName          ///< stratigraphic layer name
         , const std::vector<int>                                                 & lithoFractionsInds ///< indexes of the lithofractions
         , const std::vector<double>                                              & baseLithoFrac      ///< base lithofractions values
         , const std::vector<double>                                              & minLithoFrac       ///< min range value for the lithofractions
         , const std::vector<double>                                              & maxLithoFrac       ///< max range value for the lithofractions
         , casa::VarPrmContinuous::PDF                                              pdfType            ///< probability density functions of the lithofractions
         , const std::string                                                      & name               ///< user specified parameter name
         );

      /// @brief Destructor
      virtual ~VarPrmLithoFraction() { ; }

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
      /// @param[in, out] mdl the model where the parameters values should be read
      /// @param[in] vin an input vector (e.g. spatial/temporal) coordinates. Used to convert 3D project into 1D
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const;

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model                & mdl     /// the model to set the new averaged parameter
                                                   , const std::vector<double>   & xin     /// the vector to store 1D projects x coordinates
                                                   , const std::vector<double>   & yin     /// the vector to store 1D projects y coordinates
                                                   , const std::vector<SharedParameterPtr> & prmVec  /// the vector to stores the optimal parameter value of each 1D project
                                                   , const SmoothingParams& smoothingParams ///< set of smoothing parameters
                                                   ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmLithoFraction"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmLithoFraction( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::string                                        m_layerName;           ///< stratigraphic layer name
      std::vector<int>                                   m_lithoFractionsInds;  ///< indexes of the lithofractions
   };
}

#endif // CASA_API_VAR_PARAMETER_LITHO_FRAC_H
