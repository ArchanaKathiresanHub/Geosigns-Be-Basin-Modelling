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
      /// @param baseValue base value
      /// @param minValue  minimal range value
      /// @param maxValue  maximal range value
      /// @param pdfType type of probabiltiy density function for this variable parameter
      VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType );

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

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const { return VarPrmContinuous::save( sz, version ); }

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer ) : VarPrmContinuous( dz, objVer ) { ; }

   protected:
   };

}

#endif // CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
