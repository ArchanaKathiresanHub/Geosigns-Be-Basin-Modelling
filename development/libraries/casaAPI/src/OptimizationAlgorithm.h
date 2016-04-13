//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef OPTIMIZATION_ALGORITH_H
#define OPTIMIZATION_ALGORITH_H

// STL/C lib
#include <string>

namespace casa
{
   class ScenarioAnalysis;

   class OptimizationAlgorithm
   {
   public:
      virtual ~OptimizationAlgorithm() {;}

      virtual void runOptimization( ScenarioAnalysis & sa ) = 0;

      virtual std::string name() = 0;

   protected:
      OptimizationAlgorithm( const std::string & cbProjectName ) : m_projectName( cbProjectName ) {;}

      std::string  m_projectName;

   private:
      OptimizationAlgorithm( const OptimizationAlgorithm & );
   };

} // namespace casa

#endif // OPTIMIZATION_ALGORITH_H
