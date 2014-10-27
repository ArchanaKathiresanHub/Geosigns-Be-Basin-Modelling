//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmContinuous.C
/// @brief This file keeps API definition for handling continuous parameters.

#include "VarPrmContinuous.h"

#include <cmath>

namespace casa
{

std::vector<double> VarPrmContinuous::stdDevs() const
{
   const std::vector<double> minVals  = m_minValue->asDoubleArray();
   const std::vector<double> maxVals  = m_maxValue->asDoubleArray();
   const std::vector<double> baseVals = m_baseValue->asDoubleArray();

   std::vector<double> devs( minVals.size(), 0 );
   for( size_t i = 0; i < devs.size(); ++i )
   {
      double mi = minVals[i];
      double ma = maxVals[i];
      double to = baseVals[i];

      switch ( m_pdf )
      {
         case Block:    devs[i] = 0.5 * (ma - mi) / sqrt( 3.0 );                                              break;
         case Triangle: devs[i] = sqrt( (mi * mi + ma * ma + to * to - mi * ma - mi * to - ma * to) / 18.0 ); break;
         case Normal:   devs[i] = 0.5 * (ma -  mi) / 5.0;                                                     break;
         default:       assert(0);                                                                            break;
      }
   }
   return devs;
}


}

