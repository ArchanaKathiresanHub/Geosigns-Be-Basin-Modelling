//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ParetoSensitivityInfo.h
/// @brief This file keeps API declaration for calculated Pareto diagram data

#ifndef CASA_API_PARETO_SENSTITIVITY_INFO_H
#define CASA_API_PARETO_SENSTITIVITY_INFO_H

#include <vector>

namespace casa
{
   class VarParameter;
   
   /// @brief Data structure for keeping Pareto sensitivity calculation results
   struct ParetoSensitivityInfo
   {
      /// @brief Get a list of VarParameters that together have a cumulative sensitivity of the specified value, or more.
      /// @param fraction cumulative sensitivity: fractional number in range [0.0:1.0]
      /// @returns        a vector parameters numbers as they are numbered in VarSpace
      const std::vector< std::pair<const VarParameter *, int > > getVarParametersWithCumulativeImpact( double fraction ) const;

      /// @brief Get the sensitivity of specified VarParameter
      /// @param varPrm influential parameter object pointer
      /// @param subPrmID sub-parameter ID
      /// @returns      the sensitivity value
      double getSensitivity( const VarParameter * varPrm, int subPrmID ) const;

      /// @brief Get the cumulative sensitivity of specified VarParameter
      /// @param varPrm influential parameter  object pointer
      /// @param subPrmID  sub-parameter ID
      /// @returns the cumulative sensitivity value
      double getCumulativeSensitivity( const VarParameter * varPrm, int subPrmID ) const;

      /// @brief Add new parameter sensitivity to the list
      /// @param varPrm parameter number in VarSpace
      /// @param subPrmID subparameter ID
      /// @param val influential parameter sensitivity
      void add( const VarParameter * varPrm, int subPrmID, double val );

      std::vector< const VarParameter * > m_vprmPtr;    ///< Influential parameter pointer
      std::vector< int >                  m_vprmSubID;  ///< Influential parameter sub-parameter ID
      std::vector< double >               m_vprmSens;   ///< Influential parameter sub-parameter sensitivity
   };
}

#endif // CASA_API_PARETO_SENSTITIVITY_INFO_H

