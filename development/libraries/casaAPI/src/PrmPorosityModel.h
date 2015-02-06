//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmPorosityModel.h
/// @brief This file keeps API declaration for lithology porosity model parameters handling. 

#ifndef CASA_API_PARAMETER_POROSITY_MODEL_H
#define CASA_API_PARAMETER_POROSITY_MODEL_H

#include "Parameter.h"

// CMB API
#include <UndefinedValues.h>

#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_PorosityModelPage Lithology porosity model
/// 
/// This parameter defines the type of porosity model and it parameters for a lithology
/// Here is the list of supported porosity model types:
/// -# Exponential
/// -# Soil Mechanics
/// -# Double Exponential
///
/// To define lithology porosity model user should specify 
/// - Lithology name
/// - Porosity model type
/// - Porosity model dependent parameters
///
/// <b> Exponential model </b>
///
///  @f$ \phi = \phi_0 \cdot exp( -c \sigma ) @f$
///
/// Exponential model has 2 independent parameters each of them could be set independently:
/// -# Surface porosity @f$ \phi_0 @f$. It is continuous parameter an it range is @f$ [0:100]\% @f$
/// -# Compaction coefficient @f$ c @f$. It is dimensionless continuous parameter and it range is @f$ [0:100] @f$
///
/// <b> Soil Mechanics model </b>
///
/// @f$ \psi = \psi_0 - \beta exp( frac{\sigma}{\sigma_0} ) @f$
/// Where
/// @f$ \phi = \frac{\psi}{1+\psi} @f$
/// @f$ \psi_0 = \frac{\phi_0}{1-\phi_0} @f$
/// @f$ \sigma_0 = 10^5 Pa @f$
///
/// Soil Mechanics model depends on clay fraction parameter only. It value could be calculated from
/// surface porosity or compaction coefficient. User must specify only one of them, the another one should be set
/// to UndefinedDoubleValue.
/// -# Surface porosity @f$ \phi_0 @f$. It is continuous parameter an it range is @f$ [0:100]\% @f$
/// -# Compaction coefficient @f$ \beta @f$
///
/// <b> Double Exponential model </b>
///
/// @f$ \phi = \phi_m + \phi_a \cdot exp( -c_a \sigma ) + \phi_b \cdot exp( -c_b \sigma ) @f$
/// @f$ \phi_0 = \phi_m + \phi_a + \phi_b @f$
/// @f$ \phi_c = \phi_0 - \phi_m, \phb_a = \phi_b = \frac{1}{2}\phi_c @f$
///
/// Double Exponential model has 4 independent parameters
/// -# Surface porosity @f$ \phi_0 @f$
/// -# Minimal porosity @f$ \phi_m @f$
/// -# Compaction coefficient for the first exponent  @f$ c_a @f$
/// -# Compaction coefficient for the second exponent @f$ c_b @f$

namespace casa
{
   class VarPrmPorosityModel;

   /// @brief Source rock initial organic content parameter
   class PrmPorosityModel : public Parameter
   {
   public:
      typedef enum
      {
         Exponential,
         SoilMechanics,
         DoubleExponential,
         UndefinedModel
      } PorosityModelType;

      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get value for TOC for given layer from
      ///            if model has more than one source rock lithology for the same layer, the TOC
      ///            value will be equal the first one
      /// @param layerName layer name
      PrmPorosityModel( mbapi::Model & mdl, const char * lithoName );

      /// @brief Constructor. Create parameter from variation of variable parameter
      /// @param parent pointer to a variable parameter which created this one
      /// @param val value of the initial total organic content in source rock @f$ [ weight \% ] @f$
      /// @param layerName layer name
      PrmPorosityModel( const VarPrmPorosityModel * parent, const char * lithoName, PorosityModelType mdlType, const std::vector<double> & mdlPrms );

      /// @brief Destructor
      virtual ~PrmPorosityModel();
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Validate Porosity model parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );



      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { return static_cast<int>( m_modelType ); }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmPorosityModel"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmPorosityModel( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarParameter * m_parent;    ///< variable parameter which was used to create this one

      std::string          m_name;      ///< name of the parameter
      
      PorosityModelType    m_modelType;   ///< type of the porosity model
      std::string          m_lithoName;   ///< layer name with source rock
      
      // Soil Mechanics model parameters
      double               m_clayFraction;///< clay fraction which unique defines surface porosity and compaction coefficient

      // Exponential model parameters
      double               m_surfPor;     ///< surface porosity value
      double               m_compCoef;    ///< compaction coefficient
      // Dbl exponential adds two more
      double               m_minPorosity; ///< minimal porosity value (only for DoubleExp. model)
      double               m_compCoef1;   ///< second compaction coefficient (only for DoubleExp. model)

      // check parameters and setup model
      void initSoilMechanicsPorModel( const std::vector<double> & mdlPrms );
   };

   /// @brief For soil mechanics model calculate compaction coefficient from surface pororsity
   /// @param cc compaction coefficient value
   /// @return surface porosity value
   double SMcc2sp( double cc );

   /// @brief For soil mechanics model calculate surface pororsity from compaction coefficient
   /// @param surfPor surface pororsity value
   /// @return compaction coefficient value
   double SMsp2cc( double surfPor );

   /// @brief For soil mechanics model calculate clay fraction from surface pororsity
   /// @param surfPor surface pororsity value
   /// @return clay fraction value
   double SMsp2cf( double surfPor );

   /// @brief For soil mechanics model calculate clay fraction from compaction coefficient 
   /// @param cc compaction coefficient value
   /// @return clay fraction value
   double SMcc2cf( double cc );

   /// @brief For soil mechanics model calculate surface pororsity from clay fraction 
   /// @param clayFrac clay fraction value
   /// @return surface porosity value
   double SMcf2sp( double clayFrac );

   /// @brief For soil mechanics model calculate compaction coefficient from clay fraction 
   /// @param clayFrac clay fraction value
   /// @return surface porosity value
   double SMcf2cc( double clayFrac );
}

#endif // CASA_API_PARAMETER_POROSITY_MODEL_H
