//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmTopCrustHeatProduction.h
/// @brief This file keeps API declaration for handling variation of casa::PrmTopCrustHeatProduction parameter

#ifndef CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmTopCrustHeatProduction : public VarPrmContinuous
   {
   public:
      /// @brief Construct variable parameter for the top crust heat production rate
      ///        It takes 2 ranges as parameters but only one should be not empty
      VarPrmTopCrustHeatProduction( const std::vector<double>      & dblRng   ///< simple range of double values [min,max,base]
                                  , const std::vector<std::string> & mapRng   ///< maps range [min map, max map, base map]
                                  , PDF                              pdfType  ///< type of probabiltiy density function for this variable parameter
                                  , const char                     * name = 0 ///< user specified parameter name 
                                  );

      /// @brief Destructor
      virtual ~VarPrmTopCrustHeatProduction();

	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each subparameter
	   virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmTopCrustHeatProduction  parameter
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in] mdl the model where the parameters values should be read
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const;

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @param[in, out] mdl the model where to set the new averaged parameter
      /// @param[in] xin the vector which stores the x coordinates of each 1D project 
      /// @param[in] yin the vector which stores the y coordinates of each 1D project 
      /// @param[in] prmVec the vector that stores the optimal parameter value of each 1D project
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model & mdl, const std::vector<double>& xin, const std::vector<double>& yin, const std::vector<SharedParameterPtr>& prmVec ) const;


      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmTopCrustHeatProduction"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer );

   protected:
   };

}

#endif // CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
