//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmOneCrustTinningEvent.h
/// @brief This file keeps API declaration for handling variation of casa::PrmOneCrustThinningEvent parameter

#ifndef CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H
#define CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

#include "VarPrmContinuous.h"

namespace casa
{
   /// @brief Variation for casa::PrmOneCrustThinningEvent parameter
   class VarPrmOneCrustThinningEvent: public VarPrmContinuous
   {
   public:
      /// @brief Construct variable parameter for one crust thinning event in crust thickness history. 
      ///
      /// Crust thickness in Cauldorn is defined by a set of rapper points which define crust thickness for given time
      /// Between points the linear interpolation is used. This variable parameter can define one crust thinning even which
      /// could be defined by the following list of sub-parameters:
      /// -# @f$ d_0 @f$ - initial crust thickness - defines thickness of the crust at the beginning of simulation
      /// -# @f$ t_0 @f$ - defines time when crust thickness starts to change
      /// -# @f$ \delta t @f$ - defines duration of crust thinning event
      /// -# @f$ \sigma @f$ - crust thinning factor (@f$ d_1 = \sigma \cdot d_0 @f$)
      /// @param baseThickIni    base value for the initial thickness
      /// @param minThickIni     minimal value for the initial thickness
      /// @param maxThickIni     maximal value for the initial thickness
      /// @param baseT0          base value for the event start time
      /// @param minT0           minimal value for the event start time
      /// @param maxT0           maximal value for the event start time
      /// @param baseDeltaT      base value for the event duration
      /// @param minDeltaT       minimal value for the event duration
      /// @param maxDeltaT       maximal value for the event duration
      /// @param baseThinningFct base value for the thinning factor
      /// @param minThinningFct  minimal value for the thinning factor
      /// @param maxThinningFct  maximal value for the thinning factor
      /// @param prmPDF          probability density function for all 4 sub-parameters
       VarPrmOneCrustThinningEvent( double baseThickIni,   double minThickIni,    double maxThickIni, 
                                   double baseT0,          double minT0,          double maxT0,       
                                   double baseDeltaT,      double minDeltaT,      double maxDeltaT,   
                                   double baseThinningFct, double minThinningFct, double maxThinningFct, 
                                   PDF prmPDF );

      virtual ~VarPrmOneCrustThinningEvent();

	  /// @brief Get name of variable parameter in short form
	  /// @return array of names for each subparameter
	  virtual std::vector<std::string> name();

      /// @brief Create parameter from set of doubles. This method is used to convert data between CASA and SUMlib
      /// @param[in,out] vals iterator which points to the first parameter value.
      /// @return new parameter for given set of values
      virtual SharedParameterPtr createNewParameterFromDouble( std::vector<double>::const_iterator & vals ) const;


   protected:
   };
}

#endif // CASA_API_VAR_PARAMETER_ONE_CRUST_THINNING_EVENT_H

