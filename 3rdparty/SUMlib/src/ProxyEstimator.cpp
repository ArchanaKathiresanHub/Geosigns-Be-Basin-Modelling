// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

#include "BaseTypes.h"
#include "CubicProxy.h"
#include "DataStructureUtils.h"
#include "Exception.h"
#include "Proxy.h"
#include "NumericUtils.h"
#include "ProxyEstimator.h"
#include "ProxyCases.h"

using std::vector;

namespace {

/// "infinity"
static const double Infinity = 1e36;

} // anonymous namespace

namespace SUMlib {

// Initialisation of default values
const unsigned int ProxyEstimator::defaultNrCV_Max                = 10;
const unsigned int ProxyEstimator::defaultNrCV_Min                = 5;
const unsigned int ProxyEstimator::defaultMaxNrDeclines           = 3;
const double       ProxyEstimator::defaultCasesForTestFraction    = 0.0; //0.25

ProxyEstimator::ProxyEstimator (
      unsigned int          nrCV_Max,           // defaultNrCV_Max
      unsigned int          nrCV_Min,           // defaultNrCV_Min
      unsigned int          maxNrDeclines,      // defaultMaxNrDeclines
      bool                  doAugment,          // true
      bool                  doReduce            // true
      )
   :
      m_nrCasesForTest( 0 ),
      m_setNrCasesForTest( false ),
      m_nrCV_Max( nrCV_Max ),
      m_nrCV_Min( nrCV_Min ),
      m_maxNrDeclines( maxNrDeclines ),
      m_doReduce( doReduce ),
      m_doAugment( doAugment )
{
   // make sure internals are properly bounded
   m_nrCV_Min = std::min( m_nrCV_Min, m_nrCV_Max );
}

void ProxyEstimator::setParameterSet( ParameterSet  const&  parSet )
{
      /// assume parSet is scaled to [-1,1]
      m_parSet = parSet;
}

void ProxyEstimator::setTargetSet( TargetSet const& targetSet )
{
   m_targetSet = targetSet;
}

void ProxyEstimator::setNrCasesForTest( unsigned int num )
{
   m_nrCasesForTest = num;
   m_setNrCasesForTest = true;
}

unsigned int ProxyEstimator::getNrCases() const
{
   return m_parSet.size();
}

unsigned int ProxyEstimator::getNrCasesForTest() const
{
   return getNrCases() - getNrCasesForEstimation();
}

unsigned int ProxyEstimator::getNrCasesForEstimation() const
{
   unsigned int nTest = m_setNrCasesForTest
      ? std::min<unsigned int>( getNrCases(), m_nrCasesForTest )
      : static_cast<unsigned int>( getNrCases() * defaultCasesForTestFraction );

   // The number of active cases must be at least the minimum number of cases,
   // and at most all cases but those used for testing
   return std::max<unsigned int>( getNrCases() - nTest, getMinNrCasesForEstimation() );
}

unsigned int ProxyEstimator::getMinNrCasesForEstimation() const
{
   // The minimum number of active cases must be at least one more than
   // twice the number of parameters, and at most all cases
   if ( ! m_parSet.empty() )
   {
      return std::min( 2*m_parSet.front().size() + 1, m_parSet.size() );
   }
   else
   {
      return 0;
   }
}

bool ProxyEstimator::getDoAugment() const
{
   return m_doAugment;
}

bool ProxyEstimator::setDoAugment( bool value )
{
   bool old=getDoAugment();
   m_doAugment=value;
   return old;
}

bool ProxyEstimator::getDoReduce() const
{
   return m_doReduce;
}

bool ProxyEstimator::setDoReduce( bool value )
{
   bool old=getDoReduce();
   m_doReduce=value;
   return old;
}

bool ProxyEstimator::threeWayXterm( MonomialKeyList const& code, unsigned int i ) const
{
   if ( ( code[i].size() == 3 ) && ( code[i][0] != code[i][1] ) && ( code[i][1] != code[i][2] ) )
   {
      return true;
   }
   return false;
}

bool ProxyEstimator::approveCandidate( unsigned int N, unsigned int nrPars, unsigned int nrOrdPars,
                     unsigned int Nord2, unsigned int Ncrit, MonomialKeyList const& code,
                     int order, bool allow3WayX, unsigned int iCandidate ) const
{
   // N is the number of cases used to build the proxy
   assert( nrOrdPars <= nrPars ); //number of ordinal parameters cannot exceed total number of parameters
   assert( nrPars < Nord2 ); //total number of parameters must be smaller than the number of all 2nd order terms
   assert( Nord2 < Ncrit ); //number of all 3rd order terms excl. 3-way interaction terms > number of all 2nd order terms
   assert( order < 3 || order == 9 );
   assert( iCandidate < code.size() );

   // Handle special case first: no categorical parameter(s) present AND only allow up to pure quadratic terms
   if ( nrOrdPars == nrPars && order == 9 )
   {
      assert( code[iCandidate].size() < 3 );
      if ( code[iCandidate].size() == 2 && code[iCandidate][0] != code[iCandidate][1] ) return false;
   }

   // If no categorical parameter(s) present AND order is at default
   if ( nrOrdPars == nrPars && order == 0 )
   {
      if ( N <= 1 + nrPars )
      {
         if ( code[iCandidate].size() > 1 ) return false; //only 1st order term is allowed
      }
      else if ( N <= Nord2 )
      {
         if ( code[iCandidate].size() == 3 ) return false; //reject 3rd order term
      }
      else if ( N <= Ncrit )
      {
         if ( threeWayXterm( code, iCandidate ) ) return false; //reject 3-way interaction term
      }
      // Only accept 3-way interaction term if it helps convergence
      else if ( !allow3WayX && threeWayXterm( code, iCandidate ) ) return false;
   }

   // Else if categorical parameter(s) are present: Reject higher order terms that make no sense from a user perspective
   else if ( nrOrdPars < nrPars )
   {
      // If the user just asks for intercepts (user order = 0) with auto-search = OFF, then reject any other term
      if ( order == -1 ) //-1 is special flag indicating user order = 0 with no wish for auto search
      {
         if ( code[iCandidate].size() == 1 )
         {
            if ( code[iCandidate][0] < nrOrdPars ) return false;
         }
         else if ( code[iCandidate].size() == 2 )
         {
            if ( !CubicProxy::validOrder2Var( 0, nrOrdPars, code[iCandidate][1], code[iCandidate][0] ) )
            {
               return false;
            }
         }
         else if ( code[iCandidate].size() == 3 )
         {
            if ( !CubicProxy::validOrder3Var( 0, nrOrdPars, code[iCandidate][2], code[iCandidate][1], code[iCandidate][0] ) )
            {
               return false;
            }
         }
      }
      else
      {
         unsigned int ao = ( order == 0 ) ? 2 : order; //ao is a locally applied order
         if ( code[iCandidate].size() == 2 )
         {
            if ( !CubicProxy::validOrder2Var( ao, nrOrdPars, code[iCandidate][1], code[iCandidate][0] ) )
            {
               return false;
            }
         }
         else if ( code[iCandidate].size() == 3 )
         {
            if ( !CubicProxy::validOrder3Var( ao, nrOrdPars, code[iCandidate][2], code[iCandidate][1], code[iCandidate][0] ) )
            {
               return false;
            }
         }
      }

      // If the auto search is intentional AND the order = 0 AND the number of cases is limited
      if ( order == 0 ) //intentional auto search otherwise order would have been equal to -1
      {
         if ( N <= 1 + nrPars - nrOrdPars )
         {
            if ( code[iCandidate][0] < nrOrdPars ) return false; //only accept an intercept (monomial containing dummies only)
         }
         else if ( N <= ( 1 + nrPars - nrOrdPars ) * ( 1 + nrOrdPars ) )
         {
            if ( code[iCandidate].size() == 2 )
            {
               if ( !CubicProxy::validOrder2Var( 1, nrOrdPars, code[iCandidate][1], code[iCandidate][0] ) )
               {
                  return false; //only accept 2nd order monomial with no or one ordinal parameter
               }
            }
            else if ( code[iCandidate].size() == 3 )
            {
               if ( !CubicProxy::validOrder3Var( 1, nrOrdPars, code[iCandidate][2], code[iCandidate][1], code[iCandidate][0] ) )
               {
                  return false; //only accept 3rd order monomial with no or one ordinal parameter
               }
            }
         }
      }
   }

   return true;
}

void ProxyEstimator::updateCandidates( ProxyCases const& proxycases, VarList const& baseVars, MonomialKeyList const& code,
                     unsigned int nrOrdPars, int order, bool threeWayX, CandidateList & candidates ) const
{
   const unsigned int nrBaseVars = baseVars.size();
   const unsigned int nrTuneCases = proxycases.numTuneCases();
   const unsigned int nrPars = proxycases.caseSize();
   const unsigned int N = getNrCases();

   // Number of all potential terms up to second order
   const unsigned int Nord2 = 1 + CubicProxy::numVars( nrPars, 2 );

   // Number of all potential terms except 3-way interaction terms
   const unsigned int Ncrit = Nord2 + nrPars * nrPars;

   // For each of the candidate variables repeat with either reduced or augmented set
   // depending on whether the variable is active
   for ( unsigned int i = 0; i < candidates.size(); ++i )
   {
      const bool active = IndexListContains( baseVars, i );
      const bool insert = ( ! active && getDoAugment() && nrBaseVars < nrTuneCases - 1 );
      const bool remove = ( active && getDoReduce() && nrBaseVars <= nrTuneCases );

      // Update the candidate
      if ( remove )
      {
         candidates[i].update( proxycases, i, nrBaseVars - 1 );
      }
      else if ( insert && approveCandidate( N, nrPars, nrOrdPars, Nord2, Ncrit, code, order, threeWayX, i ) )
      {
         candidates[i].update( proxycases, i, nrBaseVars + 1 );
      }
   }
}

void ProxyEstimator::rankCandidates( CandidateList const& candidates, CandidateRanking & ranking ) const
{
   ranking.resize(candidates.size());
   for ( unsigned int k = 0; k < ranking.size(); ++k )
   {
      ranking[k] = std::make_pair( candidates[k].rmseTotal, k );
   }

   // Sort the candidates (actually array indices into candidates) wrt. RMSE of total parameter set
   // (std::pair::operator< sorts uses the first element of the pair)
   std::sort( ranking.begin(), ranking.end() );
}

unsigned int GetIndex( std::pair<double,unsigned int> const& p ) { return p.second; }

bool ProxyEstimator::isWinnerStable( CandidateRanking const& ranking, unsigned int& winner )
{
   unsigned int currentWinner = ranking[0].second;
   unsigned int initialIndex = ranking.size(); //deliberately chosen initial winner index

   // Determine whether current winner and last saved winner are the same
   bool stableCycle = currentWinner == winner;
   if ( ! stableCycle )
   {
      if ( winner == initialIndex ) //only in the first CV cycle!
      {
         stableCycle = true;
      }
      winner = currentWinner;
   }
   return stableCycle;
}

bool ProxyEstimator::exhausted( unsigned int nActiveVars, unsigned int nVars ) const
{
   if ( ! getDoReduce() && ! getDoAugment() )
   {
      return true; //no model search at all, so use the initial proxy as defined by the supplied vars
   }
   else
   {
      return ( ( ! getDoReduce() && nActiveVars >= nVars ) || ( ! getDoAugment() && nActiveVars == 0 ) );
   }
}

bool ProxyEstimator::autoEstimate( ProxyCandidate &best, unsigned int nbOrdPars, unsigned int userOrder,
                                   VarList const& vars, bool search, double targetR2, double confLevel )
{
   bool converged = false;
   int order = userOrder;

   // Check if the dimensions fit
   if ( m_parSet.empty() )
   {
      return converged;
   }
   if ( m_parSet.size() != m_targetSet.size() )
   {
      THROW2( DimensionMismatch, "Parameter set and target set must be of equal size" );
   }

   // The number of parameters = the number of entries in each case
   const unsigned int numPar = m_parSet.front().size();

   // The code containing all potential monomials/variables up to 3rd order
   MonomialKeyList code;
   CubicProxy::monomial_code( numPar, code );

   // The total number of variables for a full-cubic proxy
   unsigned int nConsideredVars = CubicProxy::numVars( numPar );

   // vars is assumed to be sorted!
   if ( ! vars.empty() && vars.back() >= nConsideredVars )
   {
      THROW2(IndexOutOfBounds, "Active variable index exceeds the total number of cubic proxy variables" );
   }

   VarList activeVars = vars;

   // The number of variables that can be determined with the cases available for estimation
   const unsigned int maxNumVars = getNrCasesForEstimation() - 1;

   // If auto search has been set to true, start model building from first order terms
   // (only if "many" cases available) or from scratch while honoring the specified order.
   // Exception: if order = 0, up to third order terms may be considered for auto search!
   if ( search && ( nbOrdPars == numPar ) ) //no categorical parameter(s) present
   {
      const unsigned int nbOrder2Vars = CubicProxy::numVars( numPar, 2 );
      if ( getNrCases() > 1 + nbOrder2Vars ) //"many" cases available
      {
         // The supplied vars are replaced by first order terms only
         activeVars.resize( numPar );
         for ( unsigned int i = 0; i < numPar; ++i )
         {
            activeVars[i] = i;
         }
      }
      else //start model building from scratch; 3rd order terms will not be searched for!
      {
         // The supplied vars are removed (intercept only)
         activeVars.clear();
      }

      // Find only influential variables that honor the specified order > 0
      if ( userOrder == 1 )
      {
         nConsideredVars = CubicProxy::numVars( numPar, userOrder );
      }
      else if ( userOrder == 2 || userOrder == 9 )
      {
         nConsideredVars = CubicProxy::numVars( numPar, 2 );
      }
      else //order = 0
      {
         if ( getNrCases() <= 1 + numPar )
         {
            nConsideredVars = numPar;
         }
         else if ( getNrCases() <= 1 + nbOrder2Vars )
         {
            nConsideredVars = nbOrder2Vars;
         }
      }
   }
   else if ( search ) //at least one categorical parameter present
   {
      // The supplied vars are removed (intercept only)
      activeVars.clear();
   }

   // If auto search has been set to false,
   // check whether there are enough cases to handle all supplied vars.
   if ( ! search && ( activeVars.size() >= getNrCases() ) )
   {
      // The supplied vars cannot be handled as too few cases are available
      search = true; //no other choice than using auto search (from scratch)
      targetR2 = 1.0; //maximum target value here as the user did not ask for auto search
      activeVars.clear(); //remove the supplied vars to enable model building from scratch

      // Find only influential variables that honor the specified order
      if ( nbOrdPars == numPar ) //no categorical parameter(s) present, so order > 0 here!
      {
         unsigned int appliedOrder = ( userOrder == 9 ) ? 2 : userOrder;
         nConsideredVars = CubicProxy::numVars( numPar, appliedOrder );
      }
      else if ( userOrder == 0 ) //categorical parameter(s) present AND user wants intercepts only
      {
         order = -1; //flag needed in approveCandidate() to look for different intercept terms
      }
   }

   // The total number of considered variables
   const unsigned int nVars = nConsideredVars;

   // Enable/disable auto search
   bool oldAugmentFlag, oldReduceFlag;
   if ( search )
   {
      // Enable auto search: exhausted() must always return false
      oldAugmentFlag = setDoAugment( true );
      oldReduceFlag = setDoReduce( true );
   }
   else
   {
      // Disable auto search: exhausted() must always return true
      oldAugmentFlag = setDoAugment( false );
      oldReduceFlag = setDoReduce( false );
   }

   unsigned int deteriorateCount = 0;

   // If all cases are to be used for estimation, then there is no point in
   // iterating because the random selection is the same with each cross-validation loop
   if ( getNrCasesForEstimation() == getNrCases() )
   {
      m_nrCV_Max = 1;
   }

   // Store parameter set and target set
   ProxyCases proxycases( m_parSet, m_targetSet );

   // The supplied vars serve as starting point for the search algorithm
   // So the proxy candidate is initialised to a proxy with the supplied vars.
   proxycases.createProxyBuilder( activeVars ); //create proxy builder first based on supplied vars
   best.setProxy( proxycases, activeVars.size() ); //set proxy candidate cheaply with help of proxy builder
   const double eps = 1e-9;
   if ( best.adjustedR2 >= ( targetR2 - eps ) ) converged = true; //initial proxy already fits well.

   // The best proxy candidate has been properly initialised now. If a better one
   // is found in the below while loop, a final SVD will be performed based on all cases.
   bool betterProxyFound = false;
   unsigned int whileIterCount = 0;
   const unsigned int maxNbOfIters = std::max<unsigned int>( 2*numPar + getNrCases()/3, 50 );
   bool allow3WayCrossTerms = false;

   while ( ! converged && ! exhausted( activeVars.size(), maxNumVars ) )
   {
      whileIterCount++;

      // Iterate at most m_nrCV_Max times over the estimation loop, with randomly selected
      // sets of active cases, attempting to improve the best estimate in each estimation loop.
      // Count the number of successive iterations that yield the same top candidate of the
      // ranking list. If that number of iterations (counted by stable) equals m_nrCV_Min, the
      // cross-validation loop terminates.
      unsigned int stable = 0;
      // Initialise the index to the top candidate (winner)
      unsigned int winner = nVars; //deliberately chosen initial index (out of bound)

      // The list of candidate proxies
      CandidateList    candidates( nVars );
      CandidateRanking ranking;
      CaseList cases( m_parSet.size(), false );
      fill_n( cases.begin(), getNrCasesForEstimation(), true );
      for ( unsigned int cv = 0; stable < m_nrCV_Min && cv < m_nrCV_Max; ++cv )
      {
         if ( getNrCasesForTest() > 0 )
         {
            // draw randomly nrCasesForEstimation
            // remaining cases are test cases
            ProxyCases::shuffle( cases );
            proxycases.setCaseList( cases );
         }

         if ( ( whileIterCount > 1 ) || ( getNrCasesForTest() > 0 ) )
         {
            // proxycases must be supplied with a proper instance of ProxyBuilder.
            proxycases.createProxyBuilder( activeVars );
         }

         // Now try all candidate augmenting/reducing variables and sort wrt. RMSE
         updateCandidates( proxycases, activeVars, code, nbOrdPars, order, allow3WayCrossTerms, candidates );

         rankCandidates( candidates, ranking );

         // Update the counter of stable cycles
         if ( isWinnerStable( ranking, winner ) )
         {
            ++stable;
         }
         else
         {
            stable = 1; //the new winner starts a brand new stable cycle
         }
      } // cross-validation loop

      // The index of the best of the candidate variables
      const unsigned int best_index = ranking[0].second;

      // Does the best candidate outperform the best model so far?
      const bool improved = betterProxyExists( candidates[ best_index ], best, confLevel, eps );

      // The main loop converged if no candidate was found that performs
      // better than the current best candidate
      // "entia non sunt multiplicanda praeter necessitatem"
      // [Occam's razor, http://en.wikipedia.org/wiki/Occam's_razor]
      converged = ( ! improved );

      // Store the best candidate if it outperforms the current selection of variables
      if ( improved )
      {
         betterProxyFound = true;
         delete best.proxy;

         // Now transfer ownership of proxy to best
         best = candidates[ best_index ];
         candidates[ best_index ].proxy = NULL;

         // Copy the selected variables to the active variables list
         best.proxy->getVarList( activeVars );

         // Convergence if either adjusted R^2 has reached its target,
         // or if the maximum number of iterations has been reached.
         if ( ( best.adjustedR2 >= targetR2 ) || ( whileIterCount > maxNbOfIters ) )
         {
            converged = true;
         }

         // Reset the counter of deteriorating iterations
         deteriorateCount = 0;
      }
      else if ( deteriorateCount < m_maxNrDeclines && candidates[best_index].proxy )
      {
         // Next iteration starts with the variables list from the
         // best of the ranked candidates, and attempts to get an improvement
         candidates[best_index].proxy->getVarList( activeVars );

         // attempt to improve the best candidate so far for
         // maxDeteriorateCount iterations
         ++deteriorateCount;
         converged = false; //otherwise the search terminates
         allow3WayCrossTerms = true; //from now on, "xyz" terms may be added to help convergence
      }
      // else done because either the best candidate was found maxDeteriorateCount
      // iterations ago (and since then no improvement has been found), or none of
      // the candidates could ever be updated.

      // Remove the other candidates
      for ( unsigned int k = 0; k < nVars; ++k )
      {
         delete candidates[k].proxy;
         candidates[k].proxy = NULL;
      }
   } // main convergence loop

   // The final SVD based on all cases
   if ( betterProxyFound )
   {
      CaseList allCasesActive( m_parSet.size(), true );
      proxycases.setCaseList( allCasesActive );
      best.proxy->getVarList( activeVars );
      proxycases.createProxyBuilder( activeVars );
      best.setProxy( proxycases, activeVars.size() );
   }

   // Restore old flags
   setDoAugment( oldAugmentFlag );
   setDoReduce( oldReduceFlag );

   // Indicate if we converged or if the number of cases was exhausted
   return converged;
}

bool ProxyEstimator::betterProxyExists( ProxyCandidate const& candidate, ProxyCandidate const& best,
                        double confLevel, double eps ) const
{
   assert( confLevel < ( 0.999 + eps ) ); //99.9% has been assumed as upper bound
   if ( ! ( candidate.adjustedR2 > ( best.adjustedR2 + eps ) ) )
   {
      return false;
   }

   std::vector<unsigned int> vars;
   candidate.proxy->getVarList( vars );
   int nAddedVars = vars.size();
   best.proxy->getVarList( vars );
   nAddedVars -= vars.size();
   const unsigned int usedDegrees = vars.size() + nAddedVars + 1;
   assert( getNrCases() >= usedDegrees );
   const unsigned int degreesOfFreedom = getNrCases() - usedDegrees;
   if ( degreesOfFreedom == 0 ) //not supposed to happen, but just in case...
   {
      return false;
   }
   if ( ( nAddedVars <= 0 ) || ( confLevel < ( 0.5 + eps ) ) )
   {
      return true; //reject the null hypothesis that candidate ~ best
   }

   const double candidateMSE = candidate.rmseTotal * candidate.rmseTotal;
   const double bestMSE = best.rmseTotal * best.rmseTotal;
   if ( candidateMSE < eps * bestMSE )
   {
      return true; //reject the null hypothesis that candidate ~ best
   }
   double statistic = -1.0 * degreesOfFreedom;
   statistic += ( degreesOfFreedom + nAddedVars ) * bestMSE / candidateMSE;
   assert( statistic > ( nAddedVars - eps ) ); //by construction
   statistic = sqrt( statistic ); //converts PDF from F(1,df) to Student-t(df)

   return ( statistic > CriticalValue( degreesOfFreedom, confLevel * 100.0 ) );
}

ProxyCandidate::ProxyCandidate() :
   proxy( NULL ),
   rmseTune( Infinity ),
   rmseTest( Infinity ),
   rmseTotal( Infinity ),
   adjustedR2( -Infinity )
{
   // empty
}

void ProxyCandidate::setProxy( ProxyCases const& proxycases, unsigned int nActualVars )
{
   delete this->proxy;
   proxy = proxycases.createProxy( );
   proxycases.test( proxy, nActualVars, rmseTune, rmseTest, rmseTotal, adjustedR2 );
   leverages = proxycases.calcLeverages();
   proxy->setDesignMatrixRank( proxycases.getDesignMatrixRank() );
   proxy->setStdErrors( proxycases.calcStdErrors() );
}

bool ProxyCandidate::update( ProxyCases const& proxycases, unsigned int var, unsigned int nActualVars )
{
   bool updated(false);

   // Create a proxy
   CubicProxy *p = proxycases.createProxy( var );

   // Test the proxy (RMSE of the tune/test sample, adjusted RMSE of the total sample)
   double rmseEst, rmseTst, rmseTot, adjR2;
   proxycases.test( p, nActualVars, rmseEst, rmseTst, rmseTot, adjR2 );

   if ( rmseTot < rmseTotal )
   {
      updated = true;
      delete this->proxy;
      proxy = p;
      rmseTune = rmseEst;
      rmseTest = rmseTst;
      rmseTotal = rmseTot;
      adjustedR2 = adjR2;
   }
   else
   {
      delete p;
   }
   return updated;
}

bool operator<(ProxyCandidate const& lhs, ProxyCandidate const& rhs ) { return lhs.rmseTotal < rhs.rmseTotal; }

} // namespace SUMlib
