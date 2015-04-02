//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmLithoSTPThermalCond.h
/// @brief This file keeps API declaration for handling variation of STP thermal conductivity coefficient for the given lithology 

#ifndef CASA_API_VAR_PARAMETER_LITHO_STP_THERMAL_COND_H
#define CASA_API_VAR_PARAMETER_LITHO_STP_THERMAL_COND_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmLithoSTPThermalCond parameter
   class VarPrmLithoSTPThermalCond : public VarPrmContinuous
   {
   public:
      /// @brief Create a new variable parameter
      VarPrmLithoSTPThermalCond( const char * lithoName ///< name of the lithology for the parameter variation
                               , double baseValue       ///< base value of parameter
                               , double minValue        ///< minimal value for the variable parameter range
                               , double maxValue        ///< maximal value for the variable parameter range
                               , PDF pdfType = Block    ///< type of PDF shape for the variable parameter
                               );

      /// @brief Destructor
      virtual ~VarPrmLithoSTPThermalCond();
     
	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each subparameter
	   virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmLithoSTPThermalCond parameter
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmLithoSTPThermalCond"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::string m_lithoName; ///< source rock lithology name
   };

}

#endif // CASA_API_VAR_PARAMETER_LITHO_STP_THERMAL_COND_H
