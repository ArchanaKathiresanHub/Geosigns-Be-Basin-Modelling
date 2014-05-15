//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxy.h
/// @brief This file keeps API declarations for response surface proxy

#ifndef CASA_API_RESPONSE_SURFACE_PROXY_H
#define CASA_API_RESPONSE_SURFACE_PROXY_H

/// @page CASA_RSProxyPage Response surface proxy 
/// A @link casa::RSProxy Response Surface proxy @endlink model of a system property (also called an observable) is an approximate representation of\n
/// this property such that it can be evaluated in a fast and efficient manner. A response surface proxy model for an observable\n
/// is calculated from a number of simulated cases and then allows to interpolate the observable value for other of the parameters value.
/// The Response surface proxy model is usually constructed from a polynomial approximation. For 2 parameters it can be written like this:
///
/// @f[ y=\left\{ \begin{array}{lcl}
///     \hat{y}_{0\, order} & = & \beta_{0}\\
/// \hat{y}_{1^{st}\, order} & = & \hat{y}_{0\, order}+\beta_{1}x_{1}+\beta_{2}x_{2}\\
/// \hat{y}_{2^{nd}\, order} & = & \hat{y}_{1^{st}\, order}+\beta_{12}x_{1}x_{2}+\beta_{11}x_{1}^{2}+\beta_{22}x_{2}^{2}\\
/// \hat{y}_{3^{d}\, order} & = & \hat{y}_{2^{nd}\, order}+\beta_{112}x_{1}^{2}x_{2}+\beta_{122}x_{1}x_{2}^{2}+\beta_{111}x_{1}^{3}+\beta_{222}x_{2}^{3}
/// \end{array}\right. @f]
///
/// Where:
///
/// @f$ y @f$ - is the response value for T/VRe/CumOil...
///
/// @f$ x_i @f$ - is the parameter value
///
/// @f$ \beta_i @f$ - is the polynomial coefficient
////
/// Response surface proxy could use also \subpage CASA_KrigingPage 


/// @page CASA_KrigingPage Kriging interpolation
///
/// All interpolation algorithms (inverse distance squared, splines, radial basis functions, triangulation, etc.) estimate the 
/// value at a given location as a weighted sum of data values at surrounding locations. Almost all assign weights according 
/// to functions that give a decreasing weight with increasing separation distance. 
/// Kriging assigns weights according to a (moderately) data-driven weighting function, rather than an arbitrary function, but 
/// it is still just an interpolation algorithm and will give very similar results to others in many cases
///
/// Some advantages of kriging: 
///
/// - Helps to compensate for the effects of data clustering, assigning individual points within a cluster less weight\n 
///   than isolated data points (or, treating clusters more like single points)
/// - Gives estimate of estimation error (kriging variance), along with estimate of the variable value itself
///
/// SUMLib uses a regression technique (Singular Value Decomposition) to determine a polynomial expression
/// for each observable. This implies that, generally, the polynomial does not exactly fit the
/// simulated observable output for the run cases. In other words, the polynomial contains regression
/// errors. As these regression errors are known for the run cases, SUMLib uses another technique
/// (Kriging) to interpolate between these known regression errors. As a result, all proxy representations
/// in SUMLib are the sum of a polynomial (can be 0 order only) and a Kriging expression.
/// So any proxy evaluation involves a polynomial evaluation and a Kriging evaluation. Evaluation of
/// the Kriging expression is ignored only if stated explicitly by setting NoKriging. Note
/// that MCMC can be rather slow, because the proxy must be calculated for all intermediate solutions.
/// By removing the Kriging part of the proxy solution, using only the polynomial expression, you can
/// speed up the calculations. In the case of LocalKrigin, only a partial Kriging expression is used.
///

#include "ErrorHandler.h"

#include <vector>

namespace casa
{   
   class RunCase;

   /// @brief Class to handle response surface proxy
   class RSProxy : public ErrorHandler
   {
   public:
      /// @brief Types of Kriging interpolation which can be used in proxy
      enum RSKrigingType
      {
         NoKriging,     ///< Do not use Kriging interpolation. The default
         LocalKriging,  ///< Use local Kriging interpolation (partial of GlobalKriging and faster)
         GlobalKriging  ///< Use global Kriging interpolation
      } ;

      /// @brief Destructor
      virtual ~RSProxy() {;}

      /// @brief Calculate polynomial coefficients for the given cases set
      /// @param caseSet list of cases which keeps simulation results with variable parameters value and observables value
      /// @return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode calculateRSProxy( const std::vector<RunCase*> & caseSet ) = 0;

      /// @brief Calculate values of observables for given set of parameters
      /// @param cs case which keeps list of parameters and list of observables to be calculated
      /// @return ErrorHandler::NoError in case of success, or error code in case of error
      virtual ErrorHandler::ReturnCode evaluateRSProxy( RunCase & cs ) = 0;

   protected:
      RSProxy() {;}

   private:
   };
}

#endif // CASA_API_VAR_SPACE_H
