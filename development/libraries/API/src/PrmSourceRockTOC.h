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
      /// @param val value of top crust heat production rate
      /// @param srLithoType source rock lithotype name
      PrmSourceRockTOC( double val, const char * srLithoType );
      
      /// @brief Destructor
      virtual ~PrmSourceRockTOC();
     
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel );

      /// @brief Get value for the parameter as double
      /// @return parameter value
      double value() const { return m_toc;  }

      // The following methods are used for testing  
      virtual bool isDouble( ) { return true; }
      virtual double doubleValue() { return value(); }

   protected:
      std::string m_srLithoType; ///< source rock lithology name
      double      m_toc;         ///< TOC value
   };

}

#endif // CASA_API_PARAMETER_SOURCE_ROCK_TOC_H
