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
      LMOptAlgorithm( const std::string & cbProjectName, bool keepHistory = false ) : OptimizationAlgorithm( cbProjectName )
                                                                                    , m_sa( 0 )
                                                                                    , m_stepNum( 0 )
                                                                                    , m_Qmin( 0.0 )
                                                                                    , m_keepHistory( keepHistory )
                                                                                    { ; }

      virtual ~LMOptAlgorithm() { ; }

      virtual void runOptimization( ScenarioAnalysis & sa );

      virtual std::string name() { return "Levenberg Marquardt"; }

      void updateParametersAndRunCase( const Eigen::VectorXd & x );
      void calculateFunctionValue( Eigen::VectorXd & fvec );

   protected:
      size_t prepareParameters( std::vector<double> & initGuess );
      size_t prepareObservables();

      size_t                                                     m_stepNum;   // current step number. Is used to name case folder
      ScenarioAnalysis                                         * m_sa;
      std::vector< std::pair<const VarPrmContinuous *, size_t> > m_optimPrms; // keeps pointer to variable parameter and subparameter number
      std::vector< size_t >                                      m_permPrms;  // permutation array - optimization prms -> all prms

      std::vector< const Observable*>                            m_optimObs;  // set of observables shoosed for optimization
      std::vector< size_t >                                      m_permObs;   // permutation array - optimization obs -> all obs
      
      std::vector< RunCase* >                                    m_casesSet;  // set of run cases for each step of LM
      std::vector<double>                                        m_xi;        // parameters value proposed by LM
      double                                                     m_Qmin;      // minimal value of Qtrgt for LM iterations
      bool                                                       m_keepHistory; // keep all intermediate steps of LM
   };

} // namespace casa

#endif // LM_OPTIMIZATION_ALGORITH_H
