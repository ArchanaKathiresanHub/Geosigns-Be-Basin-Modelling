//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockPreAsphaltStartAct.h
/// @brief This file keeps API declaration for handling variation of Preasphaltene Activation Energy parameter. 

#ifndef CASA_API_VAR_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H
#define CASA_API_VAR_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H

#include "VarPrmSourceRockProp.h"

namespace casa
{
   /// @brief Variation for casa::PrmSourceRockPreAsphaltStartAct parameter
   class VarPrmSourceRockPreAsphaltStartAct : public VarPrmSourceRockProp
   {
   public:
      /// @brief Create a new source rock preasphaltene activation energy variable parameter
      VarPrmSourceRockPreAsphaltStartAct( const char * layerName       ///< name of the layer the parameter variation
                                        , double       baseValue       ///< base value of parameter
                                        , double       minValue        ///< minimal value for the parameter variation
                                        , double       maxValue        ///< maximal value for the parameter variation
                                        , PDF          pdfType = Block ///< type of PDF shape for the variable parameter
                                        , const char * name = 0        ///< user specified parameter name
                                        , const char * srTypeName = 0  ///< source rock type name, to connect with source rock type cat. prm.
                                        , int          mixID = 1       ///< mixing ID. Could be 1 or 2
                                        );

      /// @brief Destructor
      virtual ~VarPrmSourceRockPreAsphaltStartAct() {;}
     
	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each subparameter
	   virtual std::vector<std::string> name() const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 1; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSourceRockPreAsphaltStartAct"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSourceRockPreAsphaltStartAct( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      virtual PrmSourceRockProp * createNewPrm( double val ) const; // creates PrmSourceRockTOC parameter object instance
   };
}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_PRE_ASPH_ACT_ENERGY_H
