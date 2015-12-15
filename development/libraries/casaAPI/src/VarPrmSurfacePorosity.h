//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSurfacePorosity.h
/// @brief This file keeps API declaration for handling variation of casa::PrmSurfacePorosity parameter

#ifndef CASA_API_VAR_PARAMETER_SURFACE_POROSITY_H
#define CASA_API_VAR_PARAMETER_SURFACE_POROSITY_H

#include "VarPrmLithologyProp.h"
#include "PrmSurfacePorosity.h"

namespace casa
{
   /// @brief Variation for casa::PrmSurfacePorosity parameter
   class VarPrmSurfacePorosity: public VarPrmLithologyProp
   {
   public:
      /// @brief Construct variable parameter for variation of surface porosity lithology parameter 
      VarPrmSurfacePorosity( const std::vector<std::string> & lithosName  ///< list of lithologies type name
                           , double                           baseSurfPor ///< base value for surface porosity
                           , double                           minSurfPor  ///< minimal value for surface porosity
                           , double                           maxSurfPor  ///< maximal value for surface porosity
                           , VarPrmContinuous::PDF            prmPDF      ///< probability density function 
                           , const std::string              & name = 0    ///< user specified parameter name
                           );

      virtual ~VarPrmSurfacePorosity() {;}

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSurfacePorosity"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSurfacePorosity( CasaDeserializer & dz, unsigned int objVer );
      /// @}
   
   protected:

      virtual PrmLithologyProp * createNewPrm( double val ) const;
   };
}

#endif // CASA_API_VAR_PARAMETER_SURFACE_POROSITY_H

