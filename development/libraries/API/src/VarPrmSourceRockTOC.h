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

#include "ContinuousParameter.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmSourceRockTOC : public ContinuousParameter
   {
   public:
      /// @brief Create a new initial source rock TOC variable parameter
      /// @param srLithoType source rock lithology name in project
      /// @param baseValue base value of parameter
      /// @param minValue minimal value for the variable parameter range
      /// @param maxValue maximal value for the variable parameter range
      /// @param pdfType type of PDF shape for the variable parameter
      VarPrmSourceRockTOC( const char * srLithoType, double baseValue, double minValue, double maxValue, PDF pdfType = Block );
      virtual ~VarPrmSourceRockTOC();
     
      /// @brief Get base value for the variable parameter as double
      /// @return base value
      virtual double baseValueAsDouble();

      virtual Parameter * createNewParameterFromDouble( double val );

   protected:
      std::string m_srLithoType; ///< source rock lithology name
   };

}

#endif // CASA_API_VAR_PARAMETER_SOURCE_ROCK_TOC_H
