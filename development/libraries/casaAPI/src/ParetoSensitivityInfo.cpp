//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ParetoSensitivityInfo.cpp
/// @brief This file keeps API implementation for ParetoSensitivityInfo data

#include "ParetoSensitivityInfo.h"

// C/STL
#include <cstddef>

namespace casa
{
   //////////////////////////////////////////////////////////////
   // ParetoSensitivityInfo
   //////////////////////////////////////////////////////////////
   const std::vector< std::pair<const VarParameter *, int > >
   ParetoSensitivityInfo::getVarParametersWithCumulativeImpact( double fraction ) const
   {
      std::vector< std::pair<const VarParameter *, int> > ret;
      double cumulative = 0.0;
      for ( size_t i = 0; cumulative < fraction && i < m_vprmSens.size(); ++i )
      {
         cumulative += m_vprmSens[i];
         ret.push_back( std::pair< const VarParameter *, int >( m_vprmPtr[i], m_vprmSubID[i] ) );
      }
      return ret;
   }

   // Get the sensitivity of specified VarParameter
   double ParetoSensitivityInfo::getSensitivity( const VarParameter * varPrm, int subPrmID ) const
   {
      for ( size_t i = 0; i < m_vprmPtr.size(); ++i )
      {
         if ( m_vprmPtr[i] == varPrm && m_vprmSubID[i] == subPrmID )
         {
            return m_vprmSens[i];
         }
      }
      return 0.0;
   }

   // Get the cumulative sensitivity of specified VarParameter
   double ParetoSensitivityInfo::getCumulativeSensitivity( const VarParameter * varPrm, int subPrmID ) const
   {
      double cumulative = 0.0;
      for ( size_t i = 0; i < m_vprmSens.size(); ++i )
      {
         cumulative += m_vprmSens[i];
         if ( m_vprmPtr[i] == varPrm && m_vprmSubID[i] == subPrmID ) break;
      }
      return cumulative;
   }

   // Add new parameter sensitivity to the list
   void ParetoSensitivityInfo::add( const VarParameter * varPrm, int subPrmID, double val ) 
   { 
      for ( size_t i = 0; i < m_vprmSens.size() && varPrm; ++i )
      {  // create sorted by sensitivity value array
         if ( m_vprmSens[i] < val )
         {
            m_vprmSens.insert(  m_vprmSens.begin()  + i, val );
            m_vprmSubID.insert( m_vprmSubID.begin() + i, subPrmID );
            m_vprmPtr.insert(   m_vprmPtr.begin()   + i, varPrm );
            varPrm = 0;
         }
      }
      if ( varPrm ) 
      {
         m_vprmSens.push_back(  val );
         m_vprmSubID.push_back( subPrmID );
         m_vprmPtr.push_back(   varPrm );
      }
   }

} // namespace casa
