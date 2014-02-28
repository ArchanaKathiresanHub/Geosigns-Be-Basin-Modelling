// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROXYCASES_H
#define SUMLIB_PROXYCASES_H

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib {

class Proxy;
class ProxyBuilder;
class CubicProxy;

/// @class ProxyCases supports creating and testing a proxy by:
/// \li distributing cases (i.e. Parameters from a ParameterSet) and corresponding target values
///     over subsets of tune and test cases.
/// \li making the tune parameters and target values available for creating a proxy
/// \li testing a proxy by doing proxy function calls for all cases in a parameter set,
///     and calculating sum-of-squares error values for the tune set, the test set and the full set.
class INTERFACE_SUMLIB ProxyCases
{
   public:

      /// Default constructor
      ProxyCases();

      /// Constructor which assigns all cases to the tune set
      ProxyCases( ParameterSet const&  parSet, TargetSet const& targetSet );

      /// Constructor with initialisation data
      /// see method initialise for parameter documentation
      ProxyCases( ParameterSet const&  parSet, TargetSet const& targetSet, CaseList const& cases );

      virtual ~ProxyCases();

      /// Initialise the proxycases with cases and targets
      /// @param [in] parSet    the set of parameters
      /// @param [in] targetSet the target values for each of the parameters
      /// @param [in] cases     the cases to be assigned to the tune set
      void initialise( ParameterSet const&  parSet, TargetSet const& targetSet, CaseList const& cases );

      /// Set a new case list. Causes test and tune sets to be recreated
      /// @param [in] cases     the new case list
      /// @returns the number of active cases
      unsigned int setCaseList( CaseList const& cases );

      /// Shuffles the active cases in the caseList
      /// @param [in,out] caseList
      static void shuffle( CaseList &caseList );

      /// Create a proxy builder for the specified vars
      /// @param [in] vars      list of variables to create m_builder with
      void createProxyBuilder( VarList const& vars );

      /// Create a proxy for the specified vars in m_builder
      /// @returns the new CubicProxy
      CubicProxy *createProxy( ) const;

      /// For the current cases, create a proxy with one variable included/excluded from the base set of variables
      CubicProxy *createProxy( unsigned int varIndx ) const;

      /// Determine the mean square error w.r.t. targets of the
      /// proxy response to parameters
      /// @param [in]  proxy     the proxy model to evaluate
      /// @param [in]  par       the parameter set to calculate the response for
      /// @param [in]  target    the target values to compare the response with
      /// @returns the mean-square-error
      static double calculateMSE( Proxy const * proxy, ParameterSet const& par, TargetSet const& target );

      /// Determine the mean square error w.r.t. tune targets of the
      /// proxy response to tune parameters
      /// @param [in]  proxy     the proxy model to evaluate
      /// @returns the MSE of the tune set
      double tuneMSE( Proxy const * proxy ) const;

      /// Determine the mean square error w.r.t. test targets of the
      /// proxy response to test parameters
      /// @param [in]  proxy     the proxy model to evaluate
      /// @returns the MSE of the tune set
      double testMSE( Proxy const * proxy ) const;

      /// Determine the quality of the proxy by evaluating the
      /// proxy against the test set of inactive cases
      /// @param [in]  proxy        the proxy model to test
      /// @param [in]  nrOfUsedVars number of active monomials, used as penalty in (adjusted) totalRMSE
      /// @param [out] tuneRMSE     the RMSE of the tune set
      /// @param [out] testRMSE     the RMSE of the test set
      /// @param [out] totalRMSE    the RMSE of the full set
      /// @param [out] adjustedR2   the adjusted R^2 of the full set
      void test( Proxy const * proxy, unsigned int nrOfUsedVars, double & tuneRMSE,
                 double & testRMSE, double & totalRMSE, double & adjustedR2 ) const;

      /// Get the total number of cases
      unsigned int numTotalCases() const;

      /// Get the number of tune cases
      unsigned int numTuneCases() const;
      
      /// Get the number of parameter elements
      unsigned int caseSize() const;

      /// Set the seed of the random number generator
      static void RNGseed( int seed );

   private:

#if 0 // Never called
      /// The set of parameters in the tune set
      /// @returns a subset of cases
      ParameterSet const & tuneParameters() const;

      /// The set of parameters in the test set
      /// @returns a subset of cases
      ParameterSet const & testParameters() const;

      /// The target values for the  parameters in the tune set
      /// @returns a subset of target values
      TargetSet const & tuneTargets() const;

      /// The target values for the  parameters in the test set
      /// @returns a subset of target values
      TargetSet const & testTargets() const;
#endif

      ParameterSet   m_tunePars;
      TargetSet      m_tuneTargets;

      ParameterSet   m_testPars;
      TargetSet      m_testTargets;

      CaseList       m_cases;

      // Sample variance of the total target set
      double         m_targetVariance;

      ProxyBuilder  *m_builder;
};
} // namespace SUMlib

#endif // SUMLIB_PROXYCASES_H
