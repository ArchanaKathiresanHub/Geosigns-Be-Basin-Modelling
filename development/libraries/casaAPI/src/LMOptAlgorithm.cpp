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
using namespace Eigen;

namespace casa
{
   // Does not make any sense to inherit from LevenbergMarquardt: it is not an abstract class and we ca not get its private members without modifying Eigen
   template<typename FunctorType>
   class LevenbergMarquardtConstrained
   {

   public:

      typedef typename FunctorType::QRSolver QRSolver;
      typedef typename FunctorType::JacobianType JacobianType;
      typedef typename JacobianType::Scalar Scalar;
      typedef typename JacobianType::RealScalar RealScalar;
      typedef typename JacobianType::Index Index;
      typedef typename QRSolver::Index PermIndex;
      typedef Matrix<Scalar, Dynamic, 1> FVectorType;
      typedef PermutationMatrix<Dynamic, Dynamic> PermutationType;

      // constructor
      LevenbergMarquardtConstrained( FunctorType & functor, VectorXd& xMin, VectorXd& xMax ) :
         m_xMin( xMin ),
         m_xMax( xMax ),
         m_functor( functor ), m_nfev( 0 ), m_njev( 0 ), m_fnorm( 0.0 ), m_gnorm( 0 ),
         m_isInitialized( false ), m_info( InvalidInput )
      {
            resetParameters();
            m_useExternalScaling = false;
      }

      /** Sets the tolerance for the norm of the solution vector*/
      void setXtol( RealScalar xtol ) { m_xtol = xtol; }

      /** Sets the tolerance for the norm of the vector function*/
      void setFtol( RealScalar ftol ) { m_ftol = ftol; }

      /** Sets the maximum number of function evaluation */
      void setMaxfev( Index maxfev ) { m_maxfev = maxfev; }

      /** \returns the number of functions evaluation */
      Index nfev( ) { return m_nfev; }

      /** \returns the number of iterations performed */
      Index iterations( ) { return m_iter; }

      // constrained minimize
      LevenbergMarquardtSpace::Status minimize( FVectorType  &x )
      {
         if ( m_xMin.size( ) != 0 ) LogHandler( LogHandler::DEBUG_SEVERITY ) << " Constrained LM ";
         LevenbergMarquardtSpace::Status status = minimizeInit( x );
         if ( status == LevenbergMarquardtSpace::ImproperInputParameters ) {
            m_isInitialized = true;
            return status;
         }

         int outerIter = 0;
         bool relativeReductionOk = true;

         do {
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "New jacobian calculated, iteration " << outerIter;
            status = minimizeOneStep( x );
            if ( outerIter > 0 )
            {
               RealScalar relativereduction =  1 - m_fnorm / m_oldFnorm ;
               LogHandler( LogHandler::DEBUG_SEVERITY ) << "Relative reduction of the norm of fval vector " << relativereduction;
               if ( relativereduction < 0.05 )
               {
                  LogHandler( LogHandler::DEBUG_SEVERITY ) << "Stopping because the relative reduction is less than 0.05 ";
                  relativeReductionOk = false;
               }
            }
            m_oldFnorm = m_fnorm;
            ++outerIter;
         } while ( status == LevenbergMarquardtSpace::Running && relativeReductionOk && m_fnorm > 0. ); //In PEST we do not go over 10 outerIter
         m_isInitialized = true;
         return status;
      }

   private:
      // reset solver parameters
      void resetParameters( )
      {
         m_factor = 0.2; // was 100., 0.2 proven to be optimal also in some other cases because the variation of the lambda parameter is more gradual (as in PEST).
         m_maxfev = 400;
         m_ftol = std::sqrt( NumTraits<RealScalar>::epsilon( ) );
         m_xtol = std::sqrt( NumTraits<RealScalar>::epsilon( ) );
         m_gtol = 0.;
         m_epsfcn = 0.;
      }

      LevenbergMarquardtSpace::Status minimizeInit( FVectorType  &x )
      {
         n = x.size();
         m = m_functor.values();

         m_wa1.resize( n ); m_wa2.resize( n ); m_wa3.resize( n );
         m_wa4.resize( m );
         m_fvec.resize( m );
         //FIXME Sparse Case : Allocate space for the jacobian
         m_fjac.resize( m, n );
         //     m_fjac.reserve(VectorXi::Constant(n,5)); // FIXME Find a better alternative
         if ( !m_useExternalScaling )
            m_diag.resize( n );
         eigen_assert( ( !m_useExternalScaling || m_diag.size() == n ) || "When m_useExternalScaling is set, the caller must provide a valid 'm_diag'" );
         m_qtf.resize( n );

         /* Function Body */
         m_nfev = 0;
         m_njev = 0;

         /*     check the input parameters for errors. */
         if ( n <= 0 || m < n || m_ftol < 0. || m_xtol < 0. || m_gtol < 0. || m_maxfev <= 0 || m_factor <= 0. ){
            m_info = InvalidInput;
            return LevenbergMarquardtSpace::ImproperInputParameters;
         }

         if ( m_useExternalScaling )
         for ( Index j = 0; j < n; ++j )
         if ( m_diag[j] <= 0. )
         {
            m_info = InvalidInput;
            return LevenbergMarquardtSpace::ImproperInputParameters;
         }

         /*     evaluate the function at the starting point */
         /*     and calculate its norm. */
         m_nfev = 1;
         if ( m_functor( x, m_fvec ) < 0 )
            return LevenbergMarquardtSpace::UserAsked;
         m_fnorm = m_fvec.stableNorm();

         /*     initialize levenberg-marquardt parameter and iteration counter. */
         m_par = 0.;
         m_iter = 1;

         return LevenbergMarquardtSpace::NotStarted;
      }

      //correct the parameter upgrade
      void correctDirection(
         FVectorType& x,
         FVectorType& m_wa1 )
      {
         if ( m_xMin.size() > 0 && m_xMax.size() > 0 )
         {
            for ( int i = 0; i != x.size(); ++i )
            {
               double maxVal = m_xMax( i ) - numeric_limits<double>::epsilon();
               double minVal = m_xMin( i ) + numeric_limits<double>::epsilon();
               if ( x( i ) + m_wa1( i ) < minVal )
               {
                  LogHandler( LogHandler::DEBUG_SEVERITY ) << " Parameter " << i << " out of lower bound, fixed at the boundary ";
                  m_wa1( i ) = minVal - x( i );
               }
               else if ( x( i ) + m_wa1( i )  > maxVal )
               {
                  LogHandler( LogHandler::DEBUG_SEVERITY ) << " Parameter " << i << " out of upper bound, fixed at the boundary with value ";
                  m_wa1( i ) = maxVal - x( i );
               }
            }
         }
      }

      // constrained optimization
      LevenbergMarquardtSpace::Status minimizeOneStep( FVectorType  &x )
      {
         using std::abs;
         using std::sqrt;
         RealScalar temp, temp1, temp2;
         RealScalar ratio;
         RealScalar pnorm, xnorm, fnorm1, actred, dirder, prered;
         eigen_assert( x.size() == n ); // check the caller is not cheating us

         Index df_ret;

         temp = 0.0; xnorm = 0.0;
         /* calculate the jacobian matrix. */
         df_ret = m_functor.df( x, m_fjac );


         if ( df_ret<0 )
            return LevenbergMarquardtSpace::UserAsked;
         if ( df_ret>0 )
            // numerical diff, we evaluated the function df_ret times
            m_nfev += df_ret;
         else m_njev++;

         /* compute the qr factorization of the jacobian. */
         for ( int j = 0; j < x.size(); ++j )
            m_wa2( j ) = m_fjac.col( j ).blueNorm();
         QRSolver qrfac( m_fjac );
         if ( qrfac.info() != Success ) {
            m_info = NumericalIssue;
            return LevenbergMarquardtSpace::ImproperInputParameters;
         }
         // Make a copy of the first factor with the associated permutation
         m_rfactor = qrfac.matrixR();
         m_permutation = ( qrfac.colsPermutation() );

         /* on the first iteration and if external scaling is not used, scale according */
         /* to the norms of the columns of the initial jacobian. */
         if ( m_iter == 1 ) {
            if ( !m_useExternalScaling )
            for ( Index j = 0; j < n; ++j )
               m_diag[j] = ( m_wa2[j] == 0. ) ? 1. : m_wa2[j];

            /* on the first iteration, calculate the norm of the scaled x */
            /* and initialize the step bound m_delta. */
            xnorm = m_diag.cwiseProduct( x ).stableNorm();
            m_delta = m_factor * xnorm;
            if ( m_delta == 0. )
               m_delta = m_factor;
         }

         /* form (q transpose)*m_fvec and store the first n components in */
         /* m_qtf. */
         m_wa4 = m_fvec;                             //m_fvec is the initial vector of values
         m_wa4 = qrfac.matrixQ().adjoint() * m_fvec; //qr is the qr decomposition of the jacobian matrix 
         m_qtf = m_wa4.head( n );

         /* compute the norm of the scaled gradient. */
         m_gnorm = 0.;
         if ( m_fnorm != 0. )
         for ( Index j = 0; j < n; ++j )
         if ( m_wa2[m_permutation.indices()[j]] != 0. )
            m_gnorm = ( std::max )( m_gnorm, abs( m_rfactor.col( j ).head( j + 1 ).dot( m_qtf.head( j + 1 ) / m_fnorm ) / m_wa2[m_permutation.indices()[j]] ) );

         /* test for convergence of the gradient norm, only if columns of jacobian not frozen */
         if ( m_gnorm <= m_gtol ) {
            m_info = Success;
            return LevenbergMarquardtSpace::CosinusTooSmall;
         }

         /* rescale if necessary. */
         if ( !m_useExternalScaling )
            m_diag = m_diag.cwiseMax( m_wa2 );

         do {
            /* determine the levenberg-marquardt parameter. */
            std::cout << " Lamda search, ";
            LogHandler( LogHandler::DEBUG_SEVERITY ) << " Lamda search, ";
            internal::lmpar2( qrfac, m_diag, m_qtf, m_delta, m_par, m_wa1 );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << " lambda parameter : " << m_par;

            /* store the direction p and x + p. calculate the norm of p. */
            m_wa1 = -m_wa1;
            // if outside the boundary, cut the increment at the boundary
            correctDirection( x, m_wa1 );
            m_wa2 = x + m_wa1;

            pnorm = m_diag.cwiseProduct( m_wa1 ).stableNorm();

            /* on the first iteration, adjust the initial step bound. */
            if ( m_iter == 1 )
               m_delta = ( std::min )( m_delta, pnorm );

            /* evaluate the function at x + p and calculate its norm. */
            if ( m_functor( m_wa2, m_wa4 ) < 0 )
               return LevenbergMarquardtSpace::UserAsked;
            ++m_nfev;
            fnorm1 = m_wa4.stableNorm();

            /* compute the scaled actual reduction. */
            actred = -1.;
            if ( Scalar( .1 ) * fnorm1 < m_fnorm )
               actred = 1. - numext::abs2( fnorm1 / m_fnorm );

            /* compute the scaled predicted reduction and */
            /* the scaled directional derivative. */
            m_wa3 = m_rfactor.template triangularView<Upper>() * ( m_permutation.inverse() *m_wa1 );
            temp1 = numext::abs2( m_wa3.stableNorm() / m_fnorm );
            temp2 = numext::abs2( sqrt( m_par ) * pnorm / m_fnorm );
            prered = temp1 + temp2 / Scalar( .5 );
            dirder = -( temp1 + temp2 );

            /* compute the ratio of the actual to the predicted */
            /* reduction. */
            ratio = 0.;
            if ( prered != 0. )
               ratio = actred / prered;

            /* update the step bound. */
            if ( ratio <= Scalar( .25 ) ) {
               if ( actred >= 0. )
                  temp = RealScalar( .5 );
               if ( actred < 0. )
                  temp = RealScalar( .5 ) * dirder / ( dirder + RealScalar( .5 ) * actred );
               if ( RealScalar( .1 ) * fnorm1 >= m_fnorm || temp < RealScalar( .1 ) )
                  temp = Scalar( .1 );
               /* Computing MIN */
               m_delta = temp * ( std::min )( m_delta, pnorm / RealScalar( .1 ) );
               m_par /= temp;
            }
            else if ( !( m_par != 0. && ratio < RealScalar( .75 ) ) ) {
               m_delta = pnorm / RealScalar( .5 );
               m_par = RealScalar( .5 ) * m_par;
            }

            /* test for successful iteration. */
            if ( ratio >= RealScalar( 1e-4 ) )
            {
               /* successful iteration. update x, m_fvec, and their norms. */
               x = m_wa2;
               m_wa2 = m_diag.cwiseProduct( x );
               m_fvec = m_wa4;
               xnorm = m_wa2.stableNorm();
               m_fnorm = fnorm1;
               ++m_iter;
            }

            /* tests for convergence. */
            if ( abs( actred ) <= m_ftol && prered <= m_ftol && Scalar( .5 ) * ratio <= 1. && m_delta <= m_xtol * xnorm )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::RelativeErrorAndReductionTooSmall;
            }
            if ( abs( actred ) <= m_ftol && prered <= m_ftol && Scalar( .5 ) * ratio <= 1. )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::RelativeReductionTooSmall;
            }
            if ( m_delta <= m_xtol * xnorm )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::RelativeErrorTooSmall;
            }

            /* tests for termination and stringent tolerances. */
            if ( m_nfev >= m_maxfev )
            {
               m_info = NoConvergence;
               return LevenbergMarquardtSpace::TooManyFunctionEvaluation;
            }
            if ( abs( actred ) <= NumTraits<Scalar>::epsilon() && prered <= NumTraits<Scalar>::epsilon() && Scalar( .5 ) * ratio <= 1. )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::FtolTooSmall;
            }
            if ( m_delta <= NumTraits<Scalar>::epsilon() * xnorm )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::XtolTooSmall;
            }
            if ( m_gnorm <= NumTraits<Scalar>::epsilon() )
            {
               m_info = Success;
               return LevenbergMarquardtSpace::GtolTooSmall;
            }

         } while ( ratio < Scalar( 1e-4 ) );

         return LevenbergMarquardtSpace::Running;
      }

      // new members for constrained optimization
      Eigen::VectorXd& m_xMin;
      Eigen::VectorXd& m_xMax;
      RealScalar  m_oldFnorm;

      // other private members of LevenbergMarquardt 
      JacobianType m_fjac;
      JacobianType m_rfactor; // The triangular matrix R from the QR of the jacobian matrix m_fjac
      FunctorType &m_functor;
      FVectorType m_fvec, m_qtf, m_diag;
      Index n;
      Index m;
      Index m_nfev;
      Index m_njev;
      RealScalar m_fnorm; // Norm of the current vector function
      RealScalar m_gnorm; //Norm of the gradient of the error 
      RealScalar m_factor; //
      Index m_maxfev; // Maximum number of function evaluation
      RealScalar m_ftol; //Tolerance in the norm of the vector function
      RealScalar m_xtol; // 
      RealScalar m_gtol; //tolerance of the norm of the error gradient
      RealScalar m_epsfcn; //
      Index m_iter; // Number of iterations performed
      RealScalar m_delta;
      bool m_useExternalScaling;
      PermutationType m_permutation;
      FVectorType m_wa1, m_wa2, m_wa3, m_wa4; //Temporary vectors
      RealScalar m_par;
      bool m_isInitialized; // Check whether the minimization step has been called
      ComputationInfo m_info;

   };

   struct ProjectFunctor : public Eigen::DenseFunctor<double>
   {
      ProjectFunctor( LMOptAlgorithm & lm
      , int prmSpaceDim
      , int obsSpaceDim
      , Eigen::VectorXd& xMax
      )
      : DenseFunctor<double>( prmSpaceDim, obsSpaceDim )
      , m_lm( lm )
      , m_xMax( xMax )
      {
      }

      int operator() ( const Eigen::VectorXd & x, Eigen::VectorXd & fvec ) const
      {
         LMOptAlgorithm & lm = const_cast<LMOptAlgorithm&>( m_lm );
         lm.updateParametersAndRunCase( x );
         lm.calculateFunctionValue( fvec );

         return 0;
      }

      // In case of log10 transformation we increment the base value and transform the increment back, as done in PEST 
      int df( const InputType& _x, JacobianType &jac )
      {
         using std::sqrt;
         using std::abs;
         /* Local variables */
         Scalar h;
         int nfev = 0;
         const InputType::Index n = _x.size();
         ValueType val1, val2;
         // 10% increment as in PEST
         const Scalar eps = 0.1;
         InputType x = _x;

         val1.resize( ProjectFunctor::values() );
         val2.resize( ProjectFunctor::values() );

         // compute f(x)
         ProjectFunctor::operator()( x, val1 ); nfev++;

         bool isLogTransf = m_lm.transformation( ) == "log10" ? true : false;

         // Function Body
         for ( int j = 0; j < n; ++j, ++nfev )
         {
            double backupVal = x[j];

            // calculate the base value
            if ( isLogTransf ) { x[j] = pow( 10, _x[j] ); }

            // increment always in terms of the base value
            h = x[j] == 0. ? eps : eps * abs( x[j] );

            // backward difference if the parameter exceeds the bound
            x[j] += x[j] + h > m_xMax[j] ? -h : h;

            // backtransform in log10 for the function evaluation
            if ( isLogTransf ) { x[j] = log10( x[j] ); }

            // calculate the jacobian
            ProjectFunctor::operator()( x, val2 );
            jac.col( j ) = ( val2 - val1 ) / ( x[j] - backupVal );

            //restore the original value
            x[j] = backupVal;
         }

         return nfev;
      }

      const LMOptAlgorithm & m_lm;
      Eigen::VectorXd& m_xMax;
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
               assert( basVals[k] != 0 );
               initGuess.push_back( log10( basVals[k] ) );
               assert( minVals[k] != 0 );
               minPrm.push_back( log10( minVals[k] ) );
               assert( maxVals[k] != 0 );
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
      if ( minPrms[m_permPrms[i]] > prmVal  )
      {
         cntPrms[m_permPrms[i]] = minPrms[m_permPrms[i]];
         LogHandler( LogHandler::DEBUG_SEVERITY ) << " parameter " << i << " = " << prmVal << " < min range value: " << minPrms[m_permPrms[i]];
        // x( i ) = minPrms[m_permPrms[i]];
      }
      else if ( maxPrms[m_permPrms[i]] < prmVal  )
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
      const std::vector<double> & sigma    = obs->stdDeviationForRefValue( )->asDoubleArray( );
      double                      uaWeight = obs->uaWeight();

      assert( refVal.size() == obv.size() );
            
      for ( size_t k = 0; k < refVal.size(); ++k )
      {
         if ( obv[k] == DataAccess::Interface::DefaultUndefinedScalarValue || obv[k] == DataAccess::Interface::DefaultUndefinedMapValue )
         {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid observation value: " << obs->name( )[k] << " with simulated value " << obv[k] << ", stopping...";
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Invalid observation value, stopping...";
         }

         if ( sigma[k] <= 0 )
         {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid standard deviation value: " << obs->name( )[k] << " with standard deviation " << sigma[k] << ", stopping...";
         }

         double dif = sqrt( uaWeight ) * std::abs( obv[k] - refVal[k] ) / sigma[k];

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

      if ( m_parameterTransformation == "log10" )
      {
         minV = log10( minV );
         maxV = log10( maxV );
         basV = log10( basV );
      }
      
      double fval = 0.0;
      double fpen = 0.0;
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
      if ( pval < minV )
      { 
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "pval less than minV : "<< pval <<" "<< minV;
         fpen = 50 * (minV - pval); 
      }  // penalty if v < [min:max]
      else if ( pval > maxV )
      { 
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "pval larger than maxV : " << pval << " " << minV;
         fpen  = 50 * (pval - maxV); 
      }  // penalty if v > [min:max]

      if ( m_parameterTransformation == "log10" )
      {
         fpen = 1 - exp( fpen );
      }
      
      // add penality if outside the bound
      fval += fpen;

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
      m_Qmin = trgtQ + prmQ;

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
   ProjectFunctor functor( *this, prmSpDim, prmSpDim + obsSpDim, maxPrmEig ); // use parameters also as observables to keep them in range
#else
   ProjectFunctor functor( *this, prmSpDim, 2, maxPrmEig ); // use parameters also as observables to keep them in range
#endif

   // use our constrained optimization algorithm derived from Eigen
   casa::LevenbergMarquardtConstrained< ProjectFunctor > lm( functor, minPrmEig, maxPrmEig );

   // 20 evaluations for each parameter should be sufficent
   lm.setMaxfev( 20 * guess.size( ) );  
      
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

