//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmLithologyProp.h
/// @brief This file keeps base class declaration for any lithology property implemented as variable parameter

#ifndef CASA_API_VAR_PARAMETER_LITHOLOGY_PROP_H_
#define CASA_API_VAR_PARAMETER_LITHOLOGY_PROP_H_

#include "VarPrmContinuous.h"
#include "PrmLithologyProp.h"

namespace casa
{
   /// @brief Variation for casa::PrmLithologyProp parameter
   class VarPrmLithologyProp: public VarPrmContinuous
   {
   public:

      virtual ~VarPrmLithologyProp() {;}

      /// @brief Get name of variable parameter in short form     
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Convert Cauldron parameter values to SUMlib values for some variable parameters
      /// @param prm cauldron parameter with to this variable parameter corresponded type
      /// @return parameter values suitable for SUMlib
      virtual std::vector<double> asDoubleArray( const SharedParameterPtr prm ) const;

      /// @{ VarPrmLithologyProp specific methods
      /// @brief Get lithology name for variation of Prop
      /// @return layer name
      std::vector<std::string> lithoNames() const { return m_lithosName; }

      /// @{
      /// @brief Save common data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool serializeCommonPart( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Load commond data from the given stream
      /// @param dz input stream
      /// @param objVer object version in data file
      virtual bool deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer );
      /// @}
   
   protected:
      /// @brief Default constructor, used in deserialization
      VarPrmLithologyProp() {;}

      /// @brief Construct variable parameter for variation of some lithology property 
      VarPrmLithologyProp( const std::vector<std::string> & lithosName ///< list of names of the lithology type
                         , VarPrmContinuous::PDF            prmPDF     ///< probability density function 
                         , const std::string              & name       ///< user specified parameter name
                         );
      
      virtual PrmLithologyProp * createNewPrm( double val ) const = 0;

      std::string                           m_propName;   ///< property name
      std::vector<std::string>              m_lithosName; ///< lithology name
   };
}

#endif // CASA_API_VAR_PARAMETER_LITHOLOGY_PROP_H_
