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

      // Get number of parameters 
      // return parameters number
      virtual size_t parametersNumber() const { return m_prmsSet.size(); }

      // Get i-th parameter
      // i position of requested parameter
      // return i-th parameter or null pointer if there is no such parameter
      virtual Parameter * parameter( size_t i ) const;

      // Add new parameter to the list
      void addParameter( Parameter * prm );

      // Get number of observables 
      // return observables number
      virtual size_t observablesNumber() const { return m_results.size(); }

      // Get i-th observable
      // i position of requested observable
      // return i-th observable or null pointer if there is no such observable
      virtual Observable * observable( size_t i ) const;

      // Add new observable to the list
      void addObservable( Observable * obs );

      // Get a model associated with this Case
      // return pointer to the model
      virtual mbapi::Model * caseModel() const { return m_model.get(); }

   private:
      std::auto_ptr<mbapi::Model> m_model;
      std::string                 m_modelProjectFileName;

      std::vector< std::auto_ptr<Parameter> >     m_prmsSet;
      std::vector< std::auto_ptr<Observable> >    m_results;
   };
}

#endif // CASA_API_CASE_IMPL_H
