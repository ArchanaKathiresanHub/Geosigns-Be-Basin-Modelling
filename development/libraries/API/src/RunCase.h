//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCase.h
/// @brief This file keeps API declaration to keep a single run of Cauldron or a single Monte Carlo point

#ifndef CASA_API_CASE_H
#define CASA_API_CASE_H

#include "Parameter.h"
#include "cmbAPI.h"
#include "Observable.h"

#include <vector>

namespace casa
{
   /// @brief Case class is devoted to keep parameters and results for a single Cauldron project run or for\n
   ///        a single point of Monte Carlo simulation results
   class RunCase
   {
   public:
      /// @brief Destructor
      virtual ~RunCase() { ; }

      /// @brief Get a set of the case parameters
      /// @return vector of parameter objects
      virtual std::vector<Parameter*>  & parametersSet() = 0;
      
      /// @brief Get a set of observables for the case
      /// @return vector of observable objects
      virtual std::vector<Observable*> & observablesSet() = 0;

      /// @brief Get a model associated with this Case
      /// @return pointer to the model
      virtual mbapi::Model * caseModel() = 0;

   protected:
      RunCase() { ; }
   };

}

#endif // CASA_API_CASE_H
