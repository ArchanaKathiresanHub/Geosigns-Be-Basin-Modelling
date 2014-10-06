//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmTopCrustHeatProduction.h
/// @brief This file keeps API declaration for handling variation of casa::PrmTopCrustHeatProduction parameter

#ifndef CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
#define CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmTopCrustHeatProduction parameter
   class VarPrmTopCrustHeatProduction : public VarPrmContinuous
   {
   public:
      /// @brief Construct variable parameter for the top crust heat production rate
      /// @param baseValue base value
      /// @param minValue  minimal range value
      /// @param maxValue  maximal range value
      VarPrmTopCrustHeatProduction( double baseValue, double minValue, double maxValue, PDF pdfType );

      /// @brief Destructor
      virtual ~VarPrmTopCrustHeatProduction();

	  /// @brief Get name of variable parameter in short form
	  /// @return array of names for each subparameter
	  virtual std::vector<std::string> name();

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first sub-parameter value
      /// @return new casa::PrmTopCrustHeatProduction  parameter
      virtual SharedParameterPtr createNewParameterFromDouble( std::vector<double>::const_iterator & vals ) const;

   protected:
   };

}

#endif // CASA_API_VAR_PARAMETER_TOP_CRUST_HEAT_PRODUCTION_H
