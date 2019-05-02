//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GENEX6_KERNEL__SIMULATORSTATE_RESULT_H
#define GENEX6_KERNEL__SIMULATORSTATE_RESULT_H
using namespace std;
//#include "SimulatorState.h"
#include "GenexResultManager.h"
#include "SpeciesResult.h"

namespace Genex6
{
   class SimulatorState;

   class SimulatorStateResult {
   public:

      SimulatorStateResult( SimulatorState &inSimulatorState,const bool snapshot = false );
      ~SimulatorStateResult();

      void clean();
      double getThickness() const;
      double getCurrentToc() const;
      SpeciesResult & getSpeciesResult( int i ) const;
      double * getResultsByResults() const;
      SpeciesResult * getSpeciesResults() const;

      double getHC() const;
      double getOC() const;

   private:

      int m_numberOfSpecies;
      SpeciesResult * m_SpeciesResults;
      double * m_ResultsByResultId;
 
      double m_thickness;
      double m_currentToc;

      double m_HC;
      double m_OC;
    
   };


inline double SimulatorStateResult::getThickness () const {
   return m_thickness;
}
inline double SimulatorStateResult::getCurrentToc () const {
   return m_currentToc;
}
inline SpeciesResult * SimulatorStateResult::getSpeciesResults () const {
   return m_SpeciesResults;
}

inline double SimulatorStateResult::getHC () const {
   return m_HC;
}

inline double SimulatorStateResult::getOC () const {
   return m_OC;
}

}

#endif
