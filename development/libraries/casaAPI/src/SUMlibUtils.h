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
   class ParameterPdf;
   class ParameterSpace;
   class ParameterBounds;
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
   /// @param[in]  crc casa::RunCase object reference
   /// @param[out] sc  SUMlib::Case object reference
   void convertCase( const casa::RunCase & crc, SUMlib::Case  & sc );

   /// @brief Convert set of parameters value for given SUMlib::Case to casa::RunCase
   /// @param[in]  sc  SUMlib::Case object reference
   /// @param[out] crc casa::RunCase object reference
   void convertCase( const SUMlib::Case  & sc, const casa::VarSpace & vp, casa::RunCase & crc );

   /// @brief Convert observables of given casa::RunCase set to list of SUMlib targets value
   /// @pre targetsSet must be empty and caseSet not empty
   void convertObservablesValue( const std::vector<const casa::RunCase*> & caseSet      ///< [in]  set of casa::RunCase objects
                               , SUMlib::TargetCollection                & targetsSet   ///< [out] 2D array of observables values for SUMlib
                               , std::vector< std::vector<bool> >        & matrValidObs /**< [out] bolean matrix where matrValidObs[caseIndex][obsIndex] is true if 
                                                                                             caseIndex/obsIndex is a valid combination. */
                               );

   /// @brief Convert observables from SUMlib to RunCase
   void convertObservablesValue( const SUMlib::ProxyValueList & valList ///< [in]  set of SUMlib observables value
                               , const casa::ObsSpace         & obsDef  ///< [in]  set of observables definition for CASA
                               , casa::RunCase                & cs      ///< [out] casa::RunCase which will keep the converted set of observables value
                               );

   
   /// @brief Create a SUMlib ParameterPdf.
   ///
   /// The pdf parameters are arranged in the order returned by casa::VarSpace method.
   /// The variable parameter base values are copied to the corresponding most likely pdf values.
   /// The variances are calculated from the variable paramter std. deviations.
   void convertVarSpace2ParameterPdf( const casa::VarSpace         & varSpace ///< [in]  set of variable parameters definition
                                    , const SUMlib::ParameterSpace & pSpace   ///< [in]  Parameter space of the proxy
                                    , SUMlib::ParameterPdf         & pdf      ///< [out] SUMlib::ParameterPdf instance that must be initialized
                                    );


   /// @brief Create SUMlib bounds
  void createSUMlibBounds( const casa::VarSpace            & varSp        ///< [in]  VarSpace
                          , SUMlib::Case                   & lowCs        ///< [out] SUMlib lower bounds
                          , SUMlib::Case                   & highCs       ///< [out] SUMlib upper bounds
                          , std::vector<bool>              & selectedPrms ///< [out] mask array, keeps true for unfrozen var. parameters
                          , std::vector<SUMlib::IndexList> & catIndices   ///< [out] indexes of categorical parameters
                          );

   /// @brief Create SUMlib prior info.
   /// The variable parameter base values are copied to the corresponding most likely pdf values.
   /// The variances are calculated from the variable parameter std. deviations.
   void createSUMlibPrior( const casa::VarSpace & varSpace    ///< [in]  set of variable parameters
                         , SUMlib::Case         & pBase       ///< [out] SUMlib base case
                         , SUMlib::RealMatrix   & variance    ///< [out] Variances corresponding to the continuous varModels
                         , SUMlib::RealMatrix   & disWeights  ///< [out] Weights of the discrete variable parameters 
                         , SUMlib::RealMatrix   & catWeights  ///< [out] Weights of the categorical variable parameters
                         );

   /// @brief Creates custom-made box constraints that can be passed directly to SUMlib::McmcBase.
   void createBoxConstraints( const casa::VarSpace & proxyVs            ///< [in] Proxy VarSpace (VarSpace which was used to build proxy)
                            , const casa::VarSpace & mcmcVs             /**< [in] MCMC VarSpac (VarSpace which has the same set of variable 
                                                                             parameters as Proxy VarSpace, but restricted ranges) */
                            , SUMlib::ParameterBounds & boxConstraints  ///< [out] Custom-made box contraints stored as a SUMlib::ParameterBounds instance
                            );
}

#endif // CASA_SUMLIB_UTILS


