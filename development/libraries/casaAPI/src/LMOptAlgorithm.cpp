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

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include "NumericFunctions.h"
#include "LogHandler.h"

// Eigen
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

// 1. Using fvec for each observation will produce a jacobian more similar to the one produced by PEST 
//#define ACCUMULATE_MIN_FUNCTION 1

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

   // We always assume forward differences. In case of log10 transformation we increment the base value and transform the increment back, as done in PEST 
   int df( const InputType& _x, JacobianType &jac )
   {
      using std::sqrt;
      using std::abs;
      /* Local variables */
      Scalar h;
      int nfev = 0;
      const InputType::Index n = _x.size( );
      ValueType val1, val2;
      const Scalar eps = 0.1; // 10% increment as in PEST. How to pass the eps of NumericalDiff?
      InputType x = _x;

      val1.resize( ProjectFunctor::values( ) );
      val2.resize( ProjectFunctor::values( ) );

      // compute f(x)
      ProjectFunctor::operator()( x, val1 ); nfev++;

      // Function Body
      for ( int j = 0; j < n; ++j ) 
      {
        //calculate the base value
        if ( m_lm.transformation( ) == "log10" )
        {
            x[j] = pow( 10, x[j] );
        }
        //increment always in terms of the base value
        h = 0.1 * abs( x[j] );
        x[j] += h;
        //backtransform in log10 for function evaluation
        if ( m_lm.transformation( ) == "log10" )
        {
           x[j] = log10( x[j] );
        }
        ProjectFunctor::operator()( x, val2 );
        nfev++;
        // the increment h should be in log10
        if ( m_lm.transformation( ) == "log10" )
        {
           h =  x[j]  - _x[j] ;
        }
        //restore the old value
        x[j] = _x[j];
        jac.col( j ) = ( val2 - val1 ) / h; 
      };
      return nfev;
   }

   const LMOptAlgorithm & m_lm;
};

size_t LMOptAlgorithm::prepareParameters( std::vector<double> & initGuess, std::vector<double> & minPrm, std::vector<double> & maxPrm )
{
   if ( !initGuess.empty()   ) initGuess.clear();
   if ( !minPrm.empty( ) ) minPrm.clear( );
   if ( !maxPrm.empty( ) ) maxPrm.clear( );
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
            if ( m_parameterTransformation == "log10" )
            {
               initGuess.push_back( log10( basVals[k] ) );
               minPrm.push_back( log10( minVals[k] ) );
               maxPrm.push_back( log10( maxVals[k] ) );
            }
            else
            {
               initGuess.push_back( basVals[k] );
               minPrm.push_back( minVals[k] );
               maxPrm.push_back( maxVals[k] );
            }
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
   m_xi.clear();

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

         default: assert( 0 ); break;
      }
   }

   // modify base case values
   for ( size_t i = 0; i < m_permPrms.size(); ++i )
   {
      double prmVal = x( i );
      if ( m_parameterTransformation == "log10" ) 
      {
         prmVal = pow( 10.0, prmVal);
      }
      if ( minPrms[m_permPrms[i]] > prmVal )
      {
         cntPrms[m_permPrms[i]] = minPrms[m_permPrms[i]];
         LogHandler( LogHandler::DEBUG_SEVERITY ) << " parameter " << i << " = " << prmVal << " < min range value: " << minPrms[m_permPrms[i]];
        // x( i ) = minPrms[m_permPrms[i]];
      }
      else if ( maxPrms[m_permPrms[i]] < prmVal )
      {
         cntPrms[m_permPrms[i]] = maxPrms[m_permPrms[i]];
         LogHandler( LogHandler::DEBUG_SEVERITY ) << " parameter " << i << " = " << prmVal << " > max range value: " << maxPrms[m_permPrms[i]];
      }
      else
      {
         cntPrms[m_permPrms[i]] = prmVal;
         LogHandler( LogHandler::DEBUG_SEVERITY ) << " parameter " << i << " = " << prmVal;
      }
      //later on used to calculate the penality function
      m_xi.push_back( x( i ) );
   }

   // create new case with the new parameters values
   SUMlib::Case slCase( cntPrms, std::vector<int>(), catPrms );
   std::unique_ptr<RunCaseImpl> rc( new RunCaseImpl() );

   // convert array of parameters values to case parametrers
   sumext::convertCase( slCase, m_sa->varSpace(), *(rc.get()) );
  
   if ( !m_casesSet.empty() && ( *(rc.get()) == *(m_casesSet.back()) ) )
   {
      ++m_stepNum;
      return;
   }
   // add this case as a new experiment
   std::string expName = m_keepHistory ? ("LMStep_" + std::to_string( m_stepNum )) : "LMStep";
  
   // construct case project path: pathToScenario/Case_XX/ProjectName.project3d
   ibs::FolderPath casePath( "." );
   casePath << expName;

   if ( casePath.exists() ) // clean folder if it is already exist
   {
      if ( m_keepHistory )
      {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Folder for LM step: " << m_stepNum << " is already exist. " << 
                                                       casePath.fullPath().path() << " will be deleted";
      }
      casePath.remove(); 
   }
   casePath.create();
   casePath << (std::string( m_sa->baseCaseProjectFileName() ).empty() ? m_sa->baseCaseProjectFileName() : "Project.project3d");

   // do mutation
   rc->mutateCaseTo( m_sa->baseCase(), casePath.fullPath().cpath() );
   std::string msg = rc->validateCase();
   
   if ( !msg.empty() )
   {
      throw ErrorHandler::Exception( ErrorHandler::ValidationError ) << "LM step " << m_stepNum << " generated invalid project: " 
                                                                     << casePath.path();
   }

   // submit new job
   casa::RunManager & rm = m_sa->runManager();

   if ( !m_keepHistory ) { m_sa->resetRunManager( false ); } // if we do not keep history we need to clean runManager jobs queue, 
                                                             // otherwise it could try to use completed case pipeline in the new run

   if ( ErrorHandler::NoError != rm.scheduleCase(*(rc.get()), m_sa->scenarioID()) )
   {
      throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage();
   }
   // run with queue update interval 1 sec
   if ( ErrorHandler::NoError != rm.runScheduledCases( 1 ) ) { throw ErrorHandler::Exception( rm.errorCode() ) << rm.errorMessage(); }

   // collect observables value
   if ( ErrorHandler::NoError != m_sa->dataDigger().collectRunResults( m_sa->obsSpace(), rc.get() ) )
   {
      throw ErrorHandler::Exception( m_sa->dataDigger().errorCode() ) << m_sa->dataDigger().errorMessage();
   }

   if ( m_keepHistory || m_casesSet.empty() )
   {
      m_casesSet.push_back( rc.release() );
   }
   else 
   {
      delete m_casesSet[0];
      m_casesSet[0] = rc.release();
   }
   ++m_stepNum;
}

void LMOptAlgorithm::calculateFunctionValue( Eigen::VectorXd & fvec )
{
   double trgtQ = 0.0;
   int    trgtNum = 0;
   double prmQ  = 0.0;

   // initialze minimization function with all zeros
   for ( size_t i = 0; i < fvec.rows() * fvec.cols(); ++i ) { fvec( i ) = 0.0; }
   
   RunCase * rc = m_casesSet.back();

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
         if ( obv[k] == UndefinedDoubleValue || obv[k] == 99999.0 )
         {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid observation value: " << obv[k] <<" stopping...";
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Invalid observation value, stopping...";
         }
         double dif = sqrt( uaWeight ) * std::abs( obv[k] - refVal[k] ) / sigma;

#ifndef ACCUMULATE_MIN_FUNCTION
         fvec[mi] = dif; 
#endif
         trgtQ += dif*dif;
         ++mi;
      }
   }

   trgtNum = mi;

// 2. There is need to average trgtQ by mi (it is like multiplying by a constant)
#ifdef ACCUMULATE_MIN_FUNCTION
   fvec[0] = sqrt(trgtQ);
   mi = 0;
#endif

   // the calculate minimization function terms for parameters value, to prevent going outside ranges
   for ( size_t i = 0; i < m_permPrms.size(); ++i )
   {
      const VarPrmContinuous * vprm  = m_optimPrms[i].first;
      size_t                   prmID = m_optimPrms[i].second;
      double                   minV  = vprm->minValue( )->asDoubleArray()[prmID];
      double                   maxV  = vprm->maxValue( )->asDoubleArray()[prmID];
      double                   basV  = vprm->baseValue()->asDoubleArray()[prmID];
      VarPrmContinuous::PDF    ppdf  = vprm->pdfType();
      double                   pval  = m_xi[i]; // use parameter value proposed by LM. It could be outside of parameter
                                                // range. But parameter value will be cutted on the interval boundary
      //rc->parameter( m_permPrms[i] )->asDoubleArray()[prmID];

      if ( m_parameterTransformation == "log10" )
      {
         minV = log10( minV );
         maxV = log10( maxV );
         basV = log10( basV );
      }
      
      double fval = 0.0;
      switch( ppdf )
      {
         case VarPrmContinuous::Block: fval = 1e-10; break;

         case VarPrmContinuous::Triangle: 
            {  double d = 2.0 / ( (basV - minV) * (maxV - basV) ); // area of triangle is equal 1

               if ( pval < basV ) { fval = 0.5 * d * ( 1.0 + ( pval - minV ) / ( basV - minV ) ) * ( basV - pval ); }
               else               { fval = 0.5 * d * ( 1.0 + ( pval - maxV ) / ( basV - maxV ) ) * ( pval - basV ); }
            }
            break;

         case VarPrmContinuous::Normal:
            {
               double sigma = ( maxV - minV ) / 6.0; // 3 sigma - half interval
               fval = abs( pval - basV ) / sigma;
            }
            break;
      }
      // add penalty on goin out of the range
      if (      pval < minV ) 
      { 
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "pval less than minV : "<< pval <<" "<< minV;
         fval += 50 * (minV - pval); 
      }  // penalty if v < [min:max]
      else if ( pval > maxV ) 
      { 
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "pval larger than maxV : " << pval << " " << minV;
         fval += 50 * (pval - maxV); 
      }  // penalty if v > [min:max]

#ifndef ACCUMULATE_MIN_FUNCTION
      fvec[mi] = fval;
#endif

      prmQ += fval*fval;
      ++mi;
   }

#ifdef ACCUMULATE_MIN_FUNCTION
   fvec[1] = sqrt(prmQ);
#endif

   if ( m_Qmin > trgtQ + prmQ )
   {
      m_Qmin = trgtQ;

      LogHandler( LogHandler::DEBUG_SEVERITY ) << "New minimum found with Qmin target = " << m_Qmin << ", copying project...";

      // copy better case as calibrated
      ibs::FolderPath clbPath( "." );
      clbPath << m_projectName;

      rc->caseModel()->saveModelToProjectFile( clbPath.fullPath().cpath(), true );
   }

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Qmin targets : " << trgtQ << ", "
                                            << "Qmin prime   : " << prmQ  << ", "
                                            << "Qmin         : " << trgtQ + prmQ;
}

void LMOptAlgorithm::runOptimization( ScenarioAnalysis & sa )
{
   m_sa = &sa;
   m_stepNum = 0; // start with zero timestep
   m_Qmin = 1e50;

   // extract continuous parameters with non empty min/max range, fill initial guess vector
   std::vector<double> guess;
   std::vector<double> minPrm;
   std::vector<double> maxPrm;
   size_t prmSpDim = prepareParameters( guess, minPrm, maxPrm );

   // convert std::vector to Eigen vector
   Eigen::VectorXd initialGuess( guess.size() );
   Eigen::VectorXd minPrmEig( minPrm.size( ) );
   Eigen::VectorXd maxPrmEig( maxPrm.size( ) );
   size_t xi = 0;
   for ( auto pv : guess ) initialGuess[xi++] = pv;
   xi = 0;
   for ( auto pv : minPrm ) minPrmEig[xi++] = pv;
   xi = 0;
   for ( auto pv : maxPrm ) maxPrmEig[xi++] = pv;
   
   // extract observables with reference values and calculate dimension
   size_t obsSpDim = prepareObservables();

   // Functor
#ifndef ACCUMULATE_MIN_FUNCTION
   ProjectFunctor functor( *this, prmSpDim, prmSpDim + obsSpDim ); // use parameters also as observables to keep them in range
#else
   ProjectFunctor functor( *this, prmSpDim, 2 ); // use parameters also as observables to keep them in range
#endif

   Eigen::NumericalDiff<ProjectFunctor> numDiff( functor, 0.1 ); //10% relative, as in PEST
   Eigen::LevenbergMarquardt< Eigen::NumericalDiff<ProjectFunctor, Eigen::Forward> > lm( numDiff );

   lm.setMaxfev( 200 );     
   lm.setXtol( 1.0e-8 );
   lm.setFtol( 1 - 0.974 );  //3. set the tolerance for norm of fval as in PEST: 1 - 0.95 ^ 0.5 

   int ret = lm.minimize( initialGuess );

   // store step in doeCaseSet under separate label with LM step number
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( m_sa->doeCaseSet() );
   rcs.addNewCases( m_casesSet, "LMSteps" );


   // if we should not keep history - delete the last step
   if ( !m_keepHistory )
   {
      ibs::FolderPath casePath( "." );
      casePath << "LMStep";

      if ( casePath.exists() ) { casePath.remove(); }
   }

   if ( m_parameterTransformation == "log10" )
   {
      for ( size_t i = 0; i < initialGuess.size(); ++i )
         initialGuess( i ) = pow( 10, initialGuess( i ) );
   }

   // inform user about optimization results
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "LM optimization algorithm finished with iterations number: " << (int)( lm.iterations( ) ) << " and " << (int)( lm.nfev( ) ) << " model evaluations";
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "LM return code: " << ret;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "x that minimizes the function: \n" << initialGuess;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Saving calibrated model to: " << m_projectName;

   m_sa = 0;
}

} // namespace casa

