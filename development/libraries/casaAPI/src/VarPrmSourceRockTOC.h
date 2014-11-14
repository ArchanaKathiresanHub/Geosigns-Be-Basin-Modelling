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

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmSourceRockTOC : public VarPrmContinuous
   {
   public:
      /// @brief Create a new initial source rock TOC variable parameter
      VarPrmSourceRockTOC( const char * layerName /**< name of the layer for TOC variation. If layer has mix of 
                                                      source rocks litho-types, TOC will be changed for all of them */
                         , double baseValue      ///< base value of parameter
                         , double minValue       ///< minimal value for the variable parameter range
                         , double maxValue       ///< maximal value for the variable parameter range
                         , PDF pdfType = Block   ///< type of PDF shape for the variable parameter
                         );

      /// @brief Destructor
      virtual ~VarPrmSourceRockTOC();
     
	   /// @brief Get name of variable parameter in short form
	   /// @return array of names for each subparameter
	   virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmSourceRockTOC parameter
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "VarPrmSourceRockTOC"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const { return VarPrmContinuous::save( sz, version ); }

      /// @brief Create a new var.parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      VarPrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer ) : VarPrmContinuous( dz, objVer ) { ; }
      /// {@

   protected:
      std::string m_layerName; ///< source rock lithology name
   };

}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_TOC_H
