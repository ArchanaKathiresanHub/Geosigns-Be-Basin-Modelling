//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CaseImp.h
/// @brief This file keeps declaration of the implementation part to keep a single run of Cauldron or a single Monte Carlo point

#ifndef CASA_API_CASE_IMPL_H
#define CASA_API_CASE_IMPL_H

#include "cmbAPI.h"
#include "Parameter.h"
#include "RunCase.h"
#include "Observable.h"

#include <vector>
#include <memory>

namespace casa
{
   /// @brief Case class is devoted to keep parameters and results for a single Cauldron project run or for\n
   ///        a single point of Monte Carlo simulation results
   class RunCaseImpl : public RunCase
   {
   public:
      // Default constructor, used with Monte Carol cases
      RunCaseImpl();

      // Constructor if we building case to run simulation
      RunCaseImpl( const mbapi::Model & baseCase );

      virtual ~RunCaseImpl();

      // Get a set of the case parameters
      // return vector of parameter objects
      virtual std::vector<Parameter*>  & parametersSet() { return m_prmsSet; }
      
      // Get a set of observables for the case
      // return vector of observable objects
      virtual std::vector<Observable*> & observablesSet() { return m_results; }

      // Get a model associated with this Case
      // return pointer to the model
      virtual mbapi::Model * caseModel() { return m_model.get(); }


   private:
      std::auto_ptr<mbapi::Model> m_model;
      std::string                 m_modelProjectFileName;

      std::vector<Parameter*>     m_prmsSet;
      std::vector<Observable*>    m_results;
   };
}

#endif // CASA_API_CASE_IMPL_H
