//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmCompactionCoefficient.h
/// @brief This file keeps API declaration for handling variation of casa::PrmCompactionCoefficient parameter

#ifndef CASA_API_VAR_PARAMETER_COMPACTION_COEFFICIENT_H
#define CASA_API_VAR_PARAMETER_COMPACTION_COEFFICIENT_H

#include "VarPrmLithologyProp.h"
#include "PrmCompactionCoefficient.h"

namespace casa
{
   /// @brief Variation for casa::PrmSurfacePorosity parameter
   class VarPrmCompactionCoefficient : public VarPrmLithologyProp
   {
   public:
      /// @brief Construct influential parameter for variation of surface porosity lithology parameter
      VarPrmCompactionCoefficient( const std::vector<std::string> & lithosName   ///< list of lithologies type name
                                 , double                           baseCompCoef ///< base value for compaction coefficient
                                 , double                           minCompCoef  ///< minimal value for compaction coefficient
                                 , double                           maxCompCoef  ///< maximal value for compaction coefficient
                                 , VarPrmContinuous::PDF            prmPDF       ///< probability density function
                                 , const std::string                & name = 0   ///< user specified parameter name
                                 );

      virtual ~VarPrmCompactionCoefficient( ) { ; }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return VarPrmContinuous::version() + 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "VarPrmCompactionCoefficient"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz ) const;

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmCompactionCoefficient( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:

      virtual PrmLithologyProp * createNewPrm( double val ) const;
      virtual PrmLithologyProp * createNewPrmFromModel( mbapi::Model & mdl ) const; // creates PrmLithologyProp parameter object instance from the model
   };
}

#endif // CASA_API_VAR_PARAMETER_COMPACTION_COEFFICIENT_H

