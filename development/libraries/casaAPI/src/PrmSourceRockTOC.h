//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockTOC.h
/// @brief This file keeps API declaration for Source Rock TOC parameter handling. 

#ifndef CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
#define CASA_API_PARAMETER_SOURCE_ROCK_TOC_H

#include "Parameter.h"

namespace mbapi
{
   class Model;
}

/// @page CASA_SourceRockTOCPage Source rock initial Total Organic Contents (TOC) parameter
/// 
/// This parameter defines the initial total organic content in source rock.
/// The value is defined in units: @f$ [ weight \% ] @f$
/// It is continuous parameter and his range is @f$ [0:100]\% @f$

namespace casa
{
   /// @brief Source rock initial organic content parameter
   class PrmSourceRockTOC : public Parameter
   {
   public:
      /// @brief Constructor 
      /// @param mdl Cauldron model interface object to get value for TOC for given layer from\n
      ///            if model has more than one source rock lithology for the same layer, the TOC\n
      ///            value will be equal the first one
      /// @param layerName layer name
      PrmSourceRockTOC( mbapi::Model & mdl, const char * layerName );

      /// @brief Constructor 
      /// @param val value of top crust heat production rate
      /// @param layerName layer name
      PrmSourceRockTOC( double val, const char * layerName );

      /// @brief Destructor
      virtual ~PrmSourceRockTOC();
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Validate TOC value if it is in [0:100] range, also it check are any source rock  \n
      ///        lithology in the model with the same layer name, does the parameter value is the \n
      ///        same as in source rock lithology.
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_toc;  }

      // The following methods are used for testing  
      virtual std::vector<double> asDoubleArray() const { return std::vector<double>( 1, value() ); }

   protected:
      std::string m_name;        ///< name of the parameter
      
      std::string m_layerName;   ///< layer name with source rock
      double      m_toc;         ///< TOC value
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
