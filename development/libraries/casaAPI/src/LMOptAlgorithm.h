//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef LM_OPTIMIZATION_ALGORITH_H
#define LM_OPTIMIZATION_ALGORITH_H

#include "OptimizationAlgorithm.h"

#include <eigen3/Eigen/Dense>
#include <vector>
#include <memory>

namespace casa
{
   class VarPrmContinuous;
   class Observable;
   class RunCase;


   /// @brief Class implements Levenberg Marquardt based optimization for 1D project
   /// This LM algoritm is run over 1D project and it used the following function for minimzation
   ///  @f$
   ///  @f$
   class LMOptAlgorithm : public OptimizationAlgorithm
   {
   public:
      LMOptAlgorithm( const std::string & cbProjectName, const std::string & transformation, const double relativeReduction, bool keepHistory = false ) : OptimizationAlgorithm( cbProjectName )
                                                                                    , m_sa( 0 )
                                                                                    , m_stepNum( 0 )
                                                                                    , m_Qmin( 0.0 )
                                                                                    , m_keepHistory( keepHistory )
                                                                                    , m_parameterTransformation(transformation)
                                                                                    , m_relativeReduction( relativeReduction )
                                                                                    { ; }

      virtual ~LMOptAlgorithm() { ; }

      virtual void runOptimization( ScenarioAnalysis & sa );

      virtual std::string name() { return "Levenberg Marquardt"; }

      void updateParametersAndRunCase( const Eigen::VectorXd & x );
      void calculateFunctionValue( Eigen::VectorXd & fvec );
	  void removeInvalidObservations(size_t & nValues);
      std::string transformation() const { return m_parameterTransformation; };
      double relativeReduction( ) const { return m_relativeReduction; };

   protected:
      size_t prepareParameters( std::vector<double> & initGuess, std::vector<double> & minPrm, std::vector<double> & maxPrm );
      size_t prepareObservables();

      ScenarioAnalysis                                         * m_sa;
      size_t                                                     m_stepNum;   // current step number. Is used to name case folder
      std::vector< std::pair<const VarPrmContinuous *, size_t> > m_optimPrms; // keeps pointer to variable parameter and subparameter number
      std::vector< size_t >                                      m_permPrms;  // permutation array - optimization prms -> all prms

      std::vector< int >                                         m_permObs;            // permutation array - optimization obs -> all obs
	  std::vector< std::vector<int>>                             m_optimObservations;  // for each observable in the permutation array, its mask
	  std::vector< const Observable*>                            m_optimObsarvable;    // the observables in the permutation array
      
      std::vector< std::shared_ptr<RunCase> >                    m_casesSet;         // set of run cases for each step of LM
      std::shared_ptr<RunCase>                                   m_baseCase;         // the base case 
      std::shared_ptr<RunCase>                                   m_bestMatchedCase;  // the best matched case 
      std::vector<double>                                        m_xi;               // parameters value proposed by LM
      double                                                     m_Qmin;             // minimal value of Qtrgt for LM iterations
      bool                                                       m_keepHistory;      // keep all intermediate steps of LM
      std::string                                                m_parameterTransformation; //the type of parameter transformation that is applied
      double                                                     m_relativeReduction;      // the relative reduction to determine when to stop outer loops of the LM optimizations 
   };

} // namespace casa

#endif // LM_OPTIMIZATION_ALGORITH_H
