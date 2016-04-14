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

   class LMOptAlgorithm : public OptimizationAlgorithm
   {
   public:
      LMOptAlgorithm( const std::string & cbProjectName ) : OptimizationAlgorithm( cbProjectName ), m_sa( 0 ) { ; }

      virtual ~LMOptAlgorithm() { ; }

      virtual void runOptimization( ScenarioAnalysis & sa );

      virtual std::string name() { return "Levenberg Marquardt"; }

      void updateParametersAndRunCase( const Eigen::VectorXd & x );
      void calculateFunctionValue( Eigen::VectorXd & fvec );

   protected:
      size_t prepareParameters( std::vector<double> & initGuess );
      size_t prepareObservables();

      ScenarioAnalysis                                         * m_sa;
      std::vector< std::pair<const VarPrmContinuous *, size_t> > m_optimPrms; // keeps pointer to variable parameter and subparameter number
      std::vector< size_t >                                      m_permPrms;  // permutation array - optimization prms -> all prms

      std::vector< const Observable*>                            m_optimObs;  // set of observables shoosed for optimization
      std::vector< size_t >                                      m_permObs;   // permutation array - optimization obs -> all obs

   };

} // namespace casa

#endif // LM_OPTIMIZATION_ALGORITH_H
