//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LMOptAlgorithm.h"
#include "Parameter.h"
#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "ScenarioAnalysis.h"
#include "SUMlibUtils.h"
#include "VarSpace.h"
#include "VarPrmContinuous.h"

// Utilities lib
#include "NumericFunctions.h"

#include <unsupported/Eigen/LevenbergMarquardt>
#include <unsupported/Eigen/NumericalDiff>

// SUMlib
#include <Case.h>

// STL / C lib
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <vector>

namespace casa
{

struct ProjectFunctor : public Eigen::DenseFunctor<double>
{
   ProjectFunctor( LMOptAlgorithm & lm
                 , int prmSpaceDim
                 , int obsSpaceDim
                 ) 
                 : DenseFunctor<double>( prmSpaceDim, obsSpaceDim )
                 , m_lm( lm )
   {
   }

   int operator() ( const Eigen::VectorXd & x, Eigen::VectorXd & fvec ) const
   {
      LMOptAlgorithm & lm = const_cast<LMOptAlgorithm&>( m_lm );
      lm.updateParametersAndRunCase( x );
      lm.calculateFunctionValue( fvec );

      return 0;
   }
 
   const LMOptAlgorithm & m_lm;
};

size_t LMOptAlgorithm::prepareParameters( std::vector<double> & initGuess )
{
   if ( !initGuess.empty()   ) initGuess.clear();
   if ( !m_optimPrms.empty() ) m_optimPrms.clear();
   if ( !m_permPrms.empty()  ) m_permPrms.clear();

   size_t globNum = 0;

   // filter nonfrozen parameters and add them to the list
   for ( size_t i = 0; i < m_sa->varSpace().numberOfContPrms(); ++i )
   {
      const VarPrmContinuous * vprm = m_sa->varSpace().continuousParameter( i );
      const std::vector<double> & minVals = vprm->minValue()->asDoubleArray();
      const std::vector<double> & maxVals = vprm->maxValue()->asDoubleArray();
      const std::vector<double> & basVals = vprm->baseValue()->asDoubleArray();

      for ( size_t k = 0; k < minVals.size(); ++k )
      {
         if ( !NumericFunctions::isEqual( minVals[k], maxVals[k], 1.e-6 ) )
         {
            m_optimPrms.push_back( std::pair<const VarPrmContinuous*, size_t>( vprm, k ) );
            m_permPrms.push_back( globNum + k );
            initGuess.push_back( basVals[k] );
         }
      }
      globNum += basVals.size();
   }
   return initGuess.size();
}

size_t LMOptAlgorithm::prepareObservables()
{
   if ( !m_optimObs.empty() ) m_optimObs.clear();
   if ( !m_permObs.empty()  ) m_permObs.clear();

   // filter observables which are suitable for the optimization loop
   const ObsSpace & obSp = m_sa->obsSpace();
   size_t obsSpDim = 0;
   for ( size_t i = 0; i < obSp.size(); ++i )
   {
      const std::string & msg = obSp.observable( i )->checkObservableForProject( m_sa->baseCase() );
   
      if ( msg.empty() && obSp.observable( i )->hasReferenceValue() )
      {
         obsSpDim += obSp.observable( i )->dimension();
         m_optimObs.push_back( obSp.observable( i ) );
         m_permObs.push_back( i );
      }
   }
   return obsSpDim;
}

void LMOptAlgorithm::updateParametersAndRunCase( const Eigen::VectorXd & x )
{
   // create base case parameters value
   std::vector<double>       cntPrms;
   std::vector<double>       minPrms;
   std::vector<double>       maxPrms;
   std::vector<unsigned int> catPrms;

   for ( size_t i = 0; i < m_sa->varSpace().size(); ++i )
   {
      switch( m_sa->varSpace().parameter( i )->variationType() )
      {
         case VarParameter::Continuous:
            {
               const VarPrmContinuous * vprm = dynamic_cast<const VarPrmContinuous*>( m_sa->varSpace().parameter( i ) );
           
               const std::vector<double> & pva = vprm->baseValue()->asDoubleArray();  
               cntPrms.insert( cntPrms.end(), pva.begin(), pva.end() );
            
               const std::vector<double> pminva = vprm->minValue()->asDoubleArray();
               minPrms.insert( minPrms.end(), pminva.begin(), pminva.end() );
            
               const std::vector<double> pmaxva = vprm->maxValue()->asDoubleArray();
               maxPrms.insert( maxPrms.end(), pmaxva.begin(), pmaxva.end() );
            }
            break;

         case VarParameter::Categorical:
            catPrms.push_back( dynamic_cast<const VarPrmCategorical*>( m_sa->varSpace().parameter( i ) )->baseValue()->asInteger() );
            break;
         default: break;
      }
   }

   // modify base case values
   for ( size_t i = 0; i < m_permPrms.size(); ++i )
   {
      if (      minPrms[m_permPrms[i]] > x( i ) ) { cntPrms[m_permPrms[i]] = minPrms[m_permPrms[i]]; }
      else if ( maxPrms[m_permPrms[i]] < x( i ) ) { cntPrms[m_permPrms[i]] = maxPrms[m_permPrms[i]]; }
      else                                        { cntPrms[m_permPrms[i]] = x( i ); }
   }

   // create new case with the new parameters values
   SUMlib::Case slCase( cntPrms, std::vector<int>(), catPrms );
   RunCaseImpl * rc = new RunCaseImpl();

   // convert array of parameters values to case parametrers
   sumext::convertCase( slCase, m_sa->varSpace(), *rc );
   
   // add this case as a new experiment
   std::string expName = "LMStep_" + std::to_string( m_sa->scenarioIteration() );
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( m_sa->doeCaseSet() );
  
   std::vector<RunCase*> cvec( 1, rc );
   rcs.addNewCases( cvec, expName );
   rcs.filterByExperimentName( expName );
   
   assert( rcs.size() == 1 );

   // generate new project
   if ( ErrorHandler::NoError != m_sa->applyMutations( rcs ) ) { throw ErrorHandler::Exception( m_sa->errorCode() ) << m_sa->errorMessage(); }

   casa::RunManager & rm = m_sa->runManager();
   
   // submit jobs
   if ( ErrorHandler::NoError != rm.scheduleCase(*rc, m_sa->scenarioID()) ) { throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage(); }
   if ( ErrorHandler::NoError != rm.runScheduledCases( false )            ) { throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage(); }

   // collect observables value
   if ( ErrorHandler::NoError != m_sa->dataDigger().collectRunResults( m_sa->obsSpace(), m_sa->doeCaseSet() ) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }
}

void LMOptAlgorithm::calculateFunctionValue( Eigen::VectorXd & fvec )
{
   // initialze minimization function with all zeros
   for ( size_t i = 0; i < fvec.rows() * fvec.cols(); ++i ) { fvec( i ) = 0.0; }
   
   if ( m_sa->doeCaseSet().size() != 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of run cases for minimization process.";
   }

   const RunCase * rc = m_sa->doeCaseSet()[0];
   size_t mi = 0;
   
   // at first alculate minimization function terms for observables value 
   for ( size_t i = 0; i < m_permObs.size(); ++i )
   {
      const Observable          * obs      = m_optimObs[i]; 
      const std::vector<double> & refVal   = obs->referenceValue()->asDoubleArray();
      const std::vector<double> & obv      = rc->obsValue( m_permObs[i] )->asDoubleArray();
      double                      sigma    = obs->stdDeviationForRefValue();
      double                      uaWeight = obs->uaWeight();

      assert( refVal.size() == obv.size() );
            
      for ( size_t k = 0; k < refVal.size(); ++k )
      {
         double dif = sqrt( uaWeight ) * std::abs( obv[k] - refVal[k] ) / sigma;
         fvec( mi ) = dif;
         ++mi;
      }
   }

   // the calculate minimization function terms for parameters value, to prevent going outside ranges
   size_t pi = 0;
   for ( size_t i = 0; i < m_permPrms.size(); ++i )
   {
      const VarPrmContinuous * vprm  = m_optimPrms[i].first;
      size_t                   prmID = m_optimPrms[i].second;
      double                   minV  = vprm->minValue( )->asDoubleArray()[prmID];
      double                   maxV  = vprm->maxValue( )->asDoubleArray()[prmID];
      double                   basV  = vprm->baseValue()->asDoubleArray()[prmID];
      VarPrmContinuous::PDF    ppdf  = vprm->pdfType();
      double                   pval  = rc->parameter( m_permPrms[i] )->asDoubleArray()[prmID];
 
      if (      pval < minV ) { fvec[mi] = 100.0 * exp( minV - pval ) - 100.0; }
      else if ( pval > maxV ) { fvec[mi] = 100.0 * exp( pval - maxV ) - 100.0; }
      else
      {
         switch( ppdf )
         {
            case VarPrmContinuous::Block:    fvec[mi] = 1e-10; break;
            
            case VarPrmContinuous::Triangle: 
               {  double d = 2.0 / ( (basV - minV) * (maxV - basV) ); // area of triangle is equal 1

                  if ( pval < basV ) { fvec[mi] = 0.5 * d * ( 1.0 + ( pval - minV ) / ( basV - minV ) ) * ( basV - pval ); }
                  else               { fvec[mi] = 0.5 * d * ( 1.0 + ( pval - maxV ) / ( basV - maxV ) ) * ( pval - basV ); }
               }
               break;

            case VarPrmContinuous::Normal:
               {
                  double sigma = ( maxV - minV ) / 6.0; // 3 sigma - half interval 
                  double pw = ( pval - basV ) / sigma;
                  // sqrt( 2.0 * pi ) = 2.506628274631 
                  fvec[mi] = 1.0 / ( sigma * 2.506628274631  ) * ( 1.0 - exp( -0.5 * pw * pw  ) );
               }
               break;
         }
      }
      mi++;
   }
}

void LMOptAlgorithm::runOptimization( ScenarioAnalysis & sa )
{
   m_sa = &sa;

   // extract continuous parameters with non empty min/max range, fill initial guess vector
   std::vector<double> guess;
   size_t prmSpDim = prepareParameters( guess );

   // convert std::vector to Eigen vector
   Eigen::VectorXd initialGuess( guess.size() );
   size_t xi = 0;
   for ( auto pv : guess ) initialGuess[xi++] = pv;
   
   std::cout << "x: \n" << initialGuess << std::endl;

   // extract observables with reference values and calculate dimension
   size_t obsSpDim = prepareObservables();

   // Functor
   ProjectFunctor functor( *this, prmSpDim, prmSpDim + obsSpDim ); // use parameters also as observables to keep them in range

   Eigen::NumericalDiff<ProjectFunctor> numDiff( functor );
   Eigen::LevenbergMarquardt< Eigen::NumericalDiff<ProjectFunctor> > lm( numDiff );

   lm.setMaxfev( 2000 );
   lm.setXtol( 1.0e-10 );

   std::cout << 2000 << std::endl;

   int ret = lm.minimize( initialGuess );

   std::cout << "Iterations number: " << lm.iterations() << std::endl;
   std::cout << "Return code: " << ret << std::endl;

   std::cout << "x that minimizes the function: \n" << initialGuess << std::endl;

   std::cout << "press [ENTER] to continue " << std::endl;
   std::cin.get();
   
   m_sa = 0;
}

} // namespace casa

