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

      /// @brief Get number of parameters 
      /// @return parameters number
      virtual size_t parametersNumber() const = 0;

      /// @brief Get i-th parameter
      /// @param i position of requested parameter
      /// @return i-th parameter or null pointer if there is no such parameter
      virtual Parameter * parameter( size_t i ) const = 0;

      /// @brief Get number of observables 
      /// @return observables number
      virtual size_t observablesNumber( ) const = 0;

      /// @brief Get i-th observable
      /// @param i position of requested observable
      /// @return i-th observable or null pointer if there is no such observable
      virtual Observable * observable( size_t i ) const = 0;

      /// @brief Mutate case to given project file
      /// @param baseCase base case of the scenario which will be mutated to a new case
      /// @param newProjectName the name of the mutated project
      virtual void mutateCaseTo( mbapi::Model & baseCase, const char * newProjectName ) = 0;

      /// @brief Do checking, are all variable parameters case value in their ranges
      /// @return if validation is OK, empty string. otherwise - the list of validation\n
      ///         failed parameters with theirs values
      virtual std::string validateCase() = 0;

      /// @brief Get a model associated with this Case
      /// @return pointer to the model
      virtual mbapi::Model * caseModel() const = 0;

   protected:
      RunCase() { ; }
   };

}

#endif // CASA_API_CASE_H
