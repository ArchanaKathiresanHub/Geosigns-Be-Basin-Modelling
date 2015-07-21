//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockHC.h
/// @brief This file keeps API declaration for handling variation of source rock H/C parameter. 

#ifndef CASA_API_VAR_PARAMETER_SOURCE_ROCK_HC_H
#define CASA_API_VAR_PARAMETER_SOURCE_ROCK_HC_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmSourceRockHC parameter
   class VarPrmSourceRockHC : public VarPrmContinuous
   {
   public:
      /// @brief Create a new initial source rock HI variable parameter
      VarPrmSourceRockHC( const char  * layerName       ///< name of the layer for H/C variation
                         , double       baseValue       ///< base value of parameter
                         , double       minValue        ///< minimal value for the variable parameter range
                         , double       maxValue        ///< maximal value for the variable parameter range
                         , PDF          pdfType = Block ///< type of PDF shape for the variable parameter
                         , const char * name = 0        ///< user specified parameter name
                         );

      /// @brief Destructor
      virtual ~VarPrmSourceRockHC();
     
	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each sub-parameter
	   virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmSourceRockHC parameter
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Get layer name for variation of HI
      /// @return layer name
      std::string layerName() const { return m_layerName; }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSourceRockHC"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSourceRockHC( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::string m_layerName; ///< source rock lithology name
   };

}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_HC_H
