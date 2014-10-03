//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SUMlibUtils.h
/// @brief This file keeps declarations for the set of utility functions to convert data SUMlib <-> CASA

#ifndef CASA_SUMLIB_UTILS 
#define CASA_SUMLIB_UTILS

// SUMLib includes
#include <BaseTypes.h>

namespace SUMlib
{
   class Case;
}

namespace casa
{
   class RunCase;
   class VarSpace;
   class ObsSpace;
}

namespace sumext
{
   /// @brief Convert set of parameters value for given casa::RunCase to SUMlib::Case
   /// @param[in] crc casa::RunCase object reference
   /// @param[out] sc SUMlib::Case object reference
   void convertCase( const casa::RunCase & crc, SUMlib::Case  & sc );

   /// @brief Convert set of parameters value for given SUMlib::Case to casa::RunCase
   /// @param[in] sc SUMlib::Case object reference
   /// @param[out] crc casa::RunCase object reference
   void convertCase( const SUMlib::Case  & sc, const casa::VarSpace & vp, casa::RunCase & crc );

   /// @brief Convert observables of given casa::RunCase set to list of SUMlib targets value
   /// @param[in] caseSet set of casa::RunCase objects
   /// @param[out] targetsSet 2D array of observables values for SUMlib
   /// @param[out] matrValidObs bolean matrix where matrValidObs[caseIndex][obsIndex] is true if caseIndex/obsIndex is a valid combination.
   /// @pre targetsSet must be empty and caseSet not empty
   void convertObservablesValue( const std::vector<const casa::RunCase*> & caseSet
                               , SUMlib::TargetCollection                & targetsSet
                               , std::vector< std::vector<bool> >        & matrValidObs );

   /// @brief Convert observables from SUMlib to RunCase
   /// @param[in] valList set of SUMlib observables value
   /// @param[in] obsDef set of observables definition for CASA
   /// @param[out] cs casa::RunCase which will keep the converted set of observables value
   void convertObservablesValue( const SUMlib::ProxyValueList &  valList, const casa::ObsSpace & obsDef, casa::RunCase & cs );
}

#endif // CASA_SUMLIB_UTILS


