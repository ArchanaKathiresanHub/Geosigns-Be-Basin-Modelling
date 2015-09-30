//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockTOC.h
/// @brief This file keeps API declaration for handling variation of initial source rock TOC parameter. 

#ifndef CASA_API_VAR_PARAMETER_SOURCE_ROCK_TOC_H
#define CASA_API_VAR_PARAMETER_SOURCE_ROCK_TOC_H

#include "VarPrmSourceRockProp.h"

namespace casa
{
   /// @brief Variation for casa::PrmSourceRockTOC parameter. It could be dependent on source rock type
   /// category parameter (casa::VarPrmSourceRockType). If user defines source rock type variation for
   /// the layer and TOC variation for the same layer and mixing ID, he must provides TOC for each source rock 
   /// category value.
   ///
   class VarPrmSourceRockTOC : public VarPrmSourceRockProp
   {
   public:
      /// @brief Create a new initial source rock TOC variable parameter
      VarPrmSourceRockTOC( const char          * layerName       ///< name of the layer for TOC variation
                         , double                baseValue       ///< base value of parameter
                         , double                minValue        ///< minimal value for the variable parameter range
                         , double                maxValue        ///< maximal value for the variable parameter range
                         , VarPrmContinuous::PDF pdfType = Block ///< type of PDF shape for the variable parameter
                         , const char          * name = 0        ///< user specified parameter name
                         , const char          * srTypeName = 0  ///< source rock type name, to connect with source rock type cat. prm.
                         , int                   mixID = 1       ///< mixing ID. Could be 1 or 2
                         );

      /// @brief Destructor
      virtual ~VarPrmSourceRockTOC() {;}
     
	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each subparameter
	   virtual std::vector<std::string> name() const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      // Version 1 - add different TOC ranges, dependend on source rock type category
      virtual unsigned int version() const { return VarPrmContinuous::version() + 1; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSourceRockTOC"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer );
      /// @}
      
   protected:
      virtual PrmSourceRockProp * createNewPrm( double val ) const; // creates PrmSourceRockTOC parameter object instance
   };

}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_TOC_H
