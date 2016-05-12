//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmSourceRockProp.h
/// @brief This file keeps API declaration for base class for all source rock lithology variable parameters

#ifndef CASA_API_VAR_PARAMETER_SOURCE_ROCK_PROP_H
#define CASA_API_VAR_PARAMETER_SOURCE_ROCK_PROP_H

#include "VarPrmContinuous.h"

namespace casa
{
   class PrmSourceRockProp;

   /// @brief Base class for source rock lithology parameters such as TOC, H/C, HI and so on
   class VarPrmSourceRockProp : public VarPrmContinuous
   {
   public:
      /// @brief Destructor
      virtual ~VarPrmSourceRockProp() {;}
     
      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const { return 1; }

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmSourceRockProp parameter
      virtual SharedParameterPtr newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in] the model where the parameters values should be read
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl ) const;

      /// @brief Average the values, interpolate for lithofractions and set the appropriate entries in the project3d file
      /// @param[in, out] mdl the model where to set the new averaged parameter
      /// @param[in] xin the vector which stores the x coordinates of each 1D project 
      /// @param[in] yin the vector which stores the y coordinates of each 1D project 
      /// @param[in] prmVec the vector that stores the optimal parameter value of each 1D project
      /// @return new parameter for given set of values
      virtual SharedParameterPtr makeThreeDFromOneD( mbapi::Model & mdl, const std::vector<double>& xin, const std::vector<double>& yin, const std::vector<SharedParameterPtr>& prmVec ) const;

      /// @brief Convert Cauldron parameter values to SUMlib values for some variable parameters
      /// @param prm cauldron parameter with to this variable parameter corresponded type
      /// @return parameter values suitable for SUMlib
      virtual std::vector<double> asDoubleArray( const SharedParameterPtr prm ) const;

      /// @{ VarPrmSourceRockProp specific methods
      /// @brief Get layer name for variation of Prop
      /// @return layer name
      std::string layerName() const { return m_layerName; }

      /// @brief Get source rock mixing ID for the layer
      /// @return source rock mixing ID
      int mixID() const { return m_mixID; }

      /// @brief Add one more range for the given source rock type category value
      /// @param srTypeName name of the source rock type
      /// @param baseVal base range value
      /// @param minVal minmal range value
      /// @param maxVal maximal range value
      void addSourceRockTypeRange( const char * srTypeName, double baseVal, double minVal, double maxVal, PDF pdfType );
      /// @}

      /// @brief Save common data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool serializeCommonPart( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Load commond data from the given stream
      /// @param dz input stream
      /// @param objVer object version in data file
      virtual bool deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer );

      // Slots
      virtual void onCategoryChosen( const Parameter * prm );

   protected:
      /// @brief Default constructor, used in deserialization
      VarPrmSourceRockProp();

      /// @brief Could be called only from the child class
      VarPrmSourceRockProp( const char * layerName /**< name of the layer for Prop variation. If layer has mix of 
                                                      source rocks litho-types, Prop will be changed for all of them */
                         , double       baseValue  ///< parameter base value
                         , double       minValue   ///< minimal value for the variable parameter range
                         , double       maxValue   ///< maximal value for the variable parameter range
                         , PDF          pdfType    ///< type of PDF shape for the variable parameter
                         , const char * name       ///< user specified parameter name
                         , const char * srTypeName ///< source rock type name, to connect with source rock type cat. prm.
                         , int          mixID      ///< mixing ID. Could be 1 or 2
                         );

      virtual PrmSourceRockProp * createNewPrm( double val ) const = 0;
      virtual PrmSourceRockProp * createNewPrmFromModel( mbapi::Model & mdl ) const = 0;

      std::string m_propName;   ///< Defines which source rock lithology property this parameter defines
      std::string m_layerName;  ///< active source rock lithology name
      int         m_mixID;      ///< mixing ID
      std::string m_srTypeName; ///< active source rock type name
     
      std::map<std::string, std::vector<double> > m_name2range; ///< Keeps range and base property values for each category
   };
}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_PROP_H
