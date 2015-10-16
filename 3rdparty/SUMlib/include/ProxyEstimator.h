// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PROXYESTIMATOR_H
#define SUMLIB_PROXYESTIMATOR_H

#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib {

/// Collects all data related to the sum of squares evaluation of a single candidate variable
struct ProxyCandidate;
class EliminationCriterion;

/// The list of candidate variables
typedef std::vector<ProxyCandidate> CandidateList;

/// The ranking of the variables
typedef std::vector< std::pair<double,size_t> >CandidateRanking;

class ProxyCases;

/// The proxy core object with coefficient matrices
class CubicProxy;

/// @class ProxyEstimator implements an engine for estimating an optimal proxy.
///
/// In automatic mode, ProxyEstimator performs 'greedy' proxy determination by
/// starting from an empty model, iteratively adding variables to the proxy until
/// the error is minimized.
///
/// Sets of active variables are ranked according to sum-of-squares errors
/// between estimation and validation cases, for a maximum number of cross-validation
/// iterations.
///
/// If the ranking of the top few variable sets has not changed for more than a
/// specified number of trial iterations, the trial iteration loop can be stopped.
///
/// The best set of variables after a cross-validation loop is taken as next base case.
///
/// Reference:
/// [HT] "Automatic or User-Intervened Proxy Estimation Procedure With Augment
/// and Reduction Steps, Using Cross-Validation" - Herbert Tulleken, Sept. 2009
class INTERFACE_SUMLIB ProxyEstimator
{
   public:

      /// The maximum number of cycles of the cross validation loop
      static const unsigned int defaultNrCV_Max;

      /// The default setting for the minimum number of cycles that the
      /// cross-validation loop should have a stable ranking
      static const unsigned int defaultNrCV_Min;

      /// The maximum number of trials that accept a non-improved proxy candidate
      static const unsigned int defaultMaxNrDeclines;

      /// Default for the fraction of cases to be used for testing a proxy
      static const double defaultCasesForTestFraction;

      /// Default constructor
      /// @param [in]   nrCV_Max          maximum number of cross-validation iterations
      /// @param [in]   nrCV_Min          minimum number of stable cross-validation iterations
      /// @param [in]   maxNrDeclines     maximum number of trials that accept declines
      /// @param [in]   doAugment         add candidates that augment the set of active variables
      /// @param [in]   doReduce          add candidates that reduce the set of active variables
      ProxyEstimator (
            unsigned int  nrCV_Max                = defaultNrCV_Max,
            unsigned int  nrCV_Min                = defaultNrCV_Min,
            unsigned int  maxNrDeclines           = defaultMaxNrDeclines,
            bool          augment                 = true,
            bool          reduce                  = true
            );

      /// Setter for parameterSet
      /// The parameterSet is scaled and mean and stddev values
      /// are stored in m_parMean and m_parStdDev.
      /// @param [in] parSet the parameter set (unscaled)
      void setParameterSet( ParameterSet const& parSet );

      /// Setter for the target values corresponding to the parameterSet
      /// @param [in] targetSet target values for each parameter in the parameter set (unscaled)
      void setTargetSet( TargetSet const& targetSet );

      /// Setter for the number of cases to be used for testing/validation
      /// @param [in] num the number of cases to use for testing
      void setNrCasesForTest( unsigned int num );

      /// Getter for the number of cases to be used for estimation
      /// @returns the number of cases available for estimation
      unsigned int getNrCasesForEstimation() const;

      /// Getter for the minimum number of cases for estimation.
      ///
      /// The minimum number of cases for estimation is one more than the
      /// number of parameter variables).
      /// If the parameter set has more than this minimum it is safe to
      /// use any remaining cases for test upto the specified or default
      /// number of test cases.
      /// @returns the minimum number of cases for estimation
      unsigned int getMinNrCasesForEstimation() const;

      /// Setter for the maximum number of cross-validations to use in the
      /// cross-validation loop.
      ///
      /// Each cross-validation iteration tests the same proxy with a
      /// different set of tune and test cases.
      /// @param [in] num the maximum number of cross-validations
      void setMaxNrCrossValidations( unsigned int num ) { m_nrCV_Max = num; }

      /// Getter for the number of cross-validation trials
      /// @returns the number of cross-validation trials
      unsigned int getMaxNrCrossValidations() const { return m_nrCV_Max; }

      /// Setter for the minimum number of stable cross-validation iterations
      /// before the cross-validation loop is considered converged.
      /// @param [in] num the minimum number of cross-validations
      void setMinNrCrossValidations( unsigned int num ) { m_nrCV_Min = num; }

      /// Getter for the minimum number of stable cross-validation trials
      /// @returns the specified minimum number of cross-validation trials
      unsigned int getMinNrCrossValidations() const { return m_nrCV_Min; }

      /// Setter for the maximum number of trials that accept declines
      /// before the search after the best proxy terminates.
      /// @param [in] num the maximum number of tolerated declines
      void setMaxNrDeclines( unsigned int num ) { m_maxNrDeclines = num; }

      /// Getter for the maximum number of trials that accept declines
      unsigned int getMaxNrDeclines() const { return m_maxNrDeclines; }

      /// Setter for the augment flag
      /// @param [in] value the new augment flag value
      /// @return the old augment flag value
      bool setDoAugment( bool value );

      /// Getter for the augment flag
      /// @return the current augment flag value
      bool getDoAugment() const;

      /// Setter for the reduce flag
      /// @param [in] value the new reduce flag value
      /// @return the old reduce flag value
      bool setDoReduce( bool value );

      /// Getter for the reduce flag
      /// @return the current reduce flag value
      bool getDoReduce() const;

      /// Getter for the number of cases to be used for testing
      /// @returns the number of cases available for testing/validation
      unsigned int getNrCasesForTest() const;

      /// Getter for the total number of cases
      /// @returns the total number of cases
      unsigned int getNrCases() const;

      /// Update the proxies in the list of candidates.
      ///
      /// Generate a proxy for the candidate variables in the candidates list, and store it
      /// in the candidates list if it performs better (i.e. lower rmseTotal) than the
      /// currently stored proxy. Whether a candidate variable is evaluated depends on whether
      /// it is part of the baseVars list and whether augmentation and/or reduction are applied.
      /// @param [in]     proxycases  a ProxyCases to provide the cases and calculate the ssq
      /// @param [in]     baseVars    the list of active variables for the base estimate
      /// @param [in]     code        the monomial code containing all potential candidates
      /// @param [in]     nbOrdPars   number of ordinal parameters
      /// @param [in]     order       polynomial order (-1 means userOrder = 0 AND search = OFF)
      /// @param [in]     threeWayX   flag indicating whether 3-way interaction terms are allowed
      /// @param [in,out] candidates  the list of candidate proxies
      void updateCandidates(
            ProxyCases const& proxycases,
            VarList const& baseVars,
            MonomialKeyList const& code,
            unsigned int nbOrdPars,
            int order,
            bool threeWayX,
            CandidateList &candidates ) const;

      /// Determine whether the top candidate (i.e. winner) remains the same.
      /// If not, the current top candidate becomes the new winner.
      /// @param [in]  ranking     the ranking of the candidate proxies wrt. total RMSE
      /// @param [in,out] winner   index to the top candidate
      /// @returns whether the winner is equal to the last saved winner
      static bool isWinnerStable( CandidateRanking const& ranking, unsigned int & winner );

      /// Perform a ranking of the candidates
      /// @param [in]   candidates  the list of candidate proxies
      /// @param [out]  ranking     the ranking of the candidate proxies wrt. total SSQ
      void rankCandidates(
            CandidateList const& candidates,
            CandidateRanking &ranking ) const;

      /// Automatic proxy estimation routine
      /// Performs the main while loop with m_nrCV cross-validation trials
      /// @param [out]  best        the best proxy candidate
      /// @param [in]   nbOrdPars   number of ordinal parameters
      /// @param [in]   userOrder   order of polynomial (0, 1, or 2) as set by the user
      /// @param [in]   vars        initial set of variables to take into account
      /// @param [in]   search      cubic polynomial model search
      /// @param [in]   targetR2    adjusted R^2 value above which we accept the model
      /// @param [in]   confLevel   needed for significance test of model increments
      /// @param [in]   criterion   used to eliminate statistically insignificant coefficients
      /// @returns whether the estimation loop has converged
      bool autoEstimate( ProxyCandidate &best, unsigned int nbOrdPars, unsigned int userOrder,
                         VarList const& vars, bool search, double targetR2, double confLevel,
                         EliminationCriterion& criterion );

      /// Automatic proxy estimation routine
      /// Performs the main while loop with m_nrCV cross-validation trials
      /// @param [out]  best        the best proxy candidate
      /// @param [in]   nbOrdPars   number of ordinal parameters
      /// @param [in]   userOrder   order of polynomial (0, 1, or 2) as set by the user
      /// @param [in]   vars        initial set of variables to take into account
      /// @param [in]   search      cubic polynomial model search
      /// @param [in]   targetR2    adjusted R^2 value above which we accept the model
      /// @param [in]   confLevel   needed for significance test of model increments
      /// @returns whether the estimation loop has converged
      bool autoEstimate( ProxyCandidate &best, unsigned int nbOrdPars, unsigned int userOrder,
                         VarList const& vars, bool search, double targetR2, double confLevel );

   private:

      /// Determines whether the estimation loop can do another round of augmentation or reduction
      bool exhausted( unsigned int nActiveVars, unsigned int nVars ) const;

      /// Checks whether it is allowed to insert the candidate to the model
      bool approveCandidate( unsigned int N, unsigned int nrPars, unsigned int nrOrdPars,
                             unsigned int Nord2, unsigned int Ncrit, MonomialKeyList const& code,
                             int order, bool threeWayX, unsigned int iCandidate ) const;

      /// Checks whether code[i] refers to a 3-way interaction term
      bool threeWayXterm( MonomialKeyList const& code, unsigned int i ) const;

      /// Checks whether the candidate model is significantly better than the best model so far
      bool betterProxyExists( ProxyCandidate const& candidate, ProxyCandidate const& best,
                              double confLevel, double eps ) const;

      /// Number of cases to be used for validation
      unsigned int   m_nrCasesForTest;

      /// Indicates whether the number of cases for test has been set
      bool           m_setNrCasesForTest;

      /// Maximum number of cross-validation trials
      unsigned int   m_nrCV_Max;

      /// Minimum number of stable cross-validation trials
      unsigned int   m_nrCV_Min;

      /// The maximum number of trials that accept a non-improved proxy candidate
      unsigned int   m_maxNrDeclines;

      /// Flag to indicate reducing the number of active variables
      bool           m_doReduce;

      /// Flag to indicate augmenting the number of variables
      bool           m_doAugment;

      // A parameter for each Case
      ParameterSet   m_parSet;

      // A target value for each Case
      TargetSet      m_targetSet;

      // Copy constructor (disabled)
      ProxyEstimator( ProxyEstimator const& );
};

/// Holds the proxy and bookkeeping for a candidate variable
struct INTERFACE_SUMLIB ProxyCandidate
{
   ProxyCandidate();

   /// set proxy based on proxy builder that is a data member of proxycases
   /// @param [in]  proxycases   provides the cases and calculates the RMSE
   void setProxy( ProxyCases const& proxycases );

   /// update a proxy candidate with a new proxy if the new proxy has a better RMSE
   /// @param [in]  proxycases   provides the cases and calculates the RMSE
   /// @param [in]  var          monomial to be added or to be removed
   bool update ( ProxyCases const& proxycases, unsigned int var);

   /// Model error indicators
   CubicProxy *proxy;
   double rmseTune;
   double rmseTest;
   double rmseTotal;
   double adjustedR2;
   std::vector<double> leverages;

   friend bool operator<(ProxyCandidate const& lhs, ProxyCandidate const& rhs );
};

} // namespace SUMlib

#endif // SUMLIB_PROXYESTIMATOR_H
